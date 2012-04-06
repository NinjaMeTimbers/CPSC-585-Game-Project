#include "Racer.h"

int Racer::xID = 0;
int Racer::yID = 1;
int Racer::zID = 2;

ConfigReader Racer::config = ConfigReader();
	
hkVector4 Racer::xAxis = hkVector4(1.0f, 0.0f, 0.0f);
hkVector4 Racer::yAxis = hkVector4(0.0f, 1.0f, 0.0f);
hkVector4 Racer::zAxis = hkVector4(0.0f, 0.0f, 1.0f);
hkVector4 Racer::attachFL = hkVector4(-0.8f, -0.72f, 1.5f);
hkVector4 Racer::attachFR = hkVector4(0.8f, -0.72f, 1.5f);
hkVector4 Racer::attachRL = hkVector4(-0.8f, -0.65f, -1.2f);
hkVector4 Racer::attachRR = hkVector4(0.8f, -0.65f, -1.2f);
hkVector4 Racer::attachLaser = hkVector4(0.0f, 0.1f, 1.8f);

hkReal Racer::chassisMass = config.chassisMass;
float Racer::grip = config.grip;
float Racer::accelerationScale = config.accelerationScale;
float Racer::frontSpringK = config.kFront;
float Racer::rearSpringK = config.kRear;
float Racer::frontDamperC = config.frontDamping;
float Racer::rearDamperC = config.rearDamping;

float Racer::frontExtents = config.frontExtents;
float Racer::rearExtents = config.rearExtents;
float Racer::springForceCap = config.springForceCap;

float Racer::topSpeed = config.topSpeed;
float Racer::dragCoeff = chassisMass*accelerationScale/(topSpeed*topSpeed);

hkpWorld* Racer::physicsWorld = NULL;
Sound* Racer::sound = NULL;


Racer::Racer(IDirect3DDevice9* device, Renderer* r, Physics* p, Sound* s, RacerType racerType)
{
	sound = s;

	laserDraw = new Drawable(LASERMODEL, "laser.dds", device);
	r->addDrawable(laserDraw);

	health = 100;
	kills = 0;
	laserTime = 5.0f;
	laserReady = true;

	index = -1;

	currentSteering = 0.0f;
	currentAcceleration = 0.0f;

	lookDir = hkVector4(0, 0, 1);
	lookHeight = 0;
	
	physicsWorld = p->world;

	switch (racerType)
	{
	case RACER1:
		drawable = new Drawable(RACER, "racer1.dds", device);
		break;
	case RACER2:
		drawable = new Drawable(RACER, "racer2.dds", device);
		break;
	default:
		drawable = new Drawable(RACER, "racer2.dds", device);
	}


	// Set up filter group (so the car doesn't collide with the wheels)
	int collisionGroupFilter = p->getFilter();
	
	hkpRigidBodyCinfo info;
	hkVector4 halfExtent(0.9f, 0.65f, 2.1f);		//Half extent for racer rigid body box
	info.m_shape = new hkpBoxShape(halfExtent);
	info.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
	info.m_centerOfMass = hkVector4(0.0f, -0.4f, 0.0f);	// lower CM a bit
	info.m_restitution = 0.0f;
	info.m_maxAngularVelocity = 180.0f;
	info.m_maxLinearVelocity = 170.0f;
	hkpMassProperties massProperties;
	hkpInertiaTensorComputer::computeBoxVolumeMassProperties(halfExtent, chassisMass, massProperties);
	info.setMassProperties(massProperties);
	info.m_collisionFilterInfo = hkpGroupFilter::calcFilterInfo(hkpGroupFilterSetup::LAYER_AI, collisionGroupFilter);
	body = new hkpRigidBody(info);		//Create rigid body
	body->setLinearVelocity(hkVector4(0, 0, 0));
	info.m_shape->removeReference();

	index = r->addDrawable(drawable);
	p->addRigidBody(body);


	// Create tires
	wheelFL = new FrontWheel(device, collisionGroupFilter);
	r->addDrawable(wheelFL->drawable);
	p->addRigidBody(wheelFL->body);

	
	wheelFR = new FrontWheel(device, collisionGroupFilter);
	r->addDrawable(wheelFR->drawable);
	p->addRigidBody(wheelFR->body);


	wheelRL = new RearWheel(device, collisionGroupFilter);
	r->addDrawable(wheelRL->drawable);
	p->addRigidBody(wheelRL->body);


	wheelRR = new RearWheel(device, collisionGroupFilter);
	r->addDrawable(wheelRR->drawable);
	p->addRigidBody(wheelRR->body);

	// Now constrain the tires
	hkpGenericConstraintData* constraint;
	hkpConstraintInstance* constraintInst;

	constraint = new hkpGenericConstraintData();
	buildConstraint(&attachFL, constraint, FRONT);
	constraintInst = new hkpConstraintInstance(wheelFL->body, body, constraint);
	p->world->addConstraint(constraintInst);
	constraint->removeReference();

	constraint = new hkpGenericConstraintData();
	buildConstraint(&attachFR, constraint, FRONT);
	constraintInst = new hkpConstraintInstance(wheelFR->body, body, constraint);
	p->world->addConstraint(constraintInst);
	constraint->removeReference();
	
	constraint = new hkpGenericConstraintData();
	buildConstraint(&attachRL, constraint, REAR);
	constraintInst = new hkpConstraintInstance(wheelRL->body, body, constraint);
	p->world->addConstraint(constraintInst);
	constraint->removeReference();

	constraint = new hkpGenericConstraintData();
	buildConstraint(&attachRR, constraint, REAR);
	constraintInst = new hkpConstraintInstance(wheelRR->body, body, constraint);
	p->world->addConstraint(constraintInst);
	constraint->removeReference();

	
	hkpConstraintStabilizationUtil::stabilizeRigidBodyInertia(body);

	hkpPropertyValue val;
	val.setPtr(NULL);
	body->addProperty(0, val);

	reset(&(hkVector4(0, 0, 0, 0)));
}


