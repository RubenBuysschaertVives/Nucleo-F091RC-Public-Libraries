#include "circularBuffer.h"
#include "leds.h"
#include "usart1.h"
#include "usart2.h"
#include "stdio.h"
#include "esp32c3.h"
#include "string.h"
#include "stdlib.h"
#include "wifi.h"

// AT-commmando's reference: https://docs.espressif.com/projects/esp-at/en/release-v3.1.0.0/esp32c3/AT_Command_Set/Basic_AT_Commands.html.

// ------------------- WiFi ------------------------------------------------------------
// De verwijzing naar 'het extern circulaire buffer' ophalen.
extern volatile CircularBuffer circularBuffer;

// TODO: timeout en/of error verwerking voorzien.
void ConnectToWiFiNetwork(char ssid[], char pwd[])
{
	enum CircularBufferActionResult circularBufferActionResult;
	char text[CHAR_ARRAY_BUFFER_LENGTH];
	
	// Visueel aangeven dat er geen verbinding is.
	ClearLed(0);
	
	// Circulair buffer leegmaken.
	InitCircularBuffer(&circularBuffer);
	
	// Instellingen niet opslaan in flash.
	StringToUsart1("AT+SYSSTORE=0\r\n");
	while(!LookForString1(&circularBuffer, "OK\r\n"));
	
	// Niet automatisch verbinden bij power up.
	StringToUsart1("AT+CWAUTOCONN=0\r\n");
	while(!LookForString1(&circularBuffer, "OK\r\n"));

	// Instellen op station mode en niet automatisch verbinden met het access point.
	StringToUsart1("AT+CWMODE=1,0\r\n");
	while(!LookForString1(&circularBuffer, "OK\r\n"));
	
	// Reconnect instellen: na 10 seconden herproberen en dat maximum 100 keer.
	StringToUsart1("AT+CWRECONNCFG=10,100\r\n");
	while(!LookForString1(&circularBuffer, "OK\r\n"));
	
	// Set to station mode and receive IP via DHCP.
	StringToUsart1("AT+CWDHCP=1,1\r\n");
	while(!LookForString1(&circularBuffer, "OK\r\n"));
	
	// Disable multiple connections.
	StringToUsart1("AT+CIPMUX=0\r\n");
	while(!LookForString1(&circularBuffer, "OK\r\n"));
	
	// List available access points.
	StringToUsart1("AT+CWLAP\r\n");
	while(!LookForString1(&circularBuffer, "OK\r\n"));
	
	// Verbinden met WiFi-netwerk. Wacht op: "WIFI GOT IP\r\n\r\nOK\r\n" of "OK\r\n".
	sprintf(text, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);
	StringToUsart1(text);
	while(!LookForString1(&circularBuffer, "OK\r\n"));
	
	// MAC-adres opvragen.
	StringToUsart1("AT+CIPSTAMAC?\r\n");
	while(!LookForString1(&circularBuffer, "OK\r\n"));
	
	// IP-adres opvragen.
	StringToUsart1("AT+CIPSTA?\r\n");
	while(!LookForString1(&circularBuffer, "OK\r\n"));
	
	// Controleren of er verbinding is met het access point EN er een IPv4-adres is.
	StringToUsart1("AT+CWSTATE?\r\n");
	sprintf(text, "+CWSTATE:2,\"%s\"\r\n", ssid);
	while(!LookForString1(&circularBuffer, text));
	while(!LookForString1(&circularBuffer, "OK\r\n"));
	
	// Visueel aangeven dat er verbinding is.
	SetLed(0);
}


