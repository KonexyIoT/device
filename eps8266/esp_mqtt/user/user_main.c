/* main.c -- MQTT client example
*
* Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
#include "espconn.h"

typedef struct {
	int temp;
	int temp_set;
	int wind;
	int mode;
	int hum;
	char *power[4];
} aircon_struct_t;

aircon_struct_t aircon_struct;
const char* json_string = "{\"temp\":%d,\"temp_set\":%d,\"wind\":%d,\"mode\":%d,\"hum\":%d,\"power\":\"%s\"}";

MQTT_Client mqttClient;
static volatile os_timer_t some_timer;

void wifiConnectCb(uint8_t status) {
	if(status == STATION_GOT_IP){
		MQTT_Connect(&mqttClient);
	} else {
		MQTT_Disconnect(&mqttClient);
	}
}

void mqttConnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Connected\r\n");
	os_delay_us(1000000); // delay 1s

	MQTT_Subscribe(client, MQTT_TOPIC_CONTROL, 0);
	INFO("MQTT: Subscribe %s\r\n", MQTT_TOPIC_CONTROL);

}

void mqttDisconnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len) {
	char *topicBuf = (char*)os_zalloc(topic_len+1),
			*dataBuf = (char*)os_zalloc(data_len+1);

	MQTT_Client* client = (MQTT_Client*)args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
	os_free(topicBuf);
	os_free(dataBuf);
}

void some_timerfunc(void *arg) {
	char *pbuf;
	if (mqttClient.connState == MQTT_DATA) {
		// "{\"temp\":%d,\"temp_set\":%d,\"wind\":%d,\"mode\":%d,\"hum\":%d,\"power\":\"%s\"}";
		aircon_struct.temp = os_random() % 40 + 10;
		aircon_struct.hum = os_random() % 3;

		pbuf = (char *)os_zalloc(256);
		os_sprintf(pbuf, json_string, aircon_struct.temp, aircon_struct.temp_set,
				aircon_struct.wind, aircon_struct.mode,aircon_struct.hum, aircon_struct.power);

		MQTT_Publish(&mqttClient, MQTT_TOPIC_DIEUHOA, pbuf, strlen(pbuf), 0, 0);
		os_free(pbuf);
	}
}

void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(1000000);

	CFG_Load();

	//
	os_memset(&aircon_struct, 0x00, sizeof aircon_struct);
	os_sprintf(aircon_struct.power, "%s", "on");
	aircon_struct.temp_set = 25;
	aircon_struct.wind = 1;

	// tuanpa added
	// set SSL buffer size, if your SSL packet larger than 2048 bytes (5120 or 8192)
	espconn_secure_set_size(ESPCONN_CLIENT, 8192);

	MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);
	MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
//	MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);

	WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);

	////////////////////////////////////
	// sent interval every 10s
	os_timer_disarm(&some_timer); //Disarm timer
	os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL); //Setup timer
	os_timer_arm(&some_timer, 10000, 1); // 10s interval


	INFO("\r\nSystem started ...\r\n");
}
