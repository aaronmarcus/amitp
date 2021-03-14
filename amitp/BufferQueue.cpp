#include "BufferQueue.h"

BufferQueues::BufferQueues(size_t bufferSize) : m_bufferSize(bufferSize)
{
	//initialisations are done in the constructor lsit
}

void BufferQueues::addFrame(uint32_t timestamp)
{
	frameQueue.emplace_back(Frame(timestamp));
}

void BufferQueues::addPayload(void* pPayload, size_t payloadLength, uint32_t timestamp, uint32_t extSeqNum)
{
	frameQueue.back().frameData.emplace_back(PacketPayload(timestamp, extSeqNum, payloadLength));
	memcpy(&(frameQueue.back().frameData.back().payload[0]), pPayload, payloadLength); //copy the payload into the vector
	frameQueue.back().currentLength += payloadLength;
	frameQueue.back().frameComplete = (frameQueue.back().currentLength == m_bufferSize);
}

