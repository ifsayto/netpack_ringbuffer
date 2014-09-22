/*@@@@@@@ @@@@@@@
    @@   @@
   @@   @@@@@@@ 
  @@        @@ 
 @@   @@@@@@@*/
#include "packringbuffer.h"
#include <memory>
#include <WinSock2.h>

struct netpackringbuffer {
	int capacity;
	int packN;   //net pack number
	int used;   //buffer is used
	int usedB;   //used begin
	int usedE;	
	char data[1];   //net pack
};

struct netpack {
	tPackLenType len;
	char data[1];
};

#define kNetPackLenBytes   (sizeof(tPackLenType)/sizeof(char))

struct netpackringbuffer* netpack_new(int size)
{
	struct netpackringbuffer* rb = (struct netpackringbuffer*)malloc(sizeof(*rb) + size);
	memset(rb, 0, sizeof(struct netpackringbuffer));
	rb->capacity = size;
	printf("[netpack] new %d\n", size);
	return rb;
}

int netpack_can_write(struct netpackringbuffer* rb)
{
	if (rb->used)
	{
		return rb->capacity-rb->usedB+rb->usedE;
	}
	else
		return rb->capacity;
}

int netpack_can_read(struct netpackringbuffer* rb)
{
	int can_read_bytes = 0;
	if (rb->used > kNetPackLenBytes)
	{
		struct netpack* np = (struct netpack*)(rb->data+rb->usedB);
		if (np->len <= rb->used-kNetPackLenBytes)
		{			
			can_read_bytes = np->len+kNetPackLenBytes;
		}
	}
	printf("[netpack] can_read %d, used %d\n", can_read_bytes, rb->used);
	return can_read_bytes;
}

int netpack_read(struct netpackringbuffer* rb, char* buffer)
{
	printf("[netpack][read] begin {capacity %d}\n", netpack_can_write(rb));
	struct netpack* np = (struct netpack*)(rb->data+rb->usedB);
	int need_copy = np->len+kNetPackLenBytes;
	int need_back = (rb->usedB+need_copy > rb->capacity) ? 1:0;
	int back_count = (rb->usedB+need_copy)%rb->capacity;
	if (need_back)
	{
		memcpy(buffer, rb->data+rb->usedB, need_copy-back_count);
		rb->usedB = 0;
		if (back_count)
		{
			memcpy(buffer+need_copy-back_count, rb->data+rb->usedB, back_count);
			rb->usedB = back_count;			
		}
	}
	else
	{
		memcpy(buffer, rb->data+rb->usedB, need_copy);
		rb->usedB += (need_copy);
	}
	rb->used -= need_copy;
	printf("[netpack][read] read: %d\n", need_copy);
	printf("[netpack][read] end {capacity %d}\n", netpack_can_write(rb));
	return need_copy;
}

int netpack_write(struct netpackringbuffer* rb, char* buffer, int len)
{
	int can_write = netpack_can_write(rb);
	can_write = (can_write > len) ? len:can_write;
	if (can_write == 0) return 0;
	printf("[netpack][write] begin {capacity %d}\n", can_write);

	int need_back = (rb->usedE+can_write > rb->capacity) ? 1:0;
	int back_count = (rb->usedE+can_write)%rb->capacity;
	if (need_back)
	{
		memcpy(rb->data+rb->usedE, buffer, can_write-back_count);
		rb->usedE = 0;
		if (back_count)
		{
			memcpy(buffer+can_write-back_count, rb->data+rb->usedB, back_count);
			rb->usedE = back_count;			
		}
	}
	else
	{
		memcpy(rb->data+rb->usedE, buffer, can_write);
		rb->usedE += (can_write);
	}
	printf("[netpack][write] end {capacity %d}\n", netpack_can_write(rb));
	return can_write;
}

int netpack_recv(struct netpackringbuffer* rb, int fd)
{
	int can_receive_size = netpack_can_write(rb);
	if (can_receive_size <= 0) {printf("[netpack] can_receive_size is invalid"); return 0;}
	printf("[netpack][recv] begin {capacity %d}\n", can_receive_size);
	int received_bytes = 0;
	int all_received_bytes = 0;

	do {
		int received_bytes = 0;
		if (rb->usedE >= rb->usedB)
			can_receive_size = rb->capacity-rb->usedE;
		else
			can_receive_size = rb->usedB-rb->usedE;
		printf("[netpack][recv] can_receive_size %d\n", can_receive_size);
		received_bytes = recv(fd, rb->data+rb->usedE, can_receive_size, 0);
		printf("[netpack][recv] receive_size: %d\n", received_bytes);
		printf("[netpack][recv] receive: %s\n", rb->data+rb->usedE);
		if (received_bytes > 0)
			rb->usedE = (rb->usedE+received_bytes)%rb->capacity;
		else
			return received_bytes;
		rb->used += received_bytes;
		all_received_bytes += received_bytes;
		can_receive_size = netpack_can_write(rb);
	} while(can_receive_size && received_bytes > 0);

	printf("[netpack][recv] end {capacity %d}\n", netpack_can_write(rb));
	return all_received_bytes;
}
