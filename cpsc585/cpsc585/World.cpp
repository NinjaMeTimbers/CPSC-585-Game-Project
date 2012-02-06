#include "World.h"


World::World(IDirect3DDevice9* device)
{
	drawable = new Drawable(WORLD, "someTextureName", device);


	hkpRigidBodyCinfo info;
	hkVector4 halfExtent(3.0f, 3.0f, 3.0f);
	info.m_shape = new hkpBoxShape(halfExtent);
	info.m_motionType = hkpMotion::MOTION_FIXED;	// Static object

	hkpMassProperties massProperties;
	hkpInertiaTensorComputer::computeShapeVolumeMassProperties(info.m_shape, 10.0f, massProperties);
	info.setMassProperties(massProperties);
	body = new hkpRigidBody(info);
}


World::~World(void)
{
	if(body)
	{
		body->removeReference();
	}
}

void World::setPosAndRot(float posX, float posY, float posZ,
		float rotX, float rotY, float rotZ)	// In Radians
{
	drawable->setPosAndRot(posX, posY, posZ,
		rotX, rotY, rotZ);

	hkQuaternion quat;
	quat.setAxisAngle(hkVector4(1.0f, 0.0f, 0.0f), rotX);
	quat.mul(hkQuaternion(hkVector4(0.0f, 1.0f, 0.0f), rotY));
	quat.mul(hkQuaternion(hkVector4(0.0f, 1.0f, 0.0f), rotZ));

	body->setPositionAndRotation(hkVector4(posX, posY, posZ), quat);
}
