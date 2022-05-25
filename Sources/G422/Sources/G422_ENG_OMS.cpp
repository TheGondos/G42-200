#include "../Headers/G422.h"
#include "../Headers/G422_DVC.h"
#include "../Headers/G422_MDL_DVC.h"

void G422::cueEngines(OMS& eng, OMS::SIMSTATE sst)
{
	if (eng.state == sst) return;

	eng.state = sst;
}

void G422::simEngines(double& dT, OMS& eng)
{
	double fuelMass = GetPropellantMass(asfFuel);

	if (eng.feed & OMS::FUEL_PUMP && (fuelMass < 0.001 || !(eng.feed & OMS::FUEL_OPEN))) {
		VECTOR3 V0 = _V0;
		clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW2_SYSFEED_OMS] & 0xFFFF), PANEL_MOUSE_RBPRESSED, V0);
	}

	if (eng.feed & OMS::FUEL_PUMP)
	{
		if (eng.fuelPrs < 1)
		{
			double deltaPrs = 0.22 * dT;
			eng.fuelPrs = min(eng.fuelPrs + deltaPrs, 1.0);
		}
	}
	else
	{
		if (eng.fuelPrs > 0)
		{
			double deltaPrs = -0.12 * dT;
			eng.fuelPrs = max(eng.fuelPrs + deltaPrs, 0.0);
		}
	}

	if (fuelMass < 0.001) eng.fuelPrs = 0;

	if (eng.fuelPrs == 0) 
	{
		VECTOR3 V0 = _V0;
		clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_STBYIGN_OMS] & 0xFFFF), PANEL_MOUSE_RBPRESSED, V0);
		return; 
	}

	if (eng.state == OMS::SST_INOP) return;

	if (thrAuthority)
	{
		eng.thr = GetEngineLevel(ENGINE_MAIN);

		SetThrusterLevel_SingleStep(omsGroup[0], eng.thr);
		SetThrusterLevel_SingleStep(omsGroup[1], eng.thr);
		playSound(SFX_OMSBURN, false, float(eng.thr));
	}
}