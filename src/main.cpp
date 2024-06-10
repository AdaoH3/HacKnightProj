#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <AdafruitIO.h>
#include <config.h>

#define TFT_CS    42   // Chip Select pin
#define TFT_DC    40    // Data/Command pin
#define TFT_RST   41    // Reset pin
#define TFT_BL    45    // Backlight pin (optional)

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

AdafruitIO_Feed *StringData = io.feed("StringData");

const unsigned char wifiicon[] PROGMEM  ={ // wifi icon
  0x00, 0xff, 0x00, 0x7e, 0x00, 0x18,0x00, 0x00
};

String wifiBadConnectChecks[] = {"Network connection failed.",
"Adafruit IO connection failed.",
"Adafruit IO SSL fingerprint verification failed.",
"Adafruit IO authentication failed."};

void timer(String remainingTime);
void testdrawtext(String text, uint16_t color);
void endTime();
void buttonMidPress();
void buttonBottomPress();
void sendEndRequest();
void idleTriangle();

int lastMidButtonPress = millis();
int lastBotButtonPress = millis();
bool connected = false;
bool isConnected = false;
int lastEndTime = millis();

String lastReceivedString = "";

void handleMessage(AdafruitIO_Data *data) {
  lastReceivedString = data->value();
  Serial.print("Received string: ");
  Serial.println(lastReceivedString);
}

void setup() 
{
  // Attach the interrupt to the button pin, triggering on falling edge (button press)
  attachInterrupt(digitalPinToInterrupt(1), buttonMidPress, FALLING);
  attachInterrupt(digitalPinToInterrupt(2), buttonBottomPress, FALLING);
  // wait for serial monitor to open

  //TFT Initialization
  u8g2_for_adafruit_gfx.begin(tft);      
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  tft.init(135, 240); // Init ST7789 240x135
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  Serial.println(F("TFT Initialized"));

   // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  //Setting up the feed from REST API
  StringData->onMessage(handleMessage);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  //connected
  Serial.println();
  Serial.println(io.statusText());
  
  // Draw Wi-Fi icon
  tft.drawBitmap(230,1,wifiicon,8,8,ST77XX_WHITE);
  
}

void loop() 
{
  idleTriangle();

  io.run();
  boolean networkConnected = true;
  boolean previousNetworkConnected = false;
  String check = io.statusText();
  
  // Takes in feed data and starts a timer
  // Use the last received string
  if (lastReceivedString != "" && lastReceivedString != "000000") {
    Serial.print("Stored string: ");
    Serial.println(lastReceivedString);
    timer(lastReceivedString);
    lastReceivedString = ""; // Clear the string after using it to prevent repeated calls
    previousNetworkConnected = false;
  }


  for(int i = 0; i < 6; i++)
  {
    if(check.equals(wifiBadConnectChecks[i]))
    {
      networkConnected = false;
    }
  }

  if(networkConnected == true && previousNetworkConnected != networkConnected)
  {
    // Draw Wi-Fi icon
    tft.drawBitmap(230,1,wifiicon,8,8,ST77XX_WHITE);
  }

  previousNetworkConnected = networkConnected;
}

