/*
 * EMI_database.h
 *
 *  Created on: 26-Aug-2022
 *      Author: Muhammad Asghar
 */

#ifndef MAIN_EMI_DATABASE_H_
#define MAIN_EMI_DATABASE_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>


#define USER_NAME_LENGTH 25
#define USER_ID_LENGTH 37

#define COMPANYSIZE 2
uint8_t CardID[5];

struct user
{
	char name[USER_NAME_LENGTH];
	char EMI_id[USER_ID_LENGTH];
	uint8_t card_id[5];
};







struct saved
{
	uint8_t card_id[5];
};

void saved_IDs(uint8_t*);
uint8_t Prev_ID_check(uint8_t *Current_ID , uint8_t* Prev_ID);




#endif /* MAIN_EMI_DATABASE_H_ */
