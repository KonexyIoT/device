#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#define CFG_HOLDER	0x00FF55A4	/* Change this value to load default configurations */
#define CFG_LOCATION	0x3C	/* Please don't change or if you know what you doing */
#define CLIENT_SSL_ENABLE

/*DEFAULT CONFIGURATIONS*/

#define MQTT_HOST			"iot.konexy.com" //or "mqtt.yourdomain.com"
#define MQTT_PORT			8883
#define MQTT_BUF_SIZE		2048
#define MQTT_KEEPALIVE		120	 /*second*/

#define MQTT_CLIENT_ID		"DVES_%08X"
#define MQTT_USER			"baabeetaa"
#define MQTT_PASS			"123456"
#define MQTT_TOPIC_CONTROL	"/log/tree/my_home/dieu_hoa/control" // subcribing to ( control )
#define MQTT_TOPIC_DIEUHOA	"/log/tree/my_home/dieu_hoa" // publishing to ( log )

#define STA_SSID 			"Eposi"
#define STA_PASS 			"eposi@2015"
#define STA_TYPE AUTH_WPA2_PSK

#define MQTT_RECONNECT_TIMEOUT 	30	/*second*/

#define DEFAULT_SECURITY	1
#define QUEUE_BUFFER_SIZE		 		2048

#define PROTOCOL_NAMEv311
// PROTOCOL_NAMEv31	/*MQTT version 3.1 compatible with Mosquitto v0.15*/
//PROTOCOL_NAMEv311			/*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/
#endif
