/*
 * Indicator.h
 *
 *  Created on: 12-Sep-2022
 *      Author: Muhammad Asghar
 */

#ifndef MAIN_INDICATOR_H_
#define MAIN_INDICATOR_H_



#define RED 25
#define ORANGE 26
#define GREEN 27
#define BUZZER 33
#define POWER 32

//Indicators
void all_good(char*);
void invalid_user();
void no_card_state();
void parse_date_time(char* date_time);
void publish_card_ID(uint8_t*);






#endif /* MAIN_INDICATOR_H_ */
