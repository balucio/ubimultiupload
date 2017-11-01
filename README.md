# ubimultiupload
Ubidot Multi Sensor Upload

Questo progetto consente l'upload sul servizio Ubidots di dati provenienti da diversi sensori connessi ad Arduino.


Per la realizzazione sono stati impiegati:
* un Arduino Mega
* uno Shield Wify ESP8266 di _*wangTongze*_
* un sensore DHT11 per la misurazione della temperatura e dell'umidità
* un sensore HC-SR04 di distanza.

Lo Shield Wifi di _*wangTongze*_ è un prodotto abbastanza economico, configurabile tramite una serie di switch sulla scheda.

Un problema di questo shield è che rende impossibile usare l'interfaccia USB di Arduino per fare debug. Infatti, lo shield in modalità di funzionamento normale (Swith 1 e 2 a ON) comunica con Arduino tramite i pin 0 e 1, cioè TX e RX della prima interfaccia Seriale di Arduino. Tale interfaccia tuttavia è condivisa con quella USB.

Per ovviare a questo problema ci sono due strade:

Usare shield e arduino in modo indipendente (tutti gli switch a OFF): per far comunicare Arduino con lo shield è necessario ponticellare i pin RX e TX del connettore di debug dello shield, rispettivamente con i pin TX e RX di un'altra seriale fisica di arduino. Qualora l'arduino diponga di un asola interfaccia seriale fisica usare la libreria SoftSerial e usare dei Pin disponbili di arduino.

Con gli swith 1 e 2 a ON dello shield: piegare i PIN0 e PIN1 dello shield in modo che non siano connessi ad Arduino. Ponticellare i PIN0 (TX) e PIN1 (RX) dello shield come nel caso precedente.

Per quanto riguarda l'implementazione inizialmente si era pensato di usare il firwmare ESP-Link per connettersi ai servizi Cloud di Ubidots, tuttavia come da https://claus.bloggt.es/2017/02/26/arduino-esp8266-wifi-shield-elecshop-ml-by-wangtongze-comparison/ sembra che lo shield funzioni male con questo firmware.

Un'altra soluzione sarebbe stata quella di usare la libreria ESPWifi in modo da pilotare l'ESP usando i comandi AT. Il problema di questa implementazione è che gran parte del codice andrebbe scritto lato Arduino lasciando all'ESP solo il compito di effettuare la connessione e inviare i dati.

Sicuramente la soluzione migliore, anche se richiede un po' più di tempo per l'implementazione è quella di usare direttamente anche ESP con l'SDK per Arduino IDE (https://github.com/esp8266/Arduino) "Arduino core for ESP8266 WiFi chip".

Sull'ESP viene caricato uno sketch che si occuperà di gestire il collegamento Wifi, e usando la libreria ESP di Ubidots (https://github.com/ubidots/ubidots-esp8266-serial) di caricare sul cloud i dati dei sensori. Arduino si limiterà a leggere i dati dai sensori e inviarli sulla seriale all'ESP.

La configurazione degli switch sullo shield ESP è la seguente:

