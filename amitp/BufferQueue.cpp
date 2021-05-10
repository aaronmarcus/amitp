#include "BufferQueue.h"

//construct the buffer queues
BufferQueues::BufferQueues(size_t bufferSize) : m_bufferSize(bufferSize)
{
	//initializations are done in the constructor list
}

//function to add new frame to the queue
void BufferQueues::addFrame(uint32_t timestamp)
{
	std::unique_lock<std::mutex> lock(m); //lock till the end of the function scope
	frameQueue.emplace_back(Frame(timestamp)); //add new frame holder to the back of the queue
}

//function to add the payload to the queue
void BufferQueues::addPayload(void* pPayload, size_t payloadLength, uint32_t timestamp, uint32_t extSeqNum)
{
	//do pointer arithmetic to miss out the payload header of the packet
	void* pPayloadAdj = static_cast<unsigned char*>(pPayload) + 8; //62 accounts for the payload header
	
	std::unique_lock<std::mutex> lock(m);//lock till the end of the function scope
	frameQueue.back().frameData.emplace_back(PacketPayload(timestamp, extSeqNum, payloadLength - 8)); //add a new payload to the frameData
	memcpy(&(frameQueue.back().frameData.back().payload[0]), pPayloadAdj, payloadLength - 8); //copy the payload into the vector
	frameQueue.back().currentLength += (payloadLength - 8); //update the current length
	frameQueue.back().frameComplete = (frameQueue.back().currentLength == m_bufferSize); //check to see if the frame payload is complete
}

           