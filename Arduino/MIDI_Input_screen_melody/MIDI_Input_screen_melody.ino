//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Includes //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <MIDI.h>  // Add Midi Library
#include <math.h>
#include <Tone.h>  //Bhagman Tone Library

// Includes for ADAFRUIT screen
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define I2C_ADDR 0x3C

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

#define TRIGGER_OUT 2   // Pin for triangular wave trigger

//Create an instance of the library with default name, serial port and settings
MIDI_CREATE_DEFAULT_INSTANCE();

// Define oscillator addresses and such
#define NR_OSC 3     // Nr of oscillators
#define OSC1 8       // Oscilator 1 is digital pin 8
#define OSC2 7       // Oscilator 2 is digital pin 7
#define OSC3 6       // Oscilator 3 is digital pin 6
Tone notePlayer[NR_OSC];   //[3] = 3 OSC (MAX)

int OSC_TABLE[NR_OSC];  // Create global variable for handling oscillator conflicts
int OSC_PTR[NR_OSC];    // Create global variable for handling oscillator pointers

bool b_trigger = false; // Controls if triangular wave is triggered or not

int potenInput =  A0;   // Pin for potentiometer read  
int buttonInput = A1;   // Pin for button read
int audioInput =  A2;   // Pin for Audio In

int buttonValue = 97;    // Button value
int potenValue  = 98;    // Potentiometer value
int audioValue  = 99;    // Potentiometer value

int x_buff[NR_OSC] = {0,0,0};
int y_buff[NR_OSC] = {0,0,0};

