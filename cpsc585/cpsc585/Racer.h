#pragma once

#include "Drawable.h"
#include "Physics.h"
#include "Renderer.h"
#include "Sound.h"

#include "FrontWheel.h"
#include "RearWheel.h"
#include "ConfigReader.h"
#include "LaserModel.h"

enum RacerType { RACER1, RACER2, RACER3, RACER4, RACER5, RACER6, RACER7, RACER8 };
enum WheelType { FRONT, REAR };

const int RACERSIZE = sizeof(hkVector4)*8 + sizeof(hkQuaternion) + sizeof(float) + sizeof(bool);

class Racer
{
public:
	Racer(IDirect3DDevice9* device, Renderer* r, Physics* p, Sound* s, RacerType racerType);
	~Racer(void);
	void setPosAndRot(float posX, float posY, float posZ,
		float rotX, float rotY, float rotZ);	// In Radians
	void update();

	void brake(float seconds);
	void accelerate(float seconds, float value);	// between -1.0 and 1.0 (backwards is negative)
	void steer(float seconds, float value);			// between -1.0 and 1.0 (left is negative)

	int getIndex();
	void reset(hkVector4* resetPos);	// Reset position and set velocity/momentum to 0

	void applyForces(float seconds);	// Call this every frame BEFORE stepping physics!

	void fireLaser();
	void giveDamage(Racer* attacker, int damage);

	void serialize(char buff[]);
	void unserialize(char buff[]);

	void computeRPM();

private:
	void buildConstraint(hkVector4* attachmentPt, hkpGenericConstraintData* constraint, WheelType type);
	hkVector4 getForce(hkVector4* up, hkpRigidBody* wheel, hkVector4* attach, WheelType type);
	void applySprings(float seconds);
	void applyFriction(float seconds);
	void applyFrictionToTire(hkVector4* attachPoint, hkpRigidBody* wheelBody,
		float xFrictionForce, float zFrictionForce, float seconds, WheelType type);
	void applyTireRaycast();
	void applyDrag(float seconds);
	void respawn();

public:
	Drawable* drawable;
	hkpRigidBody* body;
	static float accelerationScale;
	bool laserReady;

	int health;
	int kills;
	float laserTime;

	hkVector4 lookDir;
	float lookHeight;

	float currentAcceleration;

private:
	Drawable* laserDraw;

	int index;
	FrontWheel* wheelFL;
	FrontWheel* wheelFR;

	RearWheel* wheelRL;
	RearWheel* wheelRR;

	float currentSteering;


	static hkpWorld* physicsWorld;
	static Sound* sound;

	// Static elements that are common between all Racers
	static int xID;
	static int yID;
	static int zID;
	
	static hkVector4 xAxis;
	static hkVector4 yAxis;
	static hkVector4 zAxis;
	static hkVector4 attachFL;
	static hkVector4 attachFR;
	static hkVector4 attachRL;
	static hkVector4 attachRR;
	static hkVector4 attachLaser;

	static hkReal chassisMass;

	static float dragCoeff;
	static float topSpeed;

	static float rearSpringK;
	static float frontSpringK;
	static float rearDamperC;
	static float frontDamperC;
	static float rearExtents;
	static float frontExtents;
	static float springForceCap;
	static float grip;
	
	static ConfigReader config;
};