<table cellpadding="2" cellspacing="0" width="743">
    <tbody>
        <tr>
            <td style="width: 33.3333%;">Modalit&agrave;
                <br>
            </td>
            <td style="width: 33.3333%;">Switch
                <br>
            </td>
            <td style="width: 33.3333%;">Descrizione
                <br>
            </td>
        </tr>
        <tr>
            <td style="width: 33.3333%;" width="367">ESP connesso via porta debug (no arduino)
                <br>
            </td>
            <td style="width: 33.3333%;">OFF: 1, 2, 3, 4
                <br>
            </td>
            <td style="width: 33.3333%;" width="367">ESP funziona normalmente
                <br>
            </td>
        </tr>
        <tr>
            <td style="width: 33.3333%;" width="367">ESP connesso via porta debug (no arduino)
                <br>
            </td>
            <td style="width: 33.3333%;">ON: 1, 2 - OFF: 3, 4
                <br>
            </td>
            <td style="width: 33.3333%;" width="367">ESP funziona normalmente
                <br>
            </td>
        </tr>
        <tr>
            <td style="width: 33.3333%;" width="367">ESP connesso via porta debug (no arduino)
                <br>
            </td>
            <td style="width: 33.3333%;">ON: 3, 4 - OFF: 1, 2
                <br>
            </td>
            <td style="width: 33.3333%;" width="367">Modalit&agrave; flash ESP
                <br>
            </td>
        </tr>
        <tr>
            <td style="width: 33.3333%;" width="367">ESP come shield di Arduino
                <br>
            </td>
            <td style="width: 33.3333%;">OFF: 1, 2, 3, 4
                <br>
            </td>
            <td style="width: 33.3333%;" width="367">Arduino e ESP funzionano indipendentemente
                <br>
            </td>
        </tr>
        <tr>
            <td style="width: 33.3333%;" width="367">ESP come shield di Arduino
                <br>
            </td>
            <td style="width: 33.3333%;">ON: 1,2 - OFF: 3,4
                <br>
            </td>
            <td style="width: 33.3333%;" width="367">ESP connesso alla seriale hardware di arduino.
                <br>
            </td>
        </tr>
        <tr>
            <td style="width: 33.3333%;" width="367">Firmware ESP-Link
                <br>
            </td>
            <td style="width: 33.3333%;">
                <br>
            </td>
            <td style="width: 33.3333%;" width="367">Funzionamento non garantito.
                <br>
            </td>
        </tr>
    </tbody>
</table>


https://claus.bloggt.es/2017/02/26/arduino-esp8266-wifi-shield-elecshop-ml-by-wangtongze-comparison/

### Sensore DHT11

Si tratta dei noti sensori Temperatura/Umidità a basso costo. Tali sensori non hanno un'elevata precisione e le misurazioni sono inclini ad errori.
L'interfacciamento la comunicazione con Arduino avviene tramite le librerie Adafruit DHT, installabili direttamente da Arduino IDE.

### Sensore ad ultrasuoni HCSR04

Questo sensore consente la misurazione della distanza dell'oggetto più vicino usando ultrauoni. Le distanze rilevabili con buona approssimazione, sono comprese tra 2cm fino a un massimo di 400cm (4m).

Per misurare la distanza si sfrutta la caratteristica degli ultrasuoni (e delle onde sonore in genere) di riflettersi sugli oggetti. Conoscendo la velocità di propagazione nell'atmosfera, dal tempo impiegato dall'onda sonora a *colpire* l'oggetto e tornare indietro si può ricavare la sua distanza (relativamente al sensore).

	d = v x t

dove:
* d: è la distanza;
* v: velocità del suono nell'aria. Circa 331 m/s a 0°C e di 343,8 m/s a 20 °C;
* t: tempo in microsendi, che l'onda impiega a riflettersi sull'oggetto e tornare al sensore;

Poiché la velocità del suono nell'aria varia al variare della temperatura, per miglorare la precisione e calcolare un valore *v* ottimale, si può usare l'approssimazione linerare in funzione della temperatura:

    v = 331,45 + ( 0.62 x T ) m/s

dove:

* T : è la temperatura misurata in °C

Per migliorare l'approssimazione conviene riportare le velocità in cm/µs (centimetri al microsecondo) piuttosto che in m/s (metri al secondo):

Considerato che:

* `1 m = 10^2 cm`
* `1 s = 10^6 µs`

Si ottiene il parametro di conversione pari a `100 / 10^-6 => 10^-4 => 0.0001`:

* `331,45 + ( 0.62 x T ) m/s =  ( 331,45 + ( 0.62 x T ) ) * 0.0001 cm/µs`

