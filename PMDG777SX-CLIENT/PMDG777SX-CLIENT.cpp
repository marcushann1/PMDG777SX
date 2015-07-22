// PMDG777SX-CLIENT.cpp : Defines the entry point for the console application.
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
#include <map>

#define MYPORT "4950" // the port users will be connecting to
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

std::map<std::string, char*> PlaneParameters = {
	{ "B777X_FuelPumpLAftLight", "false" },
	{ "B777X_TaxiLightSwitch", "false" },
	{ "LogoLightSwitch", "false" },
	{ "LandingLightLeftSwitch", "false" },
	{ "WindowHeatBackupSwitch1", "false" },
	{ "WindowHeatBackupSwitch2", "false" },
	{ "B777X_ElecStandbyPowerSwitch", "false" },
	{ "WingHydraulicValveSwitch1", "false" },
	{ "WingHydraulicValveSwitch2", "false" },
	{ "WingHydraulicValveSwitch3", "false" },
	{ "TailHydraulicValveSwitch1", "false" },
	{ "TailHydraulicValveSwitch2", "false" },
	{ "TailHydraulicValveSwitch3", "false" },
	{ "ApuPowerTestSwitch", "false" },
	{ "EngineEECPowerTestSwitch1", "false" },
	{ "EngineEECPowerTestSwitch2", "false" },
	{ "ElectricTowingBatSwitch", "false" },
	{ "B777X_CargoTempSelector1", "false" },
	{ "B777X_CargoTempSelector2", "false" },
	{ "ADIRUSwitch", "false" },
	{ "ThrustAsymComp", "false" },
	{ "CabinUtilitySwitch", "false" },
	{ "IFEPassSeatsSwitch", "false" },
	{ "BatterySwitch", "false" },
	{ "APUGenSwitch", "false" },
	{ "B777X_ApuSelector", "false" },
	{ "BusTieSwitch1", "false" },
	{ "BusTieSwitch2", "false" },
	{ "ExternalPowerSwitch1", "false" },
	{ "ExternalPowerSwitch2", "false" },
	{ "GeneratorSwitch1", "false" },
	{ "GeneratorSwitch2", "false" },
	{ "BackupGeneratorSwitch1", "false" },
	{ "BackupGeneratorSwitch2", "false" },
	{ "IGDDiskSwitch1", "false" },
	{ "IGDDiskSwitch2", "false" }
};

*char[2] splitByColon(char* input){

}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void testCommunication()
{
	HRESULT hr;

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
	}
	else
		printf("Unable to connect!\n\n");
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


int _tmain(int argc, _TCHAR* argv[])
{
	testCommunication();

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
		return 1;
	}

	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			closesocket(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
	freeaddrinfo(servinfo);
	while (true)
	{
		SimConnect_CallDispatch(hSimConnect, MyDispatchProc, NULL);

		printf("listener: waiting to recvfrom...\n");
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		printf("listener: got packet from %s\n",
			inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof s));
		printf("listener: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("listener: packet contains \"%s\"\n\n", buf);
		

		//parse data and send change to P3D
		// Test the first control method: use the control data area.
		if (AircraftRunning)
		{
			bool New_TaxiLightSwitch = (buf == "LTS_Logo_Sw_ON: false") ? true : false;
			

			// Send a command only if there is no active command request and previous command has been processed by the 777X
			if (Control.Event == 0)
			{
				Control.Event = EVT_OH_LIGHTS_LOGO;		// = 69753
				if (New_TaxiLightSwitch)
					Control.Parameter = 1;
				else
					Control.Parameter = 0;
				SimConnect_SetClientData(hSimConnect, PMDG_777X_CONTROL_ID, PMDG_777X_CONTROL_DEFINITION,
					0, 0, sizeof(PMDG_777X_Control), &Control);

				printf("changed logo lights\n");
			}
		}
		else {
			printf("not running\n");
		}
		printf("----------------------------------------\n\n");
	}

	Sleep(100000);

	return 0;
}

