/*
 * File:   main.c
 * Author: Rohit 
 *
 * Created on 16 December, 2024, 4:06 PM
 */


#include <xc.h>
#include "clcd.h"
#include "mkp.h"
#include "adc.h"
#include "eeprom.h"
#include "main.h"
#include "uart.h"
#include "ds1307.h"
#include "i2c.h"
char time[9] ;
unsigned int speed,index ;

unsigned char *gear[] = {"ON" ,"GN","GR","G1" ,"G2","G3" ,"G4","-c"};
unsigned char *menu[] = {"VIEW LOG       ","CLEAR LOG      ","DOWNLOAD LOG   ","SET TIME       "};
unsigned static int address = 0x00;
unsigned static int event_count;
unsigned int flag ;
unsigned char key;
unsigned int disp_flag = 0,disp_count;
unsigned int main_flag = 0;
unsigned int store_add = 0x00;
unsigned char store_array[10][15];
unsigned int index2;
unsigned char clock_reg[3];
static void init_config(void)
{
    init_clcd();
    init_adc();
    init_mkp();
    init_uart();
    init_i2c();
	init_ds1307();
}
void dash_board(void)
{
    clcd_print("TIME     EV  SPD", LINE1(0)); 
    clcd_print(time, LINE2(0)); 
         
    speed = read_adc(CHANNEL4) / 10.33;
    clcd_putch(speed/10 + 48, LINE2(14)); 
    clcd_putch(speed%10 + 48, LINE2(15)); 
       
    key = read_matrix_keypad(STATE);
        if(key == SW2)
        {
           if(index == 7)
           {
               index = 1;
           }
           else if(index < 6)
           index++;
           store_event();
        }
      
        else if(key == SW3)
        {
           if(index == 7)
           {
               index = 1;
           }
          if(index > 1)
           index--;
            store_event();
        }
      
        else if(key == SW1)
        {
           index = 7;
           store_event();
        }
         clcd_print(gear[index], LINE2(9));
        /* 
         if (key == SW4)  //entering into the main menu
            {
             
                
                        static int a = 0x00;
                           CLEAR_DISP_SCREEN;
                           clcd_print("STORED DATA     ",LINE1(0));
                          char ch[11];
                        for(int i=0;i<10;i++)
                        {
                            ch[i] = read_internal_eeprom(a++);
                        }
                          ch[10] = '\0';
                           clcd_print(ch,LINE2(0));
                           while(1);
                           for(unsigned long int wait = 100000;wait--;);
            }
          */
}

void view_log(void)
{ 
    unsigned int val;
    unsigned static int view_index = 0; // Keeps track of the current log being viewed
    unsigned static int delay2 = 0;     
    
    if (event_count == 0)
    {
        delay2++;
        if (delay2 > 0 && delay2 <= 1000)
        {
            clcd_print("ALL LOGS        ", LINE1(0));
            clcd_print("CLEARED         ", LINE2(0));
        }
        if (delay2 == 1000)
        {
            // Reset flags after delay
            flag = 1;
            disp_flag = 0;
        }
    }
    else
    {
        clcd_print("# TIME     EV  SP", LINE1(0));

        // Loop through all the logged events
        for (int i = 0; i < event_count; i++)
        {
            val = (index2 + i) % 10; // Calculate the log index in circular fashion
            store_add = val * 10;

            // Read log details from EEPROM and store them in `store_array`
            store_array[i][0] = read_internal_eeprom(store_add++);
            store_array[i][1] = read_internal_eeprom(store_add++);
            store_array[i][2] = ':';
            store_array[i][3] = read_internal_eeprom(store_add++);
            store_array[i][4] = read_internal_eeprom(store_add++);
            store_array[i][5] = ':';
            store_array[i][6] = read_internal_eeprom(store_add++);
            store_array[i][7] = read_internal_eeprom(store_add++);
            store_array[i][8] = ' ';
            store_array[i][9] = read_internal_eeprom(store_add++);
            store_array[i][10] = read_internal_eeprom(store_add++);
            store_array[i][11] = ' ';
            store_array[i][12] = read_internal_eeprom(store_add++);
            store_array[i][13] = read_internal_eeprom(store_add++);
            store_array[i][14] = '\0';
        }

        // Check if navigation keys are pressed to change the log view
        if (key == SW2 && view_index < event_count - 1)
        {
            view_index++; // Go to the next log
        }
        else if (key == SW1 && view_index > 0)
        {
            view_index--; // Go to the previous log
        }

        // Display the current log index and log data on the LCD
        clcd_putch(view_index + 48, LINE2(0)); 
        clcd_putch(' ', LINE2(1));            // Clear any extra characters
        clcd_print(store_array[view_index], LINE2(2)); // Display the current log data
    }
}


