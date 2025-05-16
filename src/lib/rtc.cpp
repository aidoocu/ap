/** 
 * 
 * 
 * 
 */

#include "rtc.h"

uRTCLib rtc(RTC_ADDR);


void get_time(char * time) {

	rtc.refresh();

    /* dd/mm/aa,hh:mm:ss */
	sprintf(time, "%02d/%02d/%02d,%02d:%02d:%02d", rtc.month(), rtc.day(), rtc.year(), rtc.hour(), rtc.minute(), rtc.second());
	

}

bool set_date_time(char * date_time) {

	/** RTCLib::set(byte second, 
	 * byte minute, 
	 * byte hour (0-23:24-hr mode only),
	 * byte dayOfWeek (Sun = 1, Sat = 7), 
	 * byte dayOfMonth (1-12), 
	 * byte month, 
	 * byte year) */

	//Serial.println(date_time);
	//delay(20);

	if(!validate_date_time(date_time)) {
		Serial.println("Invalid date_time");
		return false;
	}

 	uint8_t second, minute, hour, dayOfWeek, dayOfMonth, month, year;

	// Parsear la cadena
	sscanf(date_time, "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu", 
			&second, 
			&minute, 
			&hour, 
			&dayOfWeek, 
			&dayOfMonth, 
			&month, 
			&year);
   
	// Configurar RTC
	rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year);

	return true;

}

bool validate_date_time(const char* date_time) {
	
	/* Formato esperado: ss,mm,hh,w,dd,mo,yy */ 
	
	/* verificamos que tiene el largo correcto, entre 13 y 19 caracters */ 
	size_t len = strlen(date_time);
  	if (len < 13 || len > 19) 
		return false;

	// Rangos mínimos y máximos para cada campo:
	const int minVals[7] = {  0,   0,  0,  1,  1,  1,   0};  // ss, mm, hh, w, dd, mo, yy
	const int maxVals[7] = { 59,  59, 23,  7, 31, 12,  99};  // asume year 00–99

	int fields[7];
	char buf[32];
	strncpy(buf, date_time, sizeof(buf));
	buf[sizeof(buf)-1] = '\0';

	// Tokeniza por comas
	char * token = strtok(buf, ",");
	int idx = 0;
  
	while (token && idx < 7) {

    	// Comprueba que es numérico y convierte
    	char * endptr;
    	long val = strtol(token, &endptr, 10);
    	if (* endptr != '\0') 
			return false;        // no era un número limpio
    
		fields[idx++] = (int)val;
    	token = strtok(nullptr, ",");
  	}

  	// no tenía 7 campos exactos
	if (idx != 7) 
		return false;                 

  	// Comprueba rangos básicos
  	for (int i = 0; i < 7; i++) {
   		if (fields[i] < minVals[i] || fields[i] > maxVals[i]) {
      		return false;
    	}
  }

  	// Validar días del mes según mes y año (simplificado: febrero 28/29)
  	int day = fields[4], month = fields[5], year = fields[6];
  	static const int daysInMonth[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
  	int dim = daysInMonth[month];

  	// Año bisiesto?
  	int fullYear = 2000 + year;  // si quieres 1900+year o ajusta tu offset
  	bool isLeap = (fullYear % 4 == 0 && (fullYear % 100 != 0 || fullYear % 400 == 0));
  	if (month == 2 && isLeap) dim = 29;
  	if (day > dim) return false;

  return true;  // pasó todas las pruebas

}
