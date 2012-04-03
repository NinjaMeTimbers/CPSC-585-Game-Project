#pragma once

#include <stdlib.h>
#include <math.h>
#include <process.h>

#include "Renderer.h"
#include "Input.h"
#include "Physics.h"
#include "Sound.h"
#include "Intention.h"
#include "Racer.h"
#include "World.h"
#include "Waypoint.h"
#include "AIMind.h"
#include "HUD.h"
#include "WaypointEditor.h"
#include "Ability.h"
#include "CheckpointTimer.h"
#include "Server.h"
#include "Client.h"
#include "TopMenu.h"

#define NUMRACERS 5
#define NETWORKTIME 0.03

class AI
{
public:
	AI(void);
	~AI(void);
	void shutdown();
	void initialize(Renderer* renderer, Input* input, Sound* sound, TopMenu* m);
	bool simulate(float milliseconds);
	void displayDebugInfo(Intention intention, float milliseconds);

private:
	std::string getFPSString(float milliseconds);
	void initializeWaypoints();
	void initializeAIRacers();
	void initializeCheckpoints();
	std::string boolToString(bool boolean);

	//Networking functions
	static unsigned __stdcall staticSetupServer(void * pThis);
	void setupServer();
	static unsigned __stdcall staticConnectToServer(void *pThis);
	void connectToServer();
	void runNetworking(float milliseconds);
	void switchToServer();

	void runMenu();

	Renderer* renderer;
	Input* input;
	Physics* physics;
	Sound* sound;
	HUD* hud;
	CheckpointTimer* checkPointTimer;
	WaypointEditor* wpEditor;

	int count;
	int fps;

	int racerIndex;

	// Abilities
	Ability* speedBoost;

	// Racers
	Racer* racers[NUMRACERS];
	Racer* player;
	Racer* ai1;
	Racer* ai2;
	Racer* ai3;
	Racer* ai4;

	// Racer Minds
	AIMind* racerMinds[NUMRACERS];
	AIMind* playerMind;
	AIMind* aiMind1;
	AIMind* aiMind2;
	AIMind* aiMind3;
	AIMind* aiMind4;

	// World
	World* world;

	// Waypoints
	Waypoint* waypoints[80];
	Waypoint* wp1;
	Waypoint* wp2;
	Waypoint* wp3;
	Waypoint* wp4;
	Waypoint* wp5;
	Waypoint* wp6;
	Waypoint* wp7;
	Waypoint* wp8;
	Waypoint* wp9;
	Waypoint* wp10;
	Waypoint* wp11;
	Waypoint* wp12;
	Waypoint* wp13;
	Waypoint* wp14;
	Waypoint* wp15;
	Waypoint* wp16;
	Waypoint* wp17;
	Waypoint* wp18;
	Waypoint* wp19;
	Waypoint* wp20;
	Waypoint* wp21;
	Waypoint* wp22;
	Waypoint* wp23;
	Waypoint* wp24;
	Waypoint* wp25;
	Waypoint* wp26;
	Waypoint* wp27;
	Waypoint* wp28;
	Waypoint* wp29;
	Waypoint* wp30;
	Waypoint* wp31;
	Waypoint* wp32;
	Waypoint* wp33;
	Waypoint* wp34;
	Waypoint* wp35;
	Waypoint* wp36;
	Waypoint* wp37;
	Waypoint* wp38;
	Waypoint* wp39;
	Waypoint* wp40;
	Waypoint* wp41;
	Waypoint* wp42;
	Waypoint* wp43;
	Waypoint* wp44;
	Waypoint* wp45;
	Waypoint* wp46;
	Waypoint* wp47;
	Waypoint* wp48;
	Waypoint* wp49;
	Waypoint* wp50;
	Waypoint* wp51;
	Waypoint* wp52;
	Waypoint* wp53;
	Waypoint* wp54;
	Waypoint* wp55;
	Waypoint* wp56;
	Waypoint* wp57;
	Waypoint* wp58;
	Waypoint* wp59;
	Waypoint* wp60;
	Waypoint* wp61;
	Waypoint* wp62;
	Waypoint* wp63;
	Waypoint* wp64;
	Waypoint* wp65;
	Waypoint* wp66;
	Waypoint* wp67;
	Waypoint* wp68;
	Waypoint* wp69;
	Waypoint* wp70;
	Waypoint* wp71;
	Waypoint* wp72;
	Waypoint* wp73;
	Waypoint* wp74;
	Waypoint* wp75;
	Waypoint* wp76;
	Waypoint* wp77;
	Waypoint* wp78;
	Waypoint* wp79;
	Waypoint* wp80;

	// Checkpoints
	Waypoint* checkpoints[7];
	Waypoint* cp1;
	Waypoint* cp2;
	Waypoint* cp3;
	Waypoint* cp4;
	Waypoint* cp5;
	Waypoint* cp6;
	Waypoint* cp7;

	//Networking
	std::string hostName;
	HANDLE   hth1;
	HANDLE   hth2;
	Server server;
	Client client;
	ConfigReader config;
	bool clientConnected;
	bool serverStarted;
	bool tryToConnectToServer;
	bool readyPressed;
	Intention racerIntents[NUMRACERS];
	bool readyStatus[NUMRACERS];
	Intention prevIntent;
	bool isClient;
	bool isServer;
	float networkTime;

	//Menu
	TopMenu *menu;
	bool quit;
};
