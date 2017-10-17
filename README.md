# ubimultiupload
Ubidot Multi Sensor Upload

Progetto per Arduino con Shield Wify ESP8266 di .

Questo progetto consente l'upload sul servizio Ubidots di dati provenienti da diversi sensori connessi ad Arduino.

Per la connessione a Internet verrà utilizzato uno Shield Wifi basato su ESP8266; si tratta di uno shield a basso costo configurabile tramite una serie di switch sulla scheda.

Un problema di questo shield è che rende impossibile usare l'interfaccia USB di Arduino per fare debug. Lo shield in modalità di funzionamento normale (Swith 1 e 2 a ON) comunica con Arduino tramite i pin 0 e 1, cioè TX e RX della prima interfaccia Seriale di Arduino. Tale interfaccia tuttavia è condivisa con quella USB.

Per ovviare a questo problema ci sono due strade:

# usare shield e arduino in modo indipendente (tutti gli switch a OFF): per far comunicare Arduino con lo shield è necessario ponticellare i pin RX e TX del connettore di debug dello shield, rispettivamente con i pin TX e RX di un'altra seriale fisica di arduino. Qualora l'arduino diponga di un asola interfaccia seriale fisica usare la libreria SoftSerial e usare dei Pin disponbili di arduino.

# con gli swith 1 e 2 a ON dello shield: piegare i PIN0 e PIN1 dello shield in modo che non siano connessi ad Arduino. Ponticellare i PIN0 (TX) e PIN1 (RX) dello shield come nel caso precedente.

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