Racer::~Racer(void)
{
	if(body)
	{
		body->removeReference();
	}
}

void Racer::setPosAndRot(float posX, float posY, float posZ,
		float rotX, float rotY, float rotZ)	// In Radians
{
	drawable->setPosAndRot(posX, posY, posZ,
		rotX, rotY, rotZ);

	hkQuaternion quat;
	quat.setAxisAngle(hkVector4(1.0f, 0.0f, 0.0f), rotX);
	quat.mul(hkQuaternion(hkVector4(0.0f, 1.0f, 0.0f), rotY));
	quat.mul(hkQuaternion(hkVector4(0.0f, 0.0f, 1.0f), rotZ));

	hkVector4 pos = hkVector4(posX, posY, posZ);

	body->setPositionAndRotation(hkVector4(posX, posY, posZ), quat);

	wheelFL->setPosAndRot(attachFL(0) + pos(0), attachFL(1) + pos(1), attachFL(2) + pos(2), rotX, rotY, rotZ);
	wheelFR->setPosAndRot(attachFR(0) + pos(0), attachFR(1) + pos(1), attachFR(2) + pos(2), rotX, rotY, rotZ);
	wheelRL->setPosAndRot(attachRL(0) + pos(0), attachRL(1) + pos(1), attachRL(2) + pos(2), rotX, rotY, rotZ);
	wheelRR->setPosAndRot(attachRR(0) + pos(0), attachRR(1) + pos(1), attachRR(2) + pos(2), rotX, rotY, rotZ);
}


void Racer::update()
{
	if (drawable && body)
	{
		hkpPropertyValue val;
		val = body->getProperty(0);

		if (val.getPtr() != NULL)
		{
			giveDamage((Racer*) val.getPtr(), 50);
			val.setPtr(NULL);
			body->setProperty(0, val);
		}

		if ((laserReady) || (laserTime < 4.0f))
		{
			((LaserModel*)(laserDraw->mesh))->drawLaser = false;
		}
		else
		{
			((LaserModel*)(laserDraw->mesh))->drawLaser = true;
		}

		D3DXMATRIX transMat;
		(body->getTransform()).get4x4ColumnMajor(transMat);
		drawable->setTransform(&transMat);
		laserDraw->setTransform(&transMat);

		
		// Now update wheels
		hkRotation carRot = body->getTransform().getRotation();
		hkTransform wheelTransform = wheelRL->body->getTransform();
		wheelTransform.setRotation(carRot);
		wheelTransform.get4x4ColumnMajor(transMat);
		wheelRL->drawable->setTransform(&transMat);

		wheelTransform = wheelRR->body->getTransform();
		wheelTransform.setRotation(carRot);
		wheelTransform.get4x4ColumnMajor(transMat);
		wheelRR->drawable->setTransform(&transMat);



		(wheelFL->body->getTransform()).get4x4ColumnMajor(transMat);
		wheelTransform = wheelFL->body->getTransform();
		wheelTransform.setRotation(carRot);
		wheelTransform.get4x4ColumnMajor(transMat);


		D3DXMATRIX rot1, rot2, trans1;
		D3DXVECTOR3 scale, trans;
		D3DXQUATERNION rot;
		
		D3DXMatrixDecompose(&scale, &rot, &trans, &transMat);
		
		D3DXMatrixRotationQuaternion(&rot1, &rot);
		D3DXMatrixRotationAxis(&rot2, &(drawable->getYVector()), currentSteering * 1.11f);
		D3DXMatrixTranslation(&trans1, trans.x, trans.y, trans.z);
		
		D3DXMatrixMultiply(&transMat, &rot1, &rot2);
		D3DXMatrixMultiply(&transMat, &transMat, &trans1);
		wheelFL->drawable->setTransform(&transMat);

		(wheelFR->body->getTransform()).get4x4ColumnMajor(transMat);
		wheelTransform = wheelFR->body->getTransform();
		wheelTransform.setRotation(carRot);
		wheelTransform.get4x4ColumnMajor(transMat);

		D3DXMatrixTranslation(&trans1, transMat._41, transMat._42, transMat._43);

		D3DXMatrixMultiply(&transMat, &rot1, &rot2);
		D3DXMatrixMultiply(&transMat, &transMat, &trans1);
		wheelFR->drawable->setTransform(&transMat);

		currentAcceleration = 0.0f;
	}
}

