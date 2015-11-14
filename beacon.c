////////////////////////////////////////////////////////////////////////////////
//
// BEACON
// (c) 2007 AB3Y
// www.davidus.sk
//
////////////////////////////////////////////////////////////////////////////////
//
// Simple beacon with telemetry designed to monitor balloons
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Config for this chip
#include "beacon.h"

// i2c data bus pins
#define DAL_SCL PIN_A1
#define DAL_SDA PIN_A2

// I2C setup
#use i2c(master, sda=DAL_SDA, scl=DAL_SCL)

// Keyer pin
#define KEY_PIN PIN_A0

// Morse ITU length
#define DIT_LENGTH 1
#define DAH_LENGTH 3
#define SIL_LENGTH 7

// Morse chars
#define M_CHARS 34

#ROM 0x3FF= {0x3440} // Calibration Word 


// Alpahet used for Morse //////////////////////////////////////////////////////
char const morse_chars[M_CHARS] = {'a', 'b', 'c' ,'d', 'e', 'f', 'h', 'i', 'j', 'k', 'l', 'm',
                                   'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'w', 'y', '/', '0',
                                   '1', '2', '3', '4', '5', '6', '7', '8', '9', ' '};

// DIT - 1, DAH - 0
BYTE const morse_binary[M_CHARS] = {0x06, 0x17, 0x15, 0x0b, 0x03, 0x1d, 0x1f, 0x07, 0x18, 0x0a, 0x1b, 0x04,
                                    0x05, 0x08, 0x19, 0x12, 0x0d, 0x0f, 0x02, 0x0e, 0x0c, 0x14, 0x2d, 0x20,
                                    0x30, 0x38, 0x3c, 0x3e, 0x3f, 0x2f, 0x27, 0x23, 0x21, 0x00};
                               
BYTE const morse_length[M_CHARS] = {2, 4, 4, 3, 1, 4, 4, 2, 4, 3, 4, 2,
                                    2, 3, 4, 4, 3, 3, 1, 3, 3, 4, 5, 5,
                                    5, 5, 5, 5, 5, 5, 5, 5, 5, 0};

// PROTOTYPES //////////////////////////////////////////////////////////////////
void dit();
void dah();
void letter_silence();
void word_silence();

void chars(char c);

void temp_config(BYTE data);
void init_temp();
signed int read_full_temp();

// MAIN ROUTINE ////////////////////////////////////////////////////////////////
void main()
{
   // Local vars
   signed int temperature;
   char message[48];
   BYTE i;

   // PIC setup
   setup_adc_ports(NO_ANALOGS|VSS_VDD);
   setup_adc(ADC_OFF);
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
   setup_timer_1(T1_DISABLED);
   setup_comparator(NC_NC);
   setup_vref(FALSE);
   
   // Init temperature chip
   init_temp();   

   // Main loop
   do {
      // Get temp
      temperature = read_full_temp();
      
      // Spell out telemetry
      sprintf(message, "cq cq cq de om/ab3y/b beacon in jn99kf 30mw t%c%02uc ar", ((temperature < 0) ? 'n' : 'p'), temperature);
               
      // Morse out
      for (i = 0; i < 51; i++)
      {
         chars(message[i]);
      }//for
      
      delay_ms(3000);

   } while (TRUE);
}//func

// FUNCTIONS ///////////////////////////////////////////////////////////////////

// Dit sound
void dit()
{
   output_high(KEY_PIN);
   delay_ms(DIT_LENGTH * 100);
   output_low(KEY_PIN);
   delay_ms(DIT_LENGTH * 100);
}//func

// Dah sound
void dah()
{
   output_high(KEY_PIN);
   delay_ms(DAH_LENGTH * 100);
   output_low(KEY_PIN);
   delay_ms(DIT_LENGTH * 100);
}//func

// Silence between letters
void letter_silence()
{
   delay_ms(DAH_LENGTH * 100);
}//func

// Silence between words
void word_silence()
{
   delay_ms(SIL_LENGTH * 100);
}//func

// Spell out chars
void chars(char c)
{
   BYTE cPtr;
   BYTE cLen;
   BYTE cBin;
   signed int cPos;
   
   // Find character
   for (cPtr = 0; cPtr < M_CHARS; cPtr++)
   {
      if (morse_chars[cPtr] == c)
      {
         cLen = morse_length[cPtr];
         cBin = morse_binary[cPtr];
         
         // Word silence was encountered
         if (cBin == 0)
         {
            word_silence();
            break;
         }//if
         
         for (cPos = (cLen - 1); cPos >= 0; cPos--)
         {
            // Test if bit is 1 or 0
            if (bit_test(cBin, cPos))
            {
               dit();
            }
            else
            {
               dah();
            }//if
         }//for
         
         letter_silence();
         
         // Letter was played, exit loop
         break;
      }//if
      
   }//for
   
}//func

// Configure the DS chip
void temp_config(BYTE data)
{

   i2c_start();
   i2c_write(0x90);
   i2c_write(0xac);
   i2c_write(data);
   i2c_stop();
}//func

// Init the DS chip
void init_temp()
{
   output_high(DAL_SDA);
   output_high(DAL_SCL);
   i2c_start();
   i2c_write(0x90);
   i2c_write(0xee);
   i2c_stop();
   temp_config(8);
}//func

// Returns degrees F (-67 to 257)
signed int read_full_temp()
{  
   signed int datah, datal;
   signed int data;

   i2c_start();
   i2c_write(0x90);
   i2c_write(0xaa);
   i2c_start();
   i2c_write(0x91);
   datah=i2c_read();
   datal=i2c_read(0);
   i2c_stop();

   data=datah;
   //data=data*9;
   //if(bit_test(datal,7))
   //{
   //   if(data < 0)
   //       data -= 4;
   //   else
   //      data += 4;
   //}//if
   //data = (data / 5) + 32;

   return(data);
}//func
