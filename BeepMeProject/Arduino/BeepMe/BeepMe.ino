
 // Connect a speaker here
#define TONE_PIN 8

 // Forgiving parse of an unsigned int from a string. All non-digits simply ignored.

unsigned int parseUnsignedInt( const char *s , int len ) {

   unsigned int x=0;
  
   while (len--) {
  
      if (isdigit(*s)) {
  
        x*=10;
         x+=(*s)-'0';
  
      }
  
      s++;
  
   } 
  
 return(x);

}

// Understood command string formats:
// 'Tnnnnn' where nnnnn is 5 digit frequency specifier - Geneate a tone
// 'N' - Stop any playing tone

#define COMMAND_LEN 6

void processCommand( const char *commandString ) {

  // Use the first byte to specifiy action...
  
  switch (commandString[0]) {
      
    case 'T':  { // Generate a tone of the frequency specified by the 5 digit number after the action...
    
      unsigned int freq = parseUnsignedInt( commandString + 1 , 5 );
    
      if (freq >=50 && freq<=5000 ) {    // Just some sanity checks...
        
          tone( TONE_PIN , freq );   // Play the specified tone
      }
      
      break;
    }
      
    case 'N': {  // Stop any currently playing tone
    
      noTone( TONE_PIN );
      break;
      
    }
      
  }
   
}




// This is the default baud rate for the serial link
#define LININOBAUD 250000 

void setup() {

    Serial1.begin(LININOBAUD); // open serial connection to Linino

}

// We to use this string to indicate where the command is inside the request string...
// Mast match the command string sent by the HTML code
#define COMMAND_PREFIX "COMMAND="

// This is the expected length of the incoming command. For now just turnong on/off so only need 1 bytes

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
