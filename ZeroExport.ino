
#include <ModbusMaster.h>


#define MAX485_DE      6
#define MAX485_RE_NEG  7

const int Max_Per = 99;

int Crnt_Per = 10;

int Min_Per = 2;

float Meter_KW_Current_Data;

const float Meter_KW_Set_Lowest_Limit = 80.0;

const float Meter_KW_Set_Up_Power_Limit = 350.0;

// instantiate ModbusMaster object
ModbusMaster node;
ModbusMaster node1;

void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}



float f_2uint_float(unsigned int uint1, unsigned int uint2) {    // reconstruct the float from 2 unsigned integers

  union f_2uint {
    float f;
    uint16_t i[2];
  };
   union f_2uint f_number;
  f_number.i[0] = uint1;
  f_number.i[1] = uint2;

  return f_number.f;

}

void setup()
{
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

 Serial.begin(9600);
  // Modbus communication runs at 115200 baud
  Serial2.begin(9600);
  

  // Modbus slave ID 1
  node.begin(11, Serial2);
  node1.begin(1,Serial2);
  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  node1.preTransmission(preTransmission);
  node1.postTransmission(postTransmission);
}

void SendPowerLimitToInverter (){
  uint16_t result2;
  result2 = node1.writeSingleRegister(40234, 10); 
   if (result2 == node1.ku8MBSuccess) {
    Serial.print("Data Sent....");
   }
}

void loop()
{
  uint16_t result;
  uint16_t result1;
  uint16_t data[6];
  
//  // Toggle the coil at address 0x0002 (Manual Load Control)
//  result = node.writeSingleCoil(0x0002, state);
//  state = !state;

  // Read 2 registers starting at 8210
  result = node.readInputRegisters(8210, 2);
  if (result == node.ku8MBSuccess)
  {

    int reg1 = node.getResponseBuffer(0x00);
    int reg0 = node.getResponseBuffer(0x01);

    float float_reconstructed = f_2uint_float(reg0, reg1); 

    // here set Data to Variable
    Meter_KW_Current_Data = float_reconstructed;
    
    Serial.print("Total KW: ");
    Serial.println(Meter_KW_Current_Data);

    if(Meter_KW_Current_Data < Meter_KW_Set_Lowest_Limit && Crnt_Per > Min_Per ){
      Serial.print("Percentage need to be reduced ");
      Serial.println(Crnt_Per);
      Crnt_Per = Crnt_Per-1;
      SendPowerLimitToInverter();
    } else if (Meter_KW_Current_Data > Meter_KW_Set_Up_Power_Limit && Crnt_Per < Max_Per) {
      Crnt_Per = Crnt_Per + 1;
      SendPowerLimitToInverter();
    }
    
         
  } else {
    Serial.println("No value received for Meter");
    Serial.println(result);
    
  }
  delay(500);
//  result1 = node1.writeSingleRegister(40118, 1);
//    node1.writeSingleRegister(40234, Current_Percentage);
    result1 = node1.readHoldingRegisters(40525, 2);
    if (result1 == node1.ku8MBSuccess)
    {
      Serial.print("Inverter Active Power: ");
      Serial.print(node1.getResponseBuffer(0x00));
      Serial.println(node1.getResponseBuffer(0x01));
      Serial.print("Inverter Current Percentage: ");
      Serial.println(Crnt_Per);
    } else {
      Serial.println("No value received for Inverter");
      Serial.println(result1);
      
    }
//writeSingleRegister
    
 
  delay(1000);
}
