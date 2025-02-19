/****************************************************************************
 *
 *   Copyright (c) 2018 MAVlink Development Team. All rights reserved.
 *   Author: Hannes Diethelm, <hannes.diethelm@gmail.com>
 *           Trent Lukaczyk, <aerialhedgehog@gmail.com>
 *           Jaycee Lock,    <jaycee.lock@gmail.com>
 *           Lorenz Meier,   <lm@inf.ethz.ch>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file udp_port.cpp
 *
 * @brief UDP interface functions
 *
 * Functions for opening, closing, reading and writing via UDP ports
 *
 * @author Hannes Diethelm, <hannes.diethelm@gmail.com>
 * @author Trent Lukaczyk, <aerialhedgehog@gmail.com>
 * @author Jaycee Lock,    <jaycee.lock@gmail.com>
 * @author Lorenz Meier,   <lm@inf.ethz.ch>
 *
 */


// ------------------------------------------------------------------------------
//   Includes
// ------------------------------------------------------------------------------

#include "udp_port.h"

#define UDP_SERVER  0
#define UDP_CLIENT  1
#define UDP_MODE    UDP_CLIENT
// ----------------------------------------------------------------------------------
//   UDP Port Manager Class
// ----------------------------------------------------------------------------------

// ------------------------------------------------------------------------------
//   Con/De structors
// ------------------------------------------------------------------------------
UDP_Port::
UDP_Port(const char *target_ip_, int udp_port_)
{
    initialize_defaults();
    target_ip = target_ip_;
    rx_port  = udp_port_;
#if (UDP_MODE == UDP_CLIENT)
    tx_port = rx_port;
#endif
    is_open = false;
}

UDP_Port::
UDP_Port()
{
    initialize_defaults();
}

UDP_Port::
~UDP_Port()
{
    // destroy mutex
    pthread_mutex_destroy(&lock);
}

void
UDP_Port::
initialize_defaults()
{
    // Initialize attributes
    target_ip = "127.0.0.1";
    rx_port  = 14550;
    
    tx_port  = -1;

    is_open = false;
    debug = false;
    sock = -1;

    // Start mutex
    int result = pthread_mutex_init(&lock, NULL);
    if ( result != 0 )
    {
        printf("\n mutex init failed\n");
        throw 1;
    }
}


// ------------------------------------------------------------------------------
//   Read from UDP
// ------------------------------------------------------------------------------
int
UDP_Port::
read_message(std::queue<mavlink_message_t> &read_msg_q)
{
    uint8_t          cp;
    mavlink_status_t status;
    uint8_t          msgReceived = 0;

    // --------------------------------------------------------------------------
    //   READ FROM PORT
    // --------------------------------------------------------------------------

    // this function locks the port during read
    int result = _read_port(cp);

    // --------------------------------------------------------------------------
    //   PARSE MESSAGE
    // --------------------------------------------------------------------------
    mavlink_message_t buf_message;
    if (result > 0){
        
        for(int i=0; i<result; i++)
        {
            // the parsing
            // msgReceived = mavlink_parse_char(mav_channel, cp, &buf_message, &status);
            uint8_t result = mavlink_parse_char(mav_channel, buff[i], &buf_message, &status);

            if(result){
                read_msg_q.push(buf_message);
            }

        }
    }
    // Couldn't read from port
    else
    {
        fprintf(stderr, "ERROR: Could not read, res = %d, errno = %d : %m\n", result, errno);
    }

    // Done!
    
    if(read_msg_q.empty()) msgReceived = 0;
    else msgReceived = 1;

    return msgReceived;
}
// ------------------------------------------------------------------------------
//   Write to UDP
// ------------------------------------------------------------------------------
int
UDP_Port::
write_message(const mavlink_message_t &message)
{
    char buf[300];

    // Translate message to buffer
    unsigned len = mavlink_msg_to_send_buffer((uint8_t*)buf, &message);

    // Write buffer to UDP port, locks port while writing
    int bytesWritten = _write_port(buf,len);
    if(bytesWritten < 0){
        fprintf(stderr, "ERROR: Could not write, res = %d, errno = %d : %m\n", bytesWritten, errno);
    }

    return bytesWritten;
}


