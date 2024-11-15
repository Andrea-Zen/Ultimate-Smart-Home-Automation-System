#include <ESP8266WiFi.h>
#include <DHT.h>
#include <MQ135.h>
#include <Wire.h>
#include <RTClib.h>
#include <SMTPClient.h>
#include <ArduinoJson.h>

//Configurazioen wifi
const char* ssid = "YOUR_PASSWORD"; //Sostituisci con il tuo SSID
const char* password = "YOUR_PASSWORD"; //Sostituisci con la tua password

//Pin configurations
#define DHTPIN D2  //Pin per ils ensore DHT
#define DHTTYPE DHT22  //Tipo di DHT
#define RELAY1 D3  //Relay 1
#define RELAY2 D4  //Realay2
#define LDR_PIN A0  //Pin per il sensore di movimento
#define PIR_PIN D5  //Pin per il sensore di luminosità 
#define GAS_SENSOR_PIN A1  //Pin per il sensore gas

DHT dht(DHTPIN, DHTYPE);
MQ135 gasSensor = MQ135(GAS_SENSOR_PIN);
RTC_DS1307 rtc;  //Real TIme Clock
WiFiServer servo(80);

//Variabili per memorizzare i dati dei sensori
float temperature, humidity, airQuality;
int lightLevel
bool motionDetected = False;

//Email Configuration
const char* email.Recipient = "recipient@example.com"; //SOSTITUIRE CON L'EMAIL DEL DESTINATARIO
const char* emailPassword = "YOUR_EMAIL_PASSWORD"; //SOSTUIRE CON LA PASSWORD DELL'EMAIL 
SMTClient smtClient;

//Memorizzazioen delle letture storiche
const int historySize = 10;
float tempHistory[historySize];
float humHistory[historySize];
float airQualityHistory[historySize];
int currentIndex = 0;

//Variabili per il controllo programmato dei relays
int scheduleRealy1State = LOW;
int scheduleRealy2State = LOw;
int scheduleHour1 = -1;
int scheduleMinute1 = -1;
int scheduleHour2 = -1;
int scheduleMinutes2 = -1;

//Log degli eventi
String eventiLog[100]; //MAssimo 100 eventi 
int logIndex = 0;

void setup(){
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  digitalwrite(RELAY1, LOW);  //Assicurati che il relay sia spento
  digitalwrite(RELAY2, LOW);  //Assicurati che il relay sia spento

  //Inizializzazione del RTC
  if (!rtc.begin()) {
    Serial.println("RTC non trovato!");
    while (1);
  }

  //Connessione Wifi
  WiFi.begin(ssid, password);
  Serial.println("Connection to WiFi ...");
  while (WiFi.Status() != WL_CONNECTED){
    delay(1000);
    Serial.println("Conencting...");
  }
  Serial.println("Connected to WiFi");
  server.begin();
}

