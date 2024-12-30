#define DEBUG
#include <WiFiEsp.h>
#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <Servo.h>
#include <Stepper.h>
#include <DHT.h>

#define AP_SSID "KCCI_STC_S"
#define AP_PASS "kcci098#"
#define SERVER_NAME "10.10.14.41"
#define SERVER_PORT 5000
#define LOG_ID "BMS_1F"
#define FLOOR_ID 1
#define PASSWD "PASSWD"

#define WIFI_RX 8  
#define WIFI_TX 7

#define DHT_PIN 4
#define DHT_TYPE DHT11
#define CDS_PIN A0

#define FAN_PIN 6
//#define BLIND_PIN 5
#define LAMP_PIN 13
#define WINDOW_PIN 5

#define CMD_SIZE 50
#define ARR_CNT 5

#define SPIN 2048

#define BLIND_IN1 9
#define BLIND_IN2 10
#define BLIND_IN3 11
#define BLIND_IN4 12


bool timerIsrFlag = false;

char sendId[10] = "BMS_SQL";
char sendBuf[CMD_SIZE];

int cds = 0;
unsigned int secCount;
unsigned int myservoTime = 0;

float temp = 0.0;
float humi = 0.0;

DHT dht(DHT_PIN, DHT_TYPE);
SoftwareSerial wifiSerial(WIFI_RX, WIFI_TX);
WiFiEspClient client;
Servo myservo;
Stepper myStepper(SPIN, BLIND_IN1, BLIND_IN3, BLIND_IN2, BLIND_IN4);

void setup() {
    pinMode(CDS_PIN, INPUT);   
    pinMode(LAMP_PIN, OUTPUT); 
    pinMode(FAN_PIN, OUTPUT);  

#ifdef DEBUG
    Serial.begin(115200); 
#endif
    wifi_Setup();
    MsTimer2::set(1000, timerIsr); 
    MsTimer2::start();

    myservo.attach(WINDOW_PIN);
    myservo.write(0);
    myservoTime = secCount;

    myStepper.setSpeed(10);

    dht.begin();
    actuator_init();
}

void loop() {
    if (client.available()) {
        socketEvent();
    }

    if (timerIsrFlag) {
        timerIsrFlag = false;

        if (!(secCount % 3)) {
            cds = map(analogRead(CDS_PIN), 0, 1023, 0, 100);
            humi = dht.readHumidity();
            temp = dht.readTemperature();

            sprintf(sendBuf, "[%s]SENSOR@%dF@%d@%d@%d\n", sendId, FLOOR_ID, cds, (int)humi, (int)temp);
            client.write(sendBuf, strlen(sendBuf));
            client.flush();

#ifdef DEBUG
            Serial.print("Cds: ");
            Serial.print(cds);
            Serial.print(" Humidity: ");
            Serial.print(humi);
            Serial.print(" Temperature: ");
            Serial.println(temp);
#endif
        }

        if (myservo.attached() && myservoTime + 2 == secCount) {
            myservo.detach();
        }
    }
}

void timerIsr() {
    timerIsrFlag = true;
    secCount++;
}

void actuator_init() {
    sprintf(sendBuf, "[%s]SET@%dF@FAN@OFF\n", sendId, FLOOR_ID);
    client.write(sendBuf, strlen(sendBuf));
    client.flush();

    sprintf(sendBuf, "[%s]SET@%dF@BLIND@DOWN\n", sendId, FLOOR_ID);
    client.write(sendBuf, strlen(sendBuf));
    client.flush();

    sprintf(sendBuf, "[%s]SET@%dF@LAMP@OFF\n", sendId, FLOOR_ID);
    client.write(sendBuf, strlen(sendBuf));
    client.flush();

    sprintf(sendBuf, "[%s]SET@%dF@WINDOW@CLOSE\n", sendId, FLOOR_ID);
    client.write(sendBuf, strlen(sendBuf));
    client.flush();
}

void blind_stop() {
    digitalWrite(BLIND_IN1, LOW);
    digitalWrite(BLIND_IN2, LOW);
    digitalWrite(BLIND_IN3, LOW);
    digitalWrite(BLIND_IN4, LOW);
}

void wifi_Setup() {
    wifiSerial.begin(38400);
    wifi_Init();
    server_Connect();
}

void wifi_Init()
{
  do {
    WiFi.init(&wifiSerial);
    if (WiFi.status() == WL_NO_SHIELD) {
#ifdef DEBUG_WIFI
      Serial.println("WiFi shield not present");
#endif
    }
    else
      break;
  } while (1);
#ifdef DEBUG_WIFI
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(AP_SSID);
#endif
  while (WiFi.begin(AP_SSID, AP_PASS) != WL_CONNECTED) {
#ifdef DEBUG_WIFI
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(AP_SSID);
#endif
  }
  // sprintf(lcdLine1, "ID:%s", LOG_ID);
  // lcdDisplay(0, 0, lcdLine1);
  // sprintf(lcdLine2, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  // lcdDisplay(0, 1, lcdLine2);

#ifdef DEBUG_WIFI
  Serial.println("You're connected to the network");
  printWifiStatus();
#endif
}
int server_Connect()
{
#ifdef DEBUG_WIFI
  Serial.println("Starting connection to server...");
#endif

  if (client.connect(SERVER_NAME, SERVER_PORT)) {
#ifdef DEBUG_WIFI
    Serial.println("Connect to server");
#endif
    client.print("["LOG_ID":"PASSWD"]");
  }
  else
  {
#ifdef DEBUG_WIFI
    Serial.println("server connection failure");
#endif
  }
}
void socketEvent() {
    char recvBuf[CMD_SIZE] = {0};
    char *pArray[ARR_CNT] = {0};
    int len;

    len = client.readBytesUntil('\n', recvBuf, CMD_SIZE);
    client.flush();

#ifdef DEBUG
    Serial.print("recv : ");
    Serial.println(recvBuf);
#endif

    int i = 0;
    char *pToken = strtok(recvBuf, "[@]");
    while (pToken != NULL) {
        pArray[i] = pToken;
        if (++i >= ARR_CNT) break;
        pToken = strtok(NULL, "[@]");
    }

    if (!strcmp(pArray[1], "SET")) {
        if (!strcmp(pArray[3], "FAN")) 
        {
            int speed = 120;
            analogWrite(FAN_PIN,!strcmp(pArray[4], "ON") ? speed : LOW);
            //digitalWrite(FAN_PIN, !strcmp(pArray[4], "ON") ? HIGH : LOW);
        } 
        else if (!strcmp(pArray[3], "BLIND")) 
        {
            //digitalWrite(BLIND_PIN, !strcmp(pArray[4], "ON") ? HIGH : LOW);
            if(!strcmp(pArray[4], "UP"))
            {
              myStepper.step(SPIN);
              blind_stop();
            }
            else if(!strcmp(pArray[4], "DOWN"))
            {
              myStepper.step(-SPIN);
              blind_stop();
            }
        } 
        else if (!strcmp(pArray[3], "LAMP")) 
        {
            digitalWrite(LAMP_PIN, !strcmp(pArray[4], "ON") ? HIGH : LOW);
        } 
        else if (!strcmp(pArray[3], "WINDOW")) 
        {
            if (!strcmp(pArray[4], "OPEN")) {
                myservo.attach(WINDOW_PIN);
                myservo.write(90); // Example: Move servo to open position
            } else {
                myservo.attach(WINDOW_PIN);
                myservo.write(0); // Example: Move servo to closed position
            }
        }
        myservoTime = secCount;
    }
}
