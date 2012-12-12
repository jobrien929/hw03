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

insert = malloc(node)
insert->local_mac = SOMETHING;
insert->connection = NULL;
HASH_ADD(ourhash, local_mac, insert)

local = HASH_GET(ourhash, local_mac);
local->connection == NULL { insert->remote_mac = SOMETHING, HASH_ADD(local->connection, remote_mac, insert }
HASH_GET(local->connection, key that we're looking for')
