/*
 * flp.h
 *
 *  Created on: 12 maj 2016
 *      Author: krystian
 */

#ifndef FLP_H_
#define FLP_H_

#include "flp_crypto.h"
#include <pthread.h>
#include <stdint.h>

/* Exported types ---------------------------------------------------------- */
typedef struct FLP_ListElement {
	uint8_t *data;
	size_t length;
	struct FLP_ListElement *next;
} FLP_ListElement_t;

typedef struct {
	int newElementAvailable;
	FLP_ListElement_t *head;
	pthread_mutex_t lock;
} FLP_Queue_t;

typedef enum {FLP_DISCONNECTED, FLP_CONNECTED} FLP_State_t;

typedef struct {
	int socket;

	int terminateWrite;
	int terminateRead;
	int terminateReceive;
	int ackReceived;

	uint8_t sessionKey[FLP_SESSION_KEY_LENGTH];

	FLP_Queue_t readQueue;

	pthread_mutex_t transmitLock;

	pthread_t thread;
	pthread_attr_t threadAttr;

	FLP_State_t state;
} FLP_Connection_t;

/* Exported functions ------------------------------------------------------ */
FLP_Connection_t* FLP_Connect(int socket);
bool FLP_Write(FLP_Connection_t *connection, uint8_t *data, size_t length);
bool FLP_Read(FLP_Connection_t *connection, uint8_t **data, size_t *length);
bool FLP_Close(FLP_Connection_t *connection);

#endif /* FLP_H_ */
