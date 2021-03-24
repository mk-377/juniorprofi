#include <PulseSensorPlayground.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#define SDPIN  4
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 32, 66);
String request;
EthernetServer server(80);
#define LOPLUS 7
#define LOMINUS 6
#define LEDRED 3
#define LEDGREEN 2
#define PULSEPIN A0
#define ECGPIN A1
#define ZOOMER 5
boolean stateEKG = true;
boolean statePulse = true;


void setup() {
  Serial.begin(9600); // Подключаем библиотеку
  pinMode(LOPLUS, INPUT);
  pinMode(LOMINUS, INPUT); // Настройка выходов датчика ЭКГ
  pinMode(LEDRED, OUTPUT);
  pinMode(LEDGREEN, OUTPUT); // Настройка светодиодов
  pinMode(ZOOMER, OUTPUT); // Настройка пищалки
  Ethernet.begin(mac, ip);
  if (!SD.begin(SDPIN)){Serial.println("SD NC");}
  else{Serial.println("SD OK");}
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF){Serial.println("Ethernet cable is connected.");}
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void(* resetFunc) (void) = 0;
void(* resetFuncAgain) (void) = 0;

void loop()
{
  EthernetClient client = server.available();
  uint32_t nowpulse = millis();
  uint32_t nowecg = millis();
  digitalWrite(LEDRED, HIGH); // сигнал об отсутствии работ
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (request.length() < 100)
        {
          request += c;
          Serial.print(c);
        }
        if (c == '\n') { int bitt = 0;
          HTTP(client);
          if (request.indexOf("StartPulseMetr") > 0)
          {
            digitalWrite(LEDRED, LOW);
            digitalWrite(LEDGREEN, HIGH);
            statePulse = false;
            if (statePulse == false){client.println("PulseMetr: in work");}
            else{client.println("PulseMetr: ready");}
            while (millis() - nowpulse < 15000)
            {
              int pulse = analogRead(PULSEPIN);
              Serial.println(pulse);
              if (pulse > 600){tone(ZOOMER, 1024); bitt += 1;}
              else{noTone(ZOOMER);}
              //
              delay(10);
            }
            client.println(PAGE_Graph(bitt));
            resetFunc();
          }
          if (request.indexOf("StartEKG") > 0)
          {
            digitalWrite(LEDRED, LOW);
            digitalWrite(LEDGREEN, HIGH);
            stateEKG = false;
            client.println("PulseMetr: in work");
            if (stateEKG == false){client.println("EKG: in work");}
            else{client.println("EKG: ready");}
            while (millis () - nowecg < 15000)
            {
              if ((digitalRead(LOPLUS) == 1) || (digitalRead(LOMINUS) == 1)){Serial.println("Error");}
              else
              {
                Serial.println(analogRead(ECGPIN));
                tone(ZOOMER, 1024);
                delay(10);
                noTone(ZOOMER);
                delay(10);
              }
              delay(10);
            }
            resetFuncAgain();
          }
          delay(1);
          client.stop();
          request = "";
        }
      }
    }
  }
}
void HTTP(EthernetClient client)
{
  client.println("<html>");
  client.println("<head>");
  client.println("MedBatya");
  client.println("</head>");
  client.println("</br>");
  client.println("<FORM ACTION='' method=get >");
  client.println("<input class=a type=submit value=StartEKG name='EKG'>");
  client.println("<input class=a type=submit value=StartPulseMetr name='PulseMetr'>");
  client.println("</FORM>");
  client.println("</html>");
}
