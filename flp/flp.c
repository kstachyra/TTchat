/*
 * flp.cpp
 *
 *  Created on: 12 maj 2016
 *      Author: krystian
 */

#include "flp.h"
#include "flp_config.h"
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/eventfd.h>

/* Private definitions and macros ------------------------------------------ */
#define FLP_TYPE_CLIENT_HELLO				((uint16_t)0x0000)
#define FLP_TYPE_SERVER_HELLO				((uint16_t)0x0001)
#define FLP_TYPE_DATA						((uint16_t)0x0002)

#define FLP_ACK_BIT							((uint16_t)0x8000)

#define FLP_IS_ACK_BIT_SET(type)			((type) & FLP_ACK_BIT)
#define FLP_SET_ACK_BIT(type)				((type) | FLP_ACK_BIT)

#if FLP_ENABLE_LOG
#define FLP_LOG(format, ...)				printf(format, ##__VA_ARGS__)
#else
#define FLP_LOG(format, ...)
#endif

/* Private types ----------------------------------------------------------- */
typedef struct {
	uint16_t type;
	uint16_t payloadLength;
	uint16_t dataLength;
} FLP_Header_t;

/* Private functions' prototypes ------------------------------------------- */
static void *FLP_Thread(void *args);
static bool FLP_Init(FLP_Connection_t *connection, int socket);
static bool FLP_PushToQueue(FLP_Queue_t *queue, uint8_t *data, size_t length);
static uint8_t *FLP_PopFromQueue(FLP_Queue_t *queue, size_t *length);
static bool FLP_EventSend(int eventFD);
static bool FLP_Transmit(FLP_Connection_t *connection, uint8_t *data, size_t length);
static bool FLP_TransmitPacket(FLP_Connection_t *connection, uint16_t type, uint8_t* payload, size_t length);
static bool FLP_Receive(FLP_Connection_t *connection, uint8_t *data, size_t length);
static bool FLP_ReceiveHeader(FLP_Connection_t *connection, FLP_Header_t *header);
static bool FLP_SendAck(FLP_Connection_t *connection, uint16_t type);
static bool FLP_Handshake(FLP_Connection_t *connection);
static bool FLP_Terminate(FLP_Connection_t *connection, bool write, bool read, bool receive);

/* Exported functions ------------------------------------------------------ */
FLP_Connection_t* FLP_Connect(int socket)
{
	bool result;
	FLP_Connection_t *connection;

	connection = (FLP_Connection_t*)malloc(sizeof(FLP_Connection_t));
	if(connection == NULL) return NULL;

	// Initialize mutexes, eventfds, socket etc.
	if(!FLP_Init(connection, socket)) {
		free(connection);
		return NULL;
	}

	// Perform FLP handshake (establish connection and exchange keys)
	result = FLP_Handshake(connection);
	if(!result) {
		free(connection);
		return NULL;
	}

	// Start receiver thread
	pthread_attr_init(&connection->threadAttr);
	pthread_create(&connection->thread, &connection->threadAttr, FLP_Thread, (void *)connection);

	return connection;
}

bool FLP_Write(FLP_Connection_t *connection, uint8_t *data, size_t length)
{
	int ndfs, result;
	fd_set rfds;
	uint8_t *encryptedData;
	size_t encryptedDataLength;

	// Check connection state
	if(connection->state != FLP_CONNECTED) return false;

	// TODO: Encrypt data
	// ...

	// Construct and transmit packet
	if(!FLP_TransmitPacket(connection, FLP_TYPE_DATA, encryptedData, encryptedDataLength)) return false;

	// Wait for ACK or termination request
	FD_ZERO(&rfds);
	FD_SET(connection->ackReceived, &rfds);
	FD_SET(connection->terminateWrite, &rfds);
	ndfs = (connection->ackReceived > connection->terminateWrite) ? (connection->ackReceived + 1) : (connection->terminateWrite + 1);
	result = select(ndfs, &rfds, NULL, NULL, NULL);
	if(result < -1) {
		perror("select()");
		return false;
	}

	// If termination request was received...
	if(FD_ISSET(connection->terminateWrite, &rfds)) {
		FLP_LOG("FLP_Write(): Terminated while waiting for ACK.\n");
		return false;
	}

	// If ACK was received...
	if(FD_ISSET(connection->ackReceived, &rfds)) {
		FLP_LOG("FLP_Write(): ACK received.\n");
		return true;
	}

	// Should never happen...
	FLP_LOG("FLP_Write(): WTF?! No file descriptor is set.\n");
	return false;
}

