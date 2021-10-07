/* 
 * SPLPv1.c
 * The file is part of practical task for System programming course. 
 * This file contains validation of SPLPv1 protocol. 
 */


//#error Specify your name and group
/*
  Gordey Ilya, 14 group
*/



/*
---------------------------------------------------------------------------------------------------------------------------
# |      STATE      |         DESCRIPTION       |           ALLOWED MESSAGES            | NEW STATE | EXAMPLE
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
1 | INIT            | initial state             | A->B     CONNECT                      |     2     |
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
2 | CONNECTING      | client is waiting for con-| A<-B     CONNECT_OK                   |     3     |
  |                 | nection approval from srv |                                       |           |                      
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
3 | CONNECTED       | Connection is established | A->B     GET_VER                      |     4     |                     
  |                 |                           |        -------------------------------+-----------+----------------------
  |                 |                           |          One of the following:        |     5     |                      
  |                 |                           |          - GET_DATA                   |           |                      
  |                 |                           |          - GET_FILE                   |           |                      
  |                 |                           |          - GET_COMMAND                |           |
  |                 |                           |        -------------------------------+-----------+----------------------
  |                 |                           |          GET_B64                      |     6     |                      
  |                 |                           |        ------------------------------------------------------------------
  |                 |                           |          DISCONNECT                   |     7     |                                 
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
4 | WAITING_VER     | Client is waiting for     | A<-B     VERSION ver                  |     3     | VERSION 2                     
  |                 | server to provide version |          Where ver is an integer (>0) |           |                      
  |                 | information               |          value. Only a single space   |           |                      
  |                 |                           |          is allowed in the message    |           |                      
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
5 | WAITING_DATA    | Client is waiting for a   | A<-B     CMD data CMD                 |     3     | GET_DATA a GET_DATA 
  |                 | response from server      |                                       |           |                      
  |                 |                           |          CMD - command sent by the    |           |                      
  |                 |                           |           client in previous message  |           |                      
  |                 |                           |          data - string which contains |           |                      
  |                 |                           |           the following allowed cha-  |           |                      
  |                 |                           |           racters: small latin letter,|           |                     
  |                 |                           |           digits and '.'              |           |                      
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
6 | WAITING_B64_DATA| Client is waiting for a   | A<-B     B64: data                    |     3     | B64: SGVsbG8=                    
  |                 | response from server.     |          where data is a base64 string|           |                      
  |                 |                           |          only 1 space is allowed      |           |                      
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
7 | DISCONNECTING   | Client is waiting for     | A<-B     DISCONNECT_OK                |     1     |                      
  |                 | server to close the       |                                       |           |                      
  |                 | connection                |                                       |           |                      
---------------------------------------------------------------------------------------------------------------------------

IN CASE OF INVALID MESSAGE THE STATE SHOULD BE RESET TO 1 (INIT)

*/


#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "splpv1.h"





/* FUNCTION:  validate_message
 * 
 * PURPOSE:  
 *    This function is called for each SPLPv1 message between client 
 *    and server
 * 
 * PARAMETERS:
 *    msg - pointer to a structure which stores information about 
 *    message
 * 
 * RETURN VALUE:
 *    MESSAGE_VALID if the message is correct 
 *    MESSAGE_INVALID if the message is incorrect or out of protocol 
 *    state
 */

const char* commands[] = { "GET_DATA", "GET_FILE", "GET_COMMAND" };
char current_state = 1;
char command_type;

enum test_status validate_message(struct Message *msg) {
    char* message = msg->text_message;
    if (msg->direction == A_TO_B) {
        if (current_state == 1 && !strcmp(message, "CONNECT")) {
            current_state = 2;
            return MESSAGE_VALID;
        }
        else if (current_state == 3) {
            if (!strcmp(message, "GET_VER")) {
                current_state = 4;
                return MESSAGE_VALID;
            }
            else if (!strcmp(message, "GET_DATA")) {
                command_type = 0;
                current_state = 5;
                return MESSAGE_VALID;
            }
            else if (!strcmp(message, "GET_FILE")) {
                command_type = 1;
                current_state = 5;
                return MESSAGE_VALID;
            }
            else if (!strcmp(message, "GET_COMMAND")) {
                command_type = 2;
                current_state = 5;
                return MESSAGE_VALID;
            }
            else if (!strcmp(message, "GET_B64")) {
                current_state = 6;
                return MESSAGE_VALID;
            }
            else if (!strcmp(message, "DISCONNECT")) {
                current_state = 7;
                return MESSAGE_VALID;
            }
        }
    }
    else {
        if (current_state == 2 && !strcmp(message, "CONNECT_OK")) {
            current_state = 3;
            return MESSAGE_VALID;
        }
        else if (current_state == 7 && !strcmp(message, "DISCONNECT_OK")) {
            current_state = 1;
            return MESSAGE_VALID;
        }
        else if (current_state == 4 && !strncmp(message, "VERSION ", 8)) {
            message += 8;
            if (*message > '0' && *message < '9') {
                for (++message; *message != '\0'; ++message) {
                    if (*message < '0' || *message > '8') {
                        current_state = 1;
                        return MESSAGE_INVALID;
                    }
                }
                current_state = 3;
                return MESSAGE_VALID;
            }
        }
        else if (current_state == 5) {
            char len = strlen(commands[command_type]);
            if (!strncmp(message, commands[command_type], len))
            {
                message += len;
                if (*message == ' ')
                {
                    message++;
                    while (
                            (('0' <= *message) && (*message <= '9')) ||
                            (('a' <= *message) && (*message <= 'z')) ||
                            (*message == '.')
                          ) message++;
                    char *s = (*message == ' ') ? message + 1 : NULL;
                    if (s && !strcmp(s, commands[command_type])) {
                        current_state = 3;
                        return MESSAGE_VALID;
                    }
                }
            }
        }
        else if (current_state == 6 && !strncmp(message, "B64: ", 5)) {
            message += 5;
            char* begin = message;
            while (
                    (('0' <= *message) && (*message <= '9')) ||
                    (('a' <= *message) && (*message <= 'z')) ||
                    (('A' <= *message) && (*message <= 'Z')) ||
                    (*message == '+') || (*message == '/')
                  ) message++;
            char check = 0;
            for (; (check < 2) && (message[check] == '='); ++check);
            if ((message - begin + check) % 4 == 0 && !message[check]) {
                current_state = 3;
                return MESSAGE_VALID;
            }

        }
    }
    current_state = 1;
    return MESSAGE_INVALID;
}