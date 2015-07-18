// PMDG777SX-SERVER.cpp : Defines the entry point for the console application.
//

#define WINVER                  WindowsVista
#define _WIN32_WINDOWS          WindowsVista
#define _WIN32_WINNT            WindowsVista
#define NTDDI_VERSION NTDDI_VISTA
#define WINVER _WIN32_WINNT_VISTA
#define _WIN32_WINNT _WIN32_WINNT_VISTA


#include "stdafx.h"
#include <windows.h>
#include <string.h>

#define SERVERPORT "4950"	// the port users will be connecting to
#define MAXBUFLEN 100

int     quit = 0;
HANDLE  hSimConnect = NULL;
bool    AircraftRunning = false;
PMDG_777X_Control Control;

static enum DATA_REQUEST_ID {
	DATA_REQUEST,
	CONTROL_REQUEST,
	AIR_PATH_REQUEST
};

static enum EVENT_ID {
	EVENT_SIM_START,	// used to track the loaded aircraft

	EVENT_LOGO_LIGHT_SWITCH,
	EVENT_FLIGHT_DIRECTOR_SWITCH,

	EVENT_KEYBOARD_A,
	EVENT_KEYBOARD_B,
	EVENT_KEYBOARD_C,
};

static enum INPUT_ID {
	INPUT0			// used to handle key presses
};

static enum GROUP_ID {
	GROUP_KEYBOARD		// used to handle key presses
};

int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;

inline char* BoolToString(bool b)
{
	return b ? "true" : "false";
}

