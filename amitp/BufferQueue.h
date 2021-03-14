#pragma once

#ifndef BUFFERQUEUE_H
#define BUFFERQUEUE_H

#include <vector>

struct PacketPayload
{
	PacketPayload(uint32_t timestamp, uint32_t extSeqNum, size_t payloadLength) : timestamp(timestamp), extSeqNum(extSeqNum), payloadLength(payloadLength)
	{
		payload.resize(payloadLength);
	}
	std::vector<unsigned char> payload; //payload container
	uint32_t timestamp;
	uint32_t extSeqNum;
	size_t payloadLength;
};

struct Frame
{
	Frame(uint32_t timestamp) : timestamp(timestamp), currentLength(0), frameComplete(false) {}
	std::vector<PacketPayload> frameData; //contains the payloads to make the frame
	uint32_t timestamp;
	size_t currentLength;
	bool frameComplete;
	
};

class BufferQueues
{
public:
	BufferQueues(size_t bufferSize);
protected:
	//frames
	std::vector<Frame> frameQueue;

	void addFrame(uint32_t timestamp);
	void addPayload(void* pPayload, size_t payloadLength, uint32_t timestamp, uint32_t extSeqNum);
	
	size_t m_bufferSize;

};

#endif
