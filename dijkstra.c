#include "cs352proxy.h"

typedef struct {
	char remote_mac[6];
	char prev_mac[6];
	uint16_t RTT;
	UT_hash_handle hh1;
	UT_hash_handle hh2;
} d_node;

int sort_function(void * a, void * b) {
	d_node * a1 = (d_node *)a;
	d_node * b1 = (d_node *)b;
	if (a1->RTT < b1->RTT) {
		return -1;
	}
	if (a1->RTT > b1->RTT) {
		return 1;
	}
	return 0;
}

int sort_function_d(void * a, void * b) {
	d_node * a1 = (d_node *)a;
	d_node * b1 = (d_node *)b;
	if (a1->RTT < b1->RTT) {
		return -1;
	}
	if (a1->RTT > b1->RTT) {
		return 1;
	}
	return 0;
}

void dijkstra() {
	d_node * distances = NULL;
	d_node * done = NULL;
	d_node * pointer;
	for (pointer = main_node->conncetions; pointer != NULL;, pointer = pointer->hh.next) {
		d_node * temp;
		HASH_FIND(hh1, done, &pointer->remote_mac, sizeof(char * 6), temp);
		if (temp == NULL) {
			d_node * insert = (d_node  *)malloc(sizeof(d_node));
			insert->remote_mac = temp->remote_mac;
			insert->prev_mac = NULL;
			insert->RTT = temp->RTT;
			HASH_ADD(hh1, distances, remote_mac, sizeof(char * 6), insert);
		}
	}
	HASH_SORT(distances, sort_function);
	while (distances != NULL) {
		pointer = distances;
		HASH_ADD(hh2, done, remote_mac, sizeof(char * 6), pointer);
		Forward * graph_pointer;
		HASH_FIND_STR(graph, &pointer->remote_mac, graph_pointer);
		if (pointer->prev_mac != NULL) {
			Member * tmppointer;
			HASH_FIND_STR(graph, &pointer->prev_mac, tmppointer);
			graph_pointer->fd = tmppointer->fd;
		}
		Member * connection_pointer;
		for (connection_pointer = graph_pointer->connections; connection_pointer != NULL; connection_pointer = connection_pointer->hh.next) {
			uint16_t newRTT = pointer->RTT + connection_pointer->RTT;
			d_node * temp;
			HASH_FIND(hh2, done, &graph_pointer->remote_mac, sizeof(char * 6), temp);
			if (temp != NULL) {
				continue;
			}
			HASH_FIND(hh1, distances, &connection_pointer->remote_mac, sizeof(char * 6), temp);
			if (temp != NULL) {
				if (newRTT < temp->RTT) {
					temp->RTT = newRTT;
					temp->prev_mac = pointer->local_mac;
				}
			}
			else {
				d_node * insert = (d_node *)malloc(sizeof(d_node));
				insert->remote_mac = connection_pointer->remote_mac;
				insert->prev_mac = pointer->local_mac;
				insert->RTT = newRTT;
				HASH_ADD(hh1, distances, remote_mac, sizeof(char * 6), insert);
			}
		}
		HASH_DEL(distances, pointer);
		HASH_SORT(distances, sort_function);
	}
	return;
}
				
				
				
