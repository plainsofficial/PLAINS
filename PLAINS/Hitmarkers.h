#pragma once

#include "Hooks.h"
#include "Interfaces.h"
#include "Settings.h"

class CHitMarkerEventManager : public IGameEventListener
{
public:
	void FireGameEvent(IGameEvent *event);
	void RegisterSelf();
	bool ShouldHitmarker();
	bool ShouldRoundStart();
private:
	bool bDoHitmarker;
	bool bDoRoundStart;
};

class CHitMarker
{
public:
	void Draw();
	CHitMarkerEventManager eventmanger;
};

extern CHitMarker hitmarkers;