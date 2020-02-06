#include "..\Header\G422.h"
#include "..\Header\G422_MDL_EXT.h"

void G422::cueSystems(int sysID, int cue, int op, void* ctx)
{
	switch (sysID)
	{
	case G422_SYSID_RCOMS:
		switch (cue)
		{
		case RCOMS::CUE_OMS_STBY:
			if (op)
			{
				rcoms.state |= RCOMS::SYS_OMSTBY;
				if(rcoms.state & RCOMS::SYS_INOP) rcoms.state &= ~RCOMS::SYS_INOP;
			}
			else
			{
				rcoms.state &= ~RCOMS::SYS_OMSTBY;
				if (!(rcoms.state & RCOMS::SYS_RCSTBY)) rcoms.state |= RCOMS::SYS_INOP;
			}
				
			return;

		case RCOMS::CUE_RCS_STBY:
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

			if (op)
			{
				rcoms.state |= RCOMS::SYS_RCSTBY;
				if (rcoms.state & RCOMS::SYS_INOP) rcoms.state &= ~RCOMS::SYS_INOP;

				if (!rcsMode)
				{
					PlayVesselWave(SFXID, SFX_RCS_NORMAL);

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
					CreateThrusterGroup(th_rcs_back, 1, THGROUP_ATT_BACK);

					SetThrusterMax0(th_rcs_fwd[0], MAXTRHUST_RCS * 2.5);
					SetThrusterMax0(th_rcs_back[0], MAXTRHUST_RCS * 2.5);
					
					CreateThrusterGroup(th_rcs_bankRight, 2, THGROUP_ATT_BANKRIGHT);
					CreateThrusterGroup(th_rcs_bankLeft, 2, THGROUP_ATT_BANKLEFT);

					SetThrusterMax0(th_rcs_bankRight[0], MAXTRHUST_RCS * 2);
					SetThrusterMax0(th_rcs_bankRight[1], MAXTRHUST_RCS * 2);

					SetThrusterMax0(th_rcs_bankLeft[0], MAXTRHUST_RCS * 2);
					SetThrusterMax0(th_rcs_bankLeft[1], MAXTRHUST_RCS * 2);
				}
				else 
				{
					PlayVesselWave(SFXID, SFX_RCS_DOCKING);

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
					CreateThrusterGroup(th_rcs_back, 1, THGROUP_ATT_UP);

					SetThrusterMax0(th_rcs_fwd[0], MAXTRHUST_RCS * 0.4);
					SetThrusterMax0(th_rcs_back[0], MAXTRHUST_RCS * 0.4);
					
					CreateThrusterGroup(th_rcs_bankRight, 2, THGROUP_ATT_YAWRIGHT);
					CreateThrusterGroup(th_rcs_bankLeft, 2, THGROUP_ATT_YAWLEFT);

					SetThrusterMax0(th_rcs_bankRight[0], MAXTRHUST_RCS * 0.3);
					SetThrusterMax0(th_rcs_bankRight[1], MAXTRHUST_RCS * 0.3);

					SetThrusterMax0(th_rcs_bankLeft[0], MAXTRHUST_RCS * 0.3);
					SetThrusterMax0(th_rcs_bankLeft[1], MAXTRHUST_RCS * 0.3);
				}
			}
			else 
			{
				rcoms.state &= ~RCOMS::SYS_RCSTBY;
				if (!(rcoms.state & RCOMS::SYS_OMSTBY)) rcoms.state |= RCOMS::SYS_INOP;
			}
			return;
		}

	default:
		return;
	}
}

void G422::simEngines(double& dT, RCOMS& eng)
{
	if (eng.state & RCOMS::SYS_INOP) return;

	if (eng.feed & RCOMS::FUEL_RC_PUMP)
	{
		if (eng.fuelPrsRcs < 1.0)
		{
			double deltaPrs = .22 * dT;
			eng.fuelPrsRcs = min(eng.fuelPrsRcs + deltaPrs, 1.0);
		}
	}
	else
	{
		if (eng.fuelPrsRcs > 0.0)
		{
			double deltaPrs = -.12 * dT;
			eng.fuelPrsRcs = max(eng.fuelPrsRcs + deltaPrs, 0.0);
		}
	}

	if (eng.feed & RCOMS::FUEL_OM_PUMP)
	{
		if (eng.fuelPrsOms < 1.0)
		{
			double deltaPrs = .22 * dT;
			eng.fuelPrsOms = min(eng.fuelPrsOms + deltaPrs, 1.0);
		}
	}
	else
	{
		if (eng.fuelPrsOms > 0.0)
		{
			double deltaPrs = -.12 * dT;
			eng.fuelPrsOms = max(eng.fuelPrsOms + deltaPrs, 0.0);
		}
	}

	if (GetPropellantMass(fuel_sys) < .001) eng.fuelPrsRcs = eng.fuelPrsOms = 0;

	if (eng.fuelPrsRcs == 0) cueSystems(G422_SYSID_RCOMS, RCOMS::CUE_RCS_STBY, false);

	if (eng.fuelPrsOms == 0) return cueSystems(G422_SYSID_RCOMS, RCOMS::CUE_OMS_STBY, false);

	if (eng.state & RCOMS::SYS_OMSTBY && thr_authority)
	{
		eng.omsThr = GetEngineLevel(ENGINE_MAIN);

		SetThrusterLevel_SingleStep(oms_thgr[0], eng.omsThr);
		SetThrusterLevel_SingleStep(oms_thgr[1], eng.omsThr);
		PlayVesselWave(SFXID, SFX_OMSBURN, NOLOOP, int(225 * eng.omsThr));
	}
}