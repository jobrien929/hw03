#include "processconfig.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int
processconfig(char* filename, Peer *head)
{

  FILE *fd;
  char line[MAX_LINE_LENGTH + 1];
  char *token;
  Peer *client;

  if((fd = fopen(filename, "r")) == NULL)
    {
      printf("Failed to open config file.\n");
      return -1;
    }

  while(fgets(line, MAX_LINE_LENGTH, fd) != NULL)
    {
      token = strtok(line, DELIMS);
      while(token != NULL && token[0] != '/' && token[1] != '/')
	{
	  if(!strcmp(token, "debug"))
	    {
	      debug = atol(strtok(NULL, DELIMS));
	    }
	  else if(!strcmp(token, "listenPort"))
	    {
	      if(!(listenPort = strdup(strtok(NULL, DELIMS))))
		{
		  ERROR("strdup error: listenPort\n", -1);
		}
	    }
	  else if(!strcmp(token, "linkTimeout"))
	    {
	      linkTimeout = atol(strtok(NULL, DELIMS));
	    }
	  else if(!strcmp(token, "linkPeriod"))
	    {
	      linkTimeout = atol(strtok(NULL, DELIMS));
	    }
	  else if(!strcmp(token, "quitAfter"))
	    {
	      quitAfter = atol(strtok(NULL, DELIMS));
	    }
	  else if(!strcmp(token, "tapDevice"))
	    {
	      if(!(if_name = strdup(strtok(NULL, DELIMS))))
		{
		  ERROR("strdup error: if_name\n", -1);
		}
	    }
	  else if(!strcmp(token, "peer"))
	    {
	      if(!(client = malloc(sizeof(*client))))
		{
		  ERROR("malloc error: client\n", -1);
		}
	      if(!(client->addr = strdup(strtok(NULL, DELIMS))))
		{
		  ERROR("strdup error: client->addr\n", -1);
		}
	      if(!(client->port = strdup(strtok(NULL, DELIMS))))
		{
		  ERROR("strdup error: client->port\n", -1);
		}
	      LL_PREPEND(head, client);
	    }
	    else if(!strcmp(token,"key"))
	    {
			memset(&(key[0]), 0x00, 32);
			char *temp = strtok(NULL, DELIMS);
			int size;
			if((size = strlen(temp)) < 31){
				memcpy(&(key[0]), temp, size);
			}else{
				memcpy(&(key[0]), temp, 31);
			}
		} else
	    {
	      ERROR("Bad config file\n", -1);
	    }
	  token = strtok(NULL, DELIMS);
	}
    }
  fclose(fd);
  return 0;
}
