// This #include statement was automatically added by the Particle IDE.
#include "FastLED/FastLED.h"
#include "LEDMatrix.h"
#include "LEDText.h"
#include "LEDSprites.h"
#include "FontRobotron.h"
#include "photon-particle-sys/photon-particle-sys.h"
#include "math.h"   //using constrain to ensure values in commands in range
#include "SpriteData.h"

FASTLED_USING_NAMESPACE;

// Matrix configuration
#define LED_PIN        6
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B
#define MATRIX_WIDTH   32
#define MATRIX_HEIGHT  8
#define MATRIX_TYPE    VERTICAL_ZIGZAG_MATRIX
#define BRIGHTNESS     50

//date conversion values
#define SECONDSINDAY 86400
#define SECONDSINHOUR 3600
#define SECONDSINMINUTE 60

//holders for date calculations
unsigned long remSeconds, remMinutes, remHours, remDays;
byte year, month, day, hour, minute, second, DoW;
unsigned int monthdays[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
bool century, h24, ampm;
unsigned long xmasDate;
unsigned long Now, dT;

//target date for countdown
byte targetYear = 2021;
byte targetMonth = 12;
byte targetDay = 25;
byte targetHour = 0;
byte targetMinute = 0;
byte targetSecond = 0;

//matrix object
cLEDMatrix<-MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

//sprite objects
cLEDSprites Sprites(&leds);
cSprite SprPacmanLeft(8, 8, PacmanLeftData, 3, _1BIT, PacmanLeftColTab, PacmanLeftMask);
cSprite SprBlinky(8, 8, BlinkyData, 2, _3BIT, BlinkyColTab, BlinkyMask);
cSprite SprPacmanRight(8, 8, PacmanRightData, 3, _1BIT, PacmanRightColTab, PacmanRightMask);
cSprite SprInvaderOne(8, 8, InvaderOneData, 2, _1BIT, InvaderOneColTab, InvaderOneMask);
cSprite SprInvaderTwo(11, 8, InvaderTwoData, 2, _1BIT, InvaderTwoColTab, InvaderTwoMask);
cSprite SprInvaderThree(12, 8, InvaderThreeData, 2, _1BIT, InvaderThreeColTab, InvaderThreeMask);
cSprite SprTree(7, 8, TreeData, 3, _3BIT, TreeColTab, TreeMask);

cLEDText ScrollingMsg1;
unsigned char AttractMsg[144];
String incomingMessage = "HAVE A NICE DAY!";

//particle system objects
const byte numParticles = 50;
ParticleSysConfig g(MATRIX_WIDTH, MATRIX_HEIGHT);
Particle_Std particles[numParticles];
//Particle_Fixed source(g.center_x, g.center_y); //starfield
Particle_Attractor source(g.center_x, g.center_y);//smoker
Emitter_Fountain emitter(8, &source);
ParticleSys pSys(&g, numParticles, particles, &emitter);
FastLEDRenderer renderer(&g);

// translates from x, y into an index into the LED array
uint16_t XY( uint8_t x, uint8_t y) {
    return leds.mXY(x,y);
}

//calculated date in seconds
unsigned long dateInSeconds(unsigned int year, byte month, byte day, byte hour, byte minute, byte second)
{
 unsigned long sse = (((unsigned long)year)*365*24*60*60)   +   ((((unsigned long)year+3)>>2) + ((unsigned long)year%4==0 && (unsigned long)month>2))*24*60*60   +   \
         ((unsigned long)monthdays[month-1] + (unsigned long)day-1) *24*60*60   +   ((unsigned long)hour*60*60)   +   ((unsigned long)minute*60)   + (unsigned long)second;

  return sse;
}

// calculates the current date in seconds
unsigned long refreshCurrentDate()
{
    year = Time.year();
	month = Time.month(century);
	//if (century) year += 100;
	day = Time.day();
	hour =Time.hour();
	minute = Time.minute();
	second = Time.second();
	Now = dateInSeconds(year, month, day, hour, minute, second);
}

//calculates remaining time to target date
void calculateRemaining()
{
    //refresh date
    Particle.syncTime();
    Particle.process();

    refreshCurrentDate();
    // Here's the time remaining.
	dT = xmasDate - Now;
    // Figure out whole days remaining
	remDays = dT/SECONDSINDAY;
	// Here's the leftover HMS
	remSeconds = dT%SECONDSINDAY;
	// Figure out the whole hours remaining
	remHours = remSeconds/SECONDSINHOUR;
	// Here's the leftover MS
	remSeconds = remSeconds%SECONDSINHOUR;
	// Figure out the whole minutes remaining
	remMinutes = remSeconds/SECONDSINMINUTE;
	// Here's the leftover S
	remSeconds = remSeconds%SECONDSINMINUTE;
}

// init the program
void setup()
{
    pinMode(A0, INPUT);
    digitalWrite(A0, HIGH);
    //seed the randomizer
    randomSeed(analogRead(2));
    int16_t HalfWholeChars, WholeEvenChars;

    //mp3 player serial port
    Serial1.begin (9600);
    xmasDate =dateInSeconds(targetYear, targetMonth, targetDay, targetHour, targetMinute, targetSecond);

    //init the fastled library
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
    FastLED.setBrightness(48);
    FastLED.clear(true);
    delay(200);
//    FastLED.showColor(CRGB::Red);
//    delay(200);
//    FastLED.showColor(CRGB::Lime);
//    delay(200);
//    FastLED.showColor(CRGB::Blue);
//    delay(200);
//    FastLED.showColor(CRGB::White);
//    delay(200);
    FastLED.show();

    //text setup
    ScrollingMsg1.SetFont(RobotronFontData);
    WholeEvenChars = ((leds.Width() + (ScrollingMsg1.FontWidth() * 2) + 1) / ((ScrollingMsg1.FontWidth() + 1) * 2)) * ((ScrollingMsg1.FontWidth() + 1) * 2);
    ScrollingMsg1.Init(&leds, WholeEvenChars, ScrollingMsg1.FontHeight() + 2, (leds.Width() - WholeEvenChars) / 2, (leds.Height() - (ScrollingMsg1.FontHeight() + 2)) / 2);
    Particle.function("8BitMessage",receive8BitMessage);

    //sprite init
    SprPacmanRight.SetPosition(-30,0);
    SprPacmanLeft.SetPosition(-30,0);
    SprBlinky.SetPosition(-30,0);
    SprInvaderOne.SetPosition(-30,0);
    SprInvaderTwo.SetPosition(-30,0);
    SprInvaderThree.SetPosition(-30,0);
    SprTree.SetPosition(-30,0);
    //sprite setup
    Sprites.AddSprite(&SprPacmanRight);
    Sprites.AddSprite(&SprPacmanLeft);
    Sprites.AddSprite(&SprBlinky);
    Sprites.AddSprite(&SprInvaderOne);
    Sprites.AddSprite(&SprInvaderTwo);
    Sprites.AddSprite(&SprInvaderThree);
    Sprites.AddSprite(&SprTree);

    //particle system setup
    source.vx = 3;
    source.vy = 1;
    source.x = random(50)+100;
    source.y = random(10)+1;
    source.atf = 2;
    emitter.minLife = 10;
    emitter.maxLife = 200;
    pSys.perCycle = 2;
    Time.zone(-8);
    renderer.reset(FastLED.leds());

    //mp3 player setup
    mp3Command("0x06 12 0");//set volume
    delay(50);
    mp3Command("0x16 0 0");//stop playback
    delay(50);

}

//the main logic loop
void loop()
{
    //flush the serial of incoming data, we never ask for anything. the device is capable of quering for state and other metadata if you want to... more info at the url below
    //https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299
    while(Serial1.available())
            Serial1.read();

    //particle emitter sound reaction
    listenToAudio();

    pSys.update();
    renderer.fade(FastLED.leds());
    renderer.render(&g, particles, numParticles, FastLED.leds());

    //render the sprites
    Sprites.UpdateSprites();
    Sprites.DetectCollisions();
    Sprites.RenderSprites();

    //update the message if it has finished
    if (ScrollingMsg1.UpdateText() == -1)
    {
        SetNewMessage();
    }

    FastLED.show();
    delay(10);
}

//receive any incoming message initiated by calling the "8BitMessage" particle function on this device... from IFTTT or another particle device or using the particle cloud apis
int receive8BitMessage(String message)
{
    incomingMessage=message.toUpperCase();
    return 0;
}

//set the new message, play a sound that relates to that message
void SetNewMessage()
{
    static int mode=4;
    mode++;
    switch(mode%5)
    {
        case 0://time
            mp3Command("0x12 1 0");
            delay(25);
            sprintf((char *)AttractMsg, "      %s", Time.format(Time.now(), "%l:%M%p").toUpperCase().c_str());
            SprPacmanLeft.SetPositionFrameMotionOptions(32/*X*/, 0/*Y*/, 0/*Frame*/, 4/*FrameRate*/, -1/*XChange*/, 4/*XRate*/, 0/*YChange*/, 0/*YRate*/, SPRITE_DETECT_EDGE | SPRITE_DETECT_COLLISION);
        break;
        case 1://date
            mp3Command("0x12 2 0");
            delay(25);
            sprintf((char *)AttractMsg, "      %s", Time.format(Time.now(), "%a, %b %d").toUpperCase().c_str());
            SprBlinky.SetPositionFrameMotionOptions(32/*X*/, 0/*Y*/, 0/*Frame*/, 8/*FrameRate*/, -1/*XChange*/, 4/*XRate*/, 0/*YChange*/, 0/*YRate*/, SPRITE_DETECT_EDGE | SPRITE_DETECT_COLLISION);        break;
        break;
        case 2:
            mp3Command("0x12 3 0");
            delay(25);
            sprintf((char *)AttractMsg, "      %s", Time.format(Time.now(), "%Y").toUpperCase().c_str());
            SprInvaderThree.SetPositionFrameMotionOptions(32/*X*/, 0/*Y*/, 0/*Frame*/, 10/*FrameRate*/, -1/*XChange*/, 4/*XRate*/, 0/*YChange*/, 0/*YRate*/, SPRITE_DETECT_EDGE | SPRITE_DETECT_COLLISION);
        break;
        case 3:
            mp3Command("0x12 4 0");
            delay(25);
            sprintf((char *)AttractMsg, "      %s", incomingMessage.c_str());
            SprInvaderOne.SetPositionFrameMotionOptions(32/*X*/, 0/*Y*/, 0/*Frame*/, 10/*FrameRate*/, -1/*XChange*/, 4/*XRate*/, 0/*YChange*/, 0/*YRate*/, SPRITE_DETECT_EDGE | SPRITE_DETECT_COLLISION);
        break;
        case 4://xmas
            mp3Command("0x12 5 0");
            delay(25);
            calculateRemaining();
            sprintf((char *)AttractMsg, "      %i DAYS'TIL XMAS!", remDays+1);
            SprTree.SetPositionFrameMotionOptions(32/*X*/, 0/*Y*/, 0/*Frame*/, 8/*FrameRate*/, -1/*XChange*/, 4/*XRate*/, 0/*YChange*/, 0/*YRate*/, SPRITE_DETECT_EDGE | SPRITE_DETECT_COLLISION);
        break;
    }

    //set common text parameters
    ScrollingMsg1.SetBackgroundMode(BACKGND_LEAVE|BACKGND_DIMMING);
    ScrollingMsg1.SetScrollDirection(SCROLL_LEFT);
    ScrollingMsg1.SetTextDirection(CHAR_UP);
    ScrollingMsg1.SetTextColrOptions(COLR_GRAD_AV,0,0,0,random(64,256),random(64,256),random(64,256));
    ScrollingMsg1.SetFrameRate(3);
    ScrollingMsg1.SetText(AttractMsg, strlen((const char *)AttractMsg));

}

/*
 * Sample the volume level
 */
void listenToAudio(){
    int minimum = 1024;
    int maximum = 0;
    for(int i=0;i<50;i++)
    {
        int value = analogRead(A0);
        minimum = min(minimum, value);
        maximum = max(maximum, value);
    }
    emitter.maxLife = map(maximum-minimum,0,1024,200,0);

}

//serial cmd buffer
uint32_t msCmdReceived;

//sends command to the mp3 player
void sendCmd(int cmd, int lb, int hb, bool reply = false)
{                 // standard format for module command stream
    uint8_t buf[] = {0x7E, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF};
    int16_t chksum = 0;
    int idx = 3;                    // position of first command byte

    buf[idx++] = (uint8_t)cmd;      // inject command byte in buffer
    if (reply) buf[idx++] = 0x01;   // set if reply is needed/wanted
    if (hb >= 0)                    // if there is a high byte data field
        buf[idx++] = (uint8_t)hb;   // add it to the buffer
    if (lb >= 0)                    // if there is a low byte data field
        buf[idx++] = (uint8_t)lb;   // add it to the buffer
    buf[2] = idx - 1;               // inject command length into buffer

    for (int i=1; i < idx; i++)     // calculate check sum for the provided data
        chksum += buf[i];
    chksum *= -1;

    buf[idx++] = (chksum >> 8);     // inject high byte of checksum before
    buf[idx++] = chksum & 0xFF;     // low byte of checksum
    buf[idx++] = 0xEF;              // place end-of-command byte

    Serial1.write(buf, idx);        // send the command to module
    for (int i = 0; i < idx; i++)   // send command as hex string to MCU
      Serial.printf("%02X ", buf[i]);
    Serial.println();
}

//formats the command to send to the mp3 player
int mp3Command(String para)
{
    int cmd = -1;
    int lb = -1;
    int hb = -1;
                                    // parse the command string
    int count = sscanf(para.c_str(), "0x%x %d %d", &cmd, &lb, &hb);

    if (count > 0 && cmd >= 0)      // if we got a well formed parameter string
      sendCmd(cmd, lb, hb, true);   // do the module talking

    msCmdReceived = millis();       // set a non-blocking delay timer

    return cmd;                     // return what command we think we received
}
