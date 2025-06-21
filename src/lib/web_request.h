/* 

*/

#ifndef WEB_REQUEST_H
#define WEB_REQUEST_H

#include <Arduino.h>

#define HTTPS_PORT 443 // HTTPS port

/* Headers para http/https */
#define HTTP_LOCAL_HOST     "10.1.111.249:8000"
#define HTTPS_RENDER_HOST   "farm-senspire.onrender.com"


#define POST_CSV_HEADER "POST /api/upload-csv/ HTTP/1.1\r\n" \
                        "Host: %s\r\n" \
                        "User-Agent: Senspire AP V1/1.0\r\n" \
                        "Accept: application/json\r\n" \
                        "X-Device-ID: SPAPV1-0001\r\n" \
                        "Content-Length: %d\r\n" \
                        "Content-Type: text/csv\r\n" \
                        "Cache-Control: no-cache\r\n" \
                        "Connection: close\r\n" \
                        "\r\n"

/** Reservamos espacio para el header, el valor de Content-Length y el cierre de cadena */
#define POST_CSV_BODY_LENGTH (BUFFER_SIZE - sizeof(POST_CSV_HEADER) - 4)

#define GET_LAST_MEASUREMENT_HEADER "GET /api/device/last_measurement/ HTTP/1.1\r\n" \
                                    "Host: %s\r\n" \
                                    "User-Agent: Senspire AP V1/1.0\r\n" \
                                    "Accept: application/json\r\n" \
                                    "X-Device-ID: SPAPV1-0001\r\n" \
                                    "X-Fields: timestamp\r\n" \
                                    "Cache-Control: no-cache\r\n" \
                                    "Connection: close\r\n" \
                                    "\r\n"

#endif // WEB_REQUEST_H