/**
 * 
 * 
 * 
 * 
 */
#include "wifi.h"
#include "ap_config.h"

WiFiClientSecure client;

// Configuración de IP Estática
IPAddress staticIP(10, 1, 111, 199);          // IP estática deseada para el ESP8266
IPAddress gateway(10, 1, 111, 1);             // Dirección IP del router/gateway
IPAddress subnet(255, 255, 255, 0);           // Máscara de subred
IPAddress dns1(193, 254, 231, 2);             // DNS primario (Google)
IPAddress dns2(193, 254, 230, 2);             // DNS secundario (Google)


/* Esta funcion aun creo que puede ser mejorada, y de paso reproducida en la de eth */
bool wifi_server_income(char * response_buffer) {

    if(client.available()) {

        uint8_t consecutive_new_lines = 0; // Contador de líneas consecutivas \r\n
        response_buffer[0] = '\0'; // Limpiar el buffer de respuesta

        while(client.available()) {

            char c = client.read();
            
            // Detectar secuencia \r\n\r\n que marca el inicio del cuerpo
            if(c == '\r' && client.peek() == '\n') {
                client.read(); // Consumir el \n
                consecutive_new_lines++;
                
                // Dos secuencias \r\n seguidas indican el inicio del cuerpo
                if(consecutive_new_lines == 2) {
                    size_t len = client.readBytes(response_buffer, BUFFER_SIZE - 1); // Leer el resto del cuerpo
                    response_buffer[len] = '\0'; // Asegurar que el buffer esté terminado en null
                    return true; // Retornar el largo del cuerpo leído
                }
            } else if(c != '\n') {
                consecutive_new_lines = 0;
            }
        }

        /* Si se llegó aquí, significa que se leyó algo del servidor, pero no se encontró el cuerpo */
        return true;

    } 

    return false; // Retornar 0 si no hay datos disponibles

}

/*  */
bool wifi_is_connected() {
    // Check if WiFi is connected
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    return false;
}

bool wifi_init() {

    // Configurar IP estático antes de conectar
    WiFi.config(staticIP, gateway, subnet, dns1, dns2);

    // Initialize WiFi connection
    WiFi.mode(WIFI_STA);

    WiFi.begin(HOSTPOT_SSID, HOSTPOT_PASSWORD);

    uint32_t wifi_timeout = millis() + WIFI_CONN_TIMEOUT;
    
    // Wait for connection
    while (millis() < wifi_timeout) {

        if (wifi_is_connected()) {
            //Serial.println("Conexión WiFi exitosa");
            //Serial.print("IP: ");
            //Serial.println(WiFi.localIP());
            return true; // Connection successful
        }

        /* Resetear el WDT */
	    wdt_reset();

        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\n WiFi conn timeout");
    return false; // Connection failed

}


/*  */
bool wifi_client_send(char * data_buffer) {

    // Asegurar que la conexión WiFi esté activa
    if (!wifi_is_connected()) {
        Serial.println("WiFi reconecting...");

        /* Reintento de conectarse a la WiFi */
        if (!wifi_init()) {
            Serial.println("Failed");
            return false;
        }  
    }

    // Skip SSL certificate verification for HTTPS (necesario para conectar sin certificados)
    client.setInsecure(); 
    
    // Conectar al servidor HTTPS
    if (!client.connect(HTTPS_RENDER_HOST, HTTPS_PORT)) {
        Serial.println("https failed");
        return false;
    }
            
    // Enviar solicitud HTTP GET con todos los headers necesarios
    Serial.println("WiFi:");
    Serial.println(data_buffer);
  
    /* Envio al cliente */
    client.print(data_buffer);
 
    // Establecer timeout para leer respuesta
    uint32_t wifi_timeout = millis() + WIFI_CONN_TIMEOUT; 
    while (client.connected() && !client.available()) {
        if (wifi_timeout < millis()) {
            Serial.println("https timeout");
            client.stop();
            return false;
        }
        delay(50);

	    /* Resetear el WDT */
	    wdt_reset();
    }

    /* Leer respuesta y... */
    bool server_response = wifi_server_income(data_buffer);
    /* ...cerrar la conexión */    
    client.stop();

    /* Debug */
    Serial.println("https anws:");
    if(!server_response)
        Serial.println("No hay :( ");
    else
        Serial.println(data_buffer);

    /* Envío exitoso */
    return server_response;

}