// ------------------------------------------------------------------------------
//   Open UDP Port
// ------------------------------------------------------------------------------
/**
 * throws EXIT_FAILURE if could not open the port
 */
void
UDP_Port::
start()
{
    // --------------------------------------------------------------------------
    //   OPEN PORT
    // --------------------------------------------------------------------------

    /* Create socket */
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        perror("error socket failed");
        throw EXIT_FAILURE;
    }

#if (UDP_MODE == UDP_SERVER)
    printf("\x1b[32m %s : %d \t Set UDP_Port as UDP_SERVER\r\n\x1b[0m",__func__,__LINE__);
    /* Bind the socket to rx_port - necessary to receive packets */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    // addr.sin_addr.s_addr = inet_addr(target_ip);
    // addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_addr.s_addr = (INADDR_ANY);
    
    addr.sin_port = htons(rx_port);

    if (bind(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr)))
    {
        perror("error bind failed");
        close(sock);
        sock = -1;
        throw EXIT_FAILURE;
    }
    // --------------------------------------------------------------------------
    //   CONNECTED!
    // --------------------------------------------------------------------------
    // printf("Listening to %s:%i\n", target_ip, rx_port);
    lastStatus.packet_rx_drop_count = 0;
#else
    printf("\x1b[32m %s : %d \t Set UDP_Port as UDP_CLIENT\r\n\x1b[0m",__func__,__LINE__);
#endif

    is_open = true;

    printf("\n");

    return;

}


// ------------------------------------------------------------------------------
//   Close UDP Port
// ------------------------------------------------------------------------------
void
UDP_Port::
stop()
{
    printf("CLOSE PORT\n");

    int result = close(sock);
    sock = -1;

    if ( result )
    {
        fprintf(stderr,"WARNING: Error on port close (%i)\n", result );
    }

    is_open = false;

    printf("\n");

}

void UDP_Port::set_mavlink_channel(int _ch){
    mav_channel = _ch;
}
int UDP_Port::get_mavlink_channel(){
    return mav_channel;
}
void UDP_Port::set_mavlink_version(int _ver){
    mav_ver = _ver;
    mavlink_set_proto_version(mav_channel, mav_ver);
}
int UDP_Port::get_mavlink_version(){
    return mav_ver;
}

// ------------------------------------------------------------------------------
//   Read Port with Lock
// ------------------------------------------------------------------------------
int
UDP_Port::
_read_port(uint8_t &cp)
{

    socklen_t len;

    // Lock
    pthread_mutex_lock(&lock);
        int result = -1;
        struct sockaddr_in addr;
        len = sizeof(struct sockaddr_in);
        result = recvfrom(sock, &buff, BUFF_LEN, 0, (struct sockaddr *)&addr, &len);
        
        // printf("%s %d \n", __func__, result);
    #if (UDP_MODE == UDP_SERVER)
        if(tx_port < 0){
            if(strcmp(inet_ntoa(addr.sin_addr), target_ip) == 0){
                tx_port = ntohs(addr.sin_port);
                printf("Got first packet, sending to %s:%i\n", target_ip, rx_port);
            }else{
                printf("ERROR: Got packet from %s:%i but listening on %s\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), target_ip);
            }
        }
    #endif

    // Unlock
    pthread_mutex_unlock(&lock);

    return result;
}


// ------------------------------------------------------------------------------
//   Write Port with Lock
// ------------------------------------------------------------------------------
int
UDP_Port::
_write_port(char *buf, unsigned len)
{
    // Write packet via UDP link
    int bytesWritten = 0;
    if(tx_port > 0){
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(target_ip);
        addr.sin_port = htons(tx_port);
        bytesWritten = sendto(sock, buf, len, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
        // printf("sendto: %s \t %i\n",target_ip,tx_port);
    }else{
        printf("ERROR: Sending before first packet received!\n");
        bytesWritten = -1;
    }
    return bytesWritten;
}