void sendDataBool(char* name, bool value) {
	char result[100];   // array to hold the result.

	strcpy_s(result, 100, name); // copy string one into the result.
	strcat_s(result, 100, ": ");
	strcat_s(result, 100, BoolToString(value)); // append string two to the result.

	if ((numbytes = sendto(sockfd, result, strlen(result), 0,
		p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}
	printf("sent packet\n");
}

void sendDataChar(char* name, unsigned char value){
	char result[100];   // array to hold the result.

	strcpy_s(result, 100, name); // copy string one into the result.
	strcat_s(result, 100, ": ");
	strcat_s(result, 100, value); // append string two to the result.

	if ((numbytes = sendto(sockfd, result, strlen(result), 0,
		p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}
	printf("sent packet\n");
}

bool B777X_FuelPumpLAftLight = true;
bool B777X_TaxiLightSwitch = false;
bool B777X_LogoLightSwitch = false;
bool B777X_LandingLightLeftSwitch = false;
bool B777X_WindowHeatBackupSwitch1 = false;
bool B777X_WindowHeatBackupSwitch2 = false;
unsigned char B777X_ElecStandbyPowerSwitch = false;
bool B777X_WingHydraulicValveSwitch1 = false;
bool B777X_WingHydraulicValveSwitch2 = false;
bool B777X_WingHydraulicValveSwitch3 = false;
bool B777X_TailHydraulicValveSwitch1 = false;
bool B777X_TailHydraulicValveSwitch2 = false;
bool B777X_TailHydraulicValveSwitch3 = false;
bool B777X_ApuPowerTestSwitch = false;
bool B777X_EngineEECPowerTestSwitch1 = false;
bool B777X_EngineEECPowerTestSwitch2 = false;

// This function is called when 777X data changes
//TODO: come up with a better way of doing this
void Process777XData(PMDG_777X_Data *pS)
{
	if (pS->ENG_EECPower_Sw_TEST[1] != B777X_EngineEECPowerTestSwitch2)
	{
		B777X_EngineEECPowerTestSwitch2 = pS->ENG_EECPower_Sw_TEST[1];
		if (B777X_EngineEECPowerTestSwitch2 == 0)
			printf("Engine EEC Power Test Switch 2: [OFF]\n");
		else
			printf("Engine EEC Power Test Switch 2: [ON]\n");

		//send packets
		sendDataBool("ENG_EECPower_Sw_TEST2", B777X_EngineEECPowerTestSwitch2);
	}

	if (pS->ENG_EECPower_Sw_TEST[0] != B777X_EngineEECPowerTestSwitch1)
	{
		B777X_EngineEECPowerTestSwitch1 = pS->ENG_EECPower_Sw_TEST[0];
		if (B777X_EngineEECPowerTestSwitch1 == 0)
			printf("Engine EEC Power Test Switch 1: [OFF]\n");
		else
			printf("Engine EEC Power Test Switch 1: [ON]\n");

		//send packets
		sendDataBool("ENG_EECPower_Sw_TEST1", B777X_EngineEECPowerTestSwitch1);
	}

	if (pS->APU_Power_Sw_TEST != B777X_ApuPowerTestSwitch)
	{
		B777X_ApuPowerTestSwitch = pS->APU_Power_Sw_TEST;
		if (B777X_ApuPowerTestSwitch == 0)
			printf("APU POWER TEST SWITCH: [OFF]\n");
		else
			printf("APU POWER TEST SWITCH: [ON]\n");

		//send packets
		sendDataBool("APU_Power_Sw_TEST", B777X_ApuPowerTestSwitch);
	}

	if (pS->FCTL_TailHydValve_Sw_SHUT_OFF[1] != B777X_TailHydraulicValveSwitch3)
	{
		B777X_TailHydraulicValveSwitch3 = pS->FCTL_TailHydValve_Sw_SHUT_OFF[1];
		if (B777X_TailHydraulicValveSwitch3 == 0)
			printf("TAIL HYDRAULIC VALVE SWITCH #3: [NORMAL]\n");
		else
			printf("TAIL HYDRAULIC VALVE SWITCH #3: [SHUT OFF]\n");

		sendDataBool("FCTL_TailHydValve_Sw_SHUT_OFF3", B777X_TailHydraulicValveSwitch3);

		//send packets
	}

	if (pS->FCTL_TailHydValve_Sw_SHUT_OFF[2] != B777X_TailHydraulicValveSwitch2)
	{
		B777X_TailHydraulicValveSwitch2 = pS->FCTL_TailHydValve_Sw_SHUT_OFF[2];
		if (B777X_TailHydraulicValveSwitch2 == 0)
			printf("TAIL HYDRAULIC VALVE SWITCH #2: [NORMAL]\n");
		else
			printf("TAIL HYDRAULIC VALVE SWITCH #2: [SHUT OFF]\n");


		sendDataBool("FCTL_TailHydValve_Sw_SHUT_OFF2", B777X_TailHydraulicValveSwitch2);
		//send packets
	}

	if (pS->FCTL_TailHydValve_Sw_SHUT_OFF[0] != B777X_TailHydraulicValveSwitch1)
	{
		B777X_TailHydraulicValveSwitch1 = pS->FCTL_TailHydValve_Sw_SHUT_OFF[0];
		if (B777X_TailHydraulicValveSwitch1 == 0)
			printf("TAIL HYDRAULIC VALVE SWITCH #1: [NORMAL]\n");
		else
			printf("TAIL HYDRAULIC VALVE SWITCH #1: [SHUT OFF]\n");

		sendDataBool("FCTL_TailHydValve_Sw_SHUT_OFF1", B777X_TailHydraulicValveSwitch1);
		//send packets
	}

	if (pS->FCTL_WingHydValve_Sw_SHUT_OFF[1] != B777X_WingHydraulicValveSwitch3)
	{
		B777X_WingHydraulicValveSwitch3 = pS->FCTL_WingHydValve_Sw_SHUT_OFF[1];
		if (B777X_WingHydraulicValveSwitch3 == 0)
			printf("WING HYDRAULIC VALVE SWITCH #3: [NORMAL]\n");
		else
			printf("WING HYDRAULIC VALVE SWITCH #3: [SHUT OFF]\n");

		//send packets
		sendDataBool("FCTL_WingHydValve_Sw_SHUT_OFF3", B777X_WingHydraulicValveSwitch3);
	}

	if (pS->FCTL_WingHydValve_Sw_SHUT_OFF[2] != B777X_WingHydraulicValveSwitch2)
	{
		B777X_WingHydraulicValveSwitch2 = pS->FCTL_WingHydValve_Sw_SHUT_OFF[2];
		if (B777X_WingHydraulicValveSwitch2 == 0)
			printf("WING HYDRAULIC VALVE SWITCH #2: [NORMAL]\n");
		else
			printf("WING HYDRAULIC VALVE SWITCH #2: [SHUT OFF]\n");

		//send packets
		sendDataBool("FCTL_WingHydValve_Sw_SHUT_OFF2", B777X_WingHydraulicValveSwitch2);
	}

	if (pS->FCTL_WingHydValve_Sw_SHUT_OFF[0] != B777X_WingHydraulicValveSwitch1)
	{
		B777X_WingHydraulicValveSwitch1 = pS->FCTL_WingHydValve_Sw_SHUT_OFF[0];
		if (B777X_WingHydraulicValveSwitch1 == 0)
			printf("WING HYDRAULIC VALVE SWITCH #1: [NORMAL]\n");
		else
			printf("WING HYDRAULIC VALVE SWITCH #1: [SHUT OFF]\n");

		//send packets
		sendDataBool("FCTL_WingHydValve_Sw_SHUT_OFF1", B777X_WingHydraulicValveSwitch1);
	}

	if (pS->ELEC_StandbyPowerSw != B777X_ElecStandbyPowerSwitch)
	{
		B777X_ElecStandbyPowerSwitch = pS->ELEC_StandbyPowerSw;
		if (B777X_ElecStandbyPowerSwitch == 0)
			printf("ELECTRIC STANDBY POWER SWITCH #1: [OFF]\n");
		else if (B777X_ElecStandbyPowerSwitch == 1)
			printf("ELECTRIC STANDBY POWER SWITCH #1: [AUTO]\n");
		else
			printf("ELECTRIC STANDBY POWER SWITCH #1: [BAT]\n");

		//send packets
		SendDataChar("B777X_ElecStandbyPowerSwitch", B777X_ElecStandbyPowerSwitch);
	}

	if (pS->ICE_WindowHeatBackUp_Sw_OFF[0] != B777X_WindowHeatBackupSwitch1)
	{
		B777X_WindowHeatBackupSwitch1 = pS->ICE_WindowHeatBackUp_Sw_OFF[0];
		if (!B777X_WindowHeatBackupSwitch1)
			printf("WINDOW HEAT BACKUP SWITCH #1: [ON]\n");
		else
			printf("WINDOW HEAT BACKUP SWITCH #1: [OFF]\n");

		//send packets
		sendDataBool("ICE_WindowHeatBackUp_Sw_OFF1", B777X_WindowHeatBackupSwitch1);
	}

	if (pS->ICE_WindowHeatBackUp_Sw_OFF[1] != B777X_WindowHeatBackupSwitch2)
	{
		B777X_WindowHeatBackupSwitch2 = pS->ICE_WindowHeatBackUp_Sw_OFF[1];
		if (!B777X_WindowHeatBackupSwitch2)
			printf("WINDOW HEAT BACKUP SWITCH #2: [ON]\n");
		else
			printf("WINDOW HEAT BACKUP SWITCH #2: [OFF]\n");

		//send packets
		sendDataBool("ICE_WindowHeatBackUp_Sw_OFF2", B777X_WindowHeatBackupSwitch2);
	}

	// get the state of switches and save it for later use
	if (pS->LTS_Taxi_Sw_ON != B777X_TaxiLightSwitch)
	{
		B777X_TaxiLightSwitch = pS->LTS_Taxi_Sw_ON;
		if (B777X_TaxiLightSwitch)
			printf("TAXI LIGHTS: [ON]\n");
		else
			printf("TAXI LIGHTS: [OFF]\n");

		//send packets
		sendDataBool("LTS_Taxi_Sw_ON", B777X_TaxiLightSwitch);
	}

	if (pS->LTS_LandingLights_Sw_ON[1] != B777X_LandingLightLeftSwitch)
	{
		B777X_LandingLightLeftSwitch = pS->LTS_LandingLights_Sw_ON[1];
		if (B777X_LandingLightLeftSwitch)
			printf("LEFT LANDING LIGHTS: [ON]\n");
		else
			printf("LEFT LANDING LIGHTS: [OFF]\n");

		//send packets
		sendDataBool("LTS_LandingLights_Sw_ON1", B777X_LandingLightLeftSwitch);
	}

	if (pS->LTS_Logo_Sw_ON != B777X_LogoLightSwitch)
	{
		B777X_LogoLightSwitch = pS->LTS_Logo_Sw_ON;
		if (B777X_LogoLightSwitch)
			printf("LOGO LIGHTS: [ON]\n");
		else
			printf("LOGO LIGHTS: [OFF]\n");


		//send packets
		sendDataBool("LTS_Logo_Sw_ON", B777X_LogoLightSwitch);
	}
}


void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
{
	switch (pData->dwID)
	{
	case SIMCONNECT_RECV_ID_CLIENT_DATA: // Receive and process the 777X data block
	{
		SIMCONNECT_RECV_CLIENT_DATA *pObjData = (SIMCONNECT_RECV_CLIENT_DATA*)pData;

		switch (pObjData->dwRequestID)
		{
		case DATA_REQUEST:
		{
			PMDG_777X_Data *pS = (PMDG_777X_Data*)&pObjData->dwData;
			Process777XData(pS);
			break;
		}
		case CONTROL_REQUEST:
		{
			// keep the present state of Control area to know if the server had received and reset the command
			PMDG_777X_Control *pS = (PMDG_777X_Control*)&pObjData->dwData;
			Control = *pS;
			break;
		}
		}
		break;
	}

	case SIMCONNECT_RECV_ID_EVENT:
	{
		SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT*)pData;
		switch (evt->uEventID)
		{
		case EVENT_SIM_START:	// Track aircraft changes
		{
			HRESULT hr = SimConnect_RequestSystemState(hSimConnect, AIR_PATH_REQUEST, "AircraftLoaded");
			break;
		}
		}
		break;
	}

	case SIMCONNECT_RECV_ID_SYSTEM_STATE: // Track aircraft changes
	{
		SIMCONNECT_RECV_SYSTEM_STATE *evt = (SIMCONNECT_RECV_SYSTEM_STATE*)pData;
		if (evt->dwRequestID == AIR_PATH_REQUEST)
		{
			if (strstr(evt->szString, "PMDG 777") != NULL) {
				printf("PMDG Running\n");
				AircraftRunning = true;
			}
			else {
				printf("PMDG Stopped\n");
				AircraftRunning = false;
			}

		}
		break;
	}

	case SIMCONNECT_RECV_ID_QUIT:
	{
		quit = 1;
		break;
	}

	default:
		printf("Received:%d\n", pData->dwID);
		break;
	}
}

void testCommunication()
{
	HRESULT hr;
	printf("Connecting to Flight Simulator!\n");

	if (SUCCEEDED(SimConnect_Open(&hSimConnect, "PMDG 777X Test", NULL, 0, 0, 0)))
	{
		printf("Connected to Flight Simulator!\n");

		// 1) Set up data connection

		// Associate an ID with the PMDG data area name
		hr = SimConnect_MapClientDataNameToID(hSimConnect, PMDG_777X_DATA_NAME, PMDG_777X_DATA_ID);

		// Define the data area structure - this is a required step
		hr = SimConnect_AddToClientDataDefinition(hSimConnect, PMDG_777X_DATA_DEFINITION, 0, sizeof(PMDG_777X_Data), 0, 0);

		// Sign up for notification of data change.  
		// SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED flag asks for the data to be sent only when some of the data is changed.
		hr = SimConnect_RequestClientData(hSimConnect, PMDG_777X_DATA_ID, DATA_REQUEST, PMDG_777X_DATA_DEFINITION,
			SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);


		// 2) Set up control connection

		// First method: control data area
		Control.Event = 0;
		Control.Parameter = 0;

		// Associate an ID with the PMDG control area name
		hr = SimConnect_MapClientDataNameToID(hSimConnect, PMDG_777X_CONTROL_NAME, PMDG_777X_CONTROL_ID);

		// Define the control area structure - this is a required step
		hr = SimConnect_AddToClientDataDefinition(hSimConnect, PMDG_777X_CONTROL_DEFINITION, 0, sizeof(PMDG_777X_Control), 0, 0);

		// Sign up for notification of control change.  
		hr = SimConnect_RequestClientData(hSimConnect, PMDG_777X_CONTROL_ID, CONTROL_REQUEST, PMDG_777X_CONTROL_DEFINITION,
			SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);

		// Second method: Create event IDs for controls that we are going to operate
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_LOGO_LIGHT_SWITCH, "#69748");		//EVT_OH_LIGHTS_LOGO
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_FLIGHT_DIRECTOR_SWITCH, "#69834");	//EVT_MCP_FD_SWITCH_L


																										// 3) Request current aircraft .air file path
		hr = SimConnect_RequestSystemState(hSimConnect, AIR_PATH_REQUEST, "AircraftLoaded");
		// also request notifications on sim start and aircraft change
		hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_SIM_START, "SimStart");


		// 4) Assign keyboard shortcuts
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_KEYBOARD_A);
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_KEYBOARD_B);
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_KEYBOARD_C);

		hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_KEYBOARD, EVENT_KEYBOARD_A);
		hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_KEYBOARD, EVENT_KEYBOARD_B);
		hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_KEYBOARD, EVENT_KEYBOARD_C);

		hr = SimConnect_SetNotificationGroupPriority(hSimConnect, GROUP_KEYBOARD, SIMCONNECT_GROUP_PRIORITY_HIGHEST);

		hr = SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT0, "shift+ctrl+a", EVENT_KEYBOARD_A);
		hr = SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT0, "shift+ctrl+b", EVENT_KEYBOARD_B);
		hr = SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT0, "shift+ctrl+c", EVENT_KEYBOARD_C);

		hr = SimConnect_SetInputGroupState(hSimConnect, INPUT0, SIMCONNECT_STATE_ON);

		printf("entering main loop\n");
		// 5) Main loop
		while (quit == 0)
		{
			//printf("running main loop\n");
			// receive and process the 777X data
			SimConnect_CallDispatch(hSimConnect, MyDispatchProc, NULL);

			Sleep(1);
		}

		hr = SimConnect_Close(hSimConnect);
	}
	else
		printf("Unable to connect!\n\n");
}


int _tmain(int argc, _TCHAR* argv[])
{
	printf("starting to setup networking");

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		Sleep(10000);
		return 10;
	}



	//if (argc != 3) {
	//	fprintf(stderr, "usage: talker hostname message\n");
	//	exit(101);
	//}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo("127.0.0.1", SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 2;
	}

	printf("binding to socket");

	// loop through all the results and make a socket
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	

	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		return 3;
	}

	printf("setup networking");
	testCommunication();

	Sleep(100000);

	return 0;
}