// char notes[12][2] = {"A ","Bb","B ","C ","Db","D ","Eb","E ","F ","Gb","G ","Ab"};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Pre-declarations //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
void scroll_logo(void);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// MIDI to Freq /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MIDI2FREQ(int pitch)
{
  double coeff = pow(2.0, ((double)pitch-69)/12);
  int freq = 440*coeff;
  return freq;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// MIDI to Note /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Pitch2Note(int pitch)
{
  return((pitch-69) % 12);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// Initial Logo ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void scroll_logo(void) {
  
  // Clear display
  display.clearDisplay();

  // Text
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 5);
  display.println(F("Varela"));

  display.setCursor(30, 25);
  display.println(F("Synth"));

  display.setTextSize(0.5);
  display.setCursor(0, 54);
  display.println(F("1.0"));
  display.setCursor(80, 54);
  display.println(F("2021"));
  
  display.display();      // Show initial text
  // delay(100);

  // Scroll to center:
  display.startscrolldiagright(0x00, 0x07);
  delay(500);
  display.stopscroll();
  delay(500);
  // Clear display
  display.clearDisplay();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////// Print MIDI ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void print_MIDI(int oscillator, int *OSC_TABLE)
{ 

  int x = 0;
  int y = 0;

  
  
  // Graphics for oscillators
  for (int j = 0; j<NR_OSC; j++)
  {

    if (x_buff[j] >SCREEN_WIDTH-1)
    {
      x_buff[j] = 0;
    }
    else
    {
      x_buff[j]+=4;
    }
  
    if(OSC_TABLE[j] != 0)
    {  
      x = x_buff[j] + 4;
      y = SCREEN_HEIGHT - round(OSC_TABLE[j]) + 40;
      
      display.drawLine(x_buff[j],y_buff[j], x, y, SSD1306_WHITE);

      x_buff[j] = x;
      y_buff[j] = y;
      
      //display.drawPixel(x, y, SSD1306_WHITE);
      //display.drawLine(0, SCREEN_HEIGHT-3-OSC_TABLE[j]/2, 60, SCREEN_HEIGHT-3-OSC_TABLE[j]/2, SSD1306_WHITE);  
    }
  }

  if (x_buff[0] > SCREEN_WIDTH)
  {
    // Clear display
    display.clearDisplay();
  }

//  // Print button value
//  display.setCursor(60, 0);
//  display.println(F("Button"));
//  display.setCursor(100, 0);
//  display.println(buttonValue);
//
//  // Print Velocity
//  display.setCursor(60, 15);
//  display.println(F("Poten"));
//  display.setCursor(100, 15);
//  display.println(potenValue);
//
//  // Print oscillator
//  display.setCursor(65, 45);
//  display.println(F("Osc"));
//  display.setCursor(100, 45);
//  display.println(oscillator);
  
  display.display();
  
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Button Function ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void button_interrupt()
{

  // Clear display
  display.clearDisplay();
  display.setCursor(35, 30);

  // If triangular wave is triggered
  if(b_trigger)
  {
    digitalWrite(TRIGGER_OUT, LOW); // sets the digital pin low

    // Print
    display.println(F("SAW WAVE IS ACTIVE"));

    b_trigger = false;
  }
  else
  {
    digitalWrite(TRIGGER_OUT, HIGH); // sets the digital pin high

    // Print
    display.println(F("TRI WAVE IS ACTIVE"));

    b_trigger = true;
  }
   
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Setup Functions ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {

////////////////////////////////////// Setup screen ///////////////////////////////

  Serial.begin(9600);

 // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDR)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();

  scroll_logo();    // Draw scrolling text

  // Setup text for later
  display.setTextSize(0.6); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);

////////////////////////////////////// Setup inputs / outputs ///////////////////////////////
  
  pinMode(TRIGGER_OUT, OUTPUT);
  digitalWrite(TRIGGER_OUT, LOW); // sets the digital pin out

  // Protect Pin 5
  pinMode(5, INPUT);
  
  // pinMode (buttonInput, INPUT); // Set button to input
//
  // Signal ready
  pinMode (LED_BUILTIN, OUTPUT); // Set Arduino board pin 13 to output
  digitalWrite(LED_BUILTIN,HIGH);  //Turn LED on  
  delay(200);
  digitalWrite(LED_BUILTIN,LOW);  //Turn LED off
  delay(200);
  digitalWrite(LED_BUILTIN,HIGH);  //Turn LED on  
  delay(200);
  digitalWrite(LED_BUILTIN,LOW);  //Turn LED off

////////////////////////////////////// Setup MIDI ///////////////////////////////
  
  MIDI.begin(MIDI_CHANNEL_OMNI); // Initialize the Midi Library.
  // OMNI sets it to listen to all channels.. MIDI.begin(2) would set it 
  // to respond to notes on channel 2 only.
  MIDI.setHandleNoteOn(MyHandleNoteOn); // This is important!! This command
  // tells the Midi Library which function you want to call when a NOTE ON command
  // is received. In this case it's "MyHandleNoteOn".
  MIDI.setHandleNoteOff(MyHandleNoteOff); // This command tells the Midi Library 
  // to call "MyHandleNoteOff" when a NOTE OFF command is received.

  // Create table with addresses for oscillators
  OSC_PTR[0] = OSC1;
  OSC_PTR[1] = OSC2;
  OSC_PTR[2] = OSC3;
  
  // Reset oscillator table
  for(int i = 0; i < NR_OSC; i++)
  {
    OSC_TABLE[i] = 0;
  }

  // Setup Timers for oscillators
  notePlayer[0].begin(OSC1);    //OSC 1 - OSC 1
  notePlayer[1].begin(OSC2);    //OSC 2 - OSC 2
  notePlayer[2].begin(OSC3);    //OSC 3 - OSC 3

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Loop Function /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() { // Main loop
  
  MIDI.read(); // Continuously check if Midi data has been received.

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Handle note ON ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// MyHandleNoteON is the function that will be called by the Midi Library
// when a MIDI NOTE ON message is received.
// It will be passed bytes for Channel, Pitch, and Velocity
void MyHandleNoteOn(byte channel, byte pitch, byte velocity) { 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Read Button  //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  // read the value from the sensor:
//  buttonValue = analogRead(buttonInput);
//  potenValue =  analogRead(potenInput);
//  
//  if(buttonValue == 0)
//  {
//    // Launch interrupt if button is pressed
//    button_interrupt();
//  }

  digitalWrite(LED_BUILTIN, HIGH); // sets the digital pin out

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Play notes  ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  int i = 0;  // index for oscillator

  // Check first oscillator that is free to play note
  // Free oscillators are playing zeroth note
  while (i < NR_OSC && OSC_TABLE[i] != 0)
  {
    // This will be the index of the free oscillator
    i++;
  }

  if(i == NR_OSC)
  {
    i = 0;
  }

  // If there is a free oscillator
  if( i < NR_OSC )
  {
    // Declare that oscillator is busy with note 'pitch'
    OSC_TABLE[i] = pitch;
    
    // Play note on oscillator of pin 'i'
    notePlayer[i].play(MIDI2FREQ(pitch));

    // Plot on screen: pitch / frequency , velocity and oscillator
    print_MIDI(i, OSC_TABLE);
    
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Handle note OFF ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// MyHandleNoteOFF is the function that will be called by the Midi Library
// when a MIDI NOTE OFF message is received.
// * A NOTE ON message with Velocity = 0 will be treated as a NOTE OFF message *
// It will be passed bytes for Channel, Pitch, and Velocity
void MyHandleNoteOff(byte channel, byte pitch, byte velocity) { 

  digitalWrite(LED_BUILTIN, LOW); // sets the digital pin out
  
  int i = 0;  // index for oscillator

  // Find out which oscillator is being used
  while (i < NR_OSC && OSC_TABLE[i] != pitch)
  {
    // This will be the index of the free oscillator
    i++;
  }
  
  if(i < NR_OSC)
  {
    // Reset oscillator
    OSC_TABLE[i] = 0;
      
    // Do a noTone of that oscillator
    notePlayer[i].stop();
  }

  // Plot on screen: pitch / frequency , velocity and oscillator
  // Very beautiful but makes the synth slow
  // print_MIDI(pitch, velocity, i, OSC_TABLE);
    
}
