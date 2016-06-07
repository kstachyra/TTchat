/*
 * Message.h
 *
 *  Created on: 4 cze 2016
 *      Author: krystian
 */
#include <inttypes.h>
#include <vector>

#ifndef MODEL_MESSAGE_H_
#define MODEL_MESSAGE_H_

using namespace std;

#define MESSAGE_NICK_LENGTH					32

class Message {
public:
	uint32_t id;
	uint32_t timestamp;
	uint8_t nick[MESSAGE_NICK_LENGTH];
	uint8_t *payload;
	size_t payloadLength;

	/* Pedalskie API dla pedalow */
	void setNick(string nick);
	string getNick(void);
};

#endif /* MODEL_MESSAGE_H_ */
