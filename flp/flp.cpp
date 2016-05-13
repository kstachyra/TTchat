/*
 * flp.cpp
 *
 *  Created on: 12 maj 2016
 *      Author: krystian
 */

#include "flp.h"

/* Settings ---------------------------------------------------------------- */
#define FLP_ENABLE_LOG						true

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
	uint16_t length;
} FLP_Header_t;

/* Private functions' prototypes ------------------------------------------- */
static void *FLP_Thread(void *args);
static bool FLP_PushToQueue(FLP_Queue_t *queue, uint8_t *data, size_t length);
static uint8_t *FLP_PopFromQueue(FLP_Queue_t *queue, size_t *length);
static bool FLP_EventSend(int eventFD);
static bool FLP_Transmit(FLP_Connection_t *connection, uint8_t *data, size_t length);
static bool FLP_Receive(FLP_Connection_t *connection, uint8_t *data, size_t length);
static bool FLP_ReceiveHeader(FLP_Connection_t *connection, FLP_Header_t *header);
static bool FLP_SendAck(FLP_Connection_t *connection, uint16_t type);
static bool FLP_Handshake(FLP_Connection_t *connection);

/* Exported functions ------------------------------------------------------ */
FLP_Connection_t* FLP_Connect(int socket)
{
	bool result;
	FLP_Connection_t *connection;

	connection = (FLP_Connection_t*)malloc(sizeof(FLP_Connection_t));

	// TODO: Add function FLP_Init initializing semaphores, eventFDs, queues, configuring socket etc.
	connection->socket = socket;

	// Perform FLP handshake (establish connection and exchange keys)
	result = FLP_Handshake(connection);
	if(!result) return NULL;

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

	// Encrypt data
	// ...

	// Transmit using low-level function
	FLP_Transmit(connection, encryptedData, encryptedDataLength);

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

uint8_t* FLP_Read(FLP_Connection_t *connection, size_t *length)
{
	int ndfs, result;
	fd_set rfds;
	uint8_t *data;
	size_t dataLength;

	// Block on queue and termination request simultaneously
	FD_ZERO(&rfds);
	FD_SET(connection->readQueue.newElementAvailable, &rfds);
	FD_SET(connection->terminateRead, &rfds);
	ndfs = (connection->readQueue.newElementAvailable > connection->terminateRead) ? (connection->readQueue.newElementAvailable + 1) : (connection->terminateRead + 1);
	result = select(ndfs, &rfds, NULL, NULL, NULL);
	if(result < -1) {
		perror("select()");
		*length = 0;
		return false;
	}

	// If termination request was received...
	if(FD_ISSET(connection->terminateRead, &rfds)) {
		FLP_LOG("FLP_Write(): Terminated while waiting for ACK.\n");
		return false;
	}

	// If new data is available...
	if(FD_ISSET(connection->readQueue.newElementAvailable, &rfds)) {
		data = FLP_PopFromQueue(&connection->readQueue, &dataLength);
		*length = dataLength;
		return data;
	}

	// Should never happen...
	FLP_LOG("FLP_Write(): WTF?! No file descriptor is set.\n");
	*length = 0;
	return false;
}

void FLP_Close(FLP_Connection_t *connection)
{
	// Send FLP_EVENT_TERMINATE to FLP_Thread, FLP_Write and FLP_Read
	// TODO: Check for errors
	FLP_EventSend(connection->terminateReceive);
	FLP_EventSend(connection->terminateRead);
	FLP_EventSend(connection->terminateWrite);

	// Close TCP socket
	// ...

	// Free memory used for connection structure
	free(connection);
}

/* Private functions ------------------------------------------------------- */
static void *FLP_Thread(void *args)
{
	bool result, shouldTerminate = false;
	uint8_t *decryptedData;
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
			// ...

			// Decrypt payload
			// ...

			// Push decrypted payload to queue and notify
			FLP_PushToQueue(&connection->readQueue, decryptedData, decryptedDataLength);
			FLP_EventSend(connection->readQueue.newElementAvailable);

			// Send ACK
			result = FLP_SendAck(connection, header.type);
			// TODO: Check for errors
		}
	}
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

	// TODO: Add timeout
	pthread_mutex_lock(&connection->transmitLock);

	while(bytesSentTotal != length) {
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
	size_t bytesRead, bytesReadTotal = 0;

	while(bytesReadTotal != length) {

		// Wait for new data or termination request
		FD_ZERO(&rfds);
		FD_SET(connection->socket, &rfds);
		FD_SET(connection->terminateReceive, &rfds);
		ndfs = (connection->socket > connection->terminateReceive) ? (connection->socket + 1) : (connection->terminateReceive + 1);
		result = select(ndfs, &rfds, NULL, NULL, NULL);
		if(result < -1) {
			perror("select()");
			*length = 0;
			return false;
		}

		// Check if there is new data
		if(FD_ISSET(connection->socket, &rfds)) {

			// Read
			// ...
		}

		// Check if termination request was received
		if(FD_ISSET(connection->terminateReceive, &rfds)) {

		}

		// Should never happen...
		FLP_LOG("FLP_Write(): WTF?! No file descriptor is set.\n");
		return false;
	}
}

static bool FLP_ReceiveHeader(FLP_Connection_t *connection, FLP_Header_t *header)
{
	return FLP_Receive(connection, (uint8_t*)header, sizeof(FLP_Header_t));
}

bool FLP_SendAck(FLP_Connection_t *connection, uint16_t type)
{
	uint8_t buffer[sizeof(FLP_Header_t)];
	FLP_Header_t *header = buffer;

	header->type = FLP_SET_ACK_BIT(type);
	header->length = 0;

	return FLP_Transmit(connection, buffer, sizeof(FLP_Header_t));
}

static bool FLP_Handshake(FLP_Connection_t *connection)
{
	// Wait for ClientHello packet
	// ...

	// Generate symmetrical key
	// ...

	// Encrypt generated key
	// ...

	// Send ServerHello
	// ...

	// Wait for ACK
	// ...

	return true;
}
