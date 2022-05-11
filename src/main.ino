#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "imagedata.h"
#include <stdlib.h>
#include <codebare.hpp>
#include <WiFiManager.h>

std::unique_ptr<WebServer> server;


int printline(int j,int size, int* barcode, int pencile){
    for (int i = (size-1); i >= 0; i-=1) {
        if ((barcode[j] & 1 << i) != 0){
            printf("1");
            Paint_DrawLine(246-pencile*2, 20, 246-pencile*2, 120, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        }
        else
            printf("0");
        pencile++;
    }
    return pencile;
}

void generateEan13(String Gencode, int time){
    UBYTE *BlackImage;
    /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
    UWORD Imagesize = ((EPD_2IN9_V2_WIDTH % 8 == 0)? (EPD_2IN9_V2_WIDTH / 8 ): (EPD_2IN9_V2_WIDTH / 8 + 1)) * EPD_2IN9_V2_HEIGHT;
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        while(1);
    }

    printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_2IN9_V2_WIDTH, EPD_2IN9_V2_HEIGHT, 270, WHITE);
    BarCode codebar = BarCode(Ean13);
    int barcode[15];
    codebar.EanToBit(Gencode,barcode);
    printf("Drawing\r\n");
    //1.Select Image
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    // 2.Drawing on the image
    printf("Drawing:BlackImage\r\n");
    int pencile = 0;
    for(int j = 0; j < 15; j++){
        if(j==0||j==14){
            pencile = printline(j,3,barcode,pencile);
            continue;
        }
        else if(j==7){
            pencile = printline(j,5,barcode,pencile);
            continue;
        }
        else{
            pencile = printline(j,7,barcode,pencile);
            continue;
        }
    }
    printf("\n");

    EPD_2IN9_V2_Display_Base(BlackImage);
    free(BlackImage);
    BlackImage = NULL;
    DEV_Delay_ms(time);
    printf("Clear...\r\n");
    EPD_2IN9_V2_Init();
    EPD_2IN9_V2_Clear();
}

String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}


void handleRoot() {
  server->send(200, "text/plain", "hello from esp8266!");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server->uri();
  message += "\nMethod: ";
  message += (server->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server->args();
  message += "\n";
  for (uint8_t i = 0; i < server->args(); i++) {
    message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
  }
  server->send(404, "text/plain", message);
}

void ean13(){
    String gencode;
    int time = 10000;
    for (uint8_t i = 0; i < server->args(); i++) {
        if(server->argName(i)=="ean")
            gencode = server->arg(i);
        if(server->argName(i)=="time")
            time = (server->arg(i)).toInt();
    }
    if(gencode.length() != 13) {
        server->send(403, "text/plain", "gencode pas valide");
        return;
    }
    generateEan13(gencode,time);
    server->send(200, "text/plain", "OK");
}

void setup()
{

    WiFi.mode(WIFI_STA);
	printf("EPD_2IN9_V2_test Demo\r\n");
	DEV_Module_Init();
    printf("e-Paper Init and Clear...\r\n");
    EPD_2IN9_V2_Init();
    EPD_2IN9_V2_Clear();
    DEV_Delay_ms(500);

    WiFiManager wm;
    bool res;
    res = wm.autoConnect("SmartGencode");

    if(!res) {
        Serial.println("Failed to connect");
        wm.resetSettings();
        ESP.restart();
    }
    else {
        Serial.println("connected...yeey :)");
        UBYTE *BlackImage;
        /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
        UWORD Imagesize = ((EPD_2IN9_V2_WIDTH % 8 == 0)? (EPD_2IN9_V2_WIDTH / 8 ): (EPD_2IN9_V2_WIDTH / 8 + 1)) * EPD_2IN9_V2_HEIGHT;
        if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
            printf("Failed to apply for black memory...\r\n");
            while(1);
        }
        Paint_NewImage(BlackImage, EPD_2IN9_V2_WIDTH, EPD_2IN9_V2_HEIGHT, 270, WHITE);

        Paint_SelectImage(BlackImage);
        Paint_Clear(WHITE);

        Paint_DrawString_EN(10, 0, "IP : ", &Font16, BLACK, WHITE);
        Paint_DrawString_EN(10, 20,IpAddress2String(WiFi.localIP()).c_str(), &Font12, WHITE, BLACK);
        EPD_2IN9_V2_Display_Base(BlackImage);
        DEV_Delay_ms(2000);
    }

    server.reset(new WebServer(WiFi.localIP(), 80));

    server->on("/", handleRoot);

    server->on("/inline", []() {
        server->send(200, "text/plain", "this works as well");
    });

    server->on("/ean13", ean13);

    server->onNotFound(handleNotFound);

    server->begin(80);

    // generateEan13("1234567891231",10000);
}


void loop()
{
  server->handleClient();
}
