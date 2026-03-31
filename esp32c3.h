#include "circularBuffer.h"

#ifndef ESP8266_DEFINED
	#define ESP8266_DEFINED
	
	// Enumerations
	enum HttpVerb { GET, POST, PUT, PATCH, DELETE};			// De HTTP-verbs voor het aanmaken van de CRUD-functies (Create, Read, Update and Delete).
	
	// Function prototypes
	void ConnectToWiFiNetwork(char ssid[], char pwd[]);
//	bool IsConnectedToWiFiNetwork(char ssid[]);
	void HttpRequest(char host[], char path[], uint16_t port, enum HttpVerb method, char data[]);
	void HttpsRequest(char host[], char path[], uint16_t port, enum HttpVerb method, char data[]);
	bool ConnectToMqttBroker(char brokerAddress[], uint16_t port, char user[], char pwd[], char mqttClientId[]);
	bool SubscribeToMqttTopic(char topic[]);
	void MqttPublish(char topic[], char data[], uint8_t qos, uint8_t retain);
#endif
