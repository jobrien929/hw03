#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "encryption.h"

//NOTE: For this class I used aes256.c which was developed by 
//Ilya O. Levin on http://www.literatecode.com
//Her copyright statement, as well as rules for citation for 
//the Rutgers CS352 project allows the use of her aes256.c class
//in this project.

#define DUMP(s, i, buf, sz)  {printf(s);                   \
                              for (i = 0; i < (sz);i++)    \
                                  printf("%02x ", buf[i]); \
                              printf("\n");}

/**convertKey takes a string version of a key and converts it into a 
 * key that is the proper length 
 * key - string (MUST BE STRING) version of the key you want to create
 * RETURN - key in the proper format (must be freed using keyDestroy or
 *  you will have memory leaks)
 * 
 * NOTE: It is HIGHLYrecomended key be a string greater than 31
 *  characters, though it can accept smaller**/
char * convertKey(char * key){
	//initializes return variable
	char * ret = (char *)malloc(sizeof(char)*32);
	memset(ret,0x00,32);
	
	//Checks length of key and intializes ret accordingly
	if(strlen(key)<=31){
		int i = 0;
		while(i<32){
			if(32-i>=strlen(key)+1){
				memcpy(&(ret[i]), key, strlen(key)+1);
			}else{
				memcpy(&(ret[i]),key, 32-i);
			}
			i= i + strlen(key) + 1;
		}

	}else{
		memcpy(ret, key, 32);
	}
	return ret;
}

/**keyDestroy destroys an intialized key, key MUST BE INITIALIZED USING
 * convertKey FIRST or else program will result in UNDEFINED BEHAVIOR!
 * YOU HAVE BEEN WARNED!!!
 * key - key your trying to free**/
void keyDestroy (char * key){
	free(key);
}


/**encrypt - encrypts the message, msg, of length, length, using key the
 * 	encrypted message replaces message
 * key - key used to lock and unlock the encryption (MUST BE FIRST 
 *    INITIALIZED USING convertKey)
 * msg - the item you wish to encrypt
 * length - size of the message you wish to encrypt in bytes**/
void fencrypt(char *key, char *msg, int length){
	
	aes256_context ctx; 
	int loopCounter;
	
	//initializes all variables required for encryption
	aes256_init(&ctx, key);
	
	//finds right length to encrypt message
	char *ret;
	
	//initialize return variable
	ret = (char *)malloc(sizeof(char)*(length));
	memset(ret, 0x00, length);
	
	//copies msg into string
	memcpy(ret, msg, length);
	
	//encrypts message
	for(loopCounter = 0; loopCounter < length - 16; loopCounter=loopCounter+16){
		aes256_encrypt_ecb(&ctx, &(ret[loopCounter]));
	}
	for(;loopCounter<length; loopCounter ++){
		ret[loopCounter] = key[loopCounter%16] ^ ret[loopCounter];
	}
	
	memcpy(msg,ret,length);
	
	//Clean up
	free(ret);
}

/**fdecrypt - decrypts the message, cipher, of length, length, using key the
 * 	encrypted message replaces message
 * key - key used to lock and unlock the encryption (MUST BE FIRST 
 *    INITIALIZED USING convertKey)
 * cipher - the item you wish to decrypt
 * length - size of the message you wish to decrypt in bytes**/
void fdecrypt(char *key, char *cipher, int length){
	aes256_context ctx; 
	int loopCounter;
	
	//initializes all variables required for decryption
	aes256_init(&ctx, key);
	
	//finds right length to encrypt message
	char *ret;
	
	//initialize return variable
	ret = (char *)malloc(sizeof(char)*(length));
	memset(ret, 0x00, length);
	
	//copies msg into string
	memcpy(ret, cipher, length);
	
	//encrypts message
	for(loopCounter = 0; loopCounter < length - 16; loopCounter=loopCounter+16){
		aes256_decrypt_ecb(&ctx, &(ret[loopCounter]));
	}
	for(;loopCounter<length; loopCounter ++){
		 ret[loopCounter] = key[loopCounter%16] ^ ret[loopCounter];
	}
	
	memcpy(cipher,ret,length);

	//Clean up
	free(ret);
}