int Racer::getIndex()
{
	return index;
}


void Racer::buildConstraint(hkVector4* attachmentPt, hkpGenericConstraintData* constraint, WheelType type)
{
	hkpConstraintConstructionKit* kit = new hkpConstraintConstructionKit();
	kit->begin(constraint);
	kit->setLinearDofA(xAxis, xID);
	kit->setLinearDofB(xAxis, xID);
	kit->setLinearDofA(yAxis, yID);
	kit->setLinearDofB(yAxis, yID);
	kit->setLinearDofA(zAxis, zID);
	kit->setLinearDofB(zAxis, zID);
	
	hkVector4 wheelAttach = hkVector4(0,0,0);

	kit->setPivotA(wheelAttach);
	kit->setPivotB(*attachmentPt);
	kit->setAngularBasisABodyFrame();
	kit->setAngularBasisBBodyFrame();

	kit->constrainLinearDof(xID);
	kit->constrainLinearDof(zID);
	
	kit->constrainAllAngularDof();

	if (type == FRONT)
	{
		kit->setLinearLimit(yID, -frontExtents, frontExtents / 2.0f);
	}
	else
	{
		kit->setLinearLimit(yID, -rearExtents, rearExtents / 2.0f);
	}
	
	kit->end();

	constraint->setSolvingMethod(hkpConstraintAtom::METHOD_STABILIZED);
}



void Racer::brake(float seconds)
{
	// Braking. Apply force to all wheels
	hkVector4 point, forward;
	hkTransform trans = body->getTransform();

	float accelForce = chassisMass * -accelerationScale;

	accelForce /= 4.0f;

	if (wheelFL->touchingGround)
	{
		forward = wheelFL->body->getLinearVelocity();
		forward.normalize3IfNotZero();
		forward.mul(accelForce);
		point.setTransformedPos(trans, attachFL);
		body->applyForce(seconds, forward, point);
	}

	if (wheelFR->touchingGround)
	{
		forward = wheelFR->body->getLinearVelocity();
		forward.normalize3IfNotZero();
		forward.mul(accelForce);
		point.setTransformedPos(trans, attachFR);
		body->applyForce(seconds, forward, point);
	}

	if (wheelRL->touchingGround)
	{
		forward = wheelRL->body->getLinearVelocity();
		forward.normalize3IfNotZero();
		forward.mul(accelForce);
		point.setTransformedPos(trans, attachRL);
		body->applyForce(seconds, forward, point);
	}

	if (wheelRR->touchingGround)
	{
		forward = wheelRR->body->getLinearVelocity();
		forward.normalize3IfNotZero();
		forward.mul(accelForce);
		point.setTransformedPos(trans, attachRR);
		body->applyForce(seconds, forward, point);
	}
}