void timer(String remainingTime)
{
  remainingTime.replace("[", "");
  remainingTime.replace("]", "");
  remainingTime.replace("\"", "");
  
  tft.fillScreen(ST77XX_BLACK);
  int hour = remainingTime.substring(0,2).toInt();
  int min = remainingTime.substring(2,4).toInt();
  int sec = remainingTime.substring(4,6).toInt();

  int totalSeconds = hour * 3600 + min * 60 + sec; // Calculate total seconds
  int prevSeconds = 0; // Previous value of seconds
  int prevHours = 0;
  int prevMinutes = 0;

  while (totalSeconds > 0) 
  {
    int currentHours = totalSeconds / 3600; // Calculate current hours
    int currentMinutes = (totalSeconds % 3600) / 60; // Calculate current minutes
    int currentSeconds = totalSeconds % 60; // Calculate current seconds

    String formattedTime = String(currentHours / 10, DEC); // Add leading zero for hours (tens place)
    formattedTime += String(currentHours % 10, DEC);       // Add leading zero for hours (ones place)
    formattedTime += ":";                                 // Add colon separator
    formattedTime += String(currentMinutes / 10, DEC);     // Add leading zero for minutes (tens place)
    formattedTime += String(currentMinutes % 10, DEC);     // Add leading zero for minutes (ones place)
    formattedTime += ":";                                 // Add colon separator
    formattedTime += String(currentSeconds / 10, DEC);     // Add leading zero for seconds (tens place)
    formattedTime += String(currentSeconds % 10, DEC); 

    // Display the formatted time
    Serial.print("Remaining Time: ");
    Serial.println(formattedTime);

    
    // Delay for 1 second
    int timeSecond = millis();
    while(millis() - timeSecond < 1000)
    {
      //Pause functionality
      if(millis() - 200 < lastMidButtonPress)
      {
        int tempTime = millis();
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(20,50);
        testdrawtext(formattedTime, ST77XX_ORANGE);
        tft.setCursor(20, 90);
        testdrawtext("Paused", ST77XX_WHITE);
        delay(300);
        
        while(millis() - lastMidButtonPress > 250)
        {
        
        }

        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(20, 50);
        lastMidButtonPress -= 500;
      }

      if(millis() - 1010 < lastBotButtonPress)
      {
        sendEndRequest();
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(20, 50);
        testdrawtext("Canceled Timer", ST77XX_RED);
        delay(3000);
        tft.fillScreen(ST77XX_BLACK);
        return;
      }
    }
    //delay(1000);


    //Update TFT
    if(currentSeconds != prevSeconds)
    {
      tft.fillRect(160, 40, 50, 50, ST77XX_BLACK);
    }
    if(currentMinutes != prevMinutes)
    {
       tft.fillRect(90, 40, 50, 50, ST77XX_BLACK);
    }
    if(currentHours != prevHours)
    {
       tft.fillRect(15, 40, 50, 50, ST77XX_BLACK);
    }
    // Decrease totalSeconds by 1
    totalSeconds--;

    
    // If the seconds value has changed, update the display
    if (currentSeconds != prevSeconds) 
    {
      u8g2_for_adafruit_gfx.setFont(u8g2_font_helvR14_tf);
      tft.setCursor(20, 50);
      testdrawtext(formattedTime, ST77XX_BLUE);
    }

    prevSeconds = currentSeconds; // Update previous seconds value
    prevMinutes = currentMinutes;
    prevHours = currentHours;
  }

  endTime();
}

void endTime() {
  int i = 0; 
  while(millis() - 1000 > lastBotButtonPress && i < 11) {
    tft.fillScreen(ST77XX_RED);
    delay(500);
    tft.fillScreen(ST77XX_BLACK);
    delay(500);
    i++;
  }
  //Send Serial end Data
  sendEndRequest();
  Serial.print("Ended");
}

void sendEndRequest()
{
  Serial.print("sending -> ");
  Serial.println("000000");
  StringData->save("000000");
}

void testdrawtext(String text, uint16_t color) {
  tft.setTextSize(4.8);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void buttonMidPress()
{
  if(millis() > (lastMidButtonPress + 1000))
  {
    lastMidButtonPress = millis();
    Serial.println("Mid Button Clicked");
  }
}

void buttonBottomPress()
{
  if(millis() > (lastBotButtonPress + 1000))
  {
    lastBotButtonPress = millis();
    Serial.println("Bottom Button Clicked");
  }
}


void idleTriangle()
{
  static float angle = 0; // Angle in degrees
  float radAngle = angle * PI / 180; // Convert to radians

  // Center coordinates
  int cx = 120;
  int cy = 70;
  
  // Triangle coordinates
  int x1 = cx + 50 * cos(radAngle);
  int y1 = cy + 50 * sin(radAngle);
  int x2 = cx + 50 * cos(radAngle + 2 * PI / 3);
  int y2 = cy + 50 * sin(radAngle + 2 * PI / 3);
  int x3 = cx + 50 * cos(radAngle + 4 * PI / 3);
  int y3 = cy + 50 * sin(radAngle + 4 * PI / 3);

  // Clear previous triangle
  tft.fillRect(0, 10, 200, 200, ST77XX_BLACK);

  // Draw new triangle
  tft.fillTriangle(x1, y1, x2, y2, x3, y3, ST77XX_YELLOW);

  // Update angle for rotation
  angle += 5;
  if (angle >= 360) {
    angle = 0;
  }

  delay(100); // Delay to control the speed of rotation
}