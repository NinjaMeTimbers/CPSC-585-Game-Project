#include "AI.h"


AI::AI(void)
{
	renderer = NULL;
	input = NULL;
	physics = NULL;
	sound = NULL;
	hud = NULL;
	checkPointTimer = NULL;
	wpEditor = NULL;

	player = NULL;
	ai1 = NULL;
	ai2 = NULL;
	ai3 = NULL;
	ai4 = NULL;
	ai5 = NULL;
	ai6 = NULL;
	ai7 = NULL; 
	world = NULL;

	dynManager = NULL;

	//Networking
	isClient = false;
	isServer = false;
	hostName = "";
	clientConnected = false;
	serverStarted = false;
	tryToConnectToServer = false;
	readyPressed = false;
}


AI::~AI(void)
{
}

void AI::shutdown()
{
	for(int i = 0; i < NUMRACERS; i++)
	{
		delete racers[i];
		racers[i] = NULL;

		delete racerMinds[i];
		racerMinds[i] = NULL;

		readyStatus[i] = false;
	}
	
	if (world)
	{
		delete world;
		world = NULL;
	}

	if (physics)
	{
		physics->shutdown();
		delete physics;
		physics = NULL;
	}

	if(checkPointTimer)
	{
		delete checkPointTimer;
		checkPointTimer = NULL;
	}

	if (wpEditor)
	{
		//wpEditor->closeFile();
		delete wpEditor;
		wpEditor = NULL;
	}

	if (dynManager)
	{
		delete dynManager;
		dynManager = NULL;
	}
}

void AI::initialize(Renderer* r, Input* i, Sound* s)
{
	raceStartTimer = 4.0f;
	raceStarted = false;
	raceEnded = false;
	playedOne = false;
	playedTwo = false;
	playedThree = false;

	renderer = r;
	input = i;
	sound = s;
	count = 25;
	fps = 0;
	racerIndex = 0;
	numberOfWaypoints = 0;

	dynManager = new DynamicObjManager();

	wpEditor = new WaypointEditor(renderer);
	//wpEditor->openFile();

	//Initialize physics
	physics = new Physics();
	physics->initialize(NUMRACERS + 1);
	
	// Initialize sound
	s->initialize();

	//Initialize Abilities
	speedBoost = new Ability(SPEED); // Speed boost with cooldown of 15 seconds and aditional speed of 1
	
	//Initialize player
	player = new Racer(r->getDevice(), RACER1);
	player->setPosAndRot(35.0f, 15.0f, -298.0f, 0.0f, 1.4f, 0.0f);
	playerMind = new AIMind(player, PLAYER, NUMRACERS, "Herald");
	racers[0] = player;
	racerMinds[0] = playerMind;
	sound->playerEmitter = player->emitter;
	

	//Initialize world
	world = new World(r->getDevice(), renderer, physics);
	world->setPosAndRot(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	//Initialize AI-Racers
	initializeAIRacers();

	//Initialize Racer Placement
	for(int i = 0; i < NUMRACERS; i++){
		racerPlacement[i] = racerMinds[i];
	}

	//Initialize Waypoints
	wpEditor->loadWaypoints(waypoints, "RaceTrack.txt"); 
	buildingWaypoint = new Waypoint(renderer->getDevice(), WAY_POINT);
	buildingWaypoint->setPosAndRot(258.0f, 31.0f, 85.0f, 0.0f, 0.0f, 0.0f);

	
	// Initializing racer's look direction at game start.
	D3DXVECTOR3 target = waypoints[0]->drawable->getPosition();
	hkVector4 targetPos = hkVector4(target.x, target.y, target.z);
	hkVector4 shooterPos = player->body->getPosition();
	shooterPos(1) += 2.0f;
	targetPos.sub(shooterPos);
	targetPos.normalize3();

	player->lookDir.setXYZ(targetPos);
	
	//Initialize Checkpoints & Finish Lines
	initializeCheckpoints();

	//Initialize HUD
	hud = renderer->getHUD();
	//checkPointTimer = new CheckpointTimer(player);

	// This is how you set an object for the camera to focus on!
	renderer->setFocus(racers[racerIndex]->getIndex());
}

void AI::initializeAIRacers()
{
	ai1 = new Racer(renderer->getDevice(), RACER2);
	ai1->setPosAndRot(40.0f, 15.0f, -294.0f, 0.0f, 1.4f, 0.0f);
	aiMind1 = new AIMind(ai1, COMPUTER, NUMRACERS, "Gerard");
	
	ai2 = new Racer(renderer->getDevice(), RACER3);
	ai2->setPosAndRot(40.0f, 15.0f, -298.0f, 0.0f, 1.4f, 0.0f);
	aiMind2 = new AIMind(ai2, COMPUTER, NUMRACERS, "Nevvel");

	ai3 = new Racer(renderer->getDevice(), RACER4);
	ai3->setPosAndRot(40.0f, 15.0f, -302.0f, 0.0f, 1.4f, 0.0f);
	aiMind3 = new AIMind(ai3, COMPUTER, NUMRACERS, "Rosey");

	ai4 = new Racer(renderer->getDevice(), RACER5);
	ai4->setPosAndRot(40.0f, 15.0f, -306.0f, 0.0f, 1.4f, 0.0f);
	aiMind4 = new AIMind(ai4, COMPUTER, NUMRACERS, "Delilah");

	ai5 = new Racer(renderer->getDevice(), RACER6);
	ai5->setPosAndRot(35.0f, 15.0f, -306.0f, 0.0f, 1.4f, 0.0f);
	aiMind5 = new AIMind(ai5, COMPUTER, NUMRACERS, "Gupreet");

	ai6 = new Racer(renderer->getDevice(), RACER7);
	ai6->setPosAndRot(35.0f, 15.0f, -302.0f, 0.0f, 1.4f, 0.0f);
	aiMind6 = new AIMind(ai6, COMPUTER, NUMRACERS, "Tiffany");

	ai7 = new Racer(renderer->getDevice(), RACER8);
	ai7->setPosAndRot(35.0f, 15.0f, -294.0f, 0.0f, 1.4f, 0.0f);
	aiMind7 = new AIMind(ai7, COMPUTER, NUMRACERS, "Rickardo");

	racers[1] = ai1;
	racers[2] = ai2;
	racers[3] = ai3;
	racers[4] = ai4;
	racers[5] = ai5;
	racers[6] = ai6;
	racers[7] = ai7;
	racerMinds[1] = aiMind1;
	racerMinds[2] = aiMind2;
	racerMinds[3] = aiMind3;
	racerMinds[4] = aiMind4;
	racerMinds[5] = aiMind5;
	racerMinds[6] = aiMind6;
	racerMinds[7] = aiMind7;
}



void AI::initializeCheckpoints()
{
	prevCheckpoints[0] = new Waypoint(renderer->getDevice(), TURN_POINT);
	prevCheckpoints[0]->setPosAndRot(-264.0f, -14.0f, 90.0f, 0.0f, 0.0f, 0.0f);

	checkpoints[0] = new Waypoint(renderer->getDevice(), TURN_POINT);
	checkpoints[0]->setPosAndRot(-252.0f, -14.0f, 36.0f, 0.0f, 0.0f, 0.0f);

	prevCheckpoints[1] = new Waypoint(renderer->getDevice(), CHECK_POINT);
	prevCheckpoints[1]->setPosAndRot(-30.0f, 7.0f, 49.0f, 0.0f, 0.0f, 0.0f);

	checkpoints[1] = new Waypoint(renderer->getDevice(), CHECK_POINT);
	checkpoints[1]->setPosAndRot(-5.0f, 10.0f, 85.0f, 0.0f, 0.0f, 0.0f);

	prevCheckpoints[2] = new Waypoint(renderer->getDevice(), CHECK_POINT);
	prevCheckpoints[2]->setPosAndRot(239.0f, -14.0f, -178.0f, 0.0f, 0.0f, 0.0f);

	checkpoints[2] = new Waypoint(renderer->getDevice(), CHECK_POINT);
	checkpoints[2]->setPosAndRot(190.0f, -14.0f, -181.0f, 0.0f, 0.0f, 0.0f);

	prevCheckpoints[3] = new Waypoint(renderer->getDevice(), CHECK_POINT);
	prevCheckpoints[3]->setPosAndRot(-14.0f, -14.0f, -212.0f, 0.0f, 0.0f, 0.0f);

	checkpoints[3] = new Waypoint(renderer->getDevice(), LAP_POINT);
	checkpoints[3]->setPosAndRot(2.0f, -14.0f, -141.0f, 0.0f, 0.0f, 0.0f);

	renderer->addDrawable(checkpoints[0]->drawable);
	renderer->addDrawable(checkpoints[1]->drawable);
	renderer->addDrawable(checkpoints[2]->drawable);
	renderer->addDrawable(checkpoints[3]->drawable);
}

unsigned __stdcall AI::staticSetupServer(void * pThis)
{
    AI * pthX = (AI*)pThis;   // the tricky cast
    pthX->setupServer();           // now call the true entry-point-function

    // A thread terminates automatically if it completes execution,
    // or it can terminate itself with a call to _endthread().

    return 1;          // the thread exit code
}

void AI::setupServer()
{
    // This is the desired entry-point-function but to get
    // here we have to use a 2 step procedure involving
    // the ThreadStaticEntryPoint() function.
	//Get IP address
	server.setupWSA();
	server.setupTCPSocket(); //for game lobby
	server.setupUDPSocket();

	char ac[80];
	in_addr addr;
	std::stringstream ss;
	if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR)
	{
		ss << "Error " << WSAGetLastError() <<
				" when getting local host name.";
	}
	else
	{
		struct hostent *phe = gethostbyname(ac);
		if (phe == 0)
		{
			ss << "Bad host lookup.";
		}
		else
		{
			memcpy(&addr, phe->h_addr_list[0], sizeof(struct in_addr));
		}
	}
	if(ss.str().length() == 0)
	{
		ss << "Your IP address is " << inet_ntoa(addr) << ".";
	}
	hostName = ss.str();
}

