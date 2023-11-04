/* Usage example of the JETGPIO library
 * Compile with: g++ -Wall -o lcd_i2c lcd_i2c.cpp -ljetgpio
 * Execute with: sudo ./lcd_i2c
*/

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <jetgpio.h>

// Global variable to interrupt the loop later on 
static volatile int interrupt = 1;

// Ctrl-c signal function handler
void inthandler(int signum) 
{
  usleep(1000);
  printf("\nCaught Ctrl-c, coming out ...\n");
  interrupt = 0;
}

void send_command(unsigned handle, unsigned i2cAddr, unsigned command){
  unsigned buffer = command & 0xF0;
  buffer |= 0x04;
  unsigned buffer2 = buffer;
  buffer |= 0x08;
  i2cWriteByteData(handle, i2cAddr, buffer, buffer);
  usleep(2000);
  buffer2 &= 0xFB;
  i2cWriteByteData(handle, i2cAddr, buffer2, buffer2);

  buffer = (command & 0x0F) << 4;
  buffer |= 0x04;
  buffer2 = buffer;
  buffer |= 0x08;
  i2cWriteByteData(handle, i2cAddr, buffer, buffer);
  usleep(2000);
  buffer2 &= 0xFB;
  i2cWriteByteData(handle, i2cAddr, buffer2, buffer2);
}

void send_data(unsigned handle, unsigned i2cAddr, unsigned data){
  unsigned buffer = data & 0xF0;
  buffer |= 0x05;
  //unsigned buffer2 = buffer;
  buffer |= 0x08;
  i2cWriteByteData(handle, i2cAddr, buffer, 0);
  usleep(2000);
  //buffer2 &= 0xFB;
  //i2cWriteByteData(handle, i2cAddr, buffer2, buffer2);

  buffer = (data & 0x0F) << 4;
  buffer |= 0x05;
  //buffer2 = buffer;
  buffer |= 0x08;
  i2cWriteByteData(handle, i2cAddr, buffer, 0);
  usleep(2000);
  //buffer2 &= 0xFB;
  //i2cWriteByteData(handle, i2cAddr, buffer2, buffer2);
}

int main(int argc, char *argv[]){
  
  const int LCD_SLAVE_ADDRESS = 0x3f;

  printf("Press Ctrl-c to stop...\n");
   
  gpioInitialise();
	

  // Opening the connection to the LCD I2C slave 0x3f, i2c port 0 (pins 27/28), flags = 0 (100 kHz)

  int lcd = i2cOpen(0,0);
  
  // Now setting stuff up, device has not register map, all characters are sent as is, not to a specific register address
  // the trick here is to send the same stuff twice for commands and for characters just once and then a 0, which does nothing

  send_command(lcd, LCD_SLAVE_ADDRESS, 0x33);
  
  usleep(5000); 

  send_command(lcd, LCD_SLAVE_ADDRESS, 0x32);
  
  usleep(5000);

  send_command(lcd, LCD_SLAVE_ADDRESS, 0x28);
  
  usleep(5000);

  send_command(lcd, LCD_SLAVE_ADDRESS, 0x0C);
  
  usleep(5000);

  send_command(lcd, LCD_SLAVE_ADDRESS, 0x01);
  
  usleep(5000);

  i2cWriteByteData(lcd, LCD_SLAVE_ADDRESS, 0x08, 0x08);

  //Now can start writing to the lcd screen, starting at position 0,0 first line ot the left

  char message[15] = {"Jetgpio"};
  char message2[15] = {"by Rubberazer"};

  while(interrupt){
    send_command(lcd, LCD_SLAVE_ADDRESS, 0x80); //Positioning cursor at point 0,0
    for (size_t i=0;i<strlen(message);i++){
      send_data(lcd, LCD_SLAVE_ADDRESS, (unsigned)message[i]);
    }

    send_command(lcd, LCD_SLAVE_ADDRESS, 0xC1); //Positioning cursor at second line
    for (size_t i=0;i<strlen(message2);i++){
      send_data(lcd, LCD_SLAVE_ADDRESS, (unsigned)message2[i]);
    }
  }
  send_command(lcd, LCD_SLAVE_ADDRESS, 0x01); //Clear screen
    
  // Closing i2c connection

  i2cClose(lcd);

  // Terminating library

  gpioTerminate();

  exit(0);
	
}
