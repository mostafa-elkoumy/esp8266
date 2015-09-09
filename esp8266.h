/**
 * C library for the ESP8266 WiFi module with any microcontroller
 * Copyright (C) 2015 Mostafa El-koumy <mostafa.elkoumy@gmail.com>
 *	LinkedIn profile <https://eg.linkedin.com/in/elkoumy>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *****************************************************************************
 *
 * File:    esp8266.h
 * Author:  Mostafa El-koumy <mostafa.elkoumy@gmail.com>
 * Website: https://github.com/mostafa-elkoumy/esp8266
 * Version: 0.1
 *
 * See:     esp8266.c
 *
 * This is the header file for the ESP8266 Software library. It contains:
 *
 *  * Constants and bitmasks
 *  * Prototypes for functions that should be written by the user for his
 *    implementation
 *  * Prototypes for functions that can be used by the user
 *  * Prototypes for functions that are intended for internal use only
 */

#include <stdbool.h>
#include <stdint.h>

#ifndef ESP8266_H
#define	ESP8266_H

#ifdef	__cplusplus
extern "C" {
#endif

/** Some constants **/

#define ESP8266_STATION 							0x01
#define ESP8266_ACCESSPOINT 						0x02

#define ESP8266_TCP 								1
#define ESP8266_UDP 								0

#define ESP8266_OK 									1
#define ESP8266_READY 								2
#define ESP8266_FAIL 								3
#define ESP8266_NOCHANGE 							4
#define ESP8266_LINKED 								5
#define ESP8266_UNLINK 								6

/** These two function Should be witten by the user for input from / output to the ESP module **/

void _esp8266_putch(unsigned char);
unsigned char _esp8266_getch(void);

/** Function prototypes **/


/**
 * Check if the module is started
 *
 * This sends the `AT` command to the ESP and waits until it gets a response.
 *
 * @return true if the module is started, false if something went wrong
 */
 
_Bool esp8266_isStarted(void);

/**
 * Restart the module
 *
 * This sends the `AT+RST` command to the ESP and waits until there is a
 * response.
 *
 * @return true iff the module restarted properly
 */
 
_Bool esp8266_restart(void);

/**
 * Enable / disable command echoing.
 *
 * Enabling this is useful for debugging: one could sniff the TX line from the
 * ESP8266 with his computer and thus receive both commands and responses.
 *
 * This sends the ATE command to the ESP module.
 *
 * @param echo whether to enable command echoing or not
 */
 
void esp8266_echoCmds(bool);        

/**
 * Set the WiFi mode.
 *
 * ESP8266_STATION : Station mode
 * ESP8266_SOFTAP : Access point mode
 *
 * This sends the AT+CWMODE command to the ESP module.
 *
 * @param mode an ORed _Boolmask of ESP8266_STATION and ESP8266_SOFTAP
 */
 
void esp8266_modeSelect(unsigned char);

/**
 * Connect to an access point.
 *
 * This sends the AT+CWJAP command to the ESP module.
 *
 * @param ssid The SSID to connect to
 * @param pass The password of the network
 * @return an ESP status code, normally either ESP8266_OK or ESP8266_FAIL
 */
 
unsigned char esp8266_connectToNetwork(unsigned char*, unsigned char*);

/**
 * Disconnect from the access point.
 *
 * This sends the AT+CWQAP command to the ESP module.
 */
 
void esp8266_disconnectFromNetwork(void);

/**
 * Store the current local IPv4 address.
 *
 * This sends the AT+CIFSR command to the ESP module.
 *
 * The result will not be stored as a string but byte by byte. For example, for
 * the IP 192.168.0.1, the value of store_in will be: {0xc0, 0xa8, 0x00, 0x01}.
 *
 * @param store_in a pointer to an array of the type unsigned char[4]; this
 * array will be filled with the local IP.
 */
 
void esp8266_getIp(unsigned char *store_in);

/**
 * Open a TCP or UDP connection.
 *
 * This sends the AT+CIPSTART command to the ESP module.
 *
 * @param protocol Either ESP8266_TCP or ESP8266_UDP
 * @param ip The IP or hostname to connect to; as a string
 * @param port The port to connect to
 *
 * @return true iff the connection is opened after this.
 */
 
_Bool esp8266_clientStart(unsigned char protocol, char* ip, unsigned char port);

// Send data (AT+CIPSEND)
/**
 * Send data over a connection.
 *
 * This sends the AT+CIPSEND command to the ESP module.
 *
 * @param data The data to send
 *
 * @return true iff the data was sent correctly.
 */
 
_Bool esp8266_sendData(unsigned char*);

/**
 * Read a string of data that is sent to the ESP8266.
 *
 * This waits for a +IPD line from the module. If more bytes than the maximum
 * are received, the remaining bytes will be discarded.
 *
 * @param store_in a pointer to a character array to store the data in
 * @param max_length maximum amount of bytes to read in
 * @param discard_headers if set to true, we will skip until the first \r\n\r\n,
 * for HTTP this means skipping the headers.
 */
 
void esp8266_receiveData(unsigned char*, uint16_t, bool);

/** Functions for internal use only **/

/**
 * Output a string to the ESP module.
 *
 * This is a function for internal use only.
 *
 * @param ptr A pointer to the string to send.
 */
 
void esp8266_writeString(unsigned const char *);

/**
 * Wait until we found a string on the input.
 *
 * Careful: this will read everything until that string (even if it's never
 * found). You may lose important data.
 *
 * @param string
 *
 * @return the number of characters read
 */
 
inline uint16_t esp8266_waitForString(unsigned char *);

/**
 * Wait until we received the ESP is done and sends its response.
 *
 * This is a function for internal use only.
 *
 * Currently the following responses are implemented:
 *  * OK
 *  * ready
 *  * FAIL
 *  * no change
 *  * Linked
 *  * Unlink
 *
 * Not implemented yet:
 *  * DNS fail (or something like that)
 *
 * @return a constant from esp8266.h describing the status response.
 */
 
inline unsigned char esp8266_waitResponse(void);

#ifdef	__cplusplus
}
#endif

#endif	/* ESP8266_H */