unsigned __stdcall AI::staticConnectToServer(void * pThis)
{
    AI * pthX = (AI*)pThis;   // the tricky cast
    pthX->connectToServer();           // now call the true entry-point-function

    // A thread terminates automatically if it completes execution,
    // or it can terminate itself with a call to _endthread().

    return 1;          // the thread exit code
}

void AI::connectToServer()
{
    clientConnected = client.connectToServer(5869, config.serverIP);
	client.setupWSA();
	client.setupUDPSocket();
}

void AI::switchToServer()
{
	server.id = client.id;
	isClient = false;
	isServer = true;
	server.gameStarted = true;
	memcpy(server.clients,client.clients,sizeof(ClientInfo)*MAXCLIENTS);
	server.setupUDPSocket();
	server.numClients = MAXCLIENTS;

	for(int i = 0; i < server.numClients; i++)
	{
		if(server.clients[i].connected && racers[i] != player)
		{
			racerMinds[i]->setTypeOfRacer(NETWORK);
		}
		else if(racers[i] == player)
		{
			racerMinds[i]->setTypeOfRacer(PLAYER);
		}
	}
}

void AI::runNetworking(float milliseconds)
{
	std::stringstream ss;
	//See if this game should be a client or a server
	if(!isServer && !isClient)
	{
		isServer = input->isServer();
		isClient = input->isClient();
	}

	if(isServer)
	{
		if(!serverStarted)
		{
			unsigned  uiThread1ID;
			hth1 = (HANDLE)_beginthreadex( NULL,         // security
                                0,            // stack size
                                AI::staticSetupServer,  // entry-point-function
                                this,           // arg list holding the "this" pointer
                                0,  //Thread starts right away
                                &uiThread1ID );

			hth1 = hth1;
			serverStarted = true;
		}
		else if(!server.gameStarted)
		{
			server.lobbyListen(milliseconds);
			milliseconds = 0;

			for(int i = 1; i < MAXCLIENTS; i++)
			{
				if(racerMinds[i]->getTypeOfRacer() == COMPUTER && server.clients[i].connected)
				{
					ss << "Player " << i << " has joined.\n";
					racerMinds[i]->setTypeOfRacer(NETWORK);
				}
			}

			// Initializing racer's look direction at game start.
			if(server.gameStarted)
			{
				for(int i = 0; i < NUMRACERS; i++)
				{
					if(racerMinds[i]->getTypeOfRacer() == NETWORK)
					{
						D3DXVECTOR3 target = waypoints[0]->drawable->getPosition();
						hkVector4 targetPos = hkVector4(target.x, target.y, target.z);
						hkVector4 shooterPos = player->body->getPosition();
						shooterPos(1) += 2.0f;
						targetPos.sub(shooterPos);
						targetPos.normalize3();

						racers[i]->lookDir.setXYZ(targetPos);
					}
				}
			}
		}
		else
		{
			//Check if any racers have disconnected and if they have, switch them to AI
			for(int i = 0; i < NUMRACERS; i++)
			{
				if(racerMinds[i]->getTypeOfRacer() == NETWORK && !server.clients[i].connected)
				{
					racerMinds[i]->setTypeOfRacer(COMPUTER);
				}
			}

			if(networkTime <= 0)
				server.update(racers,NUMRACERS,racerMinds);
			server.raceListen(milliseconds);
			milliseconds = 0;
		}
		
		std::string stringArray[] = {"You are a server.",hostName,ss.str()};
		renderer->setText(stringArray, sizeof(stringArray) / sizeof(std::string));
	}
	else if(isClient)
	{
		std::string msg1 = "";

		if(!clientConnected && !tryToConnectToServer)
		{
			std::stringstream ss;
			ss << "Connecting to server at IP address ";// << config.serverIP << "...";
			msg1 = ss.str();

			unsigned  uiThread1ID;
			tryToConnectToServer = true;
			hth2 = (HANDLE)_beginthreadex( NULL,         // security
                                0,            // stack size
                                AI::staticConnectToServer,  // entry-point-function
                                this,           // arg list holding the "this" pointer
                                0,  
                                &uiThread1ID );
		}
		else if(!clientConnected && tryToConnectToServer)
		{
			msg1 = "Could not connect to server.";
		}
		else if(clientConnected)
		{
			//boolean for determining whether to send an "alive" message (TAG1)
			bool msg_sent = false;

			//intention = input->getIntention();
			if(!client.start)
			{
				//Check if the player already has their ID
				bool idFound = false;
				if(client.id >= 0)
				{
					idFound = true;
				}

				client.getTCPMessages(milliseconds);
				//set milliseconds to 0 in case another get is called after this
				milliseconds = 0;

				//If the player has been assigned an ID, set them to that car
				if(!idFound && client.id >= 0)
				{
					player = racers[client.id];
					racerIndex = client.id;

					// Initializing racer's look direction at game start.
					D3DXVECTOR3 target = waypoints[0]->drawable->getPosition();
					hkVector4 targetPos = hkVector4(target.x, target.y, target.z);
					hkVector4 shooterPos = player->body->getPosition();
					shooterPos(1) += 2.0f;
					targetPos.sub(shooterPos);
					targetPos.normalize3();

					player->lookDir.setXYZ(targetPos);
					renderer->setFocus(racers[racerIndex]->getIndex()); //Focus camera on player

					//Delete any minds for racers now over the network
					for(int i = 0; i < NUMRACERS; i++)
					{
						if(racerMinds[i]->getTypeOfRacer() != CLIENT)
						{
							racerMinds[i]->setTypeOfRacer(CLIENT);
						}
					}
				}

				if(intention.rbumpPressed && !readyPressed)
				{
					client.ready();
					readyPressed = true;

					//a msg was sent
					msg_sent = true;
				}
				else if(intention.lbumpPressed && readyPressed)
				{
					client.unready();
					readyPressed = false;

					//a msg was sent
					msg_sent = true;
				}
				else
				{
					client.sendAliveMessage();
				}
				
				if(!client.isReady)
				{
					std::stringstream ss;
					ss << "Connected to server. Press the right bumper when you are ready.";
					msg1 = ss.str();
				}
				else
				{
					msg1 = "Ready. Waiting for game to start.";
				}

				std::stringstream ss;
				ss << "\n";
				//See if any other clients are ready
				int numClients = client.numClients;

				for(int i = 0; i < client.numClients; i++)
				{
					if(client.clients[i].connected && client.clients[i].ready)
					{
						ss << "Player " << client.clients[i].id << " is ready.\n";
					}
					else
					{
						ss << "Player " << client.clients[i].id << " is not ready.\n";
					}
				}
				msg1 = ss.str();
			}
			else
			{
				client.getTCPMessages(milliseconds);

				int temp = client.getUDPMessages(milliseconds);
				if (temp == -1)
				{
					switchToServer();	
				}
				else if (temp > -1)
				{
					temp = temp;
				}
				//set milliseconds to 0 in case another get function is called (TAG1)
				milliseconds = 0;

				//if(!intent.equals(prevIntent))
				if (networkTime <= 0.0f)
				{
					networkTime = (float) NETWORKTIME;
					client.sendButtonState(intention);

					//a msg was sent
					msg_sent = true;
				}

				msg1 = "Game started.";
			}
			prevIntent = intention;

			//send an "alive" message if no messages were sent (TAG1)
			if (!msg_sent)
			{
				//client.sendAliveMessage();
			}
		}

		std::string stringArray[] = {"You are a client.",msg1};
		renderer->setText(stringArray, sizeof(stringArray) / sizeof(std::string));
	}
	else
	{
		std::string stringArray[] = {"Press 'C' to become a client, or 'V' to become a server."};
		renderer->setText(stringArray, sizeof(stringArray) / sizeof(std::string));
	}
}

void AI::simulate(float seconds)
{
	_ASSERT(seconds > 0.0f);
	networkTime -= seconds;

	intention = input->getIntention();

	// Debugging Information ---------------------------------------
	if(input->debugging()){
		displayDebugInfo(intention, seconds);
	}
	else if(input->networking())
	{
		runNetworking(seconds);
	}
	else{
		std::string stringArray[] = {""};
		//renderer->setText(stringArray, sizeof(stringArray) / sizeof(std::string));
	}
	// ---------------------------------------------------------------
	
	if(playerMind->isfinishedRace())
	{
		displayPostGameStatistics();
	}




	// ---------- UPDATE SOUND WITH CURRENT CAMERA POSITION/ORIENTATION --------------- //

	X3DAUDIO_LISTENER* listener = &(sound->listener);
	hkVector4 vec;
	vec.setXYZ(racers[racerIndex]->lookDir);
	vec(1) = 0.0f;
	vec.normalize3();
	listener->OrientFront.x = vec(0);
	listener->OrientFront.y = vec(1);
	listener->OrientFront.z = vec(2);
	
	vec.setXYZ(racers[racerIndex]->body->getPosition());

	listener->Position.x = vec(0);
	listener->Position.y = vec(1);
	listener->Position.z = vec(2);

	vec.setXYZ(racers[racerIndex]->body->getLinearVelocity());

	listener->Velocity.x = vec(0);
	listener->Velocity.y = vec(1);
	listener->Velocity.z = vec(2);


	// -------------------------------------------------------------------------------- //

	if (!raceStarted)
	{
		raceStartTimer -= seconds;

		updateRacerPlacement(0, NUMRACERS - 1);

		for (int i = 0; i < NUMRACERS; i++)
		{
			racerPlacement[i]->setPlacement(NUMRACERS-i);
		}

		// Let player move camera before race starts
		hkReal angle;
		float height;

		angle = intention.cameraX * 0.05f;

		if (racers[racerIndex]->config.inverse)
			height = intention.cameraY * -0.02f + racers[racerIndex]->lookHeight;
		else
			height = intention.cameraY * 0.02f + racers[racerIndex]->lookHeight;

		if (height > 0.5f)
			height = 0.5f;
		else if (height < -0.5f)
			height = -0.5f;

		racers[racerIndex]->lookHeight = height;

		if (angle > M_PI)
			angle = (hkReal) M_PI;
		else if (angle < -M_PI)
			angle = (hkReal) -M_PI;


		hkQuaternion rotation;

		if (angle < 0.0f)
		{
			angle *= -1;
			rotation.setAxisAngle(hkVector4(0,-1,0), angle);
		}
		else
		{
			rotation.setAxisAngle(hkVector4(0,1,0), angle);
		}

		hkTransform transRot;
		transRot.setIdentity();
		transRot.setRotation(rotation);

		hkVector4 finalLookDir(0,0,1);
		finalLookDir.setTransformedPos(transRot, racers[racerIndex]->lookDir);

		finalLookDir(1) = height;

		racers[racerIndex]->lookDir.setXYZ(finalLookDir);

		// Update Heads Up Display
		hud->update(intention);

		hud->setSpeed(0.0f);
		hud->setHealth(100);
		hud->setPosition(racerPlacement[racerIndex]->getPlacement());
		hud->setLap(1, racerMinds[racerIndex]->numberOfLapsToWin);



		hkVector4 look = racers[racerIndex]->lookDir;
		(renderer->getCamera())->setLookDir(look(0), look(1), look(2));

		for (int i = 0; i < NUMRACERS; i++)
		{
			racers[i]->applyForces(seconds);
			racers[i]->computeRPM();
		}

		physics->step(seconds);

		for (int i = 0; i < NUMRACERS; i++)
		{
			racers[i]->update();
		}
		

		if (raceStartTimer <= 0.0f)
		{
			// Show "GO" on screen, play sound
			Sound::sound->playShotgun(Sound::sound->playerEmitter);

			raceStartTimer = -0.1f;
			raceStarted = true;
			hud->showOne = false;
		}
		else if (raceStartTimer <= 1.0f)
		{
			if (!playedOne)
			{
				// Play "One" sound
				Sound::sound->playOne(Sound::sound->playerEmitter);
				playedOne = true;

				// Start playing music
				Sound::sound->playInGameMusic();
			}

			// Show "1" on screen
			hud->showTwo = false;
			hud->showOne = true;

			return;
		}
		else if (raceStartTimer <= 2.0f)
		{
			if (!playedTwo)
			{
				// Play "Two" sound
				Sound::sound->playTwo(Sound::sound->playerEmitter);
				playedTwo = true;
			}

			// Show "2" on screen
			hud->showThree = false;
			hud->showTwo = true;

			return;
		}
		else if (raceStartTimer <= 3.0f)
		{
			if (!playedThree)
			{
				// Play "Three" sound
				Sound::sound->playThree(Sound::sound->playerEmitter);
				playedThree = true;
			}

			// Show "3" on screen
			hud->showThree = true;

			return;
		}
		else
		{

			return;
		}
	}

	// Update Checkpoint Timer
	//checkPointTimer->update(checkpoints);
	

	/*for(int i = 0; i < NUMRACERS; i++){
		racerMinds[i]->update(hud, intention, seconds, waypoints, racers, racerPlacement, buildingWaypoint);
	}*/

	for(int i = 0; i < NUMRACERS; i++)
	{
		if(player != racers[i] && server.gameStarted && server.clients[i].connected) //If we are running as the server, update the racers based on networked button presses
		{
			//racers[i]->steer(seconds, server.intents[i].steering);
			//racers[i]->accelerate(seconds, server.intents[i].acceleration);
			racerMinds[i]->update(hud, server.intents[i], seconds, waypoints, racers, racerPlacement, buildingWaypoint); //Update each racermind

			// Reset the player (in case you fall over)
			if (server.intents[i].yPressed)
			{
				// Reset the player (in case you fall over)
				D3DXVECTOR3 cwPosition = waypoints[racerMinds[i]->getCurrentWaypoint()]->drawable->getPosition();
				float rotation = 0;
				racers[i]->reset(&(hkVector4(cwPosition.x, cwPosition.y, cwPosition.z)), rotation);
			}

			//racers[i]->applyForces(seconds);
		}
		else if(client.newWorldInfo) //If we are running as a client, update the world info if there is new info to be had
		{
			racers[i]->unserialize(&client.world[i]);
			racers[i]->applyForces(seconds);
			racerMinds[i]->update(hud, intention, seconds, waypoints, racers, racerPlacement, buildingWaypoint); //Update each racermind
		}
		else
		{
			racerMinds[i]->update(hud, intention, seconds, waypoints, racers, racerPlacement, buildingWaypoint); //Update each racermind
		}
	}
	client.newWorldInfo = false;
	
	updateRacerPlacement(0, NUMRACERS - 1);

	for(int i = 0; i < NUMRACERS; i++){
		racerPlacement[i]->setPlacement(NUMRACERS-i);
	}

	if(input->placingWaypoint()){
		
		//wpEditor->update(racers[racerIndex]);
		numberOfWaypoints += 1;
		Waypoint* addWaypoint = new Waypoint(renderer->getDevice(), input->wpType);
		hkVector4 wpPosition = racers[0]->body->getPosition();
		addWaypoint->setPosAndRot(wpPosition.getComponent(0), wpPosition.getComponent(1), wpPosition.getComponent(2), 0, 0, 0);
		wpEditor->waypoints.push_back(addWaypoint); 
		input->setPlaceWaypointFalse();
		
	}

	for(int i = 0; i < 4; i++){ // currently doesn't do anything
		waypoints[i]->update();
	}

	if(intention.bPressed){ // Changes control of Computer racer to Player, and Player racer to computer, for the currently viewed racer
		racerMinds[racerIndex]->togglePlayerComputerAI();
	}

	// Switch focus (A for player, X for AI)
	if (intention.startPressed){
		if(racerIndex == 4){
			racerIndex = 0;
		}
		else{
			racerIndex += 1;
		}
		renderer->setFocus(racers[racerIndex]->getIndex());
	}
	else if (intention.selectPressed){
		if(racerIndex == 0){
			racerIndex = 4;
		}
		else{
			racerIndex -= 1;
		}
		renderer->setFocus(racers[racerIndex]->getIndex());
	}

	hkVector4 look = racers[racerIndex]->lookDir;
	(renderer->getCamera())->setLookDir(look(0), look(1), look(2));

	// Reset the player (in case you fall over)
	if (intention.yPressed && !server.gameStarted)
	{
		D3DXVECTOR3 cwPosition = waypoints[racerMinds[racerIndex]->getCurrentWaypoint()]->drawable->getPosition();
		float rotation = 0;
		racers[racerIndex]->reset(&(hkVector4(cwPosition.x, cwPosition.y, cwPosition.z)), rotation);
	}

	if(intention.xPressed){
		//wpEditor->writeToFile(wpEditor->waypoints, numberOfWaypoints, "RaceTrack.txt");
	}
	if(intention.aPressed){
		
		vector<Waypoint*> passWaypoints = vector<Waypoint*>();
		for(int i = 0; i < NUMWAYPOINTS; i++){
			passWaypoints.push_back(waypoints[i]);
		}
		//wpEditor->writeToFile(passWaypoints, NUMWAYPOINTS, "Figure8Waypoints.txt");
		
	}

	physics->step(seconds);

	for(int i = 0; i < NUMRACERS; i++){
		racers[i]->update();
	}

	dynManager->update(seconds);

	return;
}

