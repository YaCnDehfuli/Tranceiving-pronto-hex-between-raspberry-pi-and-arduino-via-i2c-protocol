#include "SWProntoTransmitter.h"
#include <IRremote.h>
#include <Wire.h>

#define SLAVE_ADDRESS 0X22

int RECV_PIN = 2;
int TRNS_PIN = 6;
volatile int start_index=1;
char sen[2];
int c_code[100];
volatile String test_code = "";
char temp[4];


SWProntoTransmitter pt(TRNS_PIN);

IRrecv irrecv(RECV_PIN);
decode_results results;

static const char messageInterrupt = A1;


int s_flag = 0;

void setup() {

    pinMode(messageInterrupt, OUTPUT);

    Serial.begin(115200);
    Serial.println("READY");
    irrecv.enableIRIn(); // Start the receiver
    Wire.begin(SLAVE_ADDRESS);
    Wire.onReceive(receiveData);
    Wire.onRequest(sendData);
    digitalWrite(messageInterrupt, LOW);
}


// Dumping raw signal
int c = 1;
void dump(IRrecv* irrecv) {
  Serial.println();
  int count = irrecv->decodedIRData.rawDataPtr->rawlen;
  Serial.println(c);
  c++;

  Serial.print("received raw signal[");
  Serial.print(count, DEC);
//  Serial.println(count);
  Serial.print("] = {");
  for (int i = 1; i < count; i++) {
    Serial.print((int)irrecv->decodedIRData.rawDataPtr->rawbuf[i]*USECPERTICK, DEC);
    Serial.print(",");
  }
  Serial.println("};");
}

// Dumping rendered pronto hex
void dump(uint16_t* phex, uint8_t len) {
  char temp[10];
  
  Serial.print("send ");
  for (int i = 0; i < len; i++) {

    sprintf(temp, "%04x \0", phex[i]);
    Serial.print(temp);
  }
  Serial.println();
}

void loop() {
  if (s_flag == 1){
//    Serial.println("make_ready called");
    s_flag = 0;
    make_ready();
//    Serial.println(int(Repeat_value));
  }
}

void trigger_messageInterrupt() 
{
    digitalWrite(messageInterrupt, HIGH);
    delay(10);
    digitalWrite(messageInterrupt, LOW);
//    Serial.println("Interrupt triggered");
}

void make_ready()
{
if (irrecv.decode(&results)) 
 {
      Serial.println("here");
      irrecv.end();
      
      uint16_t* raw_code = irrecv.decodedIRData.rawDataPtr->rawbuf;
      int len = irrecv.decodedIRData.rawDataPtr->rawlen;
      dump(&irrecv);
      for (int i = 0; i<len ; i++)
      {
        c_code[i] = (int)raw_code[i]*USECPERTICK;
      }

      uint16_t code[len + 4];
      pt.convert_raw_pronto(raw_code, len, 38000, code);
      dump(code, sizeof(code)/sizeof(code[0]));
      trigger_messageInterrupt(); 
//      ir_stop();
      irrecv.start();
//        irrecv.resume();
    }
    else
    {
      Serial.println("there");
      delay(1500);
      make_ready();
    }
}

void receiveData(int howMany)
{
   if (howMany > 1)
   {
    for (int i = 0; i < howMany; i++) {
    temp[i] = Wire.read();
    temp[i + 1] = '\0'; //add null after ea. char
    }
    for (int i = 0; i < howMany; ++i)
    {
      temp[i] = temp[i + 1];
    }
    if(howMany == 3){
      test_code += temp;
      printdata();
    }
    else{
    Serial.print(temp);
    Serial.print(" ");
    }
   }
   else{                            
      s_flag = Wire.read();
    }
}

void printdata()
{
    Serial.println();
    Serial.print("Repeat Value is : ");
    Serial.println(test_code[0]);
    Serial.print("Pin number is : ");
    Serial.println(test_code[1]);
}


void sendData(int) 
{
 int len = irrecv.decodedIRData.rawDataPtr->rawlen;
 if (start_index < len)
 {
  sen[0] = c_code[start_index]/256;
  sen[1] = c_code[start_index]%256;
  Wire.write(sen);
  start_index++;
 }
 else 
 {
  start_index = 0;
//  delay(10);
 }
}
