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
 * File:    esp8266.c
 * Author:  Mostafa El-koumy <mostafa.elkoumy@gmail.com>
 * Website: https://github.com/mostafa-elkoumy/esp8266
 * Version: 0.1
 *
 * See:     esp8266.h
 *
 * C library for interfacing the ESP8266 Wi-Fi transceiver module with PIC,
 * atmel, ST, .. etc microcontroller. 
 * Should be used with the XC8 compiler, Atmel Studio, Keil, IAR.
 */

#include "esp8266.h"
#include "softUart.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

/** These three function Should be witten by the user for input from / output to the ESP module **/

void _esp8266_putch(unsigned char ch)
{
	softuart_putchar(ch);
}
unsigned char _esp8266_getch(void)
{
	return softuart_getchar();
}

 // Write a string to the output
void esp8266_writeString(unsigned const char *ptr) 
{
	softuart_puts(ptr);
}

// Check if the module is started (AT)
_Bool esp8266_isStarted(void) 
{
    esp8266_writeString("AT\r\n");
    return (esp8266_waitResponse() == ESP8266_OK);
}

// Restart module (AT+RST)
_Bool esp8266_restart(void) 
{
    esp8266_writeString("AT+RST\r\n");
    if (esp8266_waitResponse() != ESP8266_OK) {
        return false;
    }
    return (esp8266_waitResponse() == ESP8266_READY);
}
 
 // Enabled/disable command echoing (ATE)
void esp8266_echoCmds(bool echo) 
{
    esp8266_writeString("ATE");
    if (echo) 
	{
        _esp8266_putch('1');
    } 
	else 
	{
        _esp8266_putch('0');
    }
    esp8266_writeString("\r\n");
    esp8266_waitForString("OK");
}

 // WIFI Mode (station/softAP/station+softAP) (AT+CWMODE)
void esp8266_modeSelect(unsigned char mode) 
{
    esp8266_writeString("AT+CWMODE=");
    _esp8266_putch(mode + '0');
    esp8266_writeString("\r\n");
    esp8266_waitResponse();
}
 
 // Connect to AP (AT+CWJAP)
unsigned char esp8266_connectToNetwork(unsigned char* ssid, unsigned char* pass) 
{
    esp8266_writeString("AT+CWJAP=\"");
    esp8266_writeString(ssid);
    esp8266_writeString("\",\"");
    esp8266_writeString(pass);
    esp8266_writeString("\"\r\n");
    return esp8266_waitResponse();
}
 
 // Disconnect from AP (AT+CWQAP)
void esp8266_disconnectFromNetwork(void) 
{
    esp8266_writeString("AT+CWQAP\r\n");
    esp8266_waitForString("OK");
}

 // Local IP (AT+CIFSR)
void esp8266_getIp(unsigned char *store_in) 
{
	unsigned char received;
	
    esp8266_writeString("AT+CIFSR\r\n");
    do 
	{
        received = _esp8266_getch();
    } while (received < '0' || received > '9');
	
    for (unsigned char i = 0; i < 4; i++) 
	{
        store_in[i] = 0;
        do 
		{
            store_in[i] = 10 * store_in[i] + received - '0';
            received = _esp8266_getch();
        } while (received >= '0' && received <= '9');
		
        received = _esp8266_getch();
    }
    esp8266_waitForString("OK");
}
 
 // Create connection (AT+CIPSTART)
_Bool esp8266_clientStart(unsigned char protocol, char* ip, unsigned char port) 
{
    esp8266_writeString("AT+CIPSTART=\"");
    if (protocol == ESP8266_TCP) 
	{
        esp8266_writeString("TCP");
    } else 
	{
        esp8266_writeString("UDP");
    }
    esp8266_writeString("\",\"");
    esp8266_writeString(ip);
    esp8266_writeString("\",");
    unsigned char port_str[5] = "\0\0\0\0";
    sprintf(port_str, "%u", port);
    esp8266_writeString(port_str);
    esp8266_writeString("\r\n");
    if (esp8266_waitResponse() != ESP8266_OK) 
	{
        return 0;
    }
    if (esp8266_waitResponse() != ESP8266_LINKED) 
	{
        return 0;
    }
    return 1;
}
 
 // Send data (AT+CIPSEND)
_Bool esp8266_sendData(unsigned char* data) 
{
    unsigned char length_str[6] = "\0\0\0\0\0";
    sprintf(length_str, "%u", strlen(data));
    esp8266_writeString("AT+CIPSEND=");
    esp8266_writeString(length_str);
    esp8266_writeString("\r\n");
    while (_esp8266_getch() != '>');
    esp8266_writeString(data);
    if (esp8266_waitResponse() == ESP8266_OK) {
        return 1;
    }
    return 0;
}

 // Receive data (+IPD)
void esp8266_receiveData(unsigned char* store_in, uint16_t max_length, bool discard_headers) 
{
    esp8266_waitForString("+IPD,");
    uint16_t length = 0;
    unsigned char received = _esp8266_getch();
    do 
	{
        length = length * 10 + received - '0';
        received = _esp8266_getch();
    } while (received >= '0' && received <= '9');

    if (discard_headers) 
	{
        length -= esp8266_waitForString("\r\n\r\n");
    }

    if (length < max_length) 
	{
        max_length = length;
    }

    uint16_t i;
    for (i = 0; i < max_length; i++) 
	{
        store_in[i] = _esp8266_getch();
    }
    store_in[i] = 0;
    for (; i < length; i++) 
	{
        _esp8266_getch();
    }
    esp8266_waitForString("OK");
}
 
 // Wait for a certain string on the input
 
inline uint16_t esp8266_waitForString(unsigned char *string) 
{
    unsigned char so_far = 0;
    unsigned char received;
    uint16_t counter = 0;
    do 
	{
        received = _esp8266_getch();
        counter++;
        if (received == string[so_far]) 
		{
            so_far++;
        } else 
		{
            so_far = 0;
        }
    } while (string[so_far] != 0);
    return counter;
}

// Wait for any response on the input
 
inline unsigned char esp8266_waitResponse(void) 
{
    unsigned char so_far[6] = {0,0,0,0,0,0};
    unsigned const char lengths[6] = {2,5,4,9,6,6};
    unsigned const char* strings[6] = {"OK", "ready", "FAIL", "no change", "Linked", "Unlink"};
    unsigned const char responses[6] = {ESP8266_OK, ESP8266_READY, ESP8266_FAIL, ESP8266_NOCHANGE, ESP8266_LINKED, ESP8266_UNLINK};
    unsigned char received;
    unsigned char response;
    bool continue_loop = true;
    while (continue_loop) 
	{
        received = _esp8266_getch();
        for (unsigned char i = 0; i < 6; i++) 
		{
            if (strings[i][so_far[i]] == received) 
			{
                so_far[i]++;
                if (so_far[i] == lengths[i]) 
				{
                    response = responses[i];
                    continue_loop = false;
                }
            } else 
			{
                so_far[i] = 0;
            }
        }
    }
    return response;
}