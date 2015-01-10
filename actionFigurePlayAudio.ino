
#include "WaveUtil.h"
#include "WaveHC.h"
#include <FatReader.h>
#include <SdReader.h>
#include <avr/prgmspace.h>

#define DEBOUNCE 100 // Button Debouncer
#define WAVES 10     // number of wave files

SdReader card;    // this object holds the information for the card
FatVolume vol;    // this holds the information for the partition on the card
FatReader root;   // this holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're playing

WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

// this handy function will return the number of bytes currently free in RAM, great for debugging!
int freeRam(void)
{
  
  extern int __bss_end;
  extern int *__brkval;
  int free_memory;
  
  if((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  }
  else {
    free_memory = ((int)&free_memory) - ((int)__brkval);
  }
  return free_memory;
}
 
void sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
  
}

void setup() {
  
  // setup serial port
  Serial.begin(9600);
  putstring_nl("Random Wave playing program");
  
  putstring("Free RAM: ");
  Serial.println(freeRam());    // display amount of ram available should be above 150 bytes
  
  // Set output pins for DAC control. These pins are defined in the library
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  
  // pin 13 LED
  pinMode(13, OUTPUT);
  
  // enable pull-up resistors on switch pin (analog inputs)
  digitalWrite(14,HIGH);
  
  // seed random number generator
  randomSeed(analogRead(3));
  
  // if (!card.init(true)) {                     //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {                            //play with 8 MHz spi (default faster!)
    putstring_nl("Card init. failed!");          // Something went wrong, lets print out why
    sdErrorCheck();
    while(1);                                    // then 'halt' - do nothing!
  }

  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);

  // Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {             // we have up to 5 slots to look in
    if (vol.init(card, part))
      break;                                     // we found one, lets bail
  }  
  if (part == 5) {                               // if we ended up not finding one :(
    putstring_nl("No valid FAT partition!");
    sdErrorCheck();                              // Something went wrong, lets print out why
    while(1);                                    // then 'halt' - do nothing!
  }

  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);            // FAT16 or FAT32?

  // Try to open the root directory
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root dir!");       // Something went wrong,
    while(1);                                   // then 'halt' - do nothing!
  }

  // Whew! We got past the tough parts.
  putstring_nl("Ready!");
}

void loop() {
  
  int randomNum;
  int playing = 0;
  int index;
  
  static int index2 = 0;
  randomNum = 0;
  
  if(checkSwitch() == 1)
  {
    playing = 1;
    // Generate a random number 
    randomNum = random(1,(WAVES+1));
    
    for(index = 0; index < 11; index++){
     Serial.println(analogRead(5));
    } 
    
    // play wave file corresponding to random number
    switch (randomNum) {
      case 0:
        //do nothing
        break;
      case 1:
        playcomplete("ROBOT001.wav");
        break;
      case 2:
         playcomplete("ROBOT002.wav");
        break;
      case 3:
         playcomplete("ROBOT003.wav");
        break;
      case 4:
        playcomplete("ROBOT004.wav");
        break;
      case 5:
         playcomplete("ROBOT005.wav");
        break;
      case 6:
         playcomplete("ROBOT006.wav");
        break;
      case 7:
         playcomplete("ROBOT007.wav");
        break;
      case 8:
        playcomplete("ROBOT008.wav");
        break;
      case 9:
         playcomplete("ROBOT009.wav");
        break;
      case 10:
         playcomplete("ROBOT010.wav");
        break;  
      /*  
      case 1:
        playcomplete("TROLLBOT001.wav");
        break;
      case 2:
         playcomplete("TROLLBOT002.wav");
        break;
      case 3:
         playcomplete("TROLLBOT003.wav");
        break;
      case 4:
         playcomplete("TROLLBOT004.wav");
        break;
      case 5:
         playcomplete("TROLLBOT005.wav");
        break;
      case 6:
         playcomplete("TROLLBOT006.wav");
        break;
      case 7:
         playcomplete("TROLLBOT007.wav");
        break;
      case 8:
         playcomplete("TROLLBOT008.wav");
        break;
      case 9:
         playcomplete("TROLLBOT009.wav");
        break;
      case 10:
         playcomplete("TROLLBOT010.wav");
        break;*/
    }
  }
  else {
    playing = 0;
  }  
}



byte checkSwitch() {
  
  static byte previous;
  static long time;
  byte pressed;
  byte reading;
  
  pressed = 0;
  
  reading = digitalRead(14);
  
  if(reading == LOW && (millis() - time > DEBOUNCE))
  {
    time = millis();
    pressed = 1;
    Serial.println(pressed);
    return (pressed);
    
  }
  
  previous = reading;
  return(pressed);
  
}

// Plays a full file from beginning to end with no pause.
void playcomplete(char *name) {
  int analogReading;
  
  // call our helper to find and play this name
  playfile(name);
  while (wave.isplaying) {
  // do nothing while its playing
   analogReading =  analogRead(5);
   Serial.println(analogReading);
   if(analogReading >= 5){
     digitalWrite(7,HIGH);
     digitalWrite(6,HIGH);
   }
   else{
     digitalWrite(7,LOW);
     digitalWrite(6,LOW);
   }
     
  }
  // now its done playing
   digitalWrite(7,LOW);
   digitalWrite(6,LOW);
  
}

void playfile(char *name) {
  
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
    putstring("Couldn't open file "); Serial.print(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV"); return;
  }
  
  // ok time to play! start playback
  wave.play();
  
}


