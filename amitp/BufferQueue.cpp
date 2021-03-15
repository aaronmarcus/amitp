#include "BufferQueue.h"

BufferQueues::BufferQueues(size_t bufferSize) : m_bufferSize(bufferSize)
{
	//initialisations are done in the constructor lsit
}

void BufferQueues::addFrame(uint32_t timestamp)
{
	std::lock_guard<std::mutex> lock(m); //lock till the end of the function scope
	frameQueue.emplace_back(Frame(timestamp));
	if (frameQueue.size() == 30)
	{
		std::cout << "oh" << std::endl;
	}
}

void BufferQueues::addPayload(void* pPayload, size_t payloadLength, uint32_t timestamp, uint32_t extSeqNum)
{
	uint32_t m_timestamp = timestamp;
	if (firstFrame) { m_timestamp = 0;}
	
	std::lock_guard<std::mutex> lock(m); //lock till the end of the function scope
	frameQueue.back().frameData.emplace_back(PacketPayload(m_timestamp, extSeqNum, payloadLength)); //add a new payload to the frameData
	memcpy(&(frameQueue.back().frameData.back().payload[0]), pPayload, payloadLength); //copy the payload into the vector
	frameQueue.back().currentLength += payloadLength; //update the current length
	//std::cout << frameQueue.back().currentLength << std::endl;
	frameQueue.back().frameComplete = (frameQueue.back().currentLength == m_bufferSize); //check to see if the frame payload is complete
}

