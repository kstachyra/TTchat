#include "SLPPacket.h"

#include <iostream>

SLPPacket::SLPPacket()
{
	this->type = ERR;
	resizeVec();
	setTypeToVec();
}

SLPPacket::SLPPacket(messageType type)
{
	this->type = type;
	resizeVec();
	setTypeToVec();
}

SLPPacket::SLPPacket(messageType type, int length)
{
	this->type = type;
	resizeVec(length);
	setTypeToVec();
}

SLPPacket::SLPPacket(uint8_t *data, size_t length)
{
	vecLength = length;
	vec.assign (data,data+length);
	setTypeFromVec();
}

/*SLPPacket(const SLPPacket &pck);
{
	if (pck.type==MSGCLI)
	{
		this->type = MSGSER;
		resizeVec(pck.vecLength + 8);
		setTypeToVec();
	}
	else
	{
		this->type = ERR;
		resizeVec();
		setTypeToVec();
		std::cout<<"SLPPacket.SLPPacket: wywołano konstruktor dla innego typu niż MSGCLI"
	}
}*/

int SLPPacket::getType()
{
	return this->type;
}

int SLPPacket::getChatroomId()
{
	if (type == ERR)
	{
		std::cout<< "SLPPacket: nie można odczytać RoomID dla Message typu ERR" <<"\n";
		return -1;
	}
	return toInt(2, 5);
}

void SLPPacket::setChatroomId(uint64_t id)
{
	if (type == ERR)
	{
		std::cout<< "SLPPacket: SLPPacket: nie można ustawić RoomID dla Message typu ERR" <<"\n";
	}
	else
	{
		intToVec(id, 2, 5);
	}
}

int SLPPacket::getReason()
{
	if (type == SUBREF)
	{
		return toInt(6, 9);
	}
	std::cout<< "SLPPacket: błędny typ Message dla getReason" <<"\n";
	return -1;
}

void SLPPacket::setReason(int reason)
{
	if (type == SUBREF)
	{
		intToVec(reason, 6, 9);
	}
	else std::cout<< "SLPPacket: błędny typ Message dla setReason" <<"\n";
}

int SLPPacket::getLastMessageID()
{
	if (type == ROOMINF)
	{
		return toInt(6, 9);
	}
	else if (type == PULLMSGS)
	{
		return toInt(10, 13);
	}
	std::cout<< "SLPPacket: błędny typ Message dla getLastMessageID" <<"\n";
	return -1;
}

void SLPPacket::setLastMessageID(int id)
{
	if (type == ROOMINF)
	{
		intToVec(id, 6, 9);
	}
	else if (type == PULLMSGS)
	{
		intToVec(id, 10, 3);
	}
	else std::cout<< "SLPPacket: błędny typ Message dla setLastMessageID" <<"\n";
}

int SLPPacket::getNumberOFMessages()
{
	if (type == ROOMINF)
	{
		return toInt(10, 13);
	}
	std::cout<< "SLPPacket: błędny typ Message dla getLastMessageID" <<"\n";
	return -1;
}

void SLPPacket::setNumberOFMessages(int num)
{
	if (type == ROOMINF)
	{
		intToVec(num, 10, 13);
	}
	else std::cout<< "SLPPacket: błędny typ Message dla setLastMessageID" <<"\n";
}

int SLPPacket::getFirstMessageID()
{
	if (type == PULLMSGS)
	{
		return toInt(6, 9);
	}
	std::cout<< "SLPPacket: błędny typ Message dla getFirstMessageID" <<"\n";
	return -1;
}

void SLPPacket::setFirstMessageID(int id)
{
	if (type == PULLMSGS)
	{
		intToVec(id, 6, 9);
	}
	else std::cout<< "SLPPacket: błędny typ Message dla setFirstMessageID" <<"\n";
}

int SLPPacket::getMessageID()
{
	if (type == MSGSER)
	{
		return toInt(6, 9);
	}
	std::cout<< "SLPPacket: błędny tym Message dla getMessageID" <<"\n";
	return -1;
}

void SLPPacket::setMessageID(int id)
{
	if (type == MSGSER)
	{
		intToVec(id, 6, 9);
	}
	else std::cout<< "SLPPacket: błędny typ Message dla setMessageID" <<"\n";
}

int SLPPacket::getTime()
{
	if (type == MSGSER)
	{
		return toInt(11, 14);
	}
	std::cout<< "SLPPacket: błędny tym Message dla getTime" <<"\n";
	return -1;
}

void SLPPacket::setTime(int timestamp)
{
	if (type == MSGSER)
	{
		intToVec(timestamp, 11, 14);
	}
	else std::cout<< "SLPPacket: błędny typ Message dla setTime" <<"\n";
}

string SLPPacket::getNick()
{
	if (type == MSGSER)
	{
		return toString(14, 45);
	}
	else if (type == MSGCLI)
	{
		return toString(6, 37);
	}
	std::cout<< "SLPPacket: błędny typ Message dla getNick" <<"\n";
	return NULL;
}

void SLPPacket::setNick(string nick)
{
	if (type == MSGSER)
	{
		stringToVec(nick, 14, 45);
	}
	else if (type == MSGCLI)
	{
		stringToVec(nick, 6, 37);
	}
	else std::cout<< "SLPPacket: błędny typ Message dla setNick" <<"\n";
}

int SLPPacket::getMessageLength()
{
	if (type == MSGSER)
	{
		return toInt(46, 47);
	}
	else if (type == MSGCLI)
	{
		return toInt(38, 39);
	}
	std::cout<< "SLPPacket: błędny typ Message dla getMessageLength" <<"\n";
	return -1;
}

