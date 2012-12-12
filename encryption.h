#include <stdlib.h>
#include <stdio.h>
#include "aes256.h"
#include <string.h>
#include <time.h>

/**convertKey takes a string version of a key and converts it into a 
 * key that is the proper length 
 * key - string (MUST BE STRING) version of the key you want to create
 * RETURN - key in the proper format (must be freed using keyDestroy or
 *  you will have memory leaks)
 * 
 * NOTE: It is HIGHLYrecomended key be a string greater than 31
 *  characters, though it can accept smaller**/
char * convertKey(char * key);

/**keyDestroy destroys an intialized key, key MUST BE INITIALIZED USING
 * convertKey FIRST or else program will result in UNDEFINED BEHAVIOR!
 * YOU HAVE BEEN WARNED!!!
 * key - key your trying to free**/
void keyDestroy (char * key);


/**encrypt - encrypts the message, msg, of length, length, using key the
 * 	encrypted message replaces message
 * key - key used to lock and unlock the encryption (MUST BE FIRST 
 *    INITIALIZED USING convertKey)
 * msg - the item you wish to encrypt
 * length - size of the message you wish to encrypt in bytes**/
void fencrypt(char *key, char *msg, int length);


/**fdecrypt - decrypts the message, cipher, of length, length, using key the
 * 	encrypted message replaces message
 * key - key used to lock and unlock the encryption (MUST BE FIRST 
 *    INITIALIZED USING convertKey)
 * cipher - the item you wish to decrypt
 * length - size of the message you wish to decrypt in bytes**/
void fdecrypt(char *key, char *cipher, int length);