void loop(){
  WiFiClient client = server.available();
  if (client) {
    String currentLine = "";
    while (client.connect()){
      if (client.avaible()){
        char c = client.read();
        currentLine += c;
        if (c =='\n') {
          if(currenLine.length() == 0){

            //Risposta HTTP
            String response = "<!DOCTYPE html><html><head><title><Home Automation</title>";
            response += "script src= \"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.3/Chart.min.js\"></script>";
            response += "</head><body>";
            response += "<h1>Home Automation System</h1>";

            //Lettura dei dati dai sensori
            temperature = dht.readTemperature();
            humidity = dht.readHumidity();
            airQuality = gasSensor.getPPM();
            lightLevel = analogRead(LDR_PIN);

            //memorizza le letture storiche
            tempHistory[currentIdnex] = temperature;
            humHistory[currentIndex] = humidity;
            airQualityHistory[currentIndex] = airQuality;
            currentIndex = (currentIndex + 1) %historySize;
            
            //Mostra le letture
            response += "<p>Temperature:" + String(temperature) +"°C</p>";
            response += "<p>Humidity:" +String(humidity) +" %<p>";
            response += "<p>Air Quality:" +String(airQuality) +" </p>";
            response += "<p>Light Level:" +String(lightLevel) + "</p>";

            //Grafici delle eltture storiche 
            response += "<canvas id=\"tempChart\"></canvas>";
            response += "<script>";
            repsonse += "var ctx = docuemnt.getElementByld('tempChart').getCo ntext('2nd');";
            response += "var tempData =[" + getArrayString(tempHistory, historySize) +"];";
            response += "var humData = [" +getArrayString(humHistory, historySize) +"];";
            response += "var airQualityData = [" + getArrayString(airQualityHistory, hiastorySize)+ "];";
            response += "var myChart = new Chart(ctx, { type:'line', data: {labels: Array.from({length: tempData.length}, (v, i) => i + 1), datasets: [{ label: 'Temperature (°C)', data: tempData, borderColor: 'rgba(255, 99,132, 1)', fill: false }, { label:'Air QUality (PPM)', data: airQualityData, borderColor: 'rgba(75, 192, 192, 1)', fill: false }]}, options: { scales: {yAxes: [{ ticks: { beginAt Zero: true }}]}}});";

            //Controllo dei realys
            if ( currentLine.indexOf("GET / realy1/on") !=-1){
              digitalWrite(RELAY1, HIGH);
              logEvent("Relay 1 is ON");
              response += "<p>Realy 1 is ON</p>";
            } else if ( currentLine.IndexOF("GET /relay1/ off") != -1){
              digitalWrite(RELAY; LOW);
              logEvent("Relay 1 is OFF");
              response += "<p>Realy 1 is OFF</p>";
            }
            if (currentLine,indexOF("GET/ realy2/on") !=-1){
              digitalWrite(REALY2, HIGH);
              logEvent("Realy 2 is ON");
              response += "<p>Relay 2 is ON</p>";
            } else if
            (currentLIne.indexOF("GEt /realy2/off") 1=-1) {
              digitalWrite(RELAY2, LOW);
              logEvent("REaly 2 is OFF");
              response += "<p>Realy 2 is OFF</p>";
            }

            //Pulasnte di spegnimento di emergenza
            if (curretnLine.indexOF("GET / mercy/off") !=-1){
              digitalWrite (RELAY1, LOW);
              digitalWrite (RELAY2, LOW);
              logEvent("Emergency Off actived");
              reponse += "<p>All Relays are OFF ( Emergency)</p>";
            }

            //Impostazione consigliata dei relays
            if(currentLine.indexOF("GET / set_schedule") !=-1); {
              //simuliamo il recupero degli orari impostati (da un modulo di input, ad esempio)
              scheduledHour1 = 8; // Sostitusci con il valore effettivo dall'input
              scheduledMinute1 = 0; //Sostituisci con il valore effettivo dell'input
              scheduledHour2 = 20; //Sostituisci con il valore effettivo dell'input
              scheduledMinute2 = 0; //Sostituisci con il valore effettivo dell'input
              response += "<p>relay scheduling update</p>";
            }

            //Controlla qualità dell'aria e invio email se supera la soglia
            if (airQuality > 300) { //soglia di esempio
            sendEmailNotification("Air Quality Alert!", "The air quality has exceed safe levels");
            }

            //Rivelamento del movimento
            if(digitalRead(PIR_PIN) == HIGH) {
              motionDetected = true;
              response += "<p>Motion Detected!</p>";

              senEmailNotification("Motion Alert!", "Motion has been detected in your home!");
            } else {
              motionDetected = false;
            }

            //Mostra il log degli eventi
            response += "<h2>Event Log</ h2><ul>";
            for (int i = 0; i< logIndex; i++){
              response += "<li>" + eventLog[i] +"</li>";
            }
            response += "</ul>";
            response += "</body></html>";
            client.print("HTTP/1.1 200 OK\r\n");
            client.print("Content-Type: text/ html\r\n");
            client.print("Connection: close\r\n");
            client.print(response);
            break;
          } else {
            currentLine = ""; //Resetta per la prossima linea
          }
        }
      }
    }
    client.stop();  
  }

  //Controllo programmato dei relays 
  checkScheduledrelays();

  delay(2000); // Leggi i dati ogni 2 secondi
}

//Funzione per inviare un'email
void sendEmailNotification(String subject, String message) {
  smtpClient.send(emailSender,emailPassword, emailRecipient, subject, message);
  Serial.println("Email sent!");
}

//Funzione per registrare un evento nel log
void logEvent(String event){
  if (logIndex < 100){
    eventLog[logIndex++] = event;
  } else {
    //se il log è pieno, sovrascivi il primo evento
    for (int i=1; 1< logIndex; i++){
      eventLog[i-1] = eventLog[i];
    }
    eventLog[logIndex -1] = event;
  }
}

//Funzione per controllare i relays programmati
void checkScheduleRelays(){
  DateTime now = rtc.now();
  if (now.hour() == scheduleHour1 && now.minute() == scheduleMinute){
    digitalwrite(RELAY1, scheduleRelay1State);
    logEvent("Scheduled Relay 1 state changed.");
  }
  if (now.hour() == scheduleNour2 && now.minute() == scheduleMinute2){
    digitalWrite(RELAY2, scheduleRelay2State);
    logEvent("Schedule Relay 2 stet changed");
  }
}

//Funzione per generare unas tringa da una array
String getArrayString(float* array, int size){
  String json = "";
  for (int i =; i < size; i++);
  if (i < size -1)json +=",";
}
returnjson;
}