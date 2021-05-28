#include <sd_defines.h>
#include <sd_diskio.h>

#include "Arduino.h"
#include <FS.h>
#include "Wav.h"
#include "I2S.h"
#include <SD.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define I2S_MODE I2S_MODE_ADC_BUILT_IN

const int record_time = 10;  // second
const int headerSize = 44;
const int waveDataSize = record_time * 88000;
const int numCommunicationData = 8000;
const int numPartWavData = numCommunicationData / 4;
byte header[headerSize];
char communicationData[numCommunicationData];
char partWavData[numPartWavData];
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
//Khoi tao SPIFFS:
const char filename[] = "/recording.wav";
void SD_Init() {
if (!SD.begin()) Serial.println("SD begin failed");
  while(!SD.begin()){
    Serial.print(".");
    delay(500);
  }
  CreateWavHeader(header, waveDataSize);
  SD.remove(filename);
  file = SD.open(filename, FILE_WRITE);
  
  if (!file) {
    Serial.println("File is not available!");
  }
  Serial.println("Bat dau ghi...");
  file.write(header, headerSize);
  I2S_Init(I2S_MODE, I2S_BITS_PER_SAMPLE_32BIT);
  for (int j = 0; j < waveDataSize / numPartWavData; ++j) {
    I2S_Read(communicationData, numCommunicationData);
    for (int i = 0; i < numCommunicationData / 8; ++i) {
      partWavData[2 * i] = communicationData[8 * i + 2];
      partWavData[2 * i + 1] = communicationData[8 * i + 3];
    }
    file.write((const byte*)partWavData, numPartWavData);
  }
  file.close();
}


void setup() {
  Serial.begin(115200);

  //Ket noi wifi:
  connectToWifi();
  Serial.println("Da ket noi wifi thanh cong!");
  //Khoi tao SPIFFS:
  Serial.println("Mo File");
  SD_Init();
  Serial.println("Ghi xong");
  Serial.println("Bat dau upload....");
  uploadFile();
  Serial.println("finish");
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
