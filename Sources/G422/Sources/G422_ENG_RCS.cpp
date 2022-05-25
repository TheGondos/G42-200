#include "../Headers/G422.h"
#include "../Headers/G422_DVC.h"
#include "../Headers/G422_MDL_DVC.h"

void G422::cueEngines(RCS& eng, RCS::SIMSTATE sst)
{
	DelThrusterGroup(THGROUP_ATT_PITCHUP);
	DelThrusterGroup(THGROUP_ATT_PITCHDOWN);
	DelThrusterGroup(THGROUP_ATT_UP);
	DelThrusterGroup(THGROUP_ATT_DOWN);

	DelThrusterGroup(THGROUP_ATT_YAWLEFT);
	DelThrusterGroup(THGROUP_ATT_YAWRIGHT);
	DelThrusterGroup(THGROUP_ATT_LEFT);
	DelThrusterGroup(THGROUP_ATT_RIGHT);

	DelThrusterGroup(THGROUP_ATT_FORWARD);
	DelThrusterGroup(THGROUP_ATT_BACK);

	DelThrusterGroup(THGROUP_ATT_BANKLEFT);
	DelThrusterGroup(THGROUP_ATT_BANKRIGHT);

	switch (sst)
	{
	case RCS::SST_STBY:
		if (!eng.mode)
		{
			CreateThrusterGroup(rcsPitchUp, 2, THGROUP_ATT_PITCHUP);
			CreateThrusterGroup(rcsPitchDown, 2, THGROUP_ATT_PITCHDOWN);
			CreateThrusterGroup(rcsTransUp, 2, THGROUP_ATT_UP);
			CreateThrusterGroup(rcsTransDown, 2, THGROUP_ATT_DOWN);

			SetThrusterMax0(rcsPitchUp[0], MAXTRHUST_RCS);
			SetThrusterMax0(rcsPitchUp[1], MAXTRHUST_RCS);

			SetThrusterMax0(rcsPitchDown[0], MAXTRHUST_RCS);
			SetThrusterMax0(rcsPitchDown[1], MAXTRHUST_RCS);

			SetThrusterMax0(rcsTransUp[0], MAXTRHUST_RCS);
			SetThrusterMax0(rcsTransUp[1], MAXTRHUST_RCS);

			SetThrusterMax0(rcsTransDown[0], MAXTRHUST_RCS);
			SetThrusterMax0(rcsTransDown[1], MAXTRHUST_RCS);

			CreateThrusterGroup(rcsYawLeft, 2, THGROUP_ATT_YAWLEFT);
			CreateThrusterGroup(rcsYawRight, 2, THGROUP_ATT_YAWRIGHT);
			CreateThrusterGroup(rcsTransLeft, 2, THGROUP_ATT_LEFT);
			CreateThrusterGroup(rcsTransRight, 2, THGROUP_ATT_RIGHT);

			SetThrusterMax0(rcsYawLeft[0], MAXTRHUST_RCS * 0.75);
			SetThrusterMax0(rcsYawLeft[1], MAXTRHUST_RCS * 0.75);

			SetThrusterMax0(rcsYawRight[0], MAXTRHUST_RCS * 0.75);
			SetThrusterMax0(rcsYawRight[1], MAXTRHUST_RCS * 0.75);

			SetThrusterMax0(rcsTransLeft[0], MAXTRHUST_RCS * 0.75);
			SetThrusterMax0(rcsTransLeft[1], MAXTRHUST_RCS * 0.75);

			SetThrusterMax0(rcsTransRight[0], MAXTRHUST_RCS * 0.75);
			SetThrusterMax0(rcsTransRight[1], MAXTRHUST_RCS * 0.75);

			CreateThrusterGroup(rcsForward, 1, THGROUP_ATT_FORWARD);
			SetThrusterMax0(rcsForward[0], MAXTRHUST_RCS * 2.5);

			if (rcsDoors->getToggleState()) 
			{
				CreateThrusterGroup(rcsBackward, 1, THGROUP_ATT_BACK);
				SetThrusterMax0(rcsBackward[0], MAXTRHUST_RCS * 2.5); 
			}

			CreateThrusterGroup(rcsBankRight, 2, THGROUP_ATT_BANKRIGHT);
			CreateThrusterGroup(rcsBankLeft, 2, THGROUP_ATT_BANKLEFT);

			SetThrusterMax0(rcsBankRight[0], MAXTRHUST_RCS * 2);
			SetThrusterMax0(rcsBankRight[1], MAXTRHUST_RCS * 2);

			SetThrusterMax0(rcsBankLeft[0], MAXTRHUST_RCS * 2);
			SetThrusterMax0(rcsBankLeft[1], MAXTRHUST_RCS * 2);
		}
		else
		{
			CreateThrusterGroup(rcsPitchUp, 2, THGROUP_ATT_PITCHUP);
			CreateThrusterGroup(rcsPitchDown, 2, THGROUP_ATT_PITCHDOWN);
			CreateThrusterGroup(rcsTransUp, 2, THGROUP_ATT_FORWARD);
			CreateThrusterGroup(rcsTransDown, 2, THGROUP_ATT_BACK);

			SetThrusterMax0(rcsPitchUp[0], MAXTRHUST_RCS * 0.4);
			SetThrusterMax0(rcsPitchUp[1], MAXTRHUST_RCS * 0.4);

			SetThrusterMax0(rcsPitchDown[0], MAXTRHUST_RCS * 0.4);
			SetThrusterMax0(rcsPitchDown[1], MAXTRHUST_RCS * 0.4);

			SetThrusterMax0(rcsTransUp[0], MAXTRHUST_RCS);
			SetThrusterMax0(rcsTransUp[1], MAXTRHUST_RCS);

			SetThrusterMax0(rcsTransDown[0], MAXTRHUST_RCS);
			SetThrusterMax0(rcsTransDown[1], MAXTRHUST_RCS);

			CreateThrusterGroup(rcsYawLeft, 2, THGROUP_ATT_BANKRIGHT);
			CreateThrusterGroup(rcsYawRight, 2, THGROUP_ATT_BANKLEFT);
			CreateThrusterGroup(rcsTransLeft, 2, THGROUP_ATT_LEFT);
			CreateThrusterGroup(rcsTransRight, 2, THGROUP_ATT_RIGHT);

			SetThrusterMax0(rcsYawLeft[0], MAXTRHUST_RCS * 0.8);
			SetThrusterMax0(rcsYawLeft[1], MAXTRHUST_RCS * 0.8);

			SetThrusterMax0(rcsYawRight[0], MAXTRHUST_RCS * 0.8);
			SetThrusterMax0(rcsYawRight[1], MAXTRHUST_RCS * 0.8);

			SetThrusterMax0(rcsTransLeft[0], MAXTRHUST_RCS * 0.3);
			SetThrusterMax0(rcsTransLeft[1], MAXTRHUST_RCS * 0.3);

			SetThrusterMax0(rcsTransRight[0], MAXTRHUST_RCS * 0.3);
			SetThrusterMax0(rcsTransRight[1], MAXTRHUST_RCS * 0.3);

			CreateThrusterGroup(rcsForward, 1, THGROUP_ATT_DOWN);
			SetThrusterMax0(rcsForward[0], MAXTRHUST_RCS * 0.4);

			if (rcsDoors->getToggleState())
			{
				CreateThrusterGroup(rcsBackward, 1, THGROUP_ATT_UP);
				SetThrusterMax0(rcsBackward[0], MAXTRHUST_RCS * 0.4);
			}

			CreateThrusterGroup(rcsBankRight, 2, THGROUP_ATT_YAWRIGHT);
			CreateThrusterGroup(rcsBankLeft, 2, THGROUP_ATT_YAWLEFT);

			SetThrusterMax0(rcsBankRight[0], MAXTRHUST_RCS * 0.3);
			SetThrusterMax0(rcsBankRight[1], MAXTRHUST_RCS * 0.3);

			SetThrusterMax0(rcsBankLeft[0], MAXTRHUST_RCS * 0.3);
			SetThrusterMax0(rcsBankLeft[1], MAXTRHUST_RCS * 0.3);
		}
		break;
	case RCS::SST_INOP: break;
	}

	eng.state = sst;
}

void G422::simEngines(double& dT, RCS& eng)
{
	double fuelMass = GetPropellantMass(asfFuel);
	VECTOR3 V0 = _V0;

	if (eng.feed & RCS::FUEL_PUMP && (fuelMass < 0.001 || !(eng.feed & RCS::FUEL_OPEN)))
		clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW2_SYSFEED_RCS] & 0xFFFF), PANEL_MOUSE_RBPRESSED, V0);

	if (eng.feed & RCS::FUEL_PUMP)
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
		clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_STBYIGN_RCS] & 0xFFFF), PANEL_MOUSE_RBPRESSED, V0);
}