#include "D3DBuffer.h"

TD3DBuffer::TD3DBuffer()
{
	GpuResource = std::make_shared<TD3DResource>();
}

TD3DBuffer::~TD3DBuffer()
{

}
