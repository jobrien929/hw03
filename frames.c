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
	uint16_t num_records = *(uint16_t*)(buf+HEADER_SIZE);
	int i;
	int offset = HEADER_SIZE + NUM_PEERS_SIZE;
	for (int i = 0; i < num_records; i++) {
		uint16_t local_ip = ntohs(*(uint16_t *)(buf+offset));
		offset += IP_SIZE;
		uint16_t local_port = ntohs(*(uint16_t *)(buf+offset));
		offset += PORT_SIZE;
		char[6] local_mac;
		memcpy(local_mac, (buf+offset), MAC_SIZE);
		offset += MAC_SIZE;
		uint16_t remote_ip = ntohs(*(uint16_t *)(buf+offset));
		offset += IP_SIZE;
		uint16_t remote_port = ntohs(*(uint16_t *)(buf+offset));
		offset += PORT_SIZE;
		char[6] remote_mac;
		memcpy(remote_mac, (buf + offset), MAC_SIZE);
		offset += MAC_SIZE;
		uint32_t RTT = ntohs(*(uint32_t *)(buf+offset));
		offset += RTT_SIZE;
		unsigned long long ID = ntohs(*(unsigned long long *)(buf+offset));
		offset += ID_SIZE;
		struct connection_node * lookup;
		HASH_FIND_STR(node_list, local_mac, lookup);
		if (lookup == NULL) {
			struct node * insert = (node *)malloc(sizeof(struct node));
			insert->local_mac = local_mac;
			HASH_ADD(node_list, local_mac, insert);
			//CREATE THE ACTUAL CONNECTION HERE
		}
		HASH_FIND_STR(node_list, remote_mac, lookup);
		if (lookup == NULL) {
			struct node * insert = (node *)malloc(sizeof(struct node));
			insert->local_mac = remote_mac;
			HASH_ADD(node_list, local_mac, insert);
			//CREATE THE ACTUAL CONNECTION HERE
		}
		HASH_FIND_STR(node_list, local_mac, lookup);
		struct connection_node * linkstate;
		HASH_FIND_STR(lookup->connection, remote_mac, linkstate);
		if (linkstate == NULL) {
			struct connection_node * insert =  (connection_node *)malloc(sizeof(struct connection_node));
			insert->local_mac = local_mac;
			insert->local_ip = local_ip;
			insert->local_port = local_port;
			insert->remote_mac = remote_mac;
			insert->remote_ip = remote_ip;
			insert->remote_port = remote_port;
			insert->RTT = RTT;
			insert->ID = ID;
			//PUT THE TIMESTAMP HERE, PROBLY THE NEXT_HOP AS WELL
			HASH_ADD(lookup->connection, remote_mac, insert);
		}
		else if (ID > linkstate->ID) {
			linkstate->ID = ID;
			linkstate->RTT = RTT;
			//HAVE TO UPDATE THE TIMESTAMP SOMEHOW, WHEN EXACTLY ARE WE GETING IT?
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