Il sensore viene attivato impostando allo stato logico `HIGH` il suo pin `TRIGGER` per almeno `10µs`. A questo punto il sensore imposterà il suo pin `Echo` allo stato logico `LOW` e comincerà a produrre ultrasuoni attendendo che vengano riflessi. Non appena il sensore riceverà gli ultrasuoni riflessi, imposterà il pin `Echo` allo stato logico `HIGH`. Pertano è possibile risalire al tempo impiegato per *colpire* l'oggetto, misurando il tempo di transizione dallo stato `LOW` allo stato `HIGH` del pin `Echo`. Tale misuraione va divisa per due perchè rappresenta il tempo di andata e ritorno.

Poichè l'intervallo di funzionamento del sensore è tra 2 e 400 cm, andranno scartati tutti i valori di tempo: `60µs < t > 12000µs`

### Log su SD CARD

Le misurazioni rilevate vengono anche salvate su Card SD. L'applicazione si aspetta un Sd-Card già formattata in formato Fat-16. Ad ogni avvio di Arduino viene creato nella Root della Sd Card un file di log incrementale da 00 a 99. Il conteggio riparte da 00, pertanto i file iniziali verranno sovrascritti. 

L'adattatore usato è fabbricato da Catalex (http://arduino-info.wikispaces.com/SD-Cards).

Sul sito si afferma che il funzionamento è garantito solo a 3.3v, sia per la tensione di alimentazione che per quella relativa ai pin di I/O.

L'adattatore è compatibile con Micro SD Card da 2Gb e 4Gb.

Per la gestione con Arduino può essere usata la libreria SdFAT (https://github.com/greiman/SdFat) installabile direttamente dalla gestione librerie da Arduino IDE.

L'adattatore va connesso ad arduino tramite i Pin SPI Serial Peripheral Interface. Si tratta di un protocollo seriale sincrono usato per la comunicazione tra microcontrollori e periferiche o anche tra due microcontrollori.

La connessione SPI viene gestita da un dispositivo ***master*** (nel nostro caso Arduino Mega), che comunica con i dispositivi ***slave***, tramite le seguenti linee:

*     MISO (Master In Slave Out) - Usata dallo Slave per inviare dati;
*     MOSI (Master Out Slave In) - Usata per l'invio dati da parte del Master
*     SCK (Serial Clock) - Per sincronizzare la comunicazione dati
*     SS (Slave Select) - Usato dal master per attivare o disattivare ciascuna periferica slave. 

<table cellpadding="2" cellspacing="0" width="743">
    <thead>
    <tr>
      <th>PIN Arduino Mega</th>
      <th>Nome</th>
      <th>PIN Shield</th>
    </tr>
    </thead>
    <tbody>
        <tr>
            <td style="width: 33.3333%;">53</td>
            <td style="width: 33.3333%;">Slave Select (SS)</td>
            <td style="width: 33.3333%;">CS</td>
        </tr>
        <tr>
            <td style="width: 33.3333%;">51 o ICSP-4</td>
            <td style="width: 33.3333%;">Master Out Slave In (MOSI)</td>
            <td style="width: 33.3333%;">DI - MOSI</td>
        </tr>
        <tr>
            <td style="width: 33.3333%;">50 o ICSP-1</td>
            <td style="width: 33.3333%;">Master In Slave Out (MISO)</td>
            <td style="width: 33.3333%;">DO - MISO</td>
        </tr>
        <tr>
            <td style="width: 33.3333%;">52 o ICSP-3</td>
            <td style="width: 33.3333%;">System Clock (SCK)</td>
            <td style="width: 33.3333%;">CLK -SCK</td>
        </tr>
        <tr>
            <td style="width: 33.3333%;">+5v</td>
            <td style="width: 33.3333%;">Vcc</td>
            <td style="width: 33.3333%;">Vcc</td>
        </tr>
        <tr>
            <td style="width: 33.3333%;"></td>
            <td style="width: 33.3333%;">Gnd</td>
            <td style="width: 33.3333%;">GND</td>
        </tr>


    </tbody>
 </table>

