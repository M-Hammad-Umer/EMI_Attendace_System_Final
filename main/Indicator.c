/*
 * Indicator.c
 *
 *  Created on: 12-Sep-2022
 *      Author: Muhammad Asghar
 */

#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt.h"
#include "Nextion.h"
#include "Indicator.h"

void all_good(char* name)
{
	Display_Nextion("t4", name);
	Display_Nextion("t5", "Signed In");
	gpio_set_level(GREEN, 1);  					//ACK Led ON
	gpio_set_level(BUZZER, 0);
	vTaskDelay(150/ portTICK_PERIOD_MS);
	gpio_set_level(BUZZER, 1);
	vTaskDelay(100/ portTICK_PERIOD_MS);
	gpio_set_level(BUZZER, 0);
	vTaskDelay(100/ portTICK_PERIOD_MS);
	gpio_set_level(BUZZER, 1);
	gpio_set_level(GREEN, 0);
	vTaskDelay(650/ portTICK_PERIOD_MS);
	Display_Nextion("t5", "Thankyou");
	vTaskDelay(1000/ portTICK_PERIOD_MS);
}



void invalid_user()
{
	Display_Nextion("t4", "Invalid Card");
	Display_Nextion("t5", "User Not Found");
	gpio_set_level(RED, 1);  					//ACK Led ON
	gpio_set_level(BUZZER, 0);

	vTaskDelay(250/ portTICK_PERIOD_MS);
	gpio_set_level(RED, 0);
	vTaskDelay(250/ portTICK_PERIOD_MS);
}


void no_card_state()
{

	Display_Nextion("t4", "Place your card");
	Display_Nextion("t5", "");

	gpio_set_level(GREEN, 0);  					//ACK Led ON
	gpio_set_level(RED, 0);
	gpio_set_level(BUZZER, 1);
}




uint8_t Prev_ID_check(uint8_t *Current_ID , uint8_t* Prev_ID)
{
	uint8_t count = 0;
	for(uint8_t i = 0 ; i < 5 ; i++)
	{
		if( Current_ID[i] == Prev_ID[i])
		{
			count++;
		}
	}
	if(count == 5)
	{
		return 1;
	}
	return 0;
}




void parse_date_time(char* date_time)
{
    char date[30];
    char time[30];

    int i = 0;
    while(date_time[i] != '\0')
    {
        if(date_time[i] == 44){
            memcpy(date, &date_time[0], i);
            date[i] = '\0';
            break;
        }
        i++;
    }


    memcpy(time, &date_time[i+1], strlen(date_time));
    time[strlen(date_time) - i] = '\0';
    Display_Nextion("t0" , date);
    Display_Nextion("t1" , time);
}




void publish_card_ID( uint8_t* CardID)
{
	char buffer[10];

		for(uint8_t i = 0 ; i < 5 ; i++)
		{
			itoa(CardID[i] , &buffer[i+(i*1)] , 16);
		}

		esp_mqtt_client_publish(client, PUB_CARD_ID_TOPIC , buffer , 0, 2, 0);


//	Display_Nextion("t4", buffer);
//	vTaskDelay(3000/ portTICK_PERIOD_MS);
}


