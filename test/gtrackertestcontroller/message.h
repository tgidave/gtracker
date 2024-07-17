/*                                                                              
 *  message.h                                                  
 *                                                                               
 *  Header file for implementing the message system in the icedrifter.                     
 */ 
 
#ifndef _MESSAGE_H
#define _MESSAGE_H

#define MSG_RX 3
#define MSG_TX 2

#define MAX_TRIES 500

int processMessage(uint8_t *messageBuffPtr);

#endif // _MESSAGE_H
