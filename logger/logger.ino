/*
   Sketch Arduino che legge dati provenienti da sensori e li
   invia tramite seriale all'ESP il quale invierà dati rilevati
   al sensore predefinito su cloud Ubidots.
   2017 - Saul Bertuccio

*/

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <SPI.h>
#include "SdFat.h"

/****************************************
   Definisco constanti
 ****************************************/

namespace {
const char * USER_AGENT = "SensoreArduino"; // User ageng
const char * VERSION =  "1.0"; // Versione
const char * METHOD = "POST"; // Metodo invio POST o GET
const char * TOKEN = "A1E-Ig8LF6prkagJFjbt2LGYV8EIYLqW7b"; // TOKEN di sessione
const char * DEVICE_NAME = "ArduinoSB"; // Assign the desire device name
const char * DEVICE_LABEL = "arduino_saul"; // Assign the device label
}

#define SAMPLE_INTERVAL 20000

#define TEMP_SENSOR_TYPE DHT11 // Tipo sensore DHT
#define TEMP_SENSOR_PIN  2     // Pin connessione Sensore

#define DIST_SENSOR_TRIGGER 7    // pin trigger sensore distanza
#define DIST_SENSOR_ECHO 6       // pin lettura distanza
#define DIST_SENSOR_MAX_MS 23500 // 2 * ritardo minimo in µs sensore distanza
#define DIST_SENSOR_MIN_MS 118  // 2* ritardo massimo in µs sensore distanza

// ATTENZIONE: l'SD è FAT a 16, 6 caratteria al massimo
#define FILE_BASE_NAME "dsens"   // Nome file log


char comando[700];  // Comando da inviare
char risposta[100]; // Risposta dal server

#define DEBUG Serial
#define ESP Serial1

void ubiCmd(char* cmd, char* sname, char* sval) {

  /* Intestazione comando ESP */
  sprintf(cmd, "init#%s/%s|%s|%s|%s:%s=>", USER_AGENT, VERSION, METHOD, TOKEN, DEVICE_NAME, DEVICE_LABEL);
  /* Preparo i dati */
  sprintf(cmd, "%s%s:%s|end#final", cmd, sname, sval);
}

void getResponse(char *response) {

  /* Attesa iniziale */
  delay(250);

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

  /* Attesa finale */
  delay(500);
}

