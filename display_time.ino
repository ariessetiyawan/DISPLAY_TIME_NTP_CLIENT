
#include <WiFi.h>
#include "time.h"
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include <AnimatedGIF.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "time.h"
#include "images/checked.h"
#include "images/wrong.h"
#include "images/cybfr_200.h"
#include <JPEGDecoder.h>
#define GIF_IMAGE cybfr_200
#define GIF_IMAGE1 wrong
#define GIF_IMAGE2 checked
#define NORMAL_SPEED

/*
esp32   GC9A01
3V3     BLK
15      CS
2       DC
4       RES
23      SDA
18      SCL
3V3     VCC
GND     GND

*/

uint8_t newMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x64};

#define TFT_GREY 0x5AEB
AnimatedGIF gif;
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

const char* ssid     = "Server CYBMEDIA";
const char* password = "cybmedia";

/*
const char* ssid     = "EFENDI";
const char* password = "12345678";
*/
const char* ntpServer = "id.pool.ntp.org";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 0;


uint32_t targetTime = 0;                    // for next 1 second timeout
static uint8_t conv2d(const char* p); // Forward declaration needed for IDE 1.6.x
uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3), ss = conv2d(__TIME__ + 6); // Get H, M, S from compile time

byte omm = 99, oss = 99;
byte xcolon = 0, xsecs = 0,xsecs1 =0,ysec1=0;
unsigned int colour = 0;
String tanggal = "";
String titik2="wait..";

typedef struct struct_message {
    char a[32];
    int b;
    float c;
    bool d;
} struct_message;

// Create a struct_message called myData
struct_message myData;

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