/*
	Runs a quicksort algorithm O(nlogn) to determine the position of a racer
	in the race based on numbers calculated from the most recent waypoint a
	racer has reached and what lap they are on. "OverallPosition = #waypoints x #laps"
	Source: http://www.algolist.net/Algorithms/Sorting/Quicksort
 */
void AI::updateRacerPlacement(int left, int right)
{
	int i = left, j = right;
    AIMind* tmp;

	int pivot = racerPlacement[(left + right) / 2]->getOverallPosition();

 

      /* partition */

      while (i <= j) {

		  while (racerPlacement[i]->getOverallPosition() < pivot)
                  i++;
		  while (racerPlacement[j]->getOverallPosition() > pivot)
                  j--;

            if (i <= j) {
				if(racerPlacement[i]->getCurrentWaypoint() == racerPlacement[j]->getCurrentWaypoint()){
					
					hkSimdReal distanceOfI = waypoints[racerPlacement[i]->getCurrentWaypoint()]->wpPosition.distanceTo(racerPlacement[i]->getRacerPosition());
					hkSimdReal distanceOfJ = waypoints[racerPlacement[j]->getCurrentWaypoint()]->wpPosition.distanceTo(racerPlacement[j]->getRacerPosition());

					if(distanceOfI < distanceOfJ){
						tmp = racerPlacement[i];
					    racerPlacement[i] = racerPlacement[j];
					    racerPlacement[j] = tmp;
					}
				}
				else{
					tmp = racerPlacement[i];
					    racerPlacement[i] = racerPlacement[j];
					    racerPlacement[j] = tmp;
				}
                  
                  i++;
                  j--;
            }

      }

 

      /* recursion */

      if (left < j)
            updateRacerPlacement(left, j);

      if (i < right)
            updateRacerPlacement(i, right);

}