float readDistance(float temperatura = 20) {

  // Vsuono = ( 331,45 + ( 0.62 x T ) ) * (100 / 10^6) cm/µs
  float vel_suono = ( 331.45 + ( 0.62 * temperatura ) ) * 0.0001;

  // Mi assicuro che il trigger si a in HIGH
  // DEBUG.print("Inizializzazione Sensore Distanza: velocità suono ");
  // DEBUG.print(vel_suono);
  // DEBUG.println(" cm/µs");

  digitalWrite(DIST_SENSOR_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(DIST_SENSOR_TRIGGER, HIGH);

  // Il trigger deve rimanere a HIGH almeno 10 µs
  delayMicroseconds(10);
  digitalWrite(DIST_SENSOR_TRIGGER, LOW);

  // rilevo il tempo in microsecondi
  long tempo = pulseIn(DIST_SENSOR_ECHO, HIGH);

  if (tempo < DIST_SENSOR_MIN_MS || tempo > DIST_SENSOR_MAX_MS) {
    Serial.println("Oggetto troppo vicino o troppo lontano dal sensore.");
    return INFINITY;
  }

  float distanza = vel_suono * (tempo / 2.0);
  Serial.print("Distanza: ");
  Serial.print(distanza);
  Serial.println("cm");

  return distanza;

}

void readDHT(float *temp, float *hum) {
  DHT_Unified dht(TEMP_SENSOR_PIN, TEMP_SENSOR_TYPE);
  dht.begin();
  // DEBUG.println("Inizializzazione Sensore Temperatura/Umidità");

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


boolean writeLog(SdFile &file, unsigned long logTime, char* dsensori[], int num ) {

  char line[128];

  int i = 0;

  while (i < num * 2) {
    /* Creo linea di log */
    sprintf(line, "%ld: %s %s", logTime, dsensori[i], dsensori[i++]);
    file.println(line);
    i++;
  }

  // Forzo l'aggiornamento dei dati sull'SD
  if (!file.sync() || file.getWriteError()) {
    DEBUG.println("Errore: Impossibile aggiornare file di log");
    file.close();
    return false;
  }

  return true;

}

// Gestione Sdcard e filestem come oggetti globali)
uint32_t logTime; // microsendi acquisizione dati
// filesystem
SdFat sd;
// file
SdFile file;
// indica se il file è stato aperto
bool opened = false;

void setup() {

  DEBUG.begin(115200); // Debug arduino
  ESP.begin(115200); // Comunicazione ESP

  // Imposto i pin per la gestione del sensore di distanza
  pinMode(DIST_SENSOR_TRIGGER, OUTPUT);
  pinMode(DIST_SENSOR_ECHO, INPUT);

  // Gestione SdCard e filesystem alla massima velocità supportata dalla scheda
  // che non dovrebbe essere maggiore di 50 MHz.
  // In caso di errori SPI abbassare la velocità

  const uint8_t chipSelect = SS;

  if (sd.begin(chipSelect, SD_SCK_MHZ(50))) {

    // Genero un nome file non usato.

    const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
    char fileName[13] = FILE_BASE_NAME "00.log";

    while (sd.exists(fileName)) {

      if (fileName[BASE_NAME_SIZE + 1] != '9') {
        fileName[BASE_NAME_SIZE + 1]++;
      } else if (fileName[BASE_NAME_SIZE] != '9') {
        fileName[BASE_NAME_SIZE + 1] = '0';
        fileName[BASE_NAME_SIZE]++;
      } else {
        strcpy(fileName, FILE_BASE_NAME "00.log");
        DEBUG.print("Attenzione: sovrascrivo vecchio file log ");
        DEBUG.println(fileName);
        break;
      }
    }

    opened = file.open(fileName, O_CREAT | O_WRITE | O_EXCL);
  }

  if (!opened)
    DEBUG.println("Attenzione: impossibile inizializzare SD Card, non verrà tenuto alcun log.");

  // LogTime come multiplo dell'intevallo di campionamento.
  logTime = micros() / ( 1000UL * SAMPLE_INTERVAL ) + 1;
  logTime *= 1000UL * SAMPLE_INTERVAL;
}

void loop() {

  // TODO Verifica connessione ESP

  /* Lettura dati sensori */
  float temp, hum, dist;

  readDHT(&temp, &hum);
  dist = readDistance(temp);

  /* Dati sensore */
  char temperatura[10];
  char umidita[10];
  char distanza[10];

  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(temp, 4, 2, temperatura);
  dtostrf(hum, 4, 2, umidita);
  dtostrf(dist, 4, 0, distanza);

  /* Invio temperatura */
  ubiCmd(comando, "temperatura", temperatura);
  ESP.print(comando);
  DEBUG.println(comando);
  getResponse(risposta);

  /* Invio umidita */
  ubiCmd(comando, "umidita", umidita);
  ESP.print(comando);
  DEBUG.println(comando);
  getResponse(risposta);

  /* Invio distanza */
  ubiCmd(comando, "distanza", distanza);
  ESP.print(comando);
  DEBUG.println(comando);
  getResponse(risposta);

  // aggiorno logtime
  logTime += 1000UL * SAMPLE_INTERVAL;

  if (opened) {
    char *sdata[] = {
      "temperatura", temperatura,
      "umidita", umidita,
      "distanza", distanza
    };
    int sensori = 3;

    // Scrivo dati su SD - Card
    opened = writeLog(file, logTime, sdata, sensori);
  }

  delay(SAMPLE_INTERVAL);
}
