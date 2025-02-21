TaskHandle_t USt;
TaskHandle_t HBt;

#include <WiFi.h>
#include <NewPing.h>
NewPing sonar(13, 35, 20);
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <ElegantOTA.h>
#include <ESPDash.h>

const char* ssid = "........";
const char* password = "........";

AsyncWebServer server(80);
ESPDash dashboard(&server);
Preferences preferences;

unsigned long ota_progress_millis = 0;

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}

int d = 0;
int o = 0;
int ld = 0;
int fdel = 0; //forward delay
int msp = 0; // motor speed
int ldel = 0; //left delay
int rdel = 0; //right delay
int s = 0;

Card onoff(&dashboard, BUTTON_CARD, "On/Off");
Card fordel(&dashboard, SLIDER_CARD, "forward delay", "", 0, 10000);
Card forsp(&dashboard, SLIDER_CARD, "motor speed", "", 0, 255);
Card lsdel(&dashboard, SLIDER_CARD, "left delay", "", 0, 5000);
Card rsdel(&dashboard, SLIDER_CARD, "right delay", "", 0, 5000);

void setup() {
  preferences.begin("my-app", false);
  fdel = preferences.getInt("fdel", 1000);
  msp = preferences.getInt("msp", 255);
  ldel = preferences.getInt("ldel", 0);
  rdel = preferences.getInt("rdel", 0);

  onoff.attachCallback([&](int value){
    Serial.println(value);
    s = value;
    onoff.update(s);
    dashboard.sendUpdates();
});
    onoff.update(s);
    /* Attach Slider Callback */
  fordel.attachCallback([&](int value){
    Serial.println(value);
    /* Print our new slider value received from dashboard */
    fdel = value;
    preferences.putInt("fdel", value);
    /* Make sure we update our slider's value and send update to dashboard */
    fordel.update(value);
    dashboard.sendUpdates();
  });

    /* Attach Slider Callback */
  forsp.attachCallback([&](int value){
    Serial.println(value);
    /* Print our new slider value received from dashboard */
    msp = value;
    preferences.putInt("msp", value);
    /* Make sure we update our slider's value and send update to dashboard */
    forsp.update(value);
    dashboard.sendUpdates();
  });

    /* Attach Slider Callback */
  lsdel.attachCallback([&](int value){
    Serial.println(value);
    /* Print our new slider value received from dashboard */
    ldel = value;
    preferences.putInt("ldel", value);
    /* Make sure we update our slider's value and send update to dashboard */
    lsdel.update(value);
    dashboard.sendUpdates();
  });

    /* Attach Slider Callback */
  rsdel.attachCallback([&](int value){
    Serial.println(value);
    /* Print our new slider value received from dashboard */
    rdel = value;
    preferences.putInt("rdel", value);
    /* Make sure we update our slider's value and send update to dashboard */
    rsdel.update(value);
    dashboard.sendUpdates();
  });

  WiFi.softAP(ssid, password);
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! This is ElegantOTA AsyncDemo.");
  });

  ElegantOTA.begin(&server);    // Start ElegantOTA
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  server.begin();
  Serial.println("HTTP server started");

  pinMode(25, OUTPUT);//PWMA
  pinMode(15, OUTPUT);//AIN1
  pinMode(16, OUTPUT);//AIN2
  pinMode(23, OUTPUT);//BIN1
  pinMode(18, OUTPUT);//BIN2
  pinMode(26, OUTPUT);//PWMB
  pinMode(19, OUTPUT);//seeddmotor
  pinMode(21, OUTPUT);//seedumotor
  pinMode(32, OUTPUT);//seedpmotor
  Serial.begin(115200); 
  pinMode(33, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(27, INPUT_PULLUP);//seed up
  pinMode(14, INPUT_PULLUP);//seed down

  xTaskCreate(
    USc,     // Task function
    "UltrasonicPing",      // Name of the task
    1024,          // Stack size (in words, not bytes)
    NULL,          // Task input parameter
    2,             // Priority of the task (higher priority)
    &USt        // Task handle
    );            // Core to run the task on (Core 0)

  xTaskCreate(
    HBc,     // Task function
    "H-Bridge Control",      // Name of the task
    2048,          // Stack size (in words, not bytes)
    NULL,          // Task input parameter
    2,             // Priority of the task (higher priority)
    &HBt        // Task handle
    );            // Core to run the task on (Core 1)   
  dashboard.sendUpdates();
}
void USc(void * pvParameters) {
  for (;;) {
    d = sonar.ping_cm();
    if(d>0){
      o = 1;
    }
    //Serial.println(d);
    vTaskDelay(3);
  }
}

