#include "cs352proxy.h"
#include "frames.h"
#include <time.h>
#include "uthash.h"

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

void processLinkState(char * buf, time_t timeIn) {
	uint16_t num_records = *(uint16_t*)(buf+HEADER_SIZE);
	int i;
	int offset = HEADER_SIZE + NUM_PEERS_SIZE;
	for (i = 0; i < num_records; i++) {
		uint16_t local_ip = ntohs(*(uint16_t *)(buf+offset));
		offset += IP_SIZE;
		uint16_t local_port = ntohs(*(uint16_t *)(buf+offset));
		offset += PORT_SIZE;
		char local_mac[6];
		memcpy(local_mac, (buf+offset), MAC_SIZE);
		offset += MAC_SIZE;
		uint16_t remote_ip = ntohs(*(uint16_t *)(buf+offset));
		offset += IP_SIZE;
		uint16_t remote_port = ntohs(*(uint16_t *)(buf+offset));
		offset += PORT_SIZE;
		char remote_mac[6];
		memcpy(remote_mac, (buf + offset), MAC_SIZE);
		offset += MAC_SIZE;
		uint32_t RTT = ntohs(*(uint32_t *)(buf+offset));
		offset += RTT_SIZE;
		unsigned long long ID = ntohs(*(unsigned long long *)(buf+offset));
		offset += ID_SIZE;
		struct node * lookup;
		HASH_FIND_STR(node_list, local_mac, lookup);
		if (lookup == NULL) {
			struct node * insert = (struct node *)malloc(sizeof(struct node));
			memcpy(insert->local_mac, local_mac, MAC_SIZE);
			HASH_ADD(node_list, local_mac, insert);
			//CREATE THE ACTUAL CONNECTION HERE
		}
		HASH_FIND_STR(node_list, remote_mac, lookup);
		if (lookup == NULL) {
			struct node * insert = (struct node *)malloc(sizeof(struct node));
			memcpy(insert->local_mac, remote_mac, MAC_SIZE);
			HASH_ADD(node_list, local_mac, insert);
			//CREATE THE ACTUAL CONNECTION HERE
		}
		HASH_FIND_STR(node_list, local_mac, lookup);
		struct connection_node * linkstate;
		HASH_FIND_STR(lookup->connection, remote_mac, linkstate);
		if (linkstate == NULL) {
			struct connection_node * insert =  (struct connection_node *)malloc(sizeof(struct connection_node));
			memcpy(insert->local_mac, local_mac, MAC_SIZE);
			insert->local_ip = local_ip;
			insert->local_port = local_port;
			memcpy(insert->local_mac, remote_mac, MAC_SIZE);
			insert->remote_ip = remote_ip;
			insert->remote_port = remote_port;
			insert->RTT = RTT;
			insert->ID = ID;
			insert->timestamp = timeIn;
			//PROBLY PUT THE NEXT_HOP IN HERE
			HASH_ADD(lookup->connection, remote_mac, insert);
		}
		else if (ID > linkstate->ID) {
			linkstate->ID = ID;
			linkstate->RTT = RTT;
			linkstate->timestamp = timeIn;
		}
		
	}
	
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
  time_t timeIn = time(NULL);						//number of seconds passed between 12/31/1968 at 7pm and time frame is read 
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
    	    processLinkState(buf, timeIn);
      break;
    case RTTPRBRQST:
      break;
    case RTTPRBRESP:
      break;
    default:
      ERROR("Unknown packet type\n", -1);
    }
}