bool FLP_Read(FLP_Connection_t *connection, uint8_t **data, size_t *length)
{
	int ndfs, result;
	fd_set rfds;

	*length = 0;
	*data = NULL;

	// Check connection state
	if(connection->state != FLP_CONNECTED) return false;

	// Block on queue and termination request simultaneously
	FD_ZERO(&rfds);
	FD_SET(connection->readQueue.newElementAvailable, &rfds);
	FD_SET(connection->terminateRead, &rfds);
	ndfs = (connection->readQueue.newElementAvailable > connection->terminateRead) ? (connection->readQueue.newElementAvailable + 1) : (connection->terminateRead + 1);
	result = select(ndfs, &rfds, NULL, NULL, NULL);
	if(result < -1) {
		perror("select()");
		return false;
	}

	// If termination request was received...
	if(FD_ISSET(connection->terminateRead, &rfds)) {
		FLP_LOG("FLP_Read(): Terminated while waiting for new data in the queue.\n");
		return false;
	}

	// If new data is available...
	if(FD_ISSET(connection->readQueue.newElementAvailable, &rfds)) {
		*data = FLP_PopFromQueue(&connection->readQueue, length);
		return true;
	}

	// Should never happen...
	FLP_LOG("FLP_Write(): WTF?! No file descriptor is set.\n");
	return false;
}

bool FLP_Close(FLP_Connection_t *connection)
{
	// Terminate FLP_Write, FLP_Read and FLP_Receive
	if(!FLP_Terminate(connection, true, true, true)) return false;

	// Close TCP socket
	if(close(connection->socket) < 0) return false;

	// Free memory used for connection structure
	free(connection);

	return true;
}

/* Private functions ------------------------------------------------------- */
static void *FLP_Thread(void *args)
{
	bool result, shouldTerminate = false;
	uint8_t *decryptedData, *encryptedData;
	size_t decryptedDataLength;
	FLP_Header_t header;
	FLP_Connection_t *connection = (FLP_Connection_t*)args;

	while(!shouldTerminate) {

		// Receive FLP header
		result = FLP_ReceiveHeader(connection, &header);
		// TODO: Check for errors

		if(FLP_IS_ACK_BIT_SET(header.type)) {

			// Notify FLP_Write that ACK was received
			FLP_EventSend(connection->ackReceived);

		} else {

			// Only data packets should be received at this point. If packet of other type was received, drop it.
			if(header.type != FLP_TYPE_DATA) break;

			// Receive payload
			encryptedData = (uint8_t*)malloc(sizeof(uint8_t)*header.payloadLength);
			result = FLP_Receive(connection, encryptedData, header.payloadLength);
			// TODO: Check for errors

			// TODO: Decrypt payload
			// ...

			// Push decrypted payload to queue and notify
			FLP_PushToQueue(&connection->readQueue, decryptedData, decryptedDataLength);
			FLP_EventSend(connection->readQueue.newElementAvailable);

			// Send ACK
			result = FLP_SendAck(connection, header.type);
			// TODO: Check for errors
		}
	}

	return NULL;
}

