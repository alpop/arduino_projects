/**
*	IRRemote, read IR sensor pulses return a code of it.
*
* @license   The MIT License (MIT) Included in this distribution
*
* @ref:
*    
* @author    Zaher Dirkey <zaherdirkey at yahoo dot com>
*
*/

#define IRpin_PIN      PINB
#define IRpin          3

#define getPulse       (IRpin_PIN & _BV(IRpin))

#ifdef USB_CFG_IOPORTNAME //check if vusb used, to call usbPoll()
#define VUSB
#endif

#define IR_PULSE_COUNT 32 // IR pulses count 
#define IR_MINPULSES 4 //min acceptable count, return code 0 if less than 4
#define IR_RESOLUTION 2 
#define IR_MAX_LENGTH 5000 //we dont need to not wait forever, at least need to call usbPoll()

//UNIVERSAL
#define IR_DATA_LENGTH 600
#define IR_LO_DATACOME 1500 //Skip first data
#define IR_LONG_LENGTH 2500

//SONY , used Lo pulses
/*#define IR_DATA_LENGTH 600 //higher=1 lesser = 0
#define IR_LO_DATACOME 1500 //not sure
#define IR_LONG_LENGTH 2500
*/

/*
//FAN, using low bit
#define IR_DATA_LENGTH 300
#define IR_LO_DATACOME 0
#define IR_LONG_LENGTH 2500
*/
#define DEBUG


#ifdef DEBUG
uint16_t maxLength = 0;
uint16_t minLength = 0;
#endif 

uint32_t IRRead()
{  
  uint16_t lo_pulse =0, hi_pulse =0;
  
  struct {
    bool started = false;
    uint32_t code = 0;
    uint8_t count = 0; 
    #ifdef DEBUG
    bool b = false;
    #endif
    
    void reset(){
      #ifdef DEBUG
      b = false;
      maxLength = 0;
      minLength = 0;
      #endif DEBUG
      
      count = 0;
      code = 0;
    }

    void add(uint16_t pulse)
    {   
      code = code << 1;
      if (pulse > IR_DATA_LENGTH){
        code = code | 1;      
      }      
      count++;
       
      #ifdef DEBUG
      if (pulse > 25) {
        if (!b || (maxLength < pulse))
          maxLength = pulse;
        if (!b || (minLength > pulse))
          minLength = pulse;
        b = true;
      }
      #endif DEBUG
    }
  } data;

/*
 * Starting loop to collect pulses 
 * First pulse it ignored, it was high, waiting forever
 * after lo pulse come usually it is long low pulse, used to sign of data comming
 * second high maybe it is a long for data comming sign too, we can ignore it, sony not need it 
 * 
 */
  data.reset();
  
  while (data.count <= IR_PULSE_COUNT)
  {
    #ifdef VUSB
    usbPoll(); 
    #endif

    hi_pulse = 0;
    while (getPulse) { //Wait for ever for the first pulse
      hi_pulse++;
      delayMicroseconds(IR_RESOLUTION);      
      if (hi_pulse >= IR_LONG_LENGTH)
      {
        if (data.count > 0)
        {
          #ifdef DEBUG
          Serial.print("Long Length");        
          #endif DEBUG
          if (!data.started)
            return 0;
          else 
          {
            #ifdef DEBUG
            Serial.print(" Count= ");
            Serial.println(data.count);
            #endif DEBUG
            return data.code;
          }
        }
        else if (hi_pulse >= IR_MAX_LENGTH) 
        {
          hi_pulse = 0;
          #ifdef VUSB
          usbPoll(); 
          #endif
          //continue;
        }
      }
    }
    lo_pulse = 0;
    while (!getPulse) {
      lo_pulse++;
      delayMicroseconds(IR_RESOLUTION);
      if ((lo_pulse >= IR_MAX_LENGTH)) {
        #ifdef DEBUG
        Serial.println(" lo pulse IR_MAX_LENGTH ");        
        #endif DEBUG
        #ifdef VUSB
        usbPoll(); 
        #endif
        
        return 0; //too long data length
      }
    }

    if (IR_LO_DATACOME > 0)
    {
        if (!data.started) {
          data.reset();
          data.started = true;
        }
        if ((lo_pulse >= IR_LO_DATACOME))
          continue;
    }
    else {
        if (!data.started) {
          data.reset();
          data.started = true;
        }
    }
    
    if (data.started) {
      data.add(hi_pulse + lo_pulse);
    }
  }

#ifdef DEBUG
  Serial.println("Exit");
#endif
  return data.code;
}

#ifdef DEBUG
uint32_t IRTest()
{
  uint32_t code = 0;
  uint8_t count = 0; 

  while (true) {
    uint16_t pulse = 1;
    while (IRpin_PIN & _BV(IRpin)) { // got a high pulse
      pulse++;
      delayMicroseconds(IR_RESOLUTION);
    }    
    
    if (pulse > 1000)
      Serial.println("++++++++++++++++++++");
    Serial.print("HI:");
    Serial.println(pulse);

    pulse = 1;
    while (!(IRpin_PIN & _BV(IRpin))) { // got a high pulse
      pulse++;
      delayMicroseconds(IR_RESOLUTION);
    }
    Serial.print("LO:");
    Serial.println(pulse);
  }
}
#endif

void loop()
{
  //IRTest();
  //return;
  
  unsigned long code = IRRead();
#ifdef DEBUG
  if (code > 0)
  {       
    Serial.print("Code for this button: ");
    Serial.println(code);
  }
  else
    Serial.println("No code");
  Serial.print("Max pulse: ");  
  Serial.println(maxLength);  
  Serial.print("Min pulse: ");  
  Serial.println(minLength);  
  Serial.println("");
#endif  
}

void setup()
{
#ifdef DEBUG
  Serial.begin(115200); // Fast IO
  Serial.setTimeout(10); // Quick read
  Serial.print("IR Reader Ready! at ");
  Serial.println(IR_DATA_LENGTH);
#endif

  pinMode(11, INPUT); // Set IR pin as input

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
}