// between -1.0 and 1.0 (backwards is negative)
void Racer::accelerate(float seconds, float value)
{
	if ((value == 0.0f) || ( !(wheelRL->touchingGround) && !(wheelRR->touchingGround)))
	{
		currentAcceleration = 0.0f;
		return;
	}

	hkVector4 point;
	hkTransform trans = body->getTransform();

	float accelForce = value * chassisMass * accelerationScale;

	// Cap force, due to grip constraints
	float maxGrip = chassisMass * 20.0f * grip * (drawable->getYVector()).y;

	if (accelForce >= maxGrip)
	{
		accelForce = maxGrip;
	}

	currentAcceleration = value;

	hkVector4 forward = drawable->getZhkVector();
	hkVector4 vel = body->getLinearVelocity();
	float dot = vel.dot3(forward);

	if ((dot > 0.1f) && (value < 0.0f))
	{
		// Braking. Apply force to all wheels
		accelForce /= 4.0f;

		if (wheelFL->touchingGround)
		{
			forward = wheelFL->body->getLinearVelocity();
			forward.normalize3IfNotZero();
			forward.mul(accelForce);
			point.setTransformedPos(trans, attachFL);
			body->applyForce(seconds, forward, point);
		}

		if (wheelFR->touchingGround)
		{
			forward = wheelFR->body->getLinearVelocity();
			forward.normalize3IfNotZero();
			forward.mul(accelForce);
			point.setTransformedPos(trans, attachFR);
			body->applyForce(seconds, forward, point);
		}

		if (wheelRL->touchingGround)
		{
			forward = wheelRL->body->getLinearVelocity();
			forward.normalize3IfNotZero();
			forward.mul(accelForce);
			point.setTransformedPos(trans, attachRL);
			body->applyForce(seconds, forward, point);
		}

		if (wheelRR->touchingGround)
		{
			forward = wheelRR->body->getLinearVelocity();
			forward.normalize3IfNotZero();
			forward.mul(accelForce);
			point.setTransformedPos(trans, attachRR);
			body->applyForce(seconds, forward, point);
		}

	}
	else
	{
		forward.mul(accelForce / 2.0f);

		if (wheelRL->touchingGround)
		{
			point.setTransformedPos(trans, attachRL);
			body->applyForce(seconds, forward, point);
		}

		if (wheelRR->touchingGround)
		{
			point.setTransformedPos(trans, attachRR);
			body->applyForce(seconds, forward, point);
		}
	}

	computeRPM();
}


// between -1.0 and 1.0 (left is negative)
// Make sure this is called once every frame, even if value = 0
void Racer::steer(float seconds, float value)
{
	currentSteering = value;

	if (!(wheelFL->touchingGround) && !(wheelFR->touchingGround))
		return;
	
	float torqueScale = 0.0f;
	float centripScale = 0.0f;

	hkVector4 vel = body->getLinearVelocity();
	hkVector4 yVec = drawable->getYhkVector();
	hkVector4 currentAngularVelocity = body->getAngularVelocity();


	float dot = vel.dot3(vel);

	bool negative = false;

	if (dot < 0.0f)
	{
		negative = true;
		dot *= -1;
	}



	float currentAngularSpeed = currentAngularVelocity.dot3(yVec);
	float maxAngularSpeed = 5.0f;


	if (dot == 0.0f)
	{
		maxAngularSpeed = 0.0f;
	}
	else if (dot < 8.0f)
	{
		//maxAngularSpeed = 0.4f * dot;
	}

	float desiredAngularSpeed = value * maxAngularSpeed;
	float deltaAngularSpeed = desiredAngularSpeed - currentAngularSpeed;



	
	
	
	// Now adjust forces (will need to tweak these values later)
	if (dot < 2.0f)
	{
		torqueScale = 0.4f;
	}
	else if (dot < 11.0f)	// < ~40km/h. Low speed, no slip
	{
		torqueScale = 1.0f;
		centripScale = 2.0f;
	}
	else if (dot < 15.0f)
	{

		torqueScale = 5.0f;
		centripScale = 5.0f;
	}
	else if (dot < 25.0f)
	{
		torqueScale = 5.0f;
		centripScale = 7.0f;
	}
	else if (dot < 45.0f)
	{
		torqueScale = 7.0f;
		centripScale = 10.0f;
	}
	else
	{
		torqueScale = 7.0f;
		centripScale = 12.0f;
	}
	
	if (negative)
	{
		torqueScale *= -1;
		centripScale *= -1;
	}


	hkVector4 force = hkVector4(drawable->getXhkVector());
	
	if (force.dot3(vel) < 0.0f)
	{
		force.mul(deltaAngularSpeed * chassisMass * centripScale);
		body->applyForce(seconds, force, body->getPosition());
	}

	force = hkVector4(yVec);
	force.mul(deltaAngularSpeed * chassisMass * torqueScale);
	body->applyTorque(seconds, force);


	/*currentSteering = value;

	if ((!(wheelFL->touchingGround) && !(wheelFR->touchingGround)) || (value == 0.0f))
		return;
	
	hkVector4 vel = body->getLinearVelocity();
	float dot = vel.dot3(drawable->getZhkVector());

	bool negative = false;

	if (dot < 0.0f)
	{
		negative = true;
		dot *= -1;
	}
	
	
	// Now adjust forces (will need to tweak these values later)

	float torqueScale = 0.0f;
	float centripScale = 0.0f;

	
	if (dot < 11.0f)	// < ~40km/h. Low speed, no slip
	{
		torqueScale = 1.0f;
		centripScale = 2.0f;
	}
	else if (dot < 15.0f)
	{

		torqueScale = 1.0f;
		centripScale = 5.0f;
	}
	else if (dot < 25.0f)
	{
		torqueScale = 1.0f;
		centripScale = 20.0f;
	}
	else if (dot < 45.0f)
	{
		torqueScale = 1.0f;
		centripScale = 20.0f;
	}
	else
	{
		torqueScale = 1.0f;
		centripScale = 20.0f;
	}
	
	if (negative)
	{
		torqueScale *= -1;
		centripScale *= -1;
	}
	

	hkVector4 force = hkVector4(drawable->getXhkVector());
	force.mul(value * chassisMass * centripScale);
	body->applyForce(seconds, force, body->getPosition());

	force = hkVector4(drawable->getYhkVector());
	force.mul(value * chassisMass * torqueScale);
	body->applyTorque(seconds, force);*/
}


void Racer::reset(hkVector4* resetPos)
{
	hkVector4 reset = hkVector4(0.0f, 0.0f, 0.0f);
	setPosAndRot((float)resetPos->getComponent(0), (float)resetPos->getComponent(1), (float)resetPos->getComponent(2), 0, 0, 0);
	body->setLinearVelocity(reset);
	wheelFL->body->setLinearVelocity(reset);
	wheelFR->body->setLinearVelocity(reset);
	wheelRL->body->setLinearVelocity(reset);
	wheelRR->body->setLinearVelocity(reset);

	body->setAngularVelocity(reset);
	wheelFL->body->setAngularVelocity(reset);
	wheelFR->body->setAngularVelocity(reset);
	wheelRL->body->setAngularVelocity(reset);
	wheelRR->body->setAngularVelocity(reset);
}

/**
Vector dragForce = chassisRigidBody.GetLinearVelocity();
float speed = dragForce.Normalize();
dragForce.Scale(-dragCoeff * speed * speed);
�To determine dragCoeff, set dragForce to accelForce, set speed == topSpeed, and solve
*/
void Racer::applyDrag(float seconds)
{
	hkVector4 dragForce = body->getLinearVelocity();
	float test = dragCoeff;
	float speed = dragForce.normalizeWithLength3();
	dragForce.mul(-dragCoeff*speed*speed);
	body->applyForce(seconds, dragForce);
}

void Racer::applySprings(float seconds)
{
	// Applies springs and dampers using the helper function getForce()
	hkVector4 force, point;
	hkVector4 upVector = drawable->getYhkVector();
	hkTransform transform = body->getTransform();

	if (wheelFL->touchingGround)
	{
		force = getForce(&upVector,  wheelFL->body, &attachFL, FRONT);
		point.setTransformedPos(transform, attachFL);
		body->applyForce(seconds, force, point);
	}

	if (wheelFR->touchingGround)
	{
		force = getForce(&upVector,  wheelFR->body, &attachFR, FRONT);
		point.setTransformedPos(transform, attachFR);
		body->applyForce(seconds, force, point);
	}

	if (wheelRL->touchingGround)
	{
		force = getForce(&upVector,  wheelRL->body, &attachRL, REAR);
		point.setTransformedPos(transform, attachRL);
		body->applyForce(seconds, force, point);
	}

	if (wheelRR->touchingGround)
	{
		force = getForce(&upVector,  wheelRR->body, &attachRR, REAR);
		point.setTransformedPos(transform, attachRR);
		body->applyForce(seconds, force, point);
	}
}


hkVector4 Racer::getForce(hkVector4* up, hkpRigidBody* wheel, hkVector4* attach, WheelType type)
{
	hkVector4 actualPos, restPos, force, damperForce, pointVel;
	float displacement, speedOfDisplacement;

	float k, c;

	if (type == FRONT)
	{
		k = frontSpringK;
		c = frontDamperC;
	}
	else
	{
		k = rearSpringK;
		c = rearDamperC;
	}


	actualPos = wheel->getPosition();
	restPos.setTransformedPos(body->getTransform(), *attach);
	actualPos.sub(restPos);

	displacement = actualPos.dot3(*up);

	if (type == FRONT)
	{
		if (displacement < -frontExtents)
		{
			displacement = -frontExtents;
		}
		else if (displacement > frontExtents)
		{
			displacement = frontExtents;
		}
	}
	else
	{
		if (displacement < -rearExtents)
		{
			displacement = -rearExtents;
		}
		else if (displacement > rearExtents)
		{
			displacement = rearExtents;
		}
	}

	force = hkVector4(*up);
	force.mul(k * displacement);

	body->getPointVelocity(restPos, pointVel);

	speedOfDisplacement = pointVel.dot3(*up);

	damperForce = hkVector4(*up);
	damperForce.mul(c * -speedOfDisplacement);

	force.add(damperForce);

	if (force.dot3(force) > springForceCap*springForceCap)
	{
		float forceMultiplier = springForceCap / ((float)force.length3());
		force.mul(forceMultiplier);
	}
	

	return force;
}


