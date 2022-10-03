/*
 * mqtt.c
 *
 *  Created on: 07-Sep-2022
 *      Author: Muhammad Asghar
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include <driver/gpio.h>

#include "esp_log.h"
#include "Indicator.h"
#include "mqtt_client.h"
#include "Nextion.h"
#include "Indicator.h"
#include "EMI_database.h"
#include "mqtt.h"


uint8_t length ;




static const char *TAG = "MQTT_EXAMPLE";

void mqtt_event_handler(void* data, esp_event_base_t base, int32_t event_id, void *event_data)
{
    printf("InSide Event Handlern\n\r");

	ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
	    esp_mqtt_event_handle_t event = event_data;
	    switch ((esp_mqtt_event_id_t)event_id)
	    {
	    case MQTT_EVENT_CONNECTED:
	    	printf("Connected\r\n");
	        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
	        Display_Nextion("t3", "Connected");
	    	gpio_set_level(ORANGE, 0);
	    	vTaskDelay(200/ portTICK_PERIOD_MS);

	        break;

	    case MQTT_EVENT_DISCONNECTED:
	        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
	        Display_Nextion("t3", "Disonnected");
	    	gpio_set_level(ORANGE, 1);
	    	gpio_set_level(BUZZER, 1);  // Buzzer Active LOW

	    	vTaskDelay(200/ portTICK_PERIOD_MS);

	        break;

	    case MQTT_EVENT_SUBSCRIBED:
	        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
	        Display_Nextion("t3", "Subscribed");
	        break;

	    case MQTT_EVENT_UNSUBSCRIBED:
	        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
	        Display_Nextion("t3", "UnSubscribed");
	    	break;

	    case MQTT_EVENT_PUBLISHED:
	        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
	    	gpio_set_level(ORANGE, 0);
	    	Display_Nextion("t3", "Published");
	    	vTaskDelay(1000/ portTICK_PERIOD_MS);
	        Display_Nextion("t3", "Connected");
	        vTaskDelay(200/ portTICK_PERIOD_MS);
			break;

	    case MQTT_EVENT_DATA:
	        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
	        printf("PUB_ATTENDANCE_TOPIC=%.*s\r\n", event->topic_len, event->topic);
	        printf("DATA=%.*s\r\n", event->data_len, event->data);
	        parse_date_time(event->data);
	        break;

	    case MQTT_EVENT_ERROR:
	        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
	        Display_Nextion("t3", "Error");
	        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
	            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
	            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
	            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
	            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
	        }
	        break;
	    default:
	        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
	        break;
	    }
}







void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
    	gpio_set_level(ORANGE, 1);
    	gpio_set_level(BUZZER, 1);  // Buzzer Active LOW

    	Display_Nextion("t2", "Connecting...\r\n");

        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
    	Display_Nextion("t2", "Connected");
    	gpio_set_level(ORANGE, 0);


        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
    	Display_Nextion("t2", "Disconnected");
    	gpio_set_level(ORANGE, 1);
    	gpio_set_level(BUZZER, 1);  // Buzzer Active LOW
    	wifi_connection();
        break;

    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
    	gpio_set_level(ORANGE, 0);
    	Display_Nextion("t2", "Connected");

        break;
    default:
        break;
    }
}


void wifi_connection()
{
    // 1 - Wi-Fi/LwIP Init Phase
    esp_netif_init();                    // TCP/IP initiation 					s1.1
    esp_event_loop_create_default();     // event loop 			                s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASSWORD}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect() ;

}

void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

void mqtt_app_start()

{
		esp_mqtt_client_config_t mqtt_cfg =
	    {
	        .uri = BROKER,
	    	//.host = BROKER,
			//.port = PORT,
			.username = USERNAME,
			.password = BROKER_PASSWORD,
	    };

		client = esp_mqtt_client_init(&mqtt_cfg);

	/* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

}



