#include <ESP8266WiFi.h>

//Please replace with your wifi network and password. 
const char* ssid     = "MY_WIFI_NETWORK";
const char* password = "MY_WIFI_PASSWORD";

const char* host = "services.swpc.noaa.gov";

//BULK SPEED PINS
const int BLUEPIN = 12;
const int REDPIN = 15;
const int GREENPIN = 13;

//DENSITY PINS
const int BLUEPIN1 = 0;
const int GREENPIN1 = 16;
const int REDPIN1 = 2;

//ION TEMPERATURE PINS
const int BLUEPIN2 = 14;
const int GREENPIN2 = 5;
const int REDPIN2 = 4;

const float SPEEDMULT = .51;
const float DENSMULT = 20.4;
const float TEMPMULT = 0.051;

void setup() {
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(REDPIN1, OUTPUT);
  pinMode(GREENPIN1, OUTPUT);
  pinMode(BLUEPIN1, OUTPUT);
  pinMode(REDPIN2, OUTPUT);
  pinMode(GREENPIN2, OUTPUT);
  pinMode(BLUEPIN2, OUTPUT);

  Serial.begin(115200);
  delay(100);

  // connect to wifi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() {
  delay(5000);

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // create URI for request
  String url = "/text/ace-swepam.txt";
  Serial.print("Requesting URL: ");
  Serial.println(url);

  // send request
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  delay(500);
  int num =  0;
  String trueLine = "";
  // print to serial 
  while (client.available()) {
    String line = client.readStringUntil('\n');
    num++;
    if (num == 156) {
      trueLine = line;
    }
  }
  Serial.println(trueLine);

  int statusInt = getStatus(trueLine);
  float protDensity = getProtDens(trueLine);
  float bulkSpeed = getBulkSpeed(trueLine);
  float ionTemp = getIonTemp(trueLine);
  int ionTempPower = getIonTempPower(trueLine);
  ionTempPower = ionTempPower - 2;

  int ionTempTrue = ionTemp * pow(10, ionTempPower);

  Serial.println("closing connection");
  if (statusInt != 0) {
    badData();
  } else {
    /**
     * Write the corresponding colors to LED 0 for the Bulk Speed.
     */
    int blueVal = getBlueVal(bulkSpeed - 200, SPEEDMULT);
    int greenVal = getGreenVal(bulkSpeed - 200, SPEEDMULT);
    int redVal = getRedVal(bulkSpeed - 200, SPEEDMULT);
    Serial.print("blueval ");
    Serial.println(blueVal);
    Serial.print("greenval ");
    Serial.println(greenVal);
    Serial.print("redval ");
    Serial.println(redVal);
    analogWrite(BLUEPIN, blueVal);
    analogWrite(GREENPIN, greenVal);
    analogWrite(REDPIN, redVal);

    /**
     * Write the corresponding colors to LED 2 for ion temperature. 
     */
    int blueVal2 = getBlueVal(ionTempTrue - 100, TEMPMULT);
    int greenVal2 = getGreenVal(ionTempTrue - 100, TEMPMULT);
    int redVal2 = getRedVal(ionTempTrue - 100, TEMPMULT);
    Serial.print("blueval2 ");
    Serial.println(blueVal2);
    Serial.print("greenval2 ");
    Serial.println(greenVal2);
    Serial.print("redval2 ");
    Serial.println(redVal2);
    analogWrite(BLUEPIN2, blueVal2);
    analogWrite(GREENPIN2, greenVal2);
    analogWrite(REDPIN2, redVal2);

    /**
     * Begin the color cycling for the proton density. 
     */
    rainbow(protDensity);

  }//else if its working
}//loop


int getStatus(String orig) {
  char statusChar = orig.charAt(36);
  int statusInt = statusChar - '0';
  Serial.print("Status integer: ");
  Serial.println(statusInt);
  return statusInt;
}


float getProtDens(String orig) {
  String protDens = orig.substring(41, 48);
  float protDensI = protDens.toFloat();
  Serial.println("proton density: " + protDens);
  Serial.print("proton density integer: ");
  Serial.println(protDensI);
  return protDensI;
}


float getBulkSpeed(String orig) {
  String bulkSpeed = orig.substring(52, 59);
  float bulkSpeedI = bulkSpeed.toFloat();
  Serial.println("bulk speed: " + bulkSpeed);
  Serial.print("bulk speed integer: ");
  Serial.println(bulkSpeedI);
  return bulkSpeedI;
}


float getIonTemp(String orig) {
  String ionTemp = orig.substring(63, 72);
  String ionTemp1 = ionTemp.substring(0, 5);
  float ionTempI = ionTemp1.toFloat();
  Serial.println("Ion temperature: " + ionTemp1);
  Serial.print("Ion temperature integer: ");
  Serial.println(ionTempI);
  return ionTempI;
}


int getIonTempPower(String orig) {
  String ionTemp = orig.substring(63, 72);
  String ionTemp1 = ionTemp.substring(8, 10);
  int ionTempI = ionTemp1.toInt();
  Serial.println("Ion temperature power: " + ionTemp1);
  Serial.print("Ion temperature power integer: ");
  Serial.println(ionTempI);
  return ionTempI;
}


int getGreenVal(int num, float mult) { 
  int greenVal = 0;
  int newnum = mult * num;
  Serial.print("greenvalmethod ");
  Serial.println(newnum);
  if (newnum < 256) {
    greenVal = newnum;
  }
  if (newnum >= 256) {
    greenVal = 510 - newnum;
  }
  return greenVal;
}//getGreenVal1


int getBlueVal(int num, float mult) { 
  int blueVal = 0;
  int newnum = mult * num;
  Serial.print("bluevalmethod ");
  Serial.println(newnum);
  if (newnum < 256) {
    blueVal = 255 - newnum;
  }
  return blueVal;
}//getBlueVal1


int getRedVal(int num, float mult) {
  int redVal = 0;
  int newnum = mult * num;
  Serial.print("redvalmethod ");
  Serial.println(newnum);
  if (newnum >= 256) {
    redVal = newnum;
  }
  return redVal;
}

void badData() {
  analogWrite(BLUEPIN, 0);
  analogWrite(GREENPIN, 0);
  analogWrite(BLUEPIN1, 0);
  analogWrite(GREENPIN1, 0);
  analogWrite(BLUEPIN2, 0);
  analogWrite(GREENPIN2, 0);
  for (int j = 0; j < 6; j++) {
    for (int i = 255; i > 0; i--) {
      analogWrite(REDPIN, i);
      analogWrite(REDPIN1, i);
      analogWrite(REDPIN2, i);
      delay(20);
    }
    for (int i = 0; i < 255; i++) {
      analogWrite(REDPIN, i);
      analogWrite(REDPIN1, i);
      analogWrite(REDPIN2, i);
      delay(20);
    }
  }
}

void rainbow(int val) {
  int num = 60000;
  if(val!=0){
    num = 60000 / val;
  }else{
    num = 60000;
  }
  int num1 = num / 510;
  analogWrite(REDPIN1, 255);
  for (int i=0;i<val;i++) {
    for (int i = 0; i < 255; i++) {
      analogWrite(GREENPIN1, i);
      analogWrite(REDPIN1, 255 - i);
      delay(num1);
    }
    for (int i = 0; i < 255; i++) {
      analogWrite(GREENPIN1, 255 - i);
      analogWrite(BLUEPIN1, i);
      delay(num1);
    }
    for (int i = 0; i < 255; i++) {
      analogWrite(BLUEPIN1, 255 - i);
      analogWrite(REDPIN1, i);
      delay(num1);
    }
  }

}
