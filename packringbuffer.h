/*@@@@@@@ @@@@@@@
    @@   @@
   @@   @@@@@@@ 2014.04.01
  @@        @@ 
 @@   @@@@@@@*/

#ifndef __TSH_netpackringbuffer__
#define __TSH_netpackringbuffer__

#ifdef  __cplusplus
extern "C" {
#endif

#define tPackLenType   int   //char short int

struct netpackringbuffer;
struct netpack;

typedef struct netpackringbuffer   tNetPackRingBuffer;
typedef struct netpack   tNetPack;




struct netpackringbuffer* netpack_new(int size);

int netpack_can_write(struct netpackringbuffer* rb);

int netpack_can_read(struct netpackringbuffer* rb);

int netpack_read(struct netpackringbuffer* rb, char* buffer);

int netpack_write(struct netpackringbuffer* rb, char* buffer, int len);

int netpack_recv(struct netpackringbuffer* rb, int fd);

#ifdef  __cplusplus
}
#endif

#endif
