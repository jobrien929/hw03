#ifndef CS352PROXY_H
#define CS352PROXY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <net/if.h>
#include <arpa/inet.h> 
#include <linux/if_tun.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <net/ethernet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <time.h>
#include "utlist.h"
#include "uthash.h"


#define MAX_LINE_LENGTH 256
#define MAX_DEV_LINE    256
#define PAYLOAD_SIZE    2044
#define TYPE_SIZE       2
#define LENGTH_SIZE     2
#define NUM_PEERS_SIZE  2
#define IP_SIZE         4
#define PORT_SIZE       2
#define MAC_SIZE        6
#define RTT_SIZE        4
#define ID_SIZE         8
#define TIME_SIZE       4
#define HEADER_SIZE     (TYPE_SIZE + LENGTH_SIZE)

// Packet types
#define DATA            0xABCD
#define LEAVE           0xAB01
#define QUIT            0xAB12
#define LINKSTATE       0xABAC
#define RTTPRBRQST      0xAB34
#define RTTPRBRESP      0xAB35

#define DEBUG(X)     \
  if(debug)          \
    printf(X)

#define ERROR(X, Y)  \
  printf(X);   \
  return (Y)

#define MAX(A, B)   \
  (((A) > (B)) ? (A) : (B))

int debug;
char *listenPort;
int linkTimeout;
int quitAfter;
char *if_name;

typedef struct
{
  char mac[ETHER_ADDR_LEN];
  char ipaddr[NI_MAXHOST];
} Localinfo;

typedef struct peer 
{
  char *addr;
  char *port;
  struct peer *next;
} Peer;



struct connection_node {
	int local_mac;
	int local_ip;
	int local_port;
	int remote_mac;
	int remote_ip;
	int remote_port;
	int RTT;
	int timestamp;
	int next_hop;
	UT_hash_handle hh;
};

struct node {
	int local_mac;
	struct connection_node * connection; //make sure to not search an empty hash table
	UT_hash_handle hh;
};
extern int running;
extern pthread_mutex_t runLock;
extern Forward *forwardTable;
extern pthread_mutex_t forwardLock;
extern Member *memberTable;
extern pthread_mutex_t memberLock;  // kinda sounds like something you'd need ointment to correct

int readn(int fd, char *buf, int length);
int writen(int fd, char *buf, int length);

#endif /* CS352PROXY_H */
