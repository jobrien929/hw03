#include "cs352proxy.h"
#include "processconfig.h"
#include "frames.h"



int running = 1;
pthread_mutex_t runLock = PTHREAD_MUTEX_INITIALIZER;

struct node *graph = NULL;
pthread_mutex_t graphLock = PTHREAD_MUTEX_INITIALIZER;



int
runcheck(void)
{
  int result;
  pthread_mutex_lock(&runLock);
  result = running;
  pthread_mutex_unlock(&runLock);
  return result;
}

int allocate_tunnel(char *dev, int flags) {
        int fd, error;
        struct ifreq ifr;
        char *device_name = "/dev/net/tun";
        if( (fd = open(device_name , O_RDWR)) < 0 ) {
                fprintf(stderr,"error opening /dev/net/tun\n%d: %s\n",errno,strerror(errno));
                return fd;
        }
        memset(&ifr, 0, sizeof(ifr));
        ifr.ifr_flags = flags;
        if (*dev) {
                strncpy(ifr.ifr_name, dev, IFNAMSIZ);
        }
        if( (error = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
                fprintf(stderr,"ioctl on tap failed\n%d: %s\n",errno,strerror(errno));
                close(fd);
                return error;
        }
        strcpy(dev, ifr.ifr_name);
        return fd;
}

void
getmac(char *dev, char *local_mac)
{
  char *buffer;
  sprintf(buffer,"/sys/class/net/%s/address",dev);
  FILE* f = fopen(buffer,"r");
  fread(buffer,1,MAX_DEV_LINE,f);
  sscanf(buffer,"%hhX:%hhX:%hhX:%hhX:%hhX:%hhX",local_mac,local_mac+1,local_mac+2,local_mac+3,local_mac+4,local_mac+5);
  fclose(f);
}

/* Reads exactly length bytes from fd into *buf. */
/* Does not return until length bytes have been read. */
int
readn(int fd, char *buf, int length) {
  int received;
  int received_total = 0;
  do {
    if((received = read(fd, buf+received_total, length - received_total)) < 0) {
      perror("readn");
      exit(EXIT_FAILURE);
    }
    if(received == 0 && received_total == 0)
      return 0;  // closed socket
    received_total += received;
  } while(received_total < length);

  return received_total;
}

/* Writes exactly length bytes from buf into fd. */
/* Does not return until length bytes have been written. */
int
writen(int fd, char *buf, int length) {
  int written;
  int written_total = 0;
  do {
    if((written = write(fd, buf+written_total, length - written_total)) < 0) {
      perror("readn");
      exit(EXIT_FAILURE);
    }
    if(written == 0 && written_total == 0)
      return 0;   // closed socket
    written_total += written;
  } while(written_total < length);

  return written_total;
}



int
linkstatesize(int num_peers)
{
  return TYPE_SIZE + LENGTH_SIZE + NUM_PEERS_SIZE +
    num_peers * (2 * (IP_SIZE + PORT_SIZE + MAC_SIZE)) +
    RTT_SIZE + ID_SIZE;
         
}

uint32_t
getnumip(char *addr)
{
  int d1, d2, d3, d4;
  sscanf(addr, "%d.%d.%d.%d",&d1, &d2, &d3, &d4);
  return (d1<<24) + (d2<<16) + (d3<<8) + d4;
}

int
make_connection(Peer *peerList)
{
  struct addrinfo hints, *res;
  Peer *elt, *tmp;
  int fd;
  char buf[HEADER_SIZE + PAYLOAD_SIZE];
  int offset;
  int len = HEADER_SIZE+NUM_PEERS_SIZE+IP_SIZE+PORT_SIZE+MAC_SIZE+ID_SIZE;

  LL_FOREACH_SAFE(peerList, elt, tmp)
    {

      // make connection
      memset(&hints, 0, sizeof hints);
      hints.ai_family = AF_INET; // Use IPv4
      hints.ai_socktype = SOCK_STREAM; //Use tcp

      if(getaddrinfo(elt->addr, elt->port, &hints, &res))
	{
	  ERROR("error: getaddrinfo\n", -1);
	}

      if((fd  = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
	{
	  ERROR("error: socket\n", -1);
	}

      if(connect(fd, res->ai_addr, res->ai_addrlen))
	{
	  continue;
	}
    
 
      // build packet
      *((uint16_t*)buf) = htons(LINKSTATE);
      offset = TYPE_SIZE;
      *((uint16_t*)(buf+offset)) = htons(len);
      offset += LENGTH_SIZE;
      *((uint16_t*)(buf+offset)) = htons(1);
      offset += NUM_PEERS_SIZE;
      *((uint32_t*)(buf+offset)) = htonl(host->local_ip);
      offset += IP_SIZE;
      *((uint16_t*)(buf+offset)) = htons(host->local_port);
      offset += PORT_SIZE;
      memcpy(buf+offset, host->local_mac, MAC_SIZE);
      offset += MAC_SIZE;
      *((unsigned long long*)(buf+offset)) = timestamp();

      // send the new packet on its way
      writen(fd, buf, len);

      addconnection(elt);

      // clean up
      freeaddrinfo(res);
      LL_DELETE(peerList, elt);
      free(elt->addr);
      free(elt->port);
      free(elt);
    }
  return 0;
}


int
getip(uint32_t *ip)
{
  struct ifaddrs *ifaddr, *ifa;
  if(getifaddrs(&ifaddr))
    {
      ERROR("error: getifaddr\n", -1);
    }
  for(ifa = ifaddr; ifa != NULL || !strcmp("eth0", ifa->ifa_name); ifa = ifa->ifa_next);
  if(ifa == NULL)
    {
      ERROR("error: interface not found\n", 0);
    }
  char ip_addr[NI_MAXHOST];
  if(getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), ip_addr, NI_MAXHOST, NULL, 0, NI_NUMERICHOST))
    {
      ERROR("error: getnameinfo\n", -1);
    }

  return getnumip(ip_addr);
}

int
listener(void)
{
  struct addrinfo hints, *res;
  int fd;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; // Use IPv4
  hints.ai_socktype = SOCK_STREAM; //Use tcp
  hints.ai_flags = AI_PASSIVE;

  if(getaddrinfo(NULL, listenPort, &hints, &res))
    {
      ERROR("error: listener getaddrinfo\n", -1); 
    }

  if((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
      ERROR("error:listener socket\n", -1);
    }

  if(bind(fd, res->ai_addr, res->ai_addrlen))
    {
      ERROR("error: lisenter bind\n", -1);
    }
  if(listen(fd, 10))
    {
      ERROR("error: listener listen\n", -1);
    }

  return fd;
}


int main(int argc, char **argv)
{
  
  // Check arguments
  char *configfile;
  switch(argc)
    {
    case 1:
      configfile = "config";
      break;
    case 2:
      configfile = argv[1];
      break;
    default:
      printf("Usage: cs352proxy [config_file]\n");
      exit(EXIT_FAILURE);
    }

  Peer *peerList = NULL;
  // Read config file
  if (processconfig(configfile, peerList) < 0)
    exit(EXIT_FAILURE);

  // Make node for this proxy
  char *dev;
  if(!(host = malloc(sizeof(node))))
    {
      ERROR("Malloc error making node\n", EXIT_FAILURE);
    }
  if(if_name == NULL)
    {
      dev = if_name;
    } else
    {
      dev = "eth0";
    }
  getmac(dev, host->local_mac);
  if(getip(&(host->local_ip)))
    {
      ERROR("error: getip\n", EXIT_FAILURE);
    }
  host->local_port = (uint16_t)atol(listenPort);
  host->connections = NULL;
  host->num_connections = 0;
  host->neighbor = 0;
  pthread_mutex_lock(&graphLock);
  HASH_ADD_STR(graph, local_mac, host);
  pthread_mutex_unlock(&graphLock);

  // Open tap
  int tap_fd, tcp_fd;
  if(if_name != NULL){
    if((tap_fd = allocate_tunnel(if_name, IFF_TAP | IFF_NO_PI)) < 0)
      {
	ERROR("Can't open TAP\n", EXIT_FAILURE);
      }
  }


  // Make initial connections
  if((make_connection(peerList)) < 0)
    exit(EXIT_FAILURE);

  // Make listening port
  if((tcp_fd = listener()) < 0)
    {
      ERROR("error: opening listening port\n", EXIT_FAILURE);
    }

  //  TODO set up maintenence threads here

  // Main loop
  fd_set rd;
  int max, r;
  node *temp;
  struct timeval timeout;
  while(runcheck())
    {
      timeout.tv_sec = 3; // check for quit every 3 seconds | REPLACE WITH PIPE
      FD_ZERO(&rd);
      if(if_name != NULL)
	{
	  FD_SET(tap_fd, &rd);
	}
      FD_SET(tcp_fd, &rd);
      max = MAX(tap_fd, tcp_fd);
      pthread_mutex_lock(&graphLock);
      for(temp = graph; temp != NULL; temp = temp->hh.next)
	{
	  FD_SET(temp->fd, &rd);
	  max = MAX(max, temp->fd);
	}
      pthread_mutex_unlock(&graphLock);
      r = select(max+1, &rd, NULL, NULL, &timeout);

      if(r == -1 && errno == EINTR)
	continue;
      if(r == -1)
	{
	  ERROR("error: select\n", EXIT_FAILURE);
	}
      if(FD_ISSET(tap_fd, &rd))
	outgoing();
      if(FD_ISSET(tcp_fd, &rd))
	newconnect();

      pthread_mutex_lock(&graphLock);
      for(temp = graph; temp != NULL; temp = temp->hh.next)
	{
	  if(FD_ISSET(temp->fd, &rd))
	    processFrame(temp);
	}
      pthread_mutex_unlock(&graphLock);

    }

  exit(EXIT_SUCCESS);
}
