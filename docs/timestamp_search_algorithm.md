# Implementación del búsqueda de fecha más cercana

Este documento detalla el algoritmo propuesto para implementar la funcionalidad mencionada en el TODO de la función `sd_date_time_next_line_search`, para encontrar la línea con la fecha/hora más cercana cuando no se encuentra una coincidencia exacta.

## Problema

En el código actual, si el timestamp exacto solicitado por el servidor no se encuentra en el archivo CSV, no se devuelve ningún dato. Sin embargo, se desea implementar una búsqueda que encuentre la línea con la fecha/hora más cercana al timestamp solicitado.

## Algoritmo propuesto

1. Analizar el formato exacto de los timestamps en el archivo CSV
2. Parsear el timestamp de búsqueda para extraer su fecha y hora
3. Para cada línea del archivo:
   - Extraer el timestamp de la línea
   - Calcular la diferencia entre este timestamp y el buscado
   - Mantener un seguimiento del timestamp más cercano encontrado hasta ahora
4. Después de recorrer todo el archivo, devolver la posición del timestamp más cercano

## Pseudo-código

```cpp
uint32_t sd_find_closest_date_time(char* search_timestamp) {
    // Inicializar variables
    uint32_t closest_position = 0;
    int64_t min_difference = INT64_MAX;
    int64_t search_time = convert_timestamp_to_seconds(search_timestamp);

    // Abrir el archivo
    sd_file = SD.open(SD_DATALOG);
    if (!sd_file) return 0;

    // Recorrer cada línea del archivo
    uint32_t current_position = 0;
    while (sd_file.available()) {
        // Guardar la posición actual
        current_position = sd_file.position();
        
        // Leer la línea
        char line[128];
        size_t len = sd_file.readBytesUntil('\n', line, sizeof(line) - 1);
        line[len] = '\0';
        
        // Extraer el timestamp de la línea (asumiendo que está al principio)
        char line_timestamp[24];  // Tamaño apropiado para "yyyy-mm-ddThh:mm:ssZ"
        extract_timestamp(line, line_timestamp);
        
        // Convertir a segundos y calcular la diferencia
        int64_t line_time = convert_timestamp_to_seconds(line_timestamp);
        int64_t difference = abs(line_time - search_time);
        
        // Si es la diferencia más pequeña hasta ahora
        if (difference < min_difference) {
            min_difference = difference;
            closest_position = current_position;
        }
    }
    
    sd_file.close();
    return closest_position;
}
```

## Consideraciones de implementación

1. La función `convert_timestamp_to_seconds` debe convertir un timestamp ISO 8601 a un valor entero (segundos desde una fecha de referencia)
2. La función `extract_timestamp` debe extraer el timestamp de una línea del archivo
3. Se debe considerar si buscar la fecha más cercana solo hacia adelante, hacia atrás o en cualquier dirección
4. Para sistemas embebidos, optimizar el uso de memoria es crucial

## Integración con el código actual

Esta funcionalidad se incorporaría dentro de `sd_date_time_next_line_search` cuando la búsqueda exacta falle, o como una función separada que se llame desde ella.