void Racer::applyForces(float seconds)
{
	hkVector4 aVel, vel = body->getLinearVelocity();
	float dot = vel.dot3(vel);
	aVel = body->getAngularVelocity();
	float aDot = aVel.dot3(aVel);

	// Only want to be automatically braking if the player
	// isn't trying to move or already moving
	if ((dot > 0.0f) && (dot < 6.0f) && (aDot != 0.0f) &&
		(currentAcceleration == 0.0f))
	{
		brake(seconds);
	}
	else
	{
		applyFriction(seconds);
	}

	applyDrag(seconds);
	applyTireRaycast();
	applySprings(seconds);
	

	if (!laserReady)
	{
		laserTime -= seconds;

		if (laserTime < 0.0f)
		{
			laserTime = 5.0f;
			laserReady = true;
		}
	}
}


// Raycasts tires and adjusts their positions
void Racer::applyTireRaycast()
{
	hkpWorldRayCastInput input;
	hkpWorldRayCastOutput output;
	hkVector4 from;
	hkVector4 to;
	hkVector4 raycastDir = drawable->getYhkVector();
	raycastDir.mul(-1);
	hkTransform transform = body->getTransform();

	// Raycast and reposition each tire
	input = hkpWorldRayCastInput();
	output = hkpWorldRayCastOutput();

	// FRONT LEFT TIRE
	from.setTransformedPos(transform, attachFL);
	to = hkVector4(raycastDir);
	to.mul(frontExtents + 0.35f);
	to.add(from);
	
	input.m_from = hkVector4(from);
	input.m_to = hkVector4(to);

	physicsWorld->castRay(input, output);
	
	if (output.hasHit())
	{
		wheelFL->touchingGround = true;
		to.sub(from);
		to.mul(output.m_hitFraction);
		raycastDir.mul(-0.35f);

		to.add(from);
		to.add(raycastDir);

		wheelFL->body->setPosition(to);
	}
	else
	{
		wheelFL->touchingGround = false;
		to.sub(from);
		raycastDir.mul(-0.35f);

		to.add(from);
		to.add(raycastDir);

		wheelFL->body->setPosition(to);
	}

	input = hkpWorldRayCastInput();
	output = hkpWorldRayCastOutput();

	raycastDir = drawable->getYhkVector();
	raycastDir.mul(-1);

	// FRONT RIGHT TIRE
	from.setTransformedPos(transform, attachFR);
	to = hkVector4(raycastDir);
	to.mul(frontExtents + 0.35f);
	to.add(from);

	input.m_from = hkVector4(from);
	input.m_to = hkVector4(to);

	physicsWorld->castRay(input, output);

	if (output.hasHit())
	{
		wheelFR->touchingGround = true;
		to.sub(from);
		to.mul(output.m_hitFraction);
		raycastDir.mul(-0.35f);

		to.add(from);
		to.add(raycastDir);

		wheelFR->body->setPosition(to);
	}
	else
	{
		wheelFR->touchingGround = false;
		to.sub(from);
		raycastDir.mul(-0.35f);

		to.add(from);
		to.add(raycastDir);

		wheelFR->body->setPosition(to);
	}

	input = hkpWorldRayCastInput();
	output = hkpWorldRayCastOutput();

	raycastDir = drawable->getYhkVector();
	raycastDir.mul(-1);

	// REAR LEFT TIRE
	from.setTransformedPos(transform, attachRL);
	to = hkVector4(raycastDir);
	to.mul(rearExtents + 0.42f);
	to.add(from);

	input.m_from = hkVector4(from);
	input.m_to = hkVector4(to);

	physicsWorld->castRay(input, output);

	if (output.hasHit())
	{
		wheelRL->touchingGround = true;
		to.sub(from);
		to.mul(output.m_hitFraction);
		raycastDir.mul(-0.42f);

		to.add(from);
		to.add(raycastDir);

		wheelRL->body->setPosition(to);
	}
	else
	{
		wheelRL->touchingGround = false;
		to.sub(from);
		raycastDir.mul(-0.42f);

		to.add(from);
		to.add(raycastDir);

		wheelRL->body->setPosition(to);
	}

	input = hkpWorldRayCastInput();
	output = hkpWorldRayCastOutput();

	raycastDir = drawable->getYhkVector();
	raycastDir.mul(-1);

	// REAR RIGHT TIRE
	from.setTransformedPos(transform, attachRR);
	to = hkVector4(raycastDir);
	to.mul(rearExtents + 0.42f);
	to.add(from);

	input.m_from = hkVector4(from);
	input.m_to = hkVector4(to);

	physicsWorld->castRay(input, output);

	if (output.hasHit())
	{
		wheelRR->touchingGround = true;
		to.sub(from);
		to.mul(output.m_hitFraction);
		raycastDir.mul(-0.42f);

		to.add(from);
		to.add(raycastDir);

		wheelRR->body->setPosition(to);
	}
	else
	{
		wheelRR->touchingGround = false;
		to.sub(from);
		raycastDir.mul(-0.42f);

		to.add(from);
		to.add(raycastDir);

		wheelRR->body->setPosition(to);
	}
}


