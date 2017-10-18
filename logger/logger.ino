/* 
 * Sketch Arduino che legge dati provenienti da sensori e li 
 * invia tramite seriale all'ESP il quale invierà dati rilevati 
 * al sensore predefinito su cloud Ubidots.
 * 2017 - Saul Bertuccio
 * 
 */

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

/****************************************
 * Definisco constanti
 ****************************************/

namespace {
  const char * USER_AGENT = "SensoreArduino"; // User ageng
  const char * VERSION =  "1.0"; // Versione
  const char * METHOD = "POST"; // Metodo invio POST o GET
  const char * TOKEN = "A1E-Ig8LF6prkagJFjbt2LGYV8EIYLqW7b"; // TOKEN di sessione
  const char * DEVICE_NAME = "ArduinoSB"; // Assign the desire device name 
  const char * DEVICE_LABEL = "arduino_saul"; // Assign the device label 
}

#define TEMP_SENSOR_TYPE DHT11 // Tipo sensore DHT
#define TEMP_SENSOR_PIN  2     // Pin connessione Sensore


char comando[700];  // Comando da inviare
char risposta[100]; // Risposta dal server

#define DEBUG Serial
#define ESP Serial1

void getResponse(char *response) {

    if (ESP.available() > 0) {
    /* Leggo la risposta dall'ESP */
    int j = 0;
    while (ESP.available() > 0) {
      response[j++] = (char)ESP.read();
    }
    /* Response from the server */
    DEBUG.print(response);
    j = 0;
  }
}

void readDHT(float *temp, float *hum) {
  DHT_Unified dht(TEMP_SENSOR_PIN, TEMP_SENSOR_TYPE);
  dht.begin();
  DEBUG.println("Inizializzazione Sensore Temperatura/Umidità");
  
  sensors_event_t event;

  // Rilevo il valore della temperatura
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    DEBUG.println("Impossibile rilevare la temperatura dal sensore.");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(event.temperature);
    Serial.println("°C");
  }

  *temp = event.temperature;

  // Rilevo valore umidità
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    DEBUG.println("Impossibile rilevare il valore di umidità dal sensore.");
  }
  else {
    Serial.print("Umidità: ");
    Serial.print(event.relative_humidity);
    Serial.println("%");
  }

  *hum = event.relative_humidity;
}

void setup() {
  DEBUG.begin(115200); // Debug arduino
  ESP.begin(115200); // Comunicazione ESP
}

void loop() {
  
  // TODO Verifica connessione ESP

  /* Lettura dati sensore */ 
  float temp, hum;
  readDHT(&temp, &hum);

  /* Dati sensore */
  char temperatura[10];
  char umidita[10];
    
  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(temp, 4, 2, temperatura);
  dtostrf(hum, 4, 2, umidita);

  /* Creo comando per ESP */
  sprintf(comando, "init#");
  sprintf(comando, "%s%s/%s|%s|%s|", comando, USER_AGENT, VERSION, METHOD, TOKEN);
  sprintf(comando, "%s%s:%s=>", comando, DEVICE_NAME, DEVICE_LABEL);
  sprintf(comando, "%s%s:%s", comando, "temperatura", temperatura);
  //sprintf(comando, "%s,%s:%s", comando, "umidita", umidita);
  sprintf(comando, "%s|end#final", comando); 

  DEBUG.println(comando); 
  ESP.print(comando);
  getResponse(risposta);
  delay(15000);
}
