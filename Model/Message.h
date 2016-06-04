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

class Message {
public:
	uint32_t id;
	uint32_t timestamp;
	uint8_t nick[32];
	vector<uint8_t> payload;
};

#endif /* MODEL_MESSAGE_H_ */