void Racer::applyFriction(float seconds)
{
	float yComponent = drawable->getYVector().y;

	if (yComponent < 0.01f)
		return;
	

	float xFrictionForce = yComponent * yComponent * grip * 20 * -chassisMass;	// / 4.0f
	float zFrictionForce = yComponent * yComponent * (grip * 0.02f) * 20 * -chassisMass;  // / 4.0f

	hkVector4 xForce, zForce, velocity = body->getLinearVelocity();
	velocity.normalize3IfNotZero();

	xForce = drawable->getXhkVector();

	float dot = velocity.dot3(xForce);
	

	xForce.mul(xFrictionForce * dot);
	body->applyForce(seconds, xForce);

	zForce = drawable->getZhkVector();
		
	dot = velocity.dot3(zForce);

	zForce.mul(zFrictionForce * dot);
	body->applyForce(seconds, zForce);


	/*
	if (wheelFL->touchingGround)
	{
		applyFrictionToTire(&attachFL, wheelFL->body, xFrictionForce, zFrictionForce, seconds, FRONT);
	}

	if (wheelFR->touchingGround)
	{
		applyFrictionToTire(&attachFR, wheelFR->body, xFrictionForce, zFrictionForce, seconds, FRONT);
	}

	if (wheelRL->touchingGround)
	{
		applyFrictionToTire(&attachRL, wheelRL->body, xFrictionForce, zFrictionForce, seconds, REAR);
	}

	if (wheelRR->touchingGround)
	{
		applyFrictionToTire(&attachRR, wheelRR->body, xFrictionForce, zFrictionForce, seconds, REAR);
	}
	*/
}


void Racer::applyFrictionToTire(hkVector4* attachPoint, hkpRigidBody* wheelBody,
	float xFrictionForce, float zFrictionForce, float seconds, WheelType type)
{
	if (type == REAR)
	{
		xFrictionForce *= 1.2f;
	}

	hkVector4 point, velocity, xForce, zForce;
	point.setTransformedPos(body->getTransform(), *attachPoint);
	body->getPointVelocity(point, velocity);
	

	velocity.normalize3IfNotZero();

	xForce = drawable->getXhkVector();


	float dot = velocity.dot3(xForce);
	
	xForce.mul(xFrictionForce * dot);
	body->applyForce(seconds, xForce, point);


	zForce = drawable->getZhkVector();
		
	dot = velocity.dot3(zForce);

	zForce.mul(zFrictionForce * dot);
	body->applyForce(seconds, zForce, point);
}

void Racer::fireLaser()
{
	if (!laserReady)
		return;
	else
	{
		laserReady = false;
	}

	sound->playLaser();

	hkpWorldRayCastInput input;
	hkpWorldRayCastOutput output;
	hkVector4 from;
	hkVector4 to;
	hkVector4 raycastDir = drawable->getZhkVector();
	hkTransform transform = body->getTransform();


	// Raycast
	input = hkpWorldRayCastInput();
	output = hkpWorldRayCastOutput();
	
	from.setTransformedPos(transform, attachLaser);
	to = hkVector4(raycastDir);
	to.mul(30.0f);	// Laser length
	to.add(from);

	input.m_from = hkVector4(from);
	input.m_to = hkVector4(to);

	physicsWorld->castRay(input, output);

	if (output.hasHit())
	{
		hkpRigidBody* hitBody = (hkpRigidBody*) output.m_rootCollidable->getOwner();
		hkpPropertyValue val;
		val.setPtr(this);
		
		hitBody->setProperty(0, val);
	}
}

void Racer::respawn()
{
	hkVector4 deathPos = body->getPosition();
	hkQuaternion deathRot = body->getRotation();

	reset(&(hkVector4(0, 0, 0, 0)));

	deathPos(1) += 5.0f;
	body->setPositionAndRotation(deathPos, deathRot);
}

void Racer::giveDamage(Racer* attacker, int damage)
{
	health -= damage;
	sound->playCrash();

	if (health <= 0)
	{
		health = 100;
		respawn();
		attacker->kills += 1;
	}
}

