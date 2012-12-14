#include "linkstateManage.h"


/**removeExpired uses the global variable "linkTimeOUt" to find all 
 *    expired linksStates in the "node_list" and removes them
 * returns nothing for now**/
void removeExpired() {

	if(node_list == NULL){
		return;
	}

	
	struct node *ptr, *tmp;
	//Counts the size of node_list
	int node_listSize=0;
	HASH_ITER(hh, node_list, ptr, tmp){
		node_listSize++;
	}
	
	//initializes array that determins what to delete
	int toDel[node_listSize];
	int loopCounter;
	for (loopCounter = 0; loopCounter < node_listSize; loopCounter++){
		toDel[loopCounter] = 0;
	}
						
	//Determines what needs to be deleted
	struct connection_node *ptr2, *tmp2;
	HASH_ITER(hh, node_list, ptr, tmp){
		if(ptr->connection != NULL){
			HASH_ITER(hh, ptr->connection, ptr2, tmp2){
				struct node *ctr;
				int position = 0;
				for(ctr=node_list; ctr!=NULL; ctr = ctr->hh.next){
					if (strcmp(ctr->local_mac, ptr2->remote_mac)==0){
						break;
					}
					position++;
				}
				if(ptr2->timestamp+linkTimeout <= time(NULL)){
					printf("position: %d\n", position);
					toDel[position] = 1;
					HASH_DEL(ptr->connection, ptr2);
					free(ptr2);
				}else{
					toDel[position] = 0;
				}
			}
		}

	}

	//Deletes
	struct node *ctr;
	int position = 0;
	for(ctr=node_list; ctr!=NULL; ctr = ctr->hh.next){
		if (toDel[position] == 1){
			struct connection_node *ptr3, *tmp3;
			HASH_ITER(hh, ctr->connection, ptr3, tmp3){
				HASH_DEL(ctr->connection, ptr3);
				free(ptr3);
			}
			HASH_DEL(node_list,ctr);
			free(ctr);
			
		}
		position++;
	}
	
	
	return;
}