void clear_log(void)
{
    unsigned static int delay = 0;
    delay++;
    if(delay > 0 && delay <= 1000)
    {
      clcd_print("LOGS ARE CLEARED",LINE1(0));
    }
    // After the delay, clear the logs and reset relevant variables
    if(delay == 1000)
    {
        CLEAR_DISP_SCREEN;
        event_count = 0;
        address = 0;
        disp_flag = 0;
        disp_count = 0;
        flag = 1;
        
        delay = 0;
    }
}

void download_log(void)
{
    if(event_count == 0)
    {
        CLEAR_DISP_SCREEN;
        clcd_print("NO logs         ", LINE1(0));
        clcd_print("to Download  ", LINE2(0));
        for (unsigned long int wait = 500000; wait--;);
        CLEAR_DISP_SCREEN;
        flag = 1;
        disp_flag = 0; 
    }
    else
    {
        CLEAR_DISP_SCREEN;
        clcd_print("Downloading log", LINE1(0));      //message for when logs displaying in Tera term
        clcd_print("via UART", LINE2(0));
        for (unsigned long int wait = 500000; wait--;);  //delay for displaying the message

        puts("TIME      EV SP\n\r");
        for(unsigned int i =0 ;i<event_count;i++)
        {
            puts(store_array[i]);
            puts("\n\r");
        }
        for(unsigned long int wait=50000;wait--;);
    }
    flag = 1;
    disp_flag = 0;
    disp_count = 0;
}

void display_time(void) 
{
    clcd_print(time, LINE2(2));
}

void set_time(void)
{
    static unsigned int field = 0;
    static unsigned int set_delay = 0;
    static unsigned int set_flag = 0;
    static unsigned int hr = 0, min =0 , sec =0;
    clcd_print("HH MM SS",LINE1(0));
    //key = read_matrix_keypad(STATE);
    
    if(set_flag == 0)
    {
        hr = ((time[0] - 48) * 10) + (time[1] - 48);
        min = ((time[3] - 48) * 10) + (time[4] - 48);
        sec = ((time[6] - 48) * 10) + (time[7] - 48);  
        set_flag =1;
    }
    
    if(key == SW2)
    {
        field = (field + 1) % 3;
    }
    if(key == SW1)
    {
        if(field == 0)
        {
            if(hr < 23)
                hr++;
            else
                hr = 0;
        }
        else if(field == 1)
        {
            if(min < 59)
                min++;
            else
                min = 0;
        }
        else if(field == 2)
        {
            if(sec < 59)
                sec++;
            else
                sec =0 ;
        }
    }
        
        if(set_delay++ < 600)
        {
            clcd_putch((hr / 10) + '0',LINE2(0));
            clcd_putch((hr % 10) + '0',LINE2(1)); 
            clcd_putch(':',LINE2(2));
            clcd_putch((min / 10) + '0',LINE2(3));
            clcd_putch((min % 10) + '0',LINE2(4));
            clcd_putch(':',LINE2(5));
            clcd_putch((sec / 10) + '0',LINE2(6));
            clcd_putch((sec % 10) + '0',LINE2(7)); 
        }
        else if(set_delay++ < 1000)
        {
            if(field == 0)
            {
                clcd_putch(' ',LINE2(0));
                clcd_putch(' ',LINE2(1));
            }
            else if(field == 1)
            {
                clcd_putch(' ',LINE2(3));
                clcd_putch(' ',LINE2(4));
            }
            else if(field == 2)
            {
                clcd_putch(' ',LINE2(6));
                clcd_putch(' ',LINE2(7));
            }
        }
        else
        {
            set_delay = 0;
        }
    
    if(key == SW4)
    {
        write_ds1307(HOUR_ADDR, (hr / 10) << 4 | (hr % 10));
        write_ds1307(MIN_ADDR, (min / 10) << 4 | (min % 10));
        write_ds1307(SEC_ADDR, (sec / 10) << 4 | (sec % 10));
        flag = 0;
    }
    else if(key == SW5)
    {
        flag = 0;
    }
}
static void get_time(void)
{
	clock_reg[0] = read_ds1307(HOUR_ADDR);
	clock_reg[1] = read_ds1307(MIN_ADDR);
	clock_reg[2] = read_ds1307(SEC_ADDR);

	if (clock_reg[0] & 0x40)
	{
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);
		time[1] = '0' + (clock_reg[0] & 0x0F);
	}
	else
	{
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
		time[1] = '0' + (clock_reg[0] & 0x0F);
	}
	time[2] = ':';
	time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
	time[4] = '0' + (clock_reg[1] & 0x0F);
	time[5] = ':';
	time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
	time[7] = '0' + (clock_reg[2] & 0x0F);
	time[8] = '\0';
}
void menu_display(void)
{
   if(disp_flag == 0)
   {
       clcd_putch('*',LINE1(0));
       clcd_putch(' ',LINE2(0));
   }
   else if(disp_flag == 1)
   {
       clcd_putch(' ',LINE1(0));
       clcd_putch('*',LINE2(0));
   }
   clcd_print(menu[disp_count],LINE1(1));
   clcd_print(menu[disp_count + 1],LINE2(1));
    
   /* for scroll down the menu*/
   if(key == SW2 && disp_count <= 2)  
   {
       if(disp_flag == 0)
       {
            disp_flag =1;
       }
       
       else if(disp_count < 2)
       {
           disp_count++;
       }
   }
   /* for scroll up the menu */
   else if(key == SW1 && disp_count >= 0)
   {
       if(disp_flag == 1)
       {
           disp_flag = 0;
       }
       else if(disp_count > 0)
       {
           disp_count--;
       }
   }
    
}

void store_event(void)
{
    write_internal_eeprom(address++,time[0]);  
    write_internal_eeprom(address++,time[1]); 
    write_internal_eeprom(address++,time[3]); 
    write_internal_eeprom(address++,time[4]); 
    write_internal_eeprom(address++,time[6]);
    write_internal_eeprom(address++,time[7]);
 
    write_internal_eeprom(address++,gear[index][0]); 
    write_internal_eeprom(address++,gear[index][1]);
    write_internal_eeprom(address++,(speed / 10) + 48);
    write_internal_eeprom(address++,(speed % 10) + 48);
    
    if(address >= 100)
        address = 0;
    
    if(event_count < 10)
    {
        event_count++;
    }
    else
    {
        index2 = index2++  % 10;
    }
    
}


void main(void)
{
    init_config();
   
     while (1)
    {
        get_time();
        key = read_matrix_keypad(STATE);
       
        if(flag == DASH_BOARD)
        {
           dash_board();
           if(key == SW4)
           {
               flag = 1;
           }
        }
        else if(flag == MAIN_MENU)
        {
           menu_display();
           if(key == SW4)          //for enter the view log
           {
               if(disp_count == 0 && disp_flag == 0)
               {
                    CLEAR_DISP_SCREEN;
                    main_flag = 1;
                    flag = 2;
               }
               else if((disp_count == 0 && disp_flag == 1) || (disp_count == 1 && disp_flag == 0))
               {
                   CLEAR_DISP_SCREEN;
                    main_flag = 2;
                    flag = 3;
               }
               else if((disp_count == 1 && disp_flag == 1) || (disp_count == 2 && disp_flag == 0))
               {
                   CLEAR_DISP_SCREEN;
                    main_flag = 3;
                    flag = 4;
               }
               else if(disp_count == 2 && disp_flag == 1)
               {
                    CLEAR_DISP_SCREEN;
                    main_flag = 4;
                    flag = 5;
               }
           }
           else if(key == SW5) 
           {
               CLEAR_DISP_SCREEN;
               flag = 0;
               disp_count = 0;
           }
        
        }
        else if(flag == VIEW_LOG)
        {
           //clcd_print("# TIME     EV  SP",LINE1(0));

            view_log();
            if(key == SW5)
            {
                flag = 1;
            }
        }
        else if(flag == CLEAR_LOG)
        {
            clear_log();
        }
        else if(flag == DOWNLOAD_LOG)
        {
            download_log();
        }
        else if(flag == SET_TIME)
        {
            set_time();
        }
         
    }
}





