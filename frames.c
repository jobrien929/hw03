#include "cs352proxy.h"
#include "frames.h"

int
outgoing(void)
{
  return 0;
}

int
newconnect(void)
{
  return 0;
}

void processLinkState(char * buf) {
	uint16_t num_neighbors = (uint16_t)buf[HEADER_SIZE];
	
	
}

int
processFrame(Forward *frame)
{
  //pthread_mutex_unlock(&forwardLock);
  int r;
  uint16_t type;
  uint16_t length;
  char buf[HEADER_SIZE + PAYLOAD_SIZE];
  r =  readn(frame->nexthop_fd, buf, HEADER_SIZE);
  if(r == 0)
    socket_closed(frame);

  type = ntohs(*(uint16_t*)buf);
  length = ntohs(*(uint16_t*)(buf+TYPE_SIZE));
  // figure out what we've got
  switch(type)
    {
    case DATA:
      break;
    case LEAVE:
      break;
    case QUIT:
      break;
    case LINKSTATE:
    	    DEBUG("Got Linkstate Packet\n");
    	    processLinkState(buf);
      break;
    case RTTPRBRQST:
      break;
    case RTTPRBRESP:
      break;
    default:
      ERROR("Unknown packet type\n", -1);
    }
}
