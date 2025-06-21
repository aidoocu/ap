/** WiFi module for AP (ESP8266) 
 * 
 * 
 * 
 * This module provides functions to initialize the WiFi connection, update the connection status, and check for incoming data.
 * It handles both HTTP GET and POST requests, allowing the device to send and receive data from a server.
*/

#ifndef WIFI_H
#define WIFI_H 

#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#endif // ESP8266

#define WIFI_CONN_TIMEOUT 50000L // Timeout for WiFi connection in milliseconds

#define ENDPOINT "/api/device/last_measurement/" // Con "/" al inicio para formato correcto
#define DEVICE_ID_HEADER "X-Device-ID: SPAPV1-0001"
#define TIME_STAMP_HEADER "X-Fields: timestamp" // Ejemplo de timestamp, ajustar según necesidad

// SSID de la red
#define HOSTPOT_SSID "TP-LINK_B06F78"
#define HOSTPOT_PASSWORD "tplink.cp26"


/* WiFi initialization */
bool wifi_init(void);

/* W */
bool wifi_is_connected(void);

/* WiFi client send function */
bool wifi_client_send(char * data_buffer);


#endif // WIFI_H