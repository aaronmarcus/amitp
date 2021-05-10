#pragma once

#ifndef BUFFERQUEUE_H
#define BUFFERQUEUE_H

#include <vector>
#include <mutex>
#include <iostream>

//structure to hold the received apcket payload
struct PacketPayload
{
	//constructor
	PacketPayload(uint32_t timestamp, uint32_t extSeqNum, size_t payloadLength) : timestamp(timestamp), extSeqNum(extSeqNum), payloadLength(payloadLength)
	{
		payload.resize(payloadLength); //make the vector the length of the payload
	}
	std::vector<unsigned char> payload; //payload container
	uint32_t timestamp; //packet timestamp
	uint32_t extSeqNum; //packet extended sequence number
	size_t payloadLength; //length of the payload
};

//structure to hold a single frames data
struct Frame
{
	//Frame constructor
	Frame(uint32_t timestamp) : timestamp(timestamp), currentLength(0), frameComplete(false) {}
	std::vector<PacketPayload> frameData; //contains the payloads to make the frame
	uint32_t timestamp; //timestamp of the frame
	size_t currentLength; //tracker for the frame
	bool frameComplete; //is all the data there?
};

//class to manage the frame Queue
class BufferQueues
{
public:
	BufferQueues(size_t bufferSize); //constructor
	std::mutex m; //mutex lock
	std::vector<Frame> frameQueue; //frames
protected:
	//function to add a new frame to the queue
	void addFrame(uint32_t timestamp);
	//add a new payload to the frame
	void addPayload(void* pPayload, size_t payloadLength, uint32_t timestamp, uint32_t extSeqNum);
	
	size_t m_bufferSize; //tracker for the size
	bool firstFrame; //tracker for the first frame of the stream
};

#endif
