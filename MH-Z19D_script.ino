#include <Wire.h>             
#include <LiquidCrystal_I2C.h>  
#include <HardwareSerial.h>     

LiquidCrystal_I2C lcd(0x27, 16, 2);

HardwareSerial sensor1(1); 
HardwareSerial sensor2(2); 


byte cmd_get_sensor[] = {0xFF/*Start byte*/, 0x01/*Sensor ID*/, 0x86/*read CO2 cmd*/, 0, 0, 0, 0, 0, 0x79/*Checksum*/};

int greenLED = 25;
int yellowLED = 26;
int redLED = 27;

int readCO2(HardwareSerial &serial) {
  byte response[9];
  serial.write(cmd_get_sensor, 9);  
  delay(100);                      

  if (serial.available() >= 9) {
    serial.readBytes(response, 9);
    if (response[0] == 0xFF && response[1] == 0x86) {
      return (response[2] << 8) | response[3];
    }
  }
  return -1; 
}

void setup() {
  Serial.begin(115200);  

 
  lcd.init();            
  lcd.backlight();        
  lcd.setCursor(0, 0);    
  lcd.print("CO2 Monitor");

 
  sensor1.begin(9600, SERIAL_8N1, 16, 17);
  sensor2.begin(9600, SERIAL_8N1, 4, 2); 

  delay(5000);
}

void loop() {
  int co2_1 = readCO2(sensor1);
  int co2_2 = readCO2(sensor2);

  lcd.clear();
  lcd.setCursor(0, 0);
  if (co2_1 > 0) {
    lcd.print("S1: ");
    lcd.print(co2_1);
    lcd.print(" ppm");
  } else {
    lcd.print("S1: Err");
  }

  lcd.setCursor(0, 1);
  if (co2_2 > 0) {
    lcd.print("S2: ");
    lcd.print(co2_2);
    lcd.print(" ppm");
  } else {
    lcd.print("S2: Err");
  }

  delay(2000);
}