std::string AI::getFPSString(float milliseconds)
{
	count++;

	if (count > 20)
	{
		fps = (int) floor(1000.0f / milliseconds);
		count = 0;
	}
	
	char tmp[5];

	_itoa_s(fps, tmp, 5, 10);

	return std::string("FPS: ").append((const char*) tmp);
}

std::string AI::boolToString(bool boolean)
{
	if(boolean == true){
		return "True";
	}
	else {
		return "False";
	}
}

void AI::displayDebugInfo(Intention intention, float seconds)
{
		char buf1[33];
		_itoa_s(intention.rightTrig, buf1, 10);
		char buf2[33];
		_itoa_s(intention.leftTrig, buf2, 10);
		char buf3[33];
		_itoa_s(intention.rightStickX, buf3, 10);
		char buf4[33];
		_itoa_s(intention.rightStickY, buf4, 10);
		char buf5[33];
		_itoa_s(intention.leftStickX, buf5, 10);
		char buf6[33];
		_itoa_s((int) (intention.acceleration * 100.0f), buf6, 10);
		char buf7[33];
		_itoa_s((int) (intention.leftStickY), buf7, 10);

		char buf8[33];
		hkVector4 vel = racers[racerIndex]->body->getLinearVelocity();
		float velocity = vel.dot3(racers[racerIndex]->drawable->getZhkVector());
		_itoa_s((int) (velocity), buf8, 10);

		char buf9[33];
		_itoa_s((int) (Racer::accelerationScale), buf9, 10);

		char buf10[33];
		_itoa_s((int) (racers[racerIndex]->kills), buf10, 10);
		char buf11[33];
		_itoa_s(racerMinds[racerIndex]->getCurrentWaypoint(), buf11, 10);
		char buf12[33];
		_itoa_s(racerMinds[racerIndex]->getCheckpointTime(), buf12, 10);
		char buf13[33];
		_itoa_s(racerMinds[racerIndex]->getSpeedCooldown(), buf13, 10);
		char buf14[33];
		_itoa_s(racerIndex, buf14, 10);
		char buf15[33];
		_itoa_s(racerMinds[racerIndex]->getCurrentLap(), buf15, 10);
		char buf16[33];
		_itoa_s(racers[racerIndex]->health, buf16, 10);
		char buf17[33];
		_itoa_s(racerMinds[racerIndex]->getLaserLevel(), buf17, 10);
		char buf18[33];
		_itoa_s(racerMinds[racerIndex]->getPlacement(), buf18, 10);
		char buf19[33];
		_itoa_s(racerMinds[racerIndex]->getOverallPosition(), buf19, 10);
		char buf20[33];
		_itoa_s(racerMinds[racerIndex]->getSpeedLevel(), buf20, 10);
		char buf21[33];
		_itoa_s(racerMinds[racerIndex]->getCurrentCheckpoint(), buf21, 10);
		char buf22[33];
		_itoa_s((int)(racerMinds[racerIndex]->getRotationAngle()*1000.0f), buf22, 10);
		char buf23[33];
		_itoa_s((int)(racerMinds[racerIndex]->getSpeedAmmo()), buf23, 10);
		char buf24[33];
		_itoa_s((int)(racerMinds[racerIndex]->getRocketAmmo()), buf24, 10);
		char buf25[33];
		_itoa_s((int)(racerMinds[racerIndex]->getLandmineAmmo()), buf25, 10);
		char buf26[33];
		_itoa_s(racers[racerIndex]->suicides, buf26, 10);
		char buf27[33];
		_itoa_s(racers[racerIndex]->deaths, buf27, 10);
		char buf28[33];
		_itoa_s(racers[racerIndex]->givenDamage, buf28, 10);
		char buf29[33];
		_itoa_s(racers[racerIndex]->takenDamage, buf29, 10);
		
		std::string stringArray[] = { getFPSString(seconds * 1000.0f), 
			"X: " + boolToString(intention.xPressed),
			"Y: " + boolToString(intention.yPressed),
			"A: " + boolToString(intention.aPressed),
			"B: " + boolToString(intention.bPressed),
			"Back: " + boolToString(intention.selectPressed),
			"Start: " + boolToString(intention.startPressed),
			std::string("Right Trigger: ").append(buf1),
			std::string("Left Trigger: ").append(buf2),
			std::string("RStick X: ").append(buf3),
			std::string("RStick Y: ").append(buf4),
			std::string("LStickX: ").append(buf5),
			std::string("LStickY: ").append(buf7),
			std::string("Acceleration: ").append(buf6),
			" ",
			"Player Information:",
			std::string("Currently Looking at Player #").append(buf14),
			std::string("Velocity: ").append(buf8),
			std::string("Accel. Scale: ").append(buf9),
			std::string("Current Lap: ").append(buf15),
			std::string("Kills: ").append(buf10),
			std::string("Deaths: ").append(buf27),
			std::string("Suicides: ").append(buf26),
			std::string("Given Damage: ").append(buf28),
			std::string("Taken Damage: ").append(buf29),
			std::string("Current Waypoint: ").append(buf11),
			//std::string("Current Checkpoint: ").append(buf22),
			//std::string("Checkpoint Time: ").append(buf12),
			//std::string("Speed level: ").append(buf21),
			//std::string("Speed Boost Cooldown: ").append(buf13),
			std::string("Health: ").append(buf16),
			//std::string("Laser Level: ").append(buf17),
			std::string("Placement: ").append(buf18),
			std::string("Overall position value: ").append(buf19),
			//std::string("Rotation Angle: ").append(buf22),
			std::string("Ammo Counts:"),
			std::string("Speed Boost: ").append(buf23),
			std::string("Rocket: ").append(buf24),
			std::string("Landmine: ").append(buf25)};
	
		renderer->setText(stringArray, sizeof(stringArray) / sizeof(std::string));
}

