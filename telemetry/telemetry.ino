// Sketch ESP con uso liberia UbidotsESP8266 per invio
// dati sensori su cloud Ubidots.
// I dati vengono collezionati dal logger (Arduino)
// 2017 - Saul Bertuccio

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <UbidotsESP8266.h>

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "ConfigParams.h"
#include "PortalParams.h"

#define PORTAL_SSID "WifiSensorAP"
#define PORTAL_PASS "administrator"
#define SENSOR_NAME "arduino_sensor01"

#define TRIGGER_PIN 0 // Pin che forza la modalità configurazione

#define PORTAL_TIME_OUT 300 // in configurazione tempo di attività del portale in secondi
#define SLEEP_TIME 20 // tempo di sonno profondo in secondi 

//flag per notificare il salvataggio della configurazione
bool shouldSaveConfig = false;

//callback per notificare il salvataggio della confiugurazione
void saveConfigCallback () {
  Serial.println("Configurazione modificata necessario salvarla.");
  shouldSaveConfig = true;
}


bool getConnected(bool force = false) {

  // Inizializzazione locale. Terminata la configurazione della
  // Wifi non è più necessario
  WiFiManager wifiManager;

  // callback salvataggio configurazione
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // Reset impostazioni (test)
  //wifiManager.resetSettings();

  // Gli AP che hanno un segnale che è al di sotto di questa soglia
  // non vengono visualizzati, predefinito 8%
  //wifiManager.setMinimumSignalQuality();

  // Timeout della modalità portale
  wifiManager.setTimeout(PORTAL_TIME_OUT);

  bool conn = false;

  if (force) {
    // Forziamo l'avvio del portale di configurazione
    Serial.println("Richiesta modalità configurazione...");
    conn =  wifiManager.startConfigPortal(PORTAL_SSID, PORTAL_PASS);
  } else {
    // La libreria cerca di connettersi ad un AP usando ESSID e la password
    // salvati in flash. Se l'ESP non riesce a connettersi, viene impostato
    // in modalità AP+ST (Access point + Stazione) e viene avviato il portale
    // di configurazione. La chiamata è bloccante fino a quando il sensore
    // non viene configurato correttamente
    Serial.println("Wifi non configurata o fuori portata...");
    conn = wifiManager.autoConnect(PORTAL_SSID, PORTAL_PASS);
  }

  return conn;
}

float readVcc() {

  float currentVcc = 0.00f;

  currentVcc = ESP.getVcc();
  Serial.print("ESP.GetVcc: ");
  Serial.print(currentVcc);
  Serial.print(" => ");
  Serial.print(currentVcc / 1024.00f);
  Serial.println(" V");
  return currentVcc;
}

void sendData(const ConfigParams &cfg, Campione c) {

  WiFiClient client;
  int port = atoi(cfg.server_port);
  if (!client.connect(cfg.server_ip, port )) {
    Serial.println("Impossibile connettersi al server");
    return;
  }

  String data = String("{\"operazione\": \"invio_dati\",");
  data += String("\"sensore\":\"") + cfg.sensor_name + "\",";
  data += String("\"valido\":\"") + ( c.valido ? "true" : "false" ) + "\",";
  data += String("\"tensione\":\"") + c.vcc + "\",";
  data += String("\"temperatura\":\"") + c.temperatura + "\",";
  data += String("\"umidita\":\"") + c.umidita + "\",";
  data += String("\"indice_calore\":\"") + c.indice_calore + "\"}";

  Serial.print("Invio dati: ");
  Serial.println(data);

  // This will send the request to the server
  client.print(data);
  unsigned long timeout = millis();

  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Nessuna risposta!");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

// Attiva la possibilità di leggere la tensione di alimentazione
ADC_MODE(ADC_VCC);

void setup() {

  Serial.begin(115200);

  WiFi.hostname(SENSOR_NAME);
  #ifdef ESP8266
  wifi_station_set_hostname(SENSOR_NAME);
  #endif

  pinMode(TRIGGER_PIN, INPUT);
  // Forzo la modalità configurazione se il pin è a massa
  bool force = digitalRead(TRIGGER_PIN) == LOW;

  bool conn = getConnected(force);

  if (!conn) {
    Serial.println("Collegamento fallito riavvio l'ESP...");
    delay(3000);
    ESP.restart();
  }
}

void loop() {

  Ubidots client(TOKEN);
  client.readData(); // Reads the command from the logger
  delay(5000);
}