void SLPPacket::setMessageLength(int length)
{
	if (type == MSGSER)
	{
		intToVec(length, 46, 47);
	}
	else if (type == MSGCLI)
	{
		intToVec(length, 38, 39);
	}
	else std::cout<< "SLPPacket: błędny typ Message dla setMessageLength" <<"\n";
}

string SLPPacket::getMessage()
{
	if (type == MSGSER)
	{
		return toString(48, 48+getMessageLength());
	}
	else if (type == MSGCLI)
	{
		return toString(40, 40+getMessageLength());
	}
	std::cout<< "SLPPacket: błędny typ Message dla getMessage" <<"\n";
	return NULL;
}

void SLPPacket::setMessage(string msg)
{
	if (type == MSGSER)
	{
		stringToVec(msg, 48, 48+getMessageLength());
	}
	else if (type == MSGCLI)
	{
		stringToVec(msg, 40, 40+getMessageLength());
	}
	else std::cout<< "SLPPacket: błędny typ Message dla setMessage" <<"\n";
}

void SLPPacket::toDataBuffer(uint8_t** data, size_t* length)
{
	*length = vec.size();

	*data = new uint8_t[*length];

	for (size_t i=0; i<*length; ++i)
	{
		(*data)[i] = vec[i];
	}
	setTypeFromVec();
}

void SLPPacket::print()
{
	std::cout<<"╔════════════════════════════════════════════════╗\n";
	for (int i=0; i<vecLength; ++i)
	{
		printf("%02x.", vec[i]);
	}
	std::cout<<"\n";
	std::cout<<"╚════════════════════════════════════════════════╝\n";
}

int SLPPacket::toInt(int first, int last)
{
	uint64_t Int64 = 0;
	for (int i = last; i >= first; --i)
	{
		Int64 = (Int64 << 8) + vec[i];
	}
	return Int64;
}

void SLPPacket::intToVec(uint64_t value, int first, int last)
{
	for (int i=first; i<=last; ++i)
	{
		vec[i] = (uint8_t) ((value >> 8*(i-first)) & 0xFF);
	}
}

string SLPPacket::toString(int first, int last)
{
	string str;
	for (int i=first; i<=last; ++i)
	{
		str.push_back(vec[i]);
	}
	return str;
}

void SLPPacket::stringToVec(string str, int first, int last)
{
	int k = 0;
	for (std::string::iterator it=str.begin(); it!=str.end(); ++it)
	{
		vec[first+k] = *it;
		++k;
		if (k>last) break;
	}
}

void SLPPacket::setTypeFromVec()
{
	if (vec[0] == 0x00 && vec[1] == 0x10) type = SUBREQ;
	else if (vec[0] == 0x00 && vec[1] == 0x11) type = SUBACK;
	else if (vec[0] == 0x00 && vec[1] == 0x12) type = SUBREF;
	else if (vec[0] == 0x00 && vec[1] == 0x13) type = UNSUB;
	else if (vec[0] == 0x00 && vec[1] == 0x20) type = GETINF;
	else if (vec[0] == 0x00 && vec[1] == 0x21) type = ROOMINF;
	else if (vec[0] == 0x00 && vec[1] == 0x30) type = PULLMSGS;
	else if (vec[0] == 0x00 && vec[1] == 0x31) type = MSGSER;
	else if (vec[0] == 0x00 && vec[1] == 0x32) type = MSGCLI;
	else if (vec[0] == 0x00 && vec[1] == 0x00) type = ERR;
}

void SLPPacket::setTypeToVec()
{
	switch(type)
	{
	case SUBREQ: vec[0] = 0x00; vec[1] = 0x10; break;
	case SUBACK: vec[0] = 0x00; vec[1] = 0x11; break;
	case SUBREF: vec[0] = 0x00; vec[1] = 0x12; break;
	case UNSUB: vec[0] = 0x00; vec[1] = 0x13; break;
	case GETINF: vec[0] = 0x00; vec[1] = 0x20; break;
	case ROOMINF: vec[0] = 0x00; vec[1] = 0x21; break;
	case PULLMSGS: vec[0] = 0x00; vec[1] = 0x30; break;
	case MSGSER: vec[0] = 0x00; vec[1] = 0x31; break;
	case MSGCLI: vec[0] = 0x00; vec[1] = 0x32; break;
	case ERR: vec[0] = 0x00; vec[1] = 0x00; break;
	}
}

void SLPPacket::resizeVec()
{
	switch(type)
	{
	case SUBREQ: vecLength = SUBREQ_LENGTH; break;
	case SUBACK: vecLength = SUBACK_LENGTH; break;
	case SUBREF: vecLength = SUBREF_LENGTH; break;
	case UNSUB: vecLength = UNSUB_LENGTH; break;
	case GETINF: vecLength = GETINF_LENGTH; break;
	case ROOMINF: vecLength = ROOMINF_LENGTH; break;
	case PULLMSGS: vecLength = PULLMSGS_LENGTH; break;
	case ERR: vecLength = ERR_LENGTH; break;

	default: std::cout<< "SLPPacket.resizeVec: błędny typ pakietu (brak długości)" <<"\n";
	}
	vec.resize(vecLength);
}

void SLPPacket::resizeVec(int length)
{
	switch(type)
	{
	case MSGSER: vecLength = MSGSER_LENGTH + length; break;
	case MSGCLI: vecLength = MSGCLI_LENGTH + length; break;

	default:
	{
		std::cout<< "SLPPacket: błędny typ pakietu (podana długość dla pakietu o stałej długości)" <<"\n";
		this->type = ERR;
	}
	}
	vec.resize(vecLength);
}
