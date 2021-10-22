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
const bool valid_digits[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
const bool valid_base64[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };
const bool valid_waiting_data[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };

char current_state = 1;
char command_type;
const char commands_size[] = { 8, 8, 11 };
const char* commands[] = { "GET_DATA", "GET_FILE", "GET_COMMAND" };

enum test_status connected_check(struct Message* msg)
{
    char* message = msg->text_message;

    if (!memcmp("GET_VER", message, 8))
    {
        current_state = 4;
        return MESSAGE_VALID;
    }
    if (!memcmp("GET_DATA", message, 9))
    {
        command_type = 0;
        current_state = 5;
        return MESSAGE_VALID;
    }
    if (!memcmp("GET_FILE", message, 9))
    {
        command_type = 1;
        current_state = 5;
        return MESSAGE_VALID;
    }
    if (!memcmp("GET_COMMAND", message, 12))
    {
        command_type = 2;
        current_state = 5;
        return MESSAGE_VALID;
    }
    if (!memcmp("GET_B64", message, 8))
    {
        current_state = 6;
        return MESSAGE_VALID;
    }
    if (!memcmp("DISCONNECT", message, 11))
    {
        current_state = 7;
        return MESSAGE_VALID;
    }
    current_state = 1;
    return MESSAGE_INVALID;
}

enum test_status version_check(struct Message* msg) {
    char* message = msg->text_message + 8;
    for (; valid_digits[*message]; message++);
    if (*message != '\0')
    {
        current_state = 1;
        return MESSAGE_INVALID;
    }
    current_state = 3;
    return MESSAGE_VALID;
}

enum test_status waiting_data_check(struct Message* msg) {
    char* message = msg->text_message;
    if (strncmp(commands[command_type], message, commands_size[command_type])) {
        current_state = 1;
        return MESSAGE_INVALID;
    }
    message += commands_size[command_type];

    if (*message != ' ') {
        current_state = 1;
        return MESSAGE_INVALID;
    }
    message++;

    for (; valid_waiting_data[*message]; message++);
    if (*message != ' ') {
        current_state = 1;
        return MESSAGE_INVALID;
    }
    message++;

    if (strncmp(commands[command_type], message, commands_size[command_type] + 1)) {
        current_state = 1;
        return MESSAGE_INVALID;
    }

    current_state = 3;
    return MESSAGE_VALID;
}

enum test_status waiting_base64_check(struct Message* msg) {
    char* message = msg->text_message + 5;
    int i = 0;
    char* start = message;

    for (; valid_base64[*message]; message++);

    i = message - start;

    if (*message == '\0')
    {
        if (i % 4 != 0)
        {
            current_state = 1;
            return MESSAGE_INVALID;
        }
        current_state = 3;
        return MESSAGE_VALID;
    }

    if (*message == 61 && *(message + 1) == '\0')
        i++;
    else if (*message == 61 && *(message + 1) == 61 && *(message + 2) == '\0')
        i += 2;
    else
    {
        current_state = 1;
        return MESSAGE_INVALID;
    }

    if (i % 4 != 0)
    {
        current_state = 1;
        return MESSAGE_INVALID;
    }

    current_state = 3;
    return MESSAGE_VALID;
}
enum test_status validate_message(struct Message* msg) {
    char* message = msg->text_message;
    if (msg->direction == A_TO_B) {
        if (current_state == 1 && !strcmp(message, "CONNECT")) {
            current_state = 2;
            return MESSAGE_VALID;
        }
        else if (current_state == 3) {
            return connected_check(msg);
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
            return version_check(msg);
        }
        else if (current_state == 5) {
            return waiting_data_check(msg);
        }
        else if (current_state == 6 && !strncmp(message, "B64: ", 5)) {
            return waiting_base64_check(msg);
        }
    }
    current_state = 1;
    return MESSAGE_INVALID;
}
