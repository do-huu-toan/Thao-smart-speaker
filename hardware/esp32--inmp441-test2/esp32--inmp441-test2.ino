#include <driver/i2s.h>

#include <SD.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "recordLib.h"


bool isWIFIConnected = false;
File file;

//Ket noi wifi:
void connectToWifi()
{
  isWIFIConnected = false;
  char* ssid = "HYPERLOGY-Guest-02";
  char* password = "hyperlogy2019";

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  isWIFIConnected = true;
}

const char filename[] = "/recording.wav";
void SD_Init() {
if (!SD.begin()) Serial.println("SD begin failed");
  while(!SD.begin()){
    Serial.print(".");
    delay(500);
  }
  SD.remove(filename);
  file = SD.open(filename, FILE_WRITE);
  
  if (!file) {
    Serial.println("File is not available!");
  }
  byte header[headerSize];
  wavHeader(header, FLASH_RECORD_SIZE);

  file.write(header, headerSize);
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SD_Init();
  i2sInit();
  connectToWifi();
  Serial.println("Connected");
  delay(5000);
  Serial.println("Bat dau recording");
  delay(500);
  i2s_adc(file);
  uploadFile();
}

void loop() {
  // put your main code here, to run repeatedly:

}

void uploadFile(){
  file = SD.open(filename, FILE_READ);
  if(!file){
    Serial.println("FILE IS NOT AVAILABLE!");
    return;
  }

  Serial.println("===> Upload FILE to Node.js Server");

  HTTPClient client;
  client.begin("http://172.20.30.181:8888/uploadAudio");
  client.addHeader("Content-Type", "audio/wav");
  int httpResponseCode = client.sendRequest("POST", &file, file.size());
  Serial.print("httpResponseCode : ");
  Serial.println(httpResponseCode);

  if(httpResponseCode == 200){
    String response = client.getString();
    Serial.println("==================== Transcription ====================");
    Serial.println(response);
    Serial.println("====================      End      ====================");
  }else{
    Serial.println("Error");
  }
  file.close();
  client.end();
}

//----------I2S-----------------------------------