void setup(){
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println();
  
  WiFi.mode(WIFI_STA);
  WiFi.STA.begin();
  
  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  readMacAddress();

  // Change ESP32 Mac Address
  esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);
  if (err == ESP_OK) {
    Serial.println("Success changing Mac Address");
  }

  // Read the new MAC address
  Serial.print("[NEW] ESP32 Board MAC Address: ");
  readMacAddress();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected");
  gif.begin(BIG_ENDIAN_PIXELS);
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  delay(10);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  //disconnect WiFi as it's no longer needed
  //WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println(WiFi.macAddress());
  
  if (gif.open((uint8_t *)GIF_IMAGE, sizeof(GIF_IMAGE), GIFDraw))
  {
    Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
    tft.startWrite();
    while (gif.playFrame(true, NULL))
    {
      yield();
    }
    gif.close();
    tft.endWrite(); 
  }
  delay(1000);
  tft.fillScreen(0); 
  tft.setTextSize(1);
  static uint32_t radius = 116.999;
  static uint32_t radius1 = 118;
  static uint32_t index = 0;

  uint16_t fg_color = rainbow(index);
  uint16_t bg_color = TFT_BLACK;       // This is the background colour used for smoothing (anti-aliasing)

  uint16_t x = tft.width() / 2; // Position of centre of arc
  uint16_t y = tft.height() / 2;

  //tft.drawSmoothCircle(x, y, radius, fg_color, bg_color);
  tft.fillCircle(x, y, radius1, fg_color);
  tft.fillCircle(x, y, radius, bg_color);
  tft.fillRect(0,80,tft.width(),89,TFT_BLACK);
  tft.setTextColor(TFT_GREEN,TFT_BLACK);
  tft.drawCentreString("PT Diamond", 120, 40, 4); 
  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.drawCentreString("wait..", 120, 182, 2.9); // Draw text centre at position 80, 90 using font 2
  
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  int xpos1 = 2.2,xcolon1=0;
  int ypos1 = 85; // Top left corner ot clock text, about half way down
  xpos1 += tft.drawChar('0', xpos1, ypos1, 8); // Add hours leading zero for 24 hr clock
  xpos1 += tft.drawNumber(0, xpos1, ypos1, 8);             // Draw hours
  xcolon1 = xpos1; // Save colon coord for later to flash on/off later
  xpos1 += tft.drawChar(':', xpos1, ypos1 + 20, 6);
  xpos1 += tft.drawChar('0', xpos1, ypos1, 8); // Add minutes leading zero
  xpos1 += tft.drawNumber(0, xpos1, ypos1, 8);       
  
  // Update digital time
    int xpos = 2.2;
    int ypos = 85; // Top left corner ot clock text, about half way down
    int ysecs = ypos + 24;
    hh=0;
    mm=0;
    ss=0;
  
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  if (omm != mm) { // Redraw hours and minutes time every minute
      omm = mm;
      // Draw hours and minutes
      if (hh < 10) xpos += tft.drawChar('0', xpos, ypos, 8); // Add hours leading zero for 24 hr clock
      xpos += tft.drawNumber(hh, xpos, ypos, 8);             // Draw hours
      xcolon = xpos; // Save colon coord for later to flash on/off later
      xpos += tft.drawChar(':', xpos, ypos + 20, 6);
      xsecs = xpos-13;
      if (mm < 10) xpos += tft.drawChar('0', xpos, ypos, 8); // Add minutes leading zero
      xpos += tft.drawNumber(mm, xpos, ypos, 8);             // Draw minutes
       // Sae seconds 'x' position for later display updates
    }
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.drawCentreString(".", 120, 182, 2.9);
    printLocalTime();
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop(){
  delay(1000);
  printLocalTime();
}

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  Serial.print("Packet received with ");
  Serial.print("data size = ");
  Serial.println(sizeof(incomingData));
  uint8_t type = incomingData[0];

}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo,5)){
    Serial.println("Failed to obtain time");
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    titik2 += ".";
    if (titik2.length()>13){
      titik2 = "             ";
      tft.drawCentreString(titik2, 120, 182, 2.9);
      titik2 = "wait..";
    } else {
      tft.drawCentreString(titik2, 120, 182, 2.9);
    }
    return;
  }
  char timeDate[3];
  strftime(timeDate,3, "%d", &timeinfo);
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  char timeMinute[3];
  strftime(timeMinute,3, "%M", &timeinfo);
  //Serial.println(timeMinute);
  char timeSecond[3];
  strftime(timeSecond,3, "%S", &timeinfo);
  //Serial.println(timeHour);
  char timeTahun[6];
  strftime(timeTahun,6, "%Y", &timeinfo);
  char timeWeekDay[10];
  char timeMonth[10];

  int wday=int(timeinfo.tm_wday);
  int blnstr=int(timeinfo.tm_mon)+1;
  String hari="";
  String bulan="";
  String days[] = { "Minggu","Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
  hari=String(days[wday]);
  String months[] = { "Jan", "Feb", "Mar", "Apr", "Mei", "Jun", "Jul","Agust","Sept","Okt","Nop","Des"};
  bulan=String(months[blnstr]);
  String tanggal1=hari+", "+String(timeDate)+" "+bulan+" "+String(timeTahun);
  hh=(uint8_t)atoi(timeHour);
  ss=(uint8_t)atoi(timeSecond);
  mm=(uint8_t)atoi(timeMinute);
  if (ss == 60) {    // Check for roll-over
      ss = 0;          // Reset seconds to zero
      omm = mm;        // Save last minute time for display update
      mm++;            // Advance minute
      if (mm > 59) {   // Check for roll-over
        mm = 0;
        hh++;          // Advance hour
        if (hh > 23) { // Check for 24hr roll-over (could roll-over on 13)
          hh = 0;      // 0 for 24 hour clock, set to 1 for 12 hour clock
        }
      }
    }


    // Update digital time
    int xpos = 2.2;
    int ypos = 85; // Top left corner ot clock text, about half way down
    int ysecs = ypos + 24;
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);  
    if (omm != mm) { // Redraw hours and minutes time every minute
      omm = mm;
      // Draw hours and minutes
      if (hh < 10) xpos += tft.drawChar('0', xpos, ypos, 8); // Add hours leading zero for 24 hr clock
      xpos += tft.drawNumber(hh, xpos, ypos, 8);             // Draw hours
      xcolon = xpos; // Save colon coord for later to flash on/off later
      xpos += tft.drawChar(':', xpos, ypos + 20, 6);
      xsecs = xpos-13;
      if (mm < 10) xpos += tft.drawChar('0', xpos, ypos, 8); // Add minutes leading zero
      xpos += tft.drawNumber(mm, xpos, ypos, 8);             // Draw minutes
      //xsecs = xpos; // Sae seconds 'x' position for later display updates
    }
                   // Draw seconds
    if (oss != ss) { // Redraw seconds time every second
      oss = ss;
      //xpos = xsecs;

      if (ss % 2) { // Flash the colons on/off
        tft.setTextColor(0x39C4, TFT_BLACK);        // Set colour to grey to dim colon
        tft.drawChar(':', xcolon, ypos + 20, 6);     // Hour:minute colon
        //xpos += tft.drawChar(':', xsecs, ysecs, 6); // Seconds colon
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);    // Set colour back to yellow
      }
      else {
        tft.drawChar(':', xcolon, ypos + 20, 6);     // Hour:minute colon
       // xpos += tft.drawChar(':', xsecs, ysecs, 6); // Seconds colon
      }

      tft.setTextColor(TFT_RED,TFT_BLACK);
      xsecs1=xsecs;
      ysec1=207;
      if (ss < 10) xsecs1 += tft.drawChar('0', xsecs1, ysec1, 2); // Add leading zero
      tft.drawNumber(ss, xsecs1, ysec1, 2);    
    }
   if (mm==1){
      tft.setTextColor(TFT_WHITE, TFT_BLACK); 
      tft.drawCentreString(tanggal1, 120, 182, 2.9);
    } else if (tanggal==""){
        tanggal=tanggal1;
        Serial.print("Tanggal : ");
        Serial.println(tanggal1);
        tft.setTextColor(TFT_WHITE, TFT_BLACK); 
        tft.drawCentreString(tanggal1, 120, 182, 2.9);
        WiFi.mode(WIFI_OFF);
    }
}

// Function to extract numbers from compile time string
static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}
unsigned int rainbow(byte value)
{
  // If 'value' is in the range 0-159 it is converted to a spectrum colour
  // from 0 = red through to 127 = blue to 159 = violet
  // Extending the range to 0-191 adds a further violet to red band
 
  value = value%192;
  
  byte red   = 0; // Red is the top 5 bits of a 16-bit colour value
  byte green = 0; // Green is the middle 6 bits, but only top 5 bits used here
  byte blue  = 0; // Blue is the bottom 5 bits

  byte sector = value >> 5;
  byte amplit = value & 0x1F;

  switch (sector)
  {
    case 0:
      red   = 0x1F;
      green = amplit; // Green ramps up
      blue  = 0;
      break;
    case 1:
      red   = 0x1F - amplit; // Red ramps down
      green = 0x1F;
      blue  = 0;
      break;
    case 2:
      red   = 0;
      green = 0x1F;
      blue  = amplit; // Blue ramps up
      break;
    case 3:
      red   = 0;
      green = 0x1F - amplit; // Green ramps down
      blue  = 0x1F;
      break;
    case 4:
      red   = amplit; // Red ramps up
      green = 0;
      blue  = 0x1F;
      break;
    case 5:
      red   = 0x1F;
      green = 0;
      blue  = 0x1F - amplit; // Blue ramps down
      break;
  }
  return red << 11 | green << 6 | blue;
}
