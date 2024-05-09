# include "SPI.h"

// Connections for Arduino Mega as master:
// SS = pin 53
// CLK  = pin 52
// MISO = pin 50
// MOSI = pin 51


char send_data = '?';

void setup() {
  Serial.begin(115200); // set baud rate to 115200 for usart
  pinMode(SS, OUTPUT);
  digitalWrite(SS, HIGH);
  delay(100);

  SPI.begin();
  //SPI.setClockDivider(SPI_CLOCK_DIV2); // divide the clock by 8
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  digitalWrite(SS, LOW); // enable Slave Select
  // send test char
  char recieve_data = SPI.transfer(send_data);
  digitalWrite(SS, HIGH); // disable Slave Select

  Serial.print("Recieved Data:");  Serial.println(recieve_data);
} 

void loop (void) {
  digitalWrite(SS, LOW); // enable Slave Select
  // send test char
  char recieve_data = SPI.transfer(send_data);
  digitalWrite(SS, HIGH); // disable Slave Select

  Serial.print("Recieved Data:");  Serial.println(recieve_data);

  delay(2000);
}