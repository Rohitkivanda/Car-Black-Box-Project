#ifndef MAIN_H
#define	MAIN_H

#include <xc.h>
enum menu_operation
{
    DASH_BOARD,
    MAIN_MENU,
    VIEW_LOG,
    CLEAR_LOG,
    DOWNLOAD_LOG,
    SET_TIME,
};

void dash_board(void);
void menu_display(void);
void view_log(void);
void clear_log(void);
void set_time(void);
void store_event(void);
void download_log(void);
//void store_event(char *store_array, char *time, unsigned short speed,char *gear);
void write_eeprom(char *store_array);
//static void get_time(void);
void display_time(void);

#endif	/* MAIN_H */