void Racer::computeRPM()
{
	int gear;
	float rpm;

	float gearRatios[6] = {2.97f, 2.97f, 1.8, 1.43, 1.0, .84};

	hkVector4 forward = drawable->getZhkVector();
	hkVector4 vel = body->getLinearVelocity();
	float forwardSpeed = vel.dot3(forward);

	float angularVel = forwardSpeed/0.42;
	float wheelRPM = angularVel*30/3.14159;

	gear = forwardSpeed/15+1;
	if(gear < 0)
		gear = 0;
	else if (gear > 5)
		gear = 5;

	rpm = gearRatios[gear]*3.14*wheelRPM;
	rpm = rpm;
}

/**
 * Serializes the racer to send over the net
 * Order:
 * Position
 * Rotation
 * Linear Velocity
 * Angular Velocity
 * Wheel FL Position
 * Wheel FR Position
 * Wheel RL Position
 * Wheel RR Position
 */
void Racer::serialize(char buff[])
{
	memcpy(buff,&body->getPosition(),sizeof(hkVector4)); //Position
	memcpy(buff+sizeof(hkVector4),&body->getRotation(),sizeof(hkQuaternion)); //Rotation
	memcpy(buff+sizeof(hkVector4)+sizeof(hkQuaternion),&body->getLinearVelocity(),sizeof(hkVector4)); //Linear Velocity
	memcpy(buff+sizeof(hkVector4)*2+sizeof(hkQuaternion),&body->getAngularVelocity(),sizeof(hkVector4)); //Angular Velocity
	memcpy(buff+sizeof(hkVector4)*3+sizeof(hkQuaternion),&wheelFL->body->getPosition(),sizeof(hkVector4)); //FL wheel
	memcpy(buff+sizeof(hkVector4)*4+sizeof(hkQuaternion),&wheelFR->body->getPosition(),sizeof(hkVector4)); //FR wheel
	memcpy(buff+sizeof(hkVector4)*5+sizeof(hkQuaternion),&wheelRL->body->getPosition(),sizeof(hkVector4)); //RL wheel
	memcpy(buff+sizeof(hkVector4)*6+sizeof(hkQuaternion),&wheelRR->body->getPosition(),sizeof(hkVector4)); //RR wheel
	memcpy(buff+sizeof(hkVector4)*7+sizeof(hkQuaternion),&lookDir,sizeof(hkVector4)); //Look direction
	memcpy(buff+sizeof(hkVector4)*8+sizeof(hkQuaternion),&lookHeight,sizeof(float)); //Look height
	memcpy(buff+sizeof(hkVector4)*8+sizeof(hkQuaternion)+sizeof(float),&laserReady,sizeof(bool)); //Laser status
}

void Racer::unserialize(char buff[])
{
	hkVector4 data;
	hkQuaternion rot;
	memcpy(&data,buff,sizeof(hkVector4)); //Position
	body->setPosition(data);

	memcpy(&rot,buff+sizeof(hkVector4),sizeof(hkQuaternion)); //Rotation
	body->setRotation(rot);

	memcpy(&data,buff+sizeof(hkVector4)+sizeof(hkQuaternion),sizeof(hkVector4)); //Linear velocity
	body->setLinearVelocity(data);

	memcpy(&data,buff+sizeof(hkVector4)*2+sizeof(hkQuaternion),sizeof(hkVector4)); //Angular Velocity
	body->setAngularVelocity(data);

	memcpy(&data,buff+sizeof(hkVector4)*3+sizeof(hkQuaternion),sizeof(hkVector4)); //FL wheel
	wheelFL->body->setPosition(data);
	wheelFL->body->setRotation(rot);

	memcpy(&data,buff+sizeof(hkVector4)*4+sizeof(hkQuaternion),sizeof(hkVector4)); //FR wheel
	wheelFR->body->setPosition(data);
	wheelFR->body->setRotation(rot);

	memcpy(&data,buff+sizeof(hkVector4)*5+sizeof(hkQuaternion),sizeof(hkVector4)); //RL wheel
	wheelRL->body->setPosition(data);
	wheelRL->body->setRotation(rot);

	memcpy(&data,buff+sizeof(hkVector4)*6+sizeof(hkQuaternion),sizeof(hkVector4)); //RR wheel
	wheelRR->body->setPosition(data);
	wheelRR->body->setRotation(rot);

	memcpy(&lookDir,buff+sizeof(hkVector4)*7+sizeof(hkQuaternion),sizeof(hkVector4)); //Look direction

	memcpy(&lookHeight,buff+sizeof(hkVector4)*8+sizeof(hkQuaternion),sizeof(float)); //Look height

	bool lready;
	memcpy(&lready,buff+sizeof(hkVector4)*8+sizeof(hkQuaternion)+sizeof(float),sizeof(bool)); //Is the laser ready to be firing?
	if(!lready && laserReady)
	{
		fireLaser();
	}
}