#include "..\Header\G422.h"
#include "..\Header\G422_DVC.h"
#include "..\Header\G422_MDL_DVC.h"

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
			CreateThrusterGroup(th_rcs_pitchUp, 2, THGROUP_ATT_PITCHUP);
			CreateThrusterGroup(th_rcs_pitchDn, 2, THGROUP_ATT_PITCHDOWN);
			CreateThrusterGroup(th_rcs_vertUp, 2, THGROUP_ATT_UP);
			CreateThrusterGroup(th_rcs_vertDown, 2, THGROUP_ATT_DOWN);

			SetThrusterMax0(th_rcs_pitchUp[0], MAXTRHUST_RCS);
			SetThrusterMax0(th_rcs_pitchUp[1], MAXTRHUST_RCS);

			SetThrusterMax0(th_rcs_pitchDn[0], MAXTRHUST_RCS);
			SetThrusterMax0(th_rcs_pitchDn[1], MAXTRHUST_RCS);

			SetThrusterMax0(th_rcs_vertUp[0], MAXTRHUST_RCS);
			SetThrusterMax0(th_rcs_vertUp[1], MAXTRHUST_RCS);

			SetThrusterMax0(th_rcs_vertDown[0], MAXTRHUST_RCS);
			SetThrusterMax0(th_rcs_vertDown[1], MAXTRHUST_RCS);

			CreateThrusterGroup(th_rcs_yawLeft, 2, THGROUP_ATT_YAWLEFT);
			CreateThrusterGroup(th_rcs_yawRight, 2, THGROUP_ATT_YAWRIGHT);
			CreateThrusterGroup(th_rcs_horLeft, 2, THGROUP_ATT_LEFT);
			CreateThrusterGroup(th_rcs_horRight, 2, THGROUP_ATT_RIGHT);

			SetThrusterMax0(th_rcs_yawLeft[0], MAXTRHUST_RCS * 0.75);
			SetThrusterMax0(th_rcs_yawLeft[1], MAXTRHUST_RCS * 0.75);

			SetThrusterMax0(th_rcs_yawRight[0], MAXTRHUST_RCS * 0.75);
			SetThrusterMax0(th_rcs_yawRight[1], MAXTRHUST_RCS * 0.75);

			SetThrusterMax0(th_rcs_horLeft[0], MAXTRHUST_RCS * 0.75);
			SetThrusterMax0(th_rcs_horLeft[1], MAXTRHUST_RCS * 0.75);

			SetThrusterMax0(th_rcs_horRight[0], MAXTRHUST_RCS * 0.75);
			SetThrusterMax0(th_rcs_horRight[1], MAXTRHUST_RCS * 0.75);

			CreateThrusterGroup(th_rcs_fwd, 1, THGROUP_ATT_FORWARD);
			SetThrusterMax0(th_rcs_fwd[0], MAXTRHUST_RCS * 2.5);

			if (rcsDoors->getToggleState()) 
			{
				CreateThrusterGroup(th_rcs_back, 1, THGROUP_ATT_BACK);
				SetThrusterMax0(th_rcs_back[0], MAXTRHUST_RCS * 2.5); 
			}

			CreateThrusterGroup(th_rcs_bankRight, 2, THGROUP_ATT_BANKRIGHT);
			CreateThrusterGroup(th_rcs_bankLeft, 2, THGROUP_ATT_BANKLEFT);

			SetThrusterMax0(th_rcs_bankRight[0], MAXTRHUST_RCS * 2);
			SetThrusterMax0(th_rcs_bankRight[1], MAXTRHUST_RCS * 2);

			SetThrusterMax0(th_rcs_bankLeft[0], MAXTRHUST_RCS * 2);
			SetThrusterMax0(th_rcs_bankLeft[1], MAXTRHUST_RCS * 2);
		}
		else
		{
			CreateThrusterGroup(th_rcs_pitchUp, 2, THGROUP_ATT_PITCHUP);
			CreateThrusterGroup(th_rcs_pitchDn, 2, THGROUP_ATT_PITCHDOWN);
			CreateThrusterGroup(th_rcs_vertUp, 2, THGROUP_ATT_FORWARD);
			CreateThrusterGroup(th_rcs_vertDown, 2, THGROUP_ATT_BACK);

			SetThrusterMax0(th_rcs_pitchUp[0], MAXTRHUST_RCS * 0.4);
			SetThrusterMax0(th_rcs_pitchUp[1], MAXTRHUST_RCS * 0.4);

			SetThrusterMax0(th_rcs_pitchDn[0], MAXTRHUST_RCS * 0.4);
			SetThrusterMax0(th_rcs_pitchDn[1], MAXTRHUST_RCS * 0.4);

			SetThrusterMax0(th_rcs_vertUp[0], MAXTRHUST_RCS);
			SetThrusterMax0(th_rcs_vertUp[1], MAXTRHUST_RCS);

			SetThrusterMax0(th_rcs_vertDown[0], MAXTRHUST_RCS);
			SetThrusterMax0(th_rcs_vertDown[1], MAXTRHUST_RCS);

			CreateThrusterGroup(th_rcs_yawLeft, 2, THGROUP_ATT_BANKRIGHT);
			CreateThrusterGroup(th_rcs_yawRight, 2, THGROUP_ATT_BANKLEFT);
			CreateThrusterGroup(th_rcs_horLeft, 2, THGROUP_ATT_LEFT);
			CreateThrusterGroup(th_rcs_horRight, 2, THGROUP_ATT_RIGHT);

			SetThrusterMax0(th_rcs_yawLeft[0], MAXTRHUST_RCS * 0.8);
			SetThrusterMax0(th_rcs_yawLeft[1], MAXTRHUST_RCS * 0.8);

			SetThrusterMax0(th_rcs_yawRight[0], MAXTRHUST_RCS * 0.8);
			SetThrusterMax0(th_rcs_yawRight[1], MAXTRHUST_RCS * 0.8);

			SetThrusterMax0(th_rcs_horLeft[0], MAXTRHUST_RCS * 0.3);
			SetThrusterMax0(th_rcs_horLeft[1], MAXTRHUST_RCS * 0.3);

			SetThrusterMax0(th_rcs_horRight[0], MAXTRHUST_RCS * 0.3);
			SetThrusterMax0(th_rcs_horRight[1], MAXTRHUST_RCS * 0.3);

			CreateThrusterGroup(th_rcs_fwd, 1, THGROUP_ATT_DOWN);
			SetThrusterMax0(th_rcs_fwd[0], MAXTRHUST_RCS * 0.4);

			if (rcsDoors->getToggleState())
			{
				CreateThrusterGroup(th_rcs_back, 1, THGROUP_ATT_UP);
				SetThrusterMax0(th_rcs_back[0], MAXTRHUST_RCS * 0.4);
			}

			CreateThrusterGroup(th_rcs_bankRight, 2, THGROUP_ATT_YAWRIGHT);
			CreateThrusterGroup(th_rcs_bankLeft, 2, THGROUP_ATT_YAWLEFT);

			SetThrusterMax0(th_rcs_bankRight[0], MAXTRHUST_RCS * 0.3);
			SetThrusterMax0(th_rcs_bankRight[1], MAXTRHUST_RCS * 0.3);

			SetThrusterMax0(th_rcs_bankLeft[0], MAXTRHUST_RCS * 0.3);
			SetThrusterMax0(th_rcs_bankLeft[1], MAXTRHUST_RCS * 0.3);
		}
		break;
	case RCS::SST_INOP: break;
	}

	eng.state = sst;
}

void G422::simEngines(double& dT, RCS& eng)
{
	double fuelMass = GetPropellantMass(fuel_sys);

	if (eng.feed & RCS::FUEL_PUMP && (fuelMass < 0.001 || !(eng.feed & RCS::FUEL_OPEN)))
		clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW2_SYSFEED_RCS] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);

	if (eng.state == RCS::SST_INOP) return;

	if (eng.feed & RCS::FUEL_PUMP)
	{
		if (eng.fuelPrs < 1)
		{
			double deltaPrs = 0.22 * dT;
			eng.fuelPrs = min(eng.fuelPrs + deltaPrs, 1);
		}
	}
	else
	{
		if (eng.fuelPrs > 0)
		{
			double deltaPrs = -0.12 * dT;
			eng.fuelPrs = max(eng.fuelPrs + deltaPrs, 0);
		}
	}

	if (fuelMass < 0.001) eng.fuelPrs = 0;

	if (eng.fuelPrs == 0)
		clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW3_STBYIGN_RCS] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
}