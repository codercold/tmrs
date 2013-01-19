/*****************************************************************************
*                                                                           *
* Tiger Mapping and Routing Server  (TMRS)                                  *
*                                                                           *
* Copyright (C) 2003 Sumit Birla <sbirla@users.sourceforge.net>             *
*                                                                           *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License for more details.                              *
*                                                                           *
* You should have received a copy of the GNU General Public License         *
* along with this program; if not, write to the Free Software               *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA  *
*                                                                           *
*****************************************************************************/


#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include "tmrs.h"

// temporary to avoid passing a struct to gdSink
int temp_socket;


static int socketSink(void *context, char *buffer, int len)
{
    int bytes;

    bytes = send(temp_socket, buffer, len, 0);
    //printf("server: Sending %d bytes / Sent %d bytes of data\n", len, bytes);

    return bytes;
}



/** 
* This method start listening for connection and serving requests as they are
* received.
*/
void server_start()
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_in my_addr;    // my address information
    struct sockaddr_in their_addr; // connector's address information
    int sin_size, bytes_received, i;
    char buffer[128];
    gdSink mySink;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;			// host byte order
    my_addr.sin_port = htons(9099);			// short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY;	// automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8);	// zero the rest of the struct

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, 1) == -1) {
        perror("listen");
        exit(1);
    }

    // main server loop
    while (1) 
    {  
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));

        // receive request parameters
        bytes_received = 0;
        while (bytes_received < sizeof(buffer)) 
        {
            i = recv(new_fd, &buffer[bytes_received], sizeof(buffer)-bytes_received, 0);
            // close the socket if an error occurs
            if (i < 0) {
                close(new_fd);
                break;
            }
            bytes_received += i;

            // check if we reached the end of the request (requests are terminated
            // with a NEWLINE character
            if (buffer[bytes_received-1] == '\n') {
                buffer[bytes_received-1] = 0;
                break;
            }
        }

        printf("server: received %d bytes\n", bytes_received);

        // setup sink information
        temp_socket = new_fd;
        mySink.context = NULL;
        mySink.sink = socketSink;

        // call the appropriate handler
        switch (buffer[0]) 
        {
        case 'M':
            handle_draw_map(&buffer[2], &mySink);
            break;

        case 'A':
            handle_find_address(&buffer[2], &mySink);
            break;

        default:
            send(new_fd, "Command not understood\n", 23, 0);
            break;
        }

        close(new_fd);
    }

    close(sockfd);
}


/** 
* This method start listening for connection using UNIX stream in order to 
* avoid overhead of TCP networking.
*/
void server_start_unix()
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_un my_addr; // my address information
    struct sockaddr_un their_addr;  // connector's address information
    int sun_size, bytes_received, i;
    char buffer[128];
    gdSink mySink;

    unlink("/var/tmrs_socket");
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    my_addr.sun_family = AF_UNIX;
    strcpy (my_addr.sun_path, "/var/tmrs_socket");
    
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, 1) == -1) {
        perror("listen");
        exit(1);
    }

    // main server loop
    while (1) 
    {  
        sun_size = sizeof(struct sockaddr_un);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sun_size)) == -1) {
            perror("accept");
            continue;
        }
        printf("server: got connection\n");

        // receive request parameters
        bytes_received = 0;
        while (bytes_received < sizeof(buffer)) 
        {
            i = recv(new_fd, &buffer[bytes_received], sizeof(buffer)-bytes_received, 0);
            // close the socket if an error occurs
            if (i < 0) {
                close(new_fd);
                break;
            }
            bytes_received += i;

            // check if we reached the end of the request (requests are terminated
            // with a NEWLINE character
            if (buffer[bytes_received-1] == '\n') {
                buffer[bytes_received-1] = 0;
                break;
            }
        }

        printf("server: received %d bytes\n", bytes_received);

        // setup sink information
        temp_socket = new_fd;
        mySink.context = NULL;
        mySink.sink = socketSink;

        // call the appropriate handler
        switch (buffer[0]) 
        {
        case 'M':
            handle_draw_map(&buffer[2], &mySink);
            break;

        case 'A':
            handle_find_address(&buffer[2], &mySink);
            break;

        default:
            send(new_fd, "Command not understood\n", 23, 0);
            break;
        }

        close(new_fd);
    }

    close(sockfd);
}
