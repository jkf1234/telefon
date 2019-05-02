#define PULSE_ACTIVE B00000100 // Pin 2 but here this is used to identify the pin in register PIND.
#define PULSE_IN 3
#define CALL_PIN 14
#define CLEAR_PIN 15
#define BOOT_PIN 16
#define HANGUP_BUTTON 18
#define HANGUP_BUTTON_PIN B00010000  // Pin 18 but here this is used to identify the pin in register PINC.
#define MIN_VALID_PULSE 40000
#define MAX_VALID_PULSE 50000
#define PULSE_IN_TIMEOUT 100000


/*
 *In SA phone numbers are 10 digit long - to be changed for your country 
 *For international calls, the code looks at the second digit. 
 *If it's also a 0, the automatic dialing is disabled. See comment down below.
 */
#define PHONE_NUMBER_LENGTH 10 

bool off_hook = false;
byte digitCount = 0;
byte digit = 0;
bool hasDigit = false;
bool digit2zero = false;

void setup() {
  /*
   * pins for the dialer
   */

  pinMode(PULSE_ACTIVE, INPUT);
  pinMode(PULSE_IN, INPUT);
  pinMode(HANGUP_BUTTON, INPUT);
  /*
   * pins for the cellphone 
   */
  pinMode(4, OUTPUT); //1
  pinMode(5, OUTPUT); //2
  pinMode(6, OUTPUT); //3
  pinMode(7, OUTPUT); //4
  pinMode(8, OUTPUT); //5
  pinMode(9, OUTPUT); //6
  pinMode(10, OUTPUT); //7
  pinMode(11, OUTPUT); //8
  pinMode(12, OUTPUT); //9
  pinMode(13, OUTPUT); //0

  pinMode(CALL_PIN, OUTPUT);
  pinMode(CLEAR_PIN, OUTPUT);
  pinMode(BOOT_PIN, OUTPUT);

  delay(2000);
  
  digitalWrite(BOOT_PIN, HIGH);
  delay(1000);
  digitalWrite(BOOT_PIN, LOW);

  off_hook = false;
  
  Serial.begin(9600); //for debugging
  Serial.println("Starting"); //for debugging
}

void loop() {
  /* 
   * This bit was taught to me by this article. Read it! It's really good.
   * http://www.instructables.com/id/Fast-digitalRead-digitalWrite-for-Arduino/
   * Those next 2 lines effectively replace a digitalRead() by looking straight at the registers
   * Using digitalRead() I was sometimes getting 1s instead of 2s etc... 
   * This solves most of them. 
   */
  int hookState = PINC;
  hookState = hookState & HANGUP_BUTTON_PIN;
  if(hookState){
   //Serial.println("on hook"); //for debugging
    /*
     * This prevents the HangUp() function from being called all the time when the receiver in down.
     * On my cellphone, the hangup and shutdown button are the same.
     * I don't want the phone to shutdown.
     * Turning off the cellphone will be manual due to lack of features on the old telephone.
     */
    if(off_hook){ 
      hangUp();
    }
    return;
  }
  
  /* 
   * This bit was taught to me by this article. Read it! It's really good.
   * http://www.instructables.com/id/Fast-digitalRead-digitalWrite-for-Arduino/
   * Those next 2 lines effectively replace a digitalRead() by looking straight at the registers
   * Using digitalRead() I was sometimes getting 1s instead of 2s etc... 
   * This solves most of them. 
   */
  int activeState = PIND;
  activeState = PIND & PULSE_ACTIVE;
    
  if(activeState) //Serial.println("Active"); //for debugging
  
  while(activeState){
    off_hook = true;
    
    unsigned long val = myPulseIn(3, LOW, PULSE_IN_TIMEOUT); 
   //Serial.println(val); //for debugging
    if(val > MIN_VALID_PULSE && val < MAX_VALID_PULSE){
      digit++;
      Serial.println(digit);
    }
    activeState = PIND;
    activeState = PIND & PULSE_ACTIVE ;
    hasDigit = true;
    //Serial.println(hasDigit);
  }
  if(hasDigit ){
    Serial.print("digit "); //for debugging
    Serial.println(digit); //for debugging
    //dialCellDigit(digit);
    Serial.println("Sleeping"); //for debugging
    digit = 0;
    hasDigit = false;
  }
}

void dialCellDigit(byte digit){
  /*
  * add a digit to the phone number
   */
  digitCount++;

  /*
   * if the 2nd digit is 0, I'm dialing an international number 
   * Will disable auto-dialing based on that.
   */
  if(digit == 10 && digitCount == 2)
    digit2zero = true;
    
  /*
   * the pin number is 3 more than the digit I'm dialing
   */
  byte numberPin = digit + 3;
  
  /*
   * 'press' a button on the cellphone
   */
  digitalWrite(numberPin, HIGH);
  delay(250);
  digitalWrite(numberPin, LOW);

  /*
   * When we get to the proper length for a phone number
   * If the second digit was a 0, we are doing an international call.
   * Then dialing will be manual.
   */
  if(digitCount == PHONE_NUMBER_LENGTH && !digit2zero){
    /*
     * Call the number
     */
    Serial.println("Calling"); //for debugging
    call(500);
  }
}

/*
* Pretty obvious
* But also turns on the phone at the start
*/
void call(int press_delay){
  digitalWrite(CALL_PIN, HIGH);
  delay(press_delay);
  digitalWrite(CALL_PIN, LOW);
}

/*
* Pretty obvious too
* But also used to clear the digits on the cell
*/
void hangUp(){
  digitalWrite(BOOT_PIN, HIGH);
  delay(100);
  digitalWrite(BOOT_PIN, LOW);
  digitalWrite(CLEAR_PIN, HIGH);
  delay(100);
  digitalWrite(CLEAR_PIN, LOW);
  digitCount = 0;
  digit2zero = false;
  off_hook = false;
}

/*
 * This code replaces the default Arduino PulseIn
 * Credits to Jims in this thread.
 * https://forum.arduino.cc/index.php?topic=46011.0
 * I panelbeated it a little because it got stuck if when the PULSE_ACTIVE resets.
 */
unsigned long myPulseIn(uint8_t pin, uint8_t state, long timeOut)
{
   // cache the port and bit of the pin in order to speed up the
   // pulse width measuring loop and achieve finer resolution.  calling
   // digitalRead() instead yields much coarser resolution.
   uint8_t bit = digitalPinToBitMask(pin);
   uint8_t port = digitalPinToPort(pin);
   uint8_t stateMask = (state ? bit : 0);
   unsigned long width = 0; // keep initialization out of time critical area */
       
   // wait for the pulse to start
   while ( (*portInputRegister(port) & bit) != stateMask)
       ;
   
   // wait for the pulse to stop
   while ( (*portInputRegister(port) & bit) == stateMask){
       width++;
       if(width > timeOut) //Time out if the wait is too long
        break; 
   }

   return width; 
}