// ------------------- HTTP ------------------------------------------------------------
// Functie die een HTTP-request kan versturen over TCP. De response op deze request,
// moet verwerkt worden in de main() door het circulair buffer uit te lezen en te zoeken
// naar lijnen die beginnen met: +IPD...
// TODO: timeout voorzien.
void HttpRequest(char host[], char path[], uint16_t port, enum HttpVerb method, char data[])
{
	static char httpText[CHAR_ARRAY_BUFFER_LENGTH];			// Static, zodat de inhoud kan gebruikt worden als return-waarde.
	char text[CHAR_ARRAY_BUFFER_LENGTH];
	uint8_t foundString = 0;
	bool waitingForData = true;
	uint16_t numberOfReceivedBytes = 0;
	uint8_t contentLength = 0;
	enum CircularBufferActionResult circularBufferActionResult;

	// Zoek de huidig status op van de TCP-verbinding.
	StringToUsart1("AT+CIPSTATE?\r\n");
	while(PopStringFromCircularBuffer(&circularBuffer, text) != readStringSucceeded);		// Echo van "AT-CPISTART" weggooien.
	while(PopStringFromCircularBuffer(&circularBuffer, text) != readStringSucceeded);		// Antwoord van "AT-CIPSTART" opvragen.
	// Als het niet van de vorm "+CIPSTATE:0,"SSL","34.107.226.223",443,63455,0" is, dan herstarten.
	if(text[0] != '+')
	{
		// OK zoeken.
		while(!LookForString1(&circularBuffer, "OK\r\n"));
		
		// Start een (nieuwe) TCP-verbinding met de server.
		sprintf(httpText, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", host, port); 
		StringToUsart1(httpText);
		// TODO: nog ERROR opvangen en nieuwe verbinding maken?
		while(!LookForString1(&circularBuffer, "OK\r\n"));
	}				
	
	// Bouw het HTTP-pakket op om straks te versturen naar de server. Maak gebruik van het juiste 'HTTP-verb'.
	switch(method)
	{
		case GET:
			sprintf(httpText, "GET %s?%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, data, host);
			break;
		
		case POST:
			contentLength = strlen(data);		
			sprintf(httpText, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s", path, host, contentLength, data);	
			break;
		
//		case PUT:
//			contentLength = strlen(parameters);		
//			sprintf(httpText, "PUT %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s", path, host, contentLength, parameters);	
//			break;
		
//		case PATCH:
//			contentLength = strlen(data);		
//			sprintf(httpText, "PATCH %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %d\r\n\r\n%s", path, host, contentLength, data);	
//			break;
		
//		case DELETE:
//			contentLength = strlen(parameters);		
//			sprintf(httpText, "DELETE %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s", path, host, contentLength, parameters);	
//			break;
		
		default:
			break;
	}
	
	// HTTP-pakket verzenden over TCP.
	sprintf(text, "AT+CIPSEND=%d\r\n", strlen(httpText));
	StringToUsart1(text);
	// TODO: ook hier een timeout toevoegen.
	// Wachten op '>'-karakter.
	while(!LookForString1(&circularBuffer,">\r\n"));
	StringToUsart1(httpText);
	while(!LookForString1(&circularBuffer, "SEND OK\r\n"));
	
//	// Ter info tonen wat verstuurd werd (optioneel).
//	sprintf(text, "HTTP-data sent: \r\n%s\r\n", httpText);
//	StringToUsart2(text);	
}


// ------------------- HTTPS -----------------------------------------------------------
// Functie die een HTTPS-request kan versturen over TCP. De response op deze request,
// moet verwerkt worden in de main() door het circulair buffer uit te lezen en te zoeken
// naar lijnen die beginnen met: +IPD...
// TODO: timeout voorzien.
void HttpsRequest(char host[], char path[], uint16_t port, enum HttpVerb method, char data[])
{
	static char httpText[CHAR_ARRAY_BUFFER_LENGTH];			// Static, zodat de inhoud kan gebruikt worden als return-waarde.
	char text[CHAR_ARRAY_BUFFER_LENGTH];
	uint8_t foundString = 0;
	bool waitingForData = true;
	uint16_t numberOfReceivedBytes = 0;
	uint8_t contentLength = 0;
	enum CircularBufferActionResult circularBufferActionResult;

	// Zoek de huidig status op van de TCP-verbinding.
	StringToUsart1("AT+CIPSTATE?\r\n");
	while(PopStringFromCircularBuffer(&circularBuffer, text) != readStringSucceeded);		// Echo van "AT-CPISTART" weggooien.
	while(PopStringFromCircularBuffer(&circularBuffer, text) != readStringSucceeded);		// Antwoord van "AT-CIPSTART" opvragen.
	// Als het niet van de vorm "+CIPSTATE:0,"SSL","34.107.226.223",443,63455,0" is, dan herstarten.
	if(text[0] != '+')
	{
		// OK zoeken.
		while(!LookForString1(&circularBuffer, "OK\r\n"));
		
		// Start een (nieuwe) TCP-verbinding met de server.
		sprintf(httpText, "AT+CIPSTART=\"SSL\",\"%s\",%d\r\n", host, port); 
		StringToUsart1(httpText);
		// TODO: nog ERROR opvangen en nieuwe verbinding maken?
		while(!LookForString1(&circularBuffer, "OK\r\n"));
	}				
	
	// Bouw het HTTP-pakket op om straks te versturen naar de server. Maak gebruik van het juiste 'HTTP-verb'.
	switch(method)
	{
		case GET:
			sprintf(httpText, "GET %s?%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, data, host);
			break;
		
		case POST:
			contentLength = strlen(data);		
			sprintf(httpText, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s", path, host, contentLength, data);	
			break;
		
//		case PUT:
//			contentLength = strlen(parameters);		
//			sprintf(httpText, "PUT %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s", path, host, contentLength, parameters);	
//			break;
		
		case PATCH:
			contentLength = strlen(data);		
			sprintf(httpText, "PATCH %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %d\r\n\r\n%s", path, host, contentLength, data);	
			break;
		
//		case DELETE:
//			contentLength = strlen(parameters);		
//			sprintf(httpText, "DELETE %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s", path, host, contentLength, parameters);	
//			break;
		
		default:
			break;
	}
	
	// HTTP-pakket verzenden over TCP.
	sprintf(text, "AT+CIPSEND=%d\r\n", strlen(httpText));
	StringToUsart1(text);
	// TODO: ook hier een timeout toevoegen.
	// Wachten op '>'-karakter.
	while(!LookForString1(&circularBuffer,">\r\n"));
	StringToUsart1(httpText);
	while(!LookForString1(&circularBuffer, "SEND OK\r\n"));
	
//	// Ter info tonen wat verstuurd werd (optioneel).
//	sprintf(text, "HTTPS-data sent: \r\n%s\r\n", httpText);
//	StringToUsart2(text);	
}


// ------------------- MQTT -----------------------------------------------------------
// Functie die verbinding maakt met een MQTT-broker, zonder te abonneren op een topic.
// TODO: timeout voorzien.
bool ConnectToMqttBroker(char brokerAddress[], uint16_t port, char user[], char pwd[], char mqttClientId[])
{
	char atCommand[CHAR_ARRAY_BUFFER_LENGTH];
			
	// User configuration instellen.
	// https://docs.espressif.com/projects/esp-at/en/release-v3.1.0.0/esp32c3/AT_Command_Set/MQTT_AT_Commands.html#cmd-mqttusercfg
	// Niet beveiligde MQTT.
//	sprintf(atCommand, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n", mqttClientId, user, pwd);
	// Beveiligde MQTT via TLS.
	sprintf(atCommand, "AT+MQTTUSERCFG=0,2,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n", mqttClientId, user, pwd);
	StringToUsart1(atCommand);
	while(!LookForString1(&circularBuffer, "OK\r\n"));
	
	// Verbinding instellen met Last Will en Testament (optioneel). Timeout is hier standaard 120 seconden.
	#if defined(LAST_WILL_TESTAMENT_TOPIC) & defined(LAST_WILL_TESTAMENT_MESSAGE)
		sprintf(atCommand, "AT+MQTTCONNCFG=0,0,0,\"%s\",\"%s\",0,1\r\n", LAST_WILL_TESTAMENT_TOPIC, LAST_WILL_TESTAMENT_MESSAGE);
		StringToUsart1(atCommand);
		while(!LookForString1(&circularBuffer,"OK\r\n"));
	#endif
	
	// Verbinding met de broker maken.
	sprintf(atCommand, "AT+MQTTCONN=0,\"%s\",%d,0\r\n", brokerAddress, port);
	StringToUsart1(atCommand);
	while(!LookForString1(&circularBuffer,"OK\r\n"));
	
//	// Verbinding controleren (optioneel).
//	StringToUsart1("AT+MQTTCONN?\r\n");
//	while(!LookForString1(&circularBuffer,"OK\r\n"));
	
	return true;
}

// Functie om te abonneren op een MQTT-topic. Zorg zeker dat je eerst ConnectToMqttBroker() uitvoert.
// De response op dit abonnement, moet verwerkt worden in de main() door het circulair buffer uit te 
// lezen en te zoeken naar lijnen die beginnen met: +MQTTSUBRECV...
// TODO: timeout voorzien.
bool SubscribeToMqttTopic(char topic[])
{
	char atCommand[CHAR_ARRAY_BUFFER_LENGTH];
	
//	// Mogelijks abonnement op een topic afzeggen (optioneel).
//	sprintf(atCommand, "AT+MQTTUNSUB=0,\"%s\"\r\n", topic);
//	StringToUsart1(atCommand);
//	while(!LookForString1(&circularBuffer,"OK\r\n"));
	
	// Abonneren op een topic.
	sprintf(atCommand, "AT+MQTTSUB=0,\"%s\",0\r\n", topic);
	StringToUsart1(atCommand);
	while(!LookForString1(&circularBuffer,"OK\r\n"));			
	// Als er info aankomt op een bepaald topic, is dat van de vorm: "+MQTTSUBRECV:0,"rubu/esp32c3...",9,Joehoe!!!"
	
	return true;
}

void MqttPublish(char topic[], char data[], uint8_t qos, uint8_t retain)	
{
	char text[CHAR_ARRAY_BUFFER_LENGTH];
	
//	// LET OP: het MQTTPUB raakt in de war als er komma's in het data gedeelte zitten! Gebruik MQTTPUBRAW indien komma's gewenst zijn...?
//	// Data publish'n op een topic.
//	sprintf(atCommand, "AT+MQTTPUB=0,\"%s\",\"%s\",0,0\r\n", topic, data);
//	StringToUsart1(atCommand);
//	while(!LookForString1(&circularBuffer,"OK\r\n"));	
	
	// Data publish'n op een topic.
	sprintf(text, "AT+MQTTPUBRAW=0,\"%s\",%d,%d,%d\r\n", topic, strlen(data), qos, retain);
	StringToUsart1(text);
	while(!LookForString1(&circularBuffer,"OK\r\n"));	
	
	// TODO: ook hier een timeout toevoegen.
	// Wachten op '>'-karakter.
	while(!LookForString1(&circularBuffer,">\r\n"));
	StringToUsart1(data);
	while(!LookForString1(&circularBuffer, "+MQTTPUB:OK\r\n"));
				
//	// Ter info tonen wat verstuurd werd.
//	sprintf(text, "MQTT-data published: %s\r\n\r\n", data);
//	StringToUsart2(text);	
}
