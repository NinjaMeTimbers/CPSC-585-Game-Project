#pragma once

#include "Drawable.h"
#include "Physics.h"
#include "DynamicObj.h"
#include "Racer.h"
#include "Smoke.h"

#define BLAST_RADIUS 20.0
#define BLAST_DAMAGE 15.0

class Explosion :
	public DynamicObj
{
public:
	Explosion(IDirect3DDevice9* device, const hkTransform* trans, Racer* owns);
	~Explosion(void);
	void update(float seconds);
	void doDamage();

private:
	

public:
	Racer* owner;
	hkTransform* transform;

private:
	hkVector4 pos;
	float lifetime;
};