static bool FLP_Init(FLP_Connection_t *connection, int socket)
{
	// TODO: Error checks
	// TODO: Add function FLP_Init initializing semaphores, eventFDs, queues, configuring socket etc.
	connection->socket = socket;
	fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK);

	connection->terminateWrite = eventfd(0, EFD_SEMAPHORE);
	connection->terminateRead = eventfd(0, EFD_SEMAPHORE);
	connection->terminateReceive = eventfd(0, EFD_SEMAPHORE);
	connection->ackReceived = eventfd(0, EFD_SEMAPHORE);

	connection->readQueue.newElementAvailable = eventfd(0, EFD_SEMAPHORE);
	connection->readQueue.head = NULL;
	pthread_mutex_init(&connection->readQueue.lock, NULL);

	pthread_mutex_init(&connection->transmitLock, NULL);

	connection->state = FLP_DISCONNECTED;

	return true;
}

static bool FLP_PushToQueue(FLP_Queue_t *queue, uint8_t *data, size_t length)
{
	FLP_ListElement *newElement;

	newElement = (FLP_ListElement*)malloc(sizeof(FLP_ListElement));
	if(newElement == NULL) return false;

	// Acquire exclusive access to the queue
	pthread_mutex_lock(&queue->lock);

	newElement->data = data;
	newElement->length = length;
	newElement->next = queue->head;

	queue->head = newElement;

	pthread_mutex_unlock(&queue->lock);

	return true;
}

static uint8_t *FLP_PopFromQueue(FLP_Queue_t *queue, size_t *length)
{
	FLP_ListElement *current = queue->head, *previous;
	uint8_t *data;

	// Acquire exclusive access to the queue
	pthread_mutex_lock(&queue->lock);

	// Although it shouldn't happen, check if queue is empty...
	if(queue->head == NULL) {
		*length = 0;
		data = NULL;

	} else {

		// If there's only one element left...
		if(current->next == NULL) {
			queue->head = NULL;

		// If there's more than one element...
		} else {
			while(current->next->next != NULL) current = current->next;
			previous = current;
			current = current->next;
			previous->next = NULL;
		}

		*length = current->length;
		data = current->data;
		free(current);
	}

	pthread_mutex_unlock(&queue->lock);

	return data;
}

static bool FLP_EventSend(int eventFD)
{
	ssize_t result;
	uint64_t value = 1;

	result = write(eventFD, &value, sizeof(value));
	if(result != sizeof(value)) return false;

	return true;
}

static bool FLP_Transmit(FLP_Connection_t *connection, uint8_t *data, size_t length)
{
	ssize_t bytesSent, bytesSentTotal = 0;

	pthread_mutex_lock(&connection->transmitLock);

	while(bytesSentTotal != (ssize_t)length) {
		bytesSent = write(connection->socket, &data[bytesSent], length - bytesSentTotal);
		if(bytesSent < 0) return false;
		bytesSentTotal += bytesSent;
	}

	pthread_mutex_unlock(&connection->transmitLock);

	return true;
}

static bool FLP_Receive(FLP_Connection_t *connection, uint8_t *data, size_t length)
{
	int ndfs, result;
	fd_set rfds;
	ssize_t bytesRead, bytesReadTotal = 0;

	while(bytesReadTotal != (ssize_t)length) {

		// Wait for new data or termination request
		FD_ZERO(&rfds);
		FD_SET(connection->socket, &rfds);
		FD_SET(connection->terminateReceive, &rfds);
		ndfs = (connection->socket > connection->terminateReceive) ? (connection->socket + 1) : (connection->terminateReceive + 1);
		result = select(ndfs, &rfds, NULL, NULL, NULL);
		if(result < -1) {
			perror("select()");
			return false;
		}

		// Check if there is new data
		if(FD_ISSET(connection->socket, &rfds)) {

			// Read data from socket
			bytesRead = read(connection->socket, &data[bytesReadTotal], length - bytesReadTotal);
			if(bytesRead < 0) {

				// TODO: Check errno for EAGAIN or EWOULDBLOCK
				// ...

			} else if(bytesRead == 0) {

				// Connection was probably closed by remote peer
				FLP_LOG("FLP_Receive(): Connection lost.\n");
				connection->state = FLP_DISCONNECTED;

				// At this point other threads might be blocked on FLP_Write or FLP_Read, therefore we have to send them terminating event
				FLP_Terminate(connection, true, true, false);

			} else {

				// Increase total number of bytes by the returned number
				bytesReadTotal += bytesRead;
			}
		}

		// Check if termination request was received
		if(FD_ISSET(connection->terminateReceive, &rfds)) {
			FLP_LOG("FLP_Receive(): Termination request received while waiting for new data.\n");
			return false;
		}

		// Should never happen...
		FLP_LOG("FLP_Receive(): WTF?! No file descriptor is set.\n");
		return false;
	}

	return true;
}