void AI::displayPostGameStatistics()
{
	char buf1[33];
	_itoa_s((int) (racerPlacement[7]->getKills()), buf1, 10);
	char buf2[33];
	_itoa_s((int) (racerPlacement[6]->getKills()), buf2, 10);
	char buf3[33];
	_itoa_s((int) (racerPlacement[5]->getKills()), buf3, 10);
	char buf4[33];
	_itoa_s((int) (racerPlacement[4]->getKills()), buf4, 10);
	char buf5[33];
	_itoa_s((int) (racerPlacement[3]->getKills()), buf5, 10);

	char buf6[33];
	_itoa_s((int) (racerPlacement[7]->getDeaths()), buf6, 10);
	char buf7[33];
	_itoa_s((int) (racerPlacement[6]->getDeaths()), buf7, 10);
	char buf8[33];
	_itoa_s((int) (racerPlacement[5]->getDeaths()), buf8, 10);
	char buf9[33];
	_itoa_s((int) (racerPlacement[4]->getDeaths()), buf9, 10);
	char buf10[33];
	_itoa_s((int) (racerPlacement[3]->getDeaths()), buf10, 10);

	char buf11[33];
	_itoa_s((int) (racerPlacement[7]->getSuicides()), buf11, 10);
	char buf12[33];
	_itoa_s((int) (racerPlacement[6]->getSuicides()), buf12, 10);
	char buf13[33];
	_itoa_s((int) (racerPlacement[5]->getSuicides()), buf13, 10);
	char buf14[33];
	_itoa_s((int) (racerPlacement[4]->getSuicides()), buf14, 10);
	char buf15[33];
	_itoa_s((int) (racerPlacement[3]->getSuicides()), buf15, 10);

	char buf16[33];
	_itoa_s((int) (racerPlacement[7]->getDamageDone()), buf16, 10);
	char buf17[33];
	_itoa_s((int) (racerPlacement[6]->getDamageDone()), buf17, 10);
	char buf18[33];
	_itoa_s((int) (racerPlacement[5]->getDamageDone()), buf18, 10);
	char buf19[33];
	_itoa_s((int) (racerPlacement[4]->getDamageDone()), buf19, 10);
	char buf20[33];
	_itoa_s((int) (racerPlacement[3]->getDamageDone()), buf20, 10);

	char buf21[33];
	_itoa_s((int) (racerPlacement[7]->getDamageTaken()), buf21, 10);
	char buf22[33];
	_itoa_s((int) (racerPlacement[6]->getDamageTaken()), buf22, 10);
	char buf23[33];
	_itoa_s((int) (racerPlacement[5]->getDamageTaken()), buf23, 10);
	char buf24[33];
	_itoa_s((int) (racerPlacement[4]->getDamageTaken()), buf24, 10);
	char buf25[33];
	_itoa_s((int) (racerPlacement[3]->getDamageTaken()), buf25, 10);

	std::string stringArray[] = 
	{
		std::string("Player Name:     Kills:     Deaths:     Suicides:     Damage Done:     Damage Taken:"),
		std::string(racerPlacement[7]->getRacerName() + "     " + buf1 + "     " + buf6  + "     " + buf11 + "     " + buf16+ "     " + buf21), // 1st place
		std::string(racerPlacement[6]->getRacerName() + "     " + buf2 + "     " + buf7  + "     " + buf12 + "     " + buf17+ "     " + buf22), // 2nd place
		std::string(racerPlacement[5]->getRacerName() + "     " + buf3 + "     " + buf8  + "     " + buf13 + "     " + buf18+ "     " + buf23), // 3rd place
		std::string(racerPlacement[4]->getRacerName() + "     " + buf4 + "     " + buf9  + "     " + buf14 + "     " + buf19+ "     " + buf24), // 4th place
		std::string(racerPlacement[3]->getRacerName() + "     " + buf5 + "     " + buf10 + "     " + buf15 + "     " + buf20+ "     " + buf25)  // 5th place
		//std::string(racerPlacement[3]->getRacerName() + "     " + buf26 + "     " + buf29 + "     " + buf32 + "     " + buf35+ "     " + buf38), // 6th place
		//std::string(racerPlacement[3]->getRacerName() + "     " + buf27 + "     " + buf30 + "     " + buf33 + "     " + buf36+ "     " + buf39), // 7th place
		//std::string(racerPlacement[3]->getRacerName() + "     " + buf28 + "     " + buf31 + "     " + buf34 + "     " + buf37+ "     " + buf40) // 8th place
	};


	renderer->setText(stringArray, sizeof(stringArray) / sizeof(std::string));
}