void HBc(void * pvParameters) {
  for (;;) {
    vTaskDelay(1);
    if (s==1){
      vTaskDelay(1);
    if (o==0){
    dd(); //drill down
    du(); //drill up
    ds(); //drill stop
    sd(); 
    su();
    ss();
    front(); //front
    vTaskDelay(fdel);
    sto(); //stop
  }else if ((o>0)&&(ld==0)){
    left();
    vTaskDelay(ldel);
    o = 0;
  }else if ((o>0)&&(ld==1)){
    right();
    vTaskDelay(ldel);
    o = 0;
  }    
  sto();
  }}
}

void loop() {
  ElegantOTA.loop();
}

void sd(){
  digitalWrite(4, HIGH);
  digitalWrite(22, LOW);
  analogWrite(33, 100);
  vTaskDelay(320);
}

void su(){
  digitalWrite(4, LOW);
  digitalWrite(22, HIGH);
  analogWrite(33, 100);
  vTaskDelay(300);
}

void ss(){
  digitalWrite(4, LOW);
  digitalWrite(22, LOW);
  analogWrite(33, 100);
}

void dd(){
  while(digitalRead(14)==HIGH){
  digitalWrite(19, HIGH);
  digitalWrite(21, LOW);
  analogWrite(32, 255);
  vTaskDelay(1);
}
}

void du(){
  while(digitalRead(27)==HIGH){
  digitalWrite(19, LOW);
  digitalWrite(21, HIGH);
  analogWrite(32, 255);
  vTaskDelay(1);
  }
}

void ds(){
  digitalWrite(19, LOW);
  digitalWrite(21, LOW);
  analogWrite(32, 0);
}

void left() {
  ld = 1;
  Serial.println("left");
  digitalWrite(15, HIGH);
  digitalWrite(16, LOW);
  digitalWrite(23, HIGH);
  digitalWrite(18, LOW);
  analogWrite(25, 10);
  analogWrite(26, 255);
}

void right() {
  ld = 0;
  Serial.println("right");
  digitalWrite(15, HIGH);
  digitalWrite(16, LOW);
  digitalWrite(23, HIGH);
  digitalWrite(18, LOW);
  analogWrite(25, 255);
  analogWrite(26,10);
}

void front() {
  Serial.println("front");
  digitalWrite(15, HIGH);
  digitalWrite(16, LOW);
  digitalWrite(23, HIGH);
  digitalWrite(18, LOW);
  analogWrite(25, msp);
  analogWrite(26, msp);
}

void back() {
  Serial.println("back");
  digitalWrite(15, LOW);
  digitalWrite(16, HIGH);
  digitalWrite(23, LOW);
  digitalWrite(18, HIGH);
  analogWrite(25, 255);
  analogWrite(26, 255);
}
void sto(){
  Serial.println("stop");
  digitalWrite(15, HIGH);
  digitalWrite(16, HIGH);
  digitalWrite(23, HIGH);
  digitalWrite(18, HIGH);
  analogWrite(25, 0);
  analogWrite(26, 0);
}

  
  /*
  Serial.println("front");
  front();
  Serial.println(sonar.ping_cm());
  vTaskDelay(1000);
  Serial.println("left");
  left();
  Serial.println(sonar.ping_cm());
  vTaskDelay(1000);  
  Serial.println("right");
  right();
  Serial.println(sonar.ping_cm());
  vTaskDelay(1000);
  Serial.println("back");
  back();
  Serial.println(sonar.ping_cm());
  vTaskDelay(1000);
  */