static bool FLP_ReceiveHeader(FLP_Connection_t *connection, FLP_Header_t *header)
{
	return FLP_Receive(connection, (uint8_t*)header, sizeof(FLP_Header_t));
}

bool FLP_SendAck(FLP_Connection_t *connection, uint16_t type)
{
	FLP_Header_t header;

	header.type = FLP_SET_ACK_BIT(type);
	header.payloadLength = 0;

	return FLP_Transmit(connection, (uint8_t*)&header, sizeof(FLP_Header_t));
}

static bool FLP_Handshake(FLP_Connection_t *connection)
{
	FLP_Header_t header;
	uint8_t publicKey[FLP_PUBLIC_KEY_LENGTH], *encryptedSessionKey;
	size_t encryptedSessionKeyLength;

	// Wait for ClientHello packet
	if(!FLP_ReceiveHeader(connection, &header)) return false;
	if(header.type != FLP_TYPE_CLIENT_HELLO || header.payloadLength != FLP_PUBLIC_KEY_LENGTH) return false;
	if(!FLP_Receive(connection, publicKey, FLP_PUBLIC_KEY_LENGTH)) return false;

	// Generate session key
	if(!FLP_AES_GenerateSessionKey(connection->sessionKey)) {
		FLP_LOG("FLP_Handshake: Error occurred while generating session key.\n");
		return false;
	}

	// TODO: Encrypt generated key
	// ...

	// Send ServerHello
	if(!FLP_TransmitPacket(connection, FLP_TYPE_SERVER_HELLO, encryptedSessionKey, encryptedSessionKeyLength)) return false;

	// Wait for ACK
	if(!FLP_ReceiveHeader(connection, &header)) return false;
	if(header.type != FLP_SET_ACK_BIT(FLP_TYPE_SERVER_HELLO) || header.payloadLength != 0) return false;

	// Change state of the connection
	connection->state = FLP_CONNECTED;

	return true;
}

static bool FLP_Terminate(FLP_Connection_t *connection, bool write, bool read, bool receive)
{
	bool result;

	// Send terminating event to the functions selected in parameters
	if(write) {
		result = FLP_EventSend(connection->terminateWrite);
		if(!result) return false;
	}
	if(read) {
		result = FLP_EventSend(connection->terminateRead);
		if(!result) return false;
	}
	if(receive) {
		result = FLP_EventSend(connection->terminateReceive);
		if(!result) return false;
	}

	return true;
}


static bool FLP_TransmitPacket(FLP_Connection_t *connection, uint16_t type, uint8_t* payload, size_t length)
{
	bool result;
	uint8_t *packet;
	FLP_Header_t *header;

	// Allocate memory and copy the payload
	packet = (uint8_t*)malloc(sizeof(FLP_Header_t) + length*sizeof(uint8_t));
	if(packet == NULL) return false;
	memcpy(packet + sizeof(FLP_Header_t), payload, length);

	// Fill in the header
	header = (FLP_Header_t*)packet;
	header->type = FLP_TYPE_DATA;
	header->payloadLength = length;

	result = FLP_Transmit(connection, packet, sizeof(FLP_Header_t) + length*sizeof(uint8_t));

	free(packet);

	return result;
}
