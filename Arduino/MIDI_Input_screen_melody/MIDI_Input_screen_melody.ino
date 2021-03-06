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
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

//Create an instance of the library with default name, serial port and settings
MIDI_CREATE_DEFAULT_INSTANCE();

// Define oscillator addresses and 

#define NR_OSC 3     // Nr of oscillators
#define OSC1 6       // Oscilator 1 is digital pin 6
#define OSC2 7       // Oscilator 2 is digital pin 7
#define OSC3 8       // Oscilator 3 is digital pin 8

Tone notePlayer[NR_OSC];   //[3] = 3 OSC (MAX)

int OSC_TABLE[NR_OSC]; // Create global variable for handling oscillator conflicts
int OSC_PTR[NR_OSC];   // Create global variable for handling oscillator pointers

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Pre-declarations //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
void scroll_logo(void);

int MIDI2FREQ(int pitch)
{
  double coeff = pow(2.0, ((double)pitch-69)/12);
  int freq = 440*coeff;
  return freq;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Screen Functions //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void scroll_logo(void) {

  // Clear display
  display.clearDisplay();

  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(F("Varela Synth 1.0"));
  display.display();      // Show initial text
  delay(100);

  // Scroll to center:
  display.startscrolldiagright(0x00, 0x07);
  delay(1000);
  display.stopscroll();
  delay(1000);

  // Clear display
  display.clearDisplay();
}

void print_MIDI(byte pitch, byte vel, int oscillator, int *OSC_TABLE)
{

  
  // Clear display
  display.clearDisplay();

  // Graphics for oscillators
  for (int j = 0; j<NR_OSC; j++)
  {
    if(OSC_TABLE[j] != 0)
    {
      display.drawLine(0, SCREEN_HEIGHT-3-OSC_TABLE[j]/2, 60, SCREEN_HEIGHT-3-OSC_TABLE[j]/2, SSD1306_WHITE);  
    }
  }
  


  // Print pitch
  display.setTextSize(0.6); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(65, 0);
  display.println(F("Note"));
  display.setCursor(105, 0);
  display.println(pitch);

  // Print Velocity
  display.setCursor(65, 15);
  display.println(F("Velo"));
  display.setCursor(105, 15);
  display.println(vel);

  // Print freq
  display.setCursor(65, 30);
  display.println(F("Freq"));
  display.setCursor(95, 30);
  display.println(MIDI2FREQ(pitch));

  // Print oscillator
  display.setCursor(65, 45);
  display.println(F("Osc"));
  display.setCursor(105, 45);
  display.println(oscillator);
  
  display.display();
  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Setup Functions ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {

////////////////////////////////////// Setup screen ///////////////////////////////

  Serial.begin(9600);

 // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();

  scroll_logo();    // Draw scrolling text

////////////////////////////////////// Setup MIDI ///////////////////////////////
  
  pinMode (LED_BUILTIN, OUTPUT); // Set Arduino board pin 13 to output
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
  notePlayer[0].begin(OSC1);    //OSC 1 - OSC 1 OUTPUT PIN 3
  notePlayer[1].begin(OSC2);    //OSC 2 - OSC 2 OUTPUT PIN 5
  notePlayer[2].begin(OSC3);    //OSC 3 - OSC 3 OUTPUT PIN 11
  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Loop Function /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() { // Main loop
  MIDI.read(); // Continuously check if Midi data has been received.
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// Handle notes //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// MyHandleNoteON is the function that will be called by the Midi Library
// when a MIDI NOTE ON message is received.
// It will be passed bytes for Channel, Pitch, and Velocity
void MyHandleNoteOn(byte channel, byte pitch, byte velocity) { 
  digitalWrite(LED_BUILTIN,HIGH);  //Turn LED on

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
    print_MIDI(pitch, velocity, i, OSC_TABLE);
    
  }

}

// MyHandleNoteOFF is the function that will be called by the Midi Library
// when a MIDI NOTE OFF message is received.
// * A NOTE ON message with Velocity = 0 will be treated as a NOTE OFF message *
// It will be passed bytes for Channel, Pitch, and Velocity
void MyHandleNoteOff(byte channel, byte pitch, byte velocity) { 
  digitalWrite(LED_BUILTIN,LOW);  //Turn LED off

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
