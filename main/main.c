// mosquitto_sub -h "Server_name" -t "topic_name" -d -q 2
// mosquitto_pub -h "Server_name" -t "topic_name" -m "message" -d -q 2

// mosquitto_sub -h test.mosquitto.org -t my_t -d -q 2
// mosquitto_pub -h test.mosquitto.org -t my_t -m message -d -q 2


//TODOs
// Screen
// Wifi ki Library Alg krni ha
// Commenting
// schmetics

#include <stdio.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <esp_event.h>
#include <esp_system.h>
#include "driver/uart.h"
#include "cJSON.h"
#include "mqtt_client.h"

// My Files
#include "TM_MFRC522.h"
#include "Nextion.h"
#include "EMI_database.h"
#include "rfid.h"
#include "mqtt.h"
#include "Indicator.h"
#include <driver/gpio.h>




// RFID
uint8_t Prev_ID[5]={};


const rc522_start_args_t start_args =
	{
    .miso_io  = 19,
    .mosi_io  = 23,
    .sck_io   = 18,
    .sda_io   = 5,
	};


void app_main(void)
{
	//Indicators
	gpio_set_direction(BUZZER, GPIO_MODE_OUTPUT);
	gpio_set_pull_mode(BUZZER, GPIO_PULLDOWN_ONLY );

	gpio_set_direction(ORANGE, GPIO_MODE_OUTPUT);
	gpio_set_pull_mode(ORANGE, GPIO_PULLDOWN_ONLY );

	gpio_set_direction(GREEN, GPIO_MODE_OUTPUT);
	gpio_set_pull_mode(GREEN, GPIO_PULLDOWN_ONLY );

	gpio_set_direction(RED, GPIO_MODE_OUTPUT);
	gpio_set_pull_mode(RED, GPIO_PULLDOWN_ONLY );

	gpio_set_direction(POWER, GPIO_MODE_OUTPUT);
	gpio_set_pull_mode(POWER, GPIO_PULLDOWN_ONLY );



	// Initializations
	nvs_flash_init();
	uart_init();
	rc522_start(start_args);
	wifi_connection();
	mqtt_app_start();



	xTaskCreatePinnedToCore(Task1_RFID_and_Publishing,                                          //Task Handler / Funtion
							"This read Cards and Publish data to the MQTT broker",					//Task description
							2048,                        										//Stack Size for Task
							NULL,																//Parameter to Task
							2,																	//Task Priority
							NULL,																//Task Handler
							0);																	// Core ID

	xTaskCreatePinnedToCore(Task2_Subscription,			                                        //Task Handler / Funtion
							"Its gets data from broker",										//Task description
							2048,                        										//Stack Size for Task
							NULL,																//Parameter to Task
							1,																	//Task Priority
							NULL,																//Task Handler
							1);
}


void Task1_RFID_and_Publishing()
{
    gpio_set_level(POWER, 1);  //Power Led ON

	//Define Company details below
    struct user User0 = {"M.Hammad Umer \r\n" , "Hammad_ID" , {0x17 , 0x21 , 0xE6 , 0x35 , 0xE5}};
    struct user User1 = {"Issac Denial \r\n" , "ISSAC_ID" , {0x1B , 0xDB , 0x5F , 0x0A , 0x95}};

    struct user Users[] = {User0, User1};

	while(true)
	{
		if ((TM_MFRC522_Check(CardID) == MI_OK))
			{
				for(uint8_t i = 0 ; i < COMPANYSIZE ; i++ )
					{
						if(TM_MFRC522_Compare(CardID, Users[i].card_id) == MI_OK)
							{
								if(Prev_ID_check(Users[i].card_id ,Prev_ID ) == 0)
									{
										esp_mqtt_client_publish(client, PUB_ATTENDANCE_TOPIC, Users[i].EMI_id , 0, 2, 0);
										all_good(Users[i].name);
										for(uint8_t j = 0 ; j < 5 ; j++)
											{
												Prev_ID[j] = Users[i].card_id[j];
												printf("%x " , Prev_ID[j]);
											}
									}
								else if(Prev_ID_check(Prev_ID , Users[i].card_id) == 1)
									{
										Display_Nextion("t4", "Remove your Card");
									}
								break;
							}
						else if ((i == (COMPANYSIZE-1)) && (TM_MFRC522_Compare(CardID, Users[i].card_id) != MI_OK))
							{
								invalid_user();
							}
					 }
			}
		else
			{
				uint32_t j = 0 ;  										//variable to count ticks
				while(j <= 3000000000)                                  //loop to reset RFID as it may burn if not reset
					{
						no_card_state();
							for(uint8_t i = 0 ; i < 5 ; i++ )
								{
									Prev_ID[i] = 0x00;
								}
							if(j == 3000000000)
							{
								rc522_start(start_args);
							}
							else if(TM_MFRC522_Check(CardID) == MI_OK)
								{
									break;
								}
							else
								{
									j++;
								}
					}
			}
	}

}

void Task2_Subscription()
{
	while(1)
	{
			esp_mqtt_client_subscribe(client, SUB_TIME_TOPIC, 2);
			vTaskDelay(5000/portTICK_PERIOD_MS);
	}

}



