// A very simple example skecth that allows the  onbaord LED to be reomotely 
// controlled by a webpage. See http://wp.josh.com/2014/05/01/a-platform-for-casual-encounters/ for more info.

// This is the default baud rate for the serial link 
#define LININOBAUD 250000  

// Use the onboard LED
#define LED_PIN 13

void setup() {
    Serial1.begin(LININOBAUD); // open serial connection to Linino
    pinMode( LED_PIN, OUTPUT );
}

// We to use this string to indicate where the command is inside the request string...
// Mast match the command string sent by the HTML code
#define COMMAND_PREFIX "COMMAND="


// Understood command string formats:
// '0' - Turn of LED
// '1' - Turn on LED

// This is the expected length of the incoming command. For now just turnong on/off so only need 1 bytes
#define COMMAND_LEN 1

void processCommand( const char *commandString ) {

  // Use the first byte to specifiy action...
  
  switch (commandString[0]) {
      
    case '0': 
     
    pinMode( LED_PIN, OUTPUT );
     digitalWrite( LED_PIN , LOW );      // Turn off the LED
     break;
     
     
    case '1':
     digitalWrite( LED_PIN , HIGH );      // Turn on the LED
     break;
              
  }
         
}


char commandBuffer[COMMAND_LEN];

void loop() {

  // Does this look like a command request?  
  if (Serial1.find(COMMAND_PREFIX)) {
 
    // Read it into the command buffer and only process if long enough...
    if ( Serial1.readBytes( commandBuffer , COMMAND_LEN ) == COMMAND_LEN ) {
  
      processCommand( commandBuffer );
       
    }
    
  }
  
}

 

