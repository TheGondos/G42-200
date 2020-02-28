#include "..\Header\G422.h"
#include "..\Header\G422_DVC.h"
#include "..\Header\G422_MDL_EXT.h"
#include "..\Header\G422_MDL_DVC.h"

inline bool MovingPart::operate(double& dt)
{
	G422::HYDRAULIC& hydSys = vsl->hydSysA.hydPrs < 2800 ? vsl->hydSysB : vsl->hydSysA;
	
	if (hydSys.hydPrs > 2800 && (mp_old_status == MP_MOVING || mp_old_status == MP_REVERSING))
	{
		mp_status = mp_old_status;
		mp_old_status = MP_INOP;
		vsl->clbkGeneric(VMSG_MPSTRT, sysID, this);
	}

	switch (mp_status)
	{
	case MP_MOVING:
		if ((!vsl->gear_mode || sysID != G422_SYSID_LGRS) && hydSys.hydPrs < 2800)
		{
			mp_old_status = MP_MOVING;
			mp_status = MP_INOP;
			vsl->clbkGeneric(VMSG_MPSTOP, sysID, this);
		}

		pos = pos + (rate * dt);

		if (pos >= 1)
		{
			pos = 1;
			mp_status = MP_HI_DETENT;
			vsl->clbkGeneric(VMSG_MPSTOP, sysID, this);
		}
		vsl->SetAnimation(anim_idx, pos);

		return true;

	case MP_REVERSING:
		if (hydSys.hydPrs < 2800)
		{
			mp_old_status = MP_REVERSING;
			mp_status = MP_INOP;
			vsl->clbkGeneric(VMSG_MPSTOP, sysID, this);
		}

		pos = pos - (rate * dt);

		if (pos <= 0)
		{
			pos = 0;
			mp_status = MP_LOW_DETENT;
			vsl->clbkGeneric(VMSG_MPSTOP, sysID, this);
		}
		vsl->SetAnimation(anim_idx, pos);

		return true;
	}
	return false;
}

void G422::simSystems(double& dT, int sysid)
{
	switch (sysid)
	{
	case G422_SYSID_HYDRAULICS:
		if (GetADCtrlMode() != 0 && hydSysA.hydPrs < 2800 && hydSysB.hydPrs < 2800)
		{
			SetADCtrlMode(0);
			PlayVesselWave(SFXID, SFX_VC_TICK);
			PlayVesselWave(SFXID, SFX_VC_AFLIK);
			VCKnobs[VC_kbIndexByMGID[MGID_KB3_ACS_MODE]].setPos(KB3_DOWN, this);
		}

		if (apuPackA.state != APU::ENG_RUN && apuPackB.state != APU::ENG_RUN && apuPackA.hydFeed == APU::HYD_PUMP)
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW3_APU_HYD] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
		break;

	case G422_SYSID_MAINPUMPS:
		if (engMain_L.feed & RT66::FUEL_PUMP && (GetPropellantMass(fuel_main_allTanks) < 0.001 || !(engMain_L.feed & RT66::FUEL_OPEN)))
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW2_MSFEED_L] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);

		if (engMain_L.feed & RT66::OXY_PUMP && (GetPropellantMass(fuel_oxy) < 0.001 || !(engMain_L.feed & RT66::OXY_PUMP)))
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW3_OXYFEED_L] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);

		if (engMain_R.feed & RT66::FUEL_PUMP && (GetPropellantMass(fuel_main_allTanks) < 0.001 || !(engMain_R.feed & RT66::FUEL_OPEN)))
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW2_MSFEED_R] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);

		if (engMain_R.feed & RT66::OXY_PUMP && (GetPropellantMass(fuel_oxy) < 0.001 || !(engMain_R.feed & RT66::OXY_PUMP)))
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW3_OXYFEED_R] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
		break;

	case G422_SYSID_FUELDUMP:
		if (fuelDump.masterDump)
		{
			if (fuelDump.dumpLeft || fuelDump.dumpFwd || fuelDump.dumpAft || fuelDump.dumpRight)
			{
				double dumpMass = GetPropellantMass(fuel_main_allTanks) - 33 * dT;

				if (dumpMass > 0) SetPropellantMass(fuel_main_allTanks, dumpMass);
				else 
				{
					SetPropellantMass(fuel_main_allTanks, 0);
					fuelDump.dumpLeft = fuelDump.dumpFwd = fuelDump.dumpAft = fuelDump.dumpRight = false; 
				}
			}

			if (fuelDump.dumpAsf)
			{
				double dumpMass = GetPropellantMass(fuel_sys) - 33 * dT;

				if (dumpMass > 0) SetPropellantMass(fuel_sys, dumpMass);
				else { SetPropellantMass(fuel_sys, 0); fuelDump.dumpAsf = false; }
			}

			if (fuelDump.dumpOxy)
			{
				double dumpMass = GetPropellantMass(fuel_oxy) - 33 * dT;

				if (dumpMass > 0) SetPropellantMass(fuel_oxy, dumpMass);
				else { SetPropellantMass(fuel_oxy, 0); fuelDump.dumpOxy = false; }
			}

			if (fuelDump.dumpLeft || fuelDump.dumpFwd || fuelDump.dumpAft || fuelDump.dumpRight || fuelDump.dumpAsf || fuelDump.dumpOxy)
				PlayVesselWave(SFXID, SFX_FUEL_FLOW);
			else fuelDump.masterDump = false;
		}
		else if (fuelDump.dumpingFuel)
		{
			StopVesselWave(SFXID, SFX_FUEL_FLOW);
			fuelDump.dumpingFuel = false;
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW2_MASTER_DUMP] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
		}
		break;

	case G422_SYSID_DCKSUPPLY:
		if (dockSupply.supplyFuel)
		{
			if (GetDockStatus(GetDockHandle(0)))
			{
				double supplyMass = GetPropellantMass(fuel_main_allTanks) + 17 * dT;

				if (supplyMass < MAXFUEL_MAIN_ALL) SetPropellantMass(fuel_main_allTanks, supplyMass);
				else
				{
					SetPropellantMass(fuel_main_allTanks, MAXFUEL_MAIN_ALL);
					clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW2_DCKSPL_FUEL] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
				}
			}  else clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW2_DCKSPL_FUEL] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
		}

		if (dockSupply.supplyOxy)
		{
			if (GetDockStatus(GetDockHandle(0)))
			{
				double supplyMass = GetPropellantMass(fuel_oxy) + 17 * dT;

				if (supplyMass < MAXFUEL_OXY) SetPropellantMass(fuel_oxy, supplyMass);
				else
				{
					SetPropellantMass(fuel_oxy, MAXFUEL_OXY);
					clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW2_DCKSPL_OXY] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
				}
			}  else clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW2_DCKSPL_OXY] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
		}

		if (dockSupply.supplyAsf)
		{
			if (GetDockStatus(GetDockHandle(0)))
			{
				double supplyMass = GetPropellantMass(fuel_sys) + 17 * dT;

				if (supplyMass < MAXFUEL_SYS) SetPropellantMass(fuel_sys, supplyMass);
				else
				{
					SetPropellantMass(fuel_sys, MAXFUEL_SYS);
					clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW2_DCKSPL_ASF] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
				}
			}  else clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[MGID_SW2_DCKSPL_ASF] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
		}

		if (dockSupply.supplyFuel || dockSupply.supplyOxy || dockSupply.supplyAsf) PlayVesselWave(SFXID, SFX_FUEL_FLOW);
		else if (dockSupply.supplyingFuel || dockSupply.supplyingOxy || dockSupply.supplyingAsf)
		{
			StopVesselWave(SFXID, SFX_FUEL_FLOW);
			dockSupply.supplyingFuel = dockSupply.supplyingOxy = dockSupply.supplyingAsf = false;
		}
		break;

	case G422_SYSID_ANTISLIP:
		if (GroundContact()) apSet = false;

		if (apSet)
		{
			if (GetADCtrlMode() == 7 && GetDynPressure() > 7e3)
			{
				double slipAngle = GetSlipAngle() * DEG;
				SetControlSurfaceLevel(AIRCTRL_RUDDER, -slipAngle * 0.25);
				rudderSet = true;
			}
			else apSet = false;
		}
		else if (rudderSet) { SetControlSurfaceLevel(AIRCTRL_RUDDER, 0); rudderSet = false; }
		break;

	case G422_SYSID_WARNING:
		if (GroundContact())
		{
			VECTOR3 acc; GetAngularAcc(acc);

			if (acc.z > 0)
			{
				double airspeed = GetAirspeed();

				if (airspeed >= 140 && airspeed < 185) { if (!warningSys.v1) { warningSys.v1 = true; PlayVesselWave(SFXID, SFX_V1); } }
				else if (airspeed >= 185) { if (!warningSys.rotate) { warningSys.rotate = true; PlayVesselWave(SFXID, SFX_ROTATE); } }
				else warningSys.v1 = warningSys.rotate = false;
			}
		}

		double mainLevel = GetPropellantMass(fuel_main_allTanks);

		if (mainLevel <= 42e3 && mainLevel > 0.001) 
		{ 
			if (!warningSys.mainLow) { warningSys.mainLow = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) PlayVesselWave(SFXID, SFX_WARN_MAINLOW);
		}
		else if (warningSys.mainLow)  { warningSys.mainLow = false; StopVesselWave(SFXID, SFX_WARN_MAINLOW); }

		if (mainLevel < 0.001) 
		{ 
			if (!warningSys.mainDep) { warningSys.mainDep = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) PlayVesselWave(SFXID, SFX_WARN_MAINDEP);
		}
		else if (warningSys.mainDep) { warningSys.mainDep = false; StopVesselWave(SFXID, SFX_WARN_MAINDEP); }

		double oxyLevel = GetPropellantMass(fuel_oxy);

		if (oxyLevel <= 16e3 && oxyLevel > 0.001) 
		{ 
			if (!warningSys.oxyLow) { warningSys.oxyLow = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) PlayVesselWave(SFXID, SFX_WARN_OXYLOW);
		}
		else if (warningSys.oxyLow) { warningSys.oxyLow = false; StopVesselWave(SFXID, SFX_WARN_OXYLOW); }

		if (oxyLevel < 0.001) 
		{ 
			if (!warningSys.oxyDep) { warningSys.oxyDep = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) PlayVesselWave(SFXID, SFX_WARN_OXYDEP);
		}
		else if (warningSys.oxyDep) { warningSys.oxyDep = false; StopVesselWave(SFXID, SFX_WARN_OXYDEP); }

		double asfLevel = GetPropellantMass(fuel_sys);

		if (asfLevel <= 3e3 && asfLevel > 0.001) 
		{ 
			if (!warningSys.asfLow) { warningSys.asfLow = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) PlayVesselWave(SFXID, SFX_WARN_ASFLOW);
		}
		else if (warningSys.asfLow) { warningSys.asfLow = false; StopVesselWave(SFXID, SFX_WARN_ASFLOW); }

		if (asfLevel < 0.001) 
		{ 
			if (!warningSys.asfDep) { warningSys.asfDep = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) PlayVesselWave(SFXID, SFX_WARN_ASFDEP);
		}
		else if (warningSys.asfDep) { warningSys.asfDep = false; StopVesselWave(SFXID, SFX_WARN_ASFDEP); }

		if (GetADCtrlMode() == 0 && GetDynPressure() > 5e3) 
		{
			if (!warningSys.controlOff) { warningSys.controlOff = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) PlayVesselWave(SFXID, SFX_WARN_CTRLOFF);
		}
		else if (warningSys.controlOff) { warningSys.controlOff = false; StopVesselWave(SFXID, SFX_WARN_CTRLOFF); }

		break;
	}
}

VECTOR3 vF, vW, vA; // define some static stuff for use below...

void G422::clbkPreStep(double simt, double dT, double mjd)
{
	if (timer < 5) timer += dT;

	// get flight envelope coords for engine reference lookup table
	thrFX_X = GetMachNumber() * 0.05;
	thrFX_Y = 1 - (GetAltitude() * 12.5e-6);

	simSystems(dT, G422_SYSID_HYDRAULICS);
	simSystems(dT, G422_SYSID_MAINPUMPS);
	simSystems(dT, G422_SYSID_FUELDUMP);
	simSystems(dT, G422_SYSID_DCKSUPPLY);
	simSystems(dT, G422_SYSID_ANTISLIP);
	simSystems(dT, G422_SYSID_WARNING);

	simEngines(dT, apuPackA);
	simEngines(dT, apuPackB);

	simEngines(dT, engMain_L);

	simEngines(dT, engMain_R);

	simEngines(dT, engRamx);

	simEngines(dT, rcs);
	simEngines(dT, oms);

	if (engMain_L.state == RT66::SST_RUN_EXT)
	{
		// not quite sure how or why this works.... got to it by tweaking around with plot-o-matic...
		double alt = GetAltitude() * .00001;
		fx_contrailLvl = max(0, ((alt - 0.028) * alt) * ((0.18 - alt) * 300.0) * engMain_L.epr);
		fx_exhaustLvl = engMain_L.epr;
	}
	else { fx_contrailLvl = 0; fx_exhaustLvl = 0; }

	SetAnimation(an_dvc_thr, GetEngineLevel(ENGINE_MAIN));
	SetAnimation(an_dvc_ramx, GetEngineLevel(ENGINE_HOVER));

	SetAnimation(an_dvc_csPitch, (GetControlSurfaceLevel(AIRCTRL_ELEVATOR) * -0.5) + .5);
	SetAnimation(an_dvc_csRoll, (GetControlSurfaceLevel(AIRCTRL_AILERON) * .5) + .5);

	SetAnimation(an_acs_cnrd, (canards->pos == 1) ? ((GetControlSurfaceLevel(AIRCTRL_ELEVATOR) * -0.5) + .5) : .5);

	SetAnimation(an_acs_elvr, (GetControlSurfaceLevel(AIRCTRL_ELEVATOR) * -0.5) + .5);

	SetAnimation(an_acs_rddr, (GetControlSurfaceLevel(AIRCTRL_RUDDER) * -0.5) + .5);

	SetAnimation(an_acs_alrn, (GetControlSurfaceLevel(AIRCTRL_AILERON) * -0.5) + .5);

	if (visor->pos < 1) // visor open (down)
	{
		double dprs = GetDynPressure();

		if (dprs > 0.1) PlayVesselWave(SFXID, SFX_WIND_OPEN, LOOP, min(240, int(dprs * 11.0e-3)));
	}

	// operate moving parts
	landingGears->operate(dT);
	visor->operate(dT);
	canards->operate(dT);
	inltDoors->operate(dT);
	ramxDoors->operate(dT);
	bayDoors->operate(dT);
	rcsDoors->operate(dT);

	// wings can only pivot one way if the other is at low-detent
	if (wingTipFthr->pos <= 0) wingTipWvrd->operate(dT);
	if (wingTipWvrd->pos <= 0) wingTipFthr->operate(dT);

	// light positions must update accordingly as well...
	if (wingTipFthr->pos > 0 && wingTipFthr->pos < 1 // when necessary...
		|| wingTipWvrd->pos > 0 && wingTipWvrd->pos < 1)
	{
		*nav_red.pos = wingTipLightsL[0];
		*stb_l.pos = wingTipLightsL[1];
		*nav_green.pos = wingTipLightsR[0];
		*stb_r.pos = wingTipLightsR[1];
	}

	SetAnimation(an_dvc_adi_A, 0.5 + (GetBank() * SCALAR_ANGLE));
	SetAnimation(an_dvc_adi_B, 0.5 + (GetSlipAngle() * SCALAR_ANGLE));
	SetAnimation(an_dvc_adi_C, 0.5 + (GetPitch() * SCALAR_ANGLE));

	if (GroundContact())
	{
		double steer = GetControlSurfaceLevel(AIRCTRL_RUDDER);
		double speed = GetGroundspeed();

		SetAnimation(an_dvc_tiller, 0.5 - (steer * 0.5));

		if (speed < 250) AddForce({ ((-speed + 250) / 250) * steer * 3e5, 0, 0 }, TOUCHDOWN_GEARDOWN[0].pos);

		if (prk_brake_mode)
		{
			if (speed > 0.2) SetWheelbrakeLevel(1, 0, false);
			else if (!(GetFlightStatus() & 1)) setStatusLanded();
		}
	} 
}

int G422::clbkGeneric(int msgid = 0, int prm = 0, void* context = NULL)
{
	switch (msgid)
	{
	case VMSG_ENGRST: // engines reset
		SetThrusterLevel(ramcaster, 0);
		SetThrusterLevel(oms_thgr[0], 0);
		SetThrusterLevel(oms_thgr[1], 0);

		SetThrusterLevel(RT66_gasGen_thgr[0], 0);
		SetThrusterLevel(RT66_gasGen_thgr[1], 0);

		SetThrusterLevel(RT66_burner_thgr[0], 0);
		SetThrusterLevel(RT66_burner_thgr[1], 0);

		SetThrusterLevel(RT66_rocket_thgr[0], 0);
		SetThrusterLevel(RT66_rocket_thgr[1], 0);
		return 0;

	case VMSG_MPSTRT:  // moving part started moving
		switch (prm)
		{
		case G422_SYSID_LGRS:
			if (landingGears->pos == 1)
			{
				SetTouchdownPoints(TOUCHDOWN_GEARUP, TOUCHDOWN_COUNT);
				SetMaxWheelbrakeForce(0);
			}

			if(!gear_mode) PlayVesselWave(SFXID, SFX_GEARS);
			landingGears->sysReset = true;
			break;

		case G422_SYSID_CNRD:
			if (canards->pos == 1) // canards retracted or retracting...
				DelControlSurface(acsCndrs);

			PlayVesselWave(SFXID, SFX_CNRDS);
			canards->sysReset = true;
			break;

		case G422_SYSID_VISR:
			if (visor->pos == 1) PlayVesselWave(SFXID, SFX_VSRDN);
			else                 PlayVesselWave(SFXID, SFX_VSRUP);
			break;

		case G422_SYSID_BAY:
			PlayVesselWave(SFXID, SFX_BAY_DOORS);
			if (bayDoors->pos == 1) ucso->SetSlotDoor(false);
			break;

		case G422_SYSID_RCSDR:
			PlayVesselWave(SFXID, SFX_RCS_DOORS);
			if (rcsDoors->pos == 1 && rcs.state == RCS::SST_STBY) cueEngines(rcs, RCS::SST_STBY);
			break;

		default:
			break;
		}
		return 0;

	case VMSG_MPSTOP:  // moving part stopped moving
		switch (prm)
		{
		case G422_SYSID_LGRS:
			if (landingGears->pos == 1) // gears are down and locked
			{
				SetTouchdownPoints(TOUCHDOWN_GEARDOWN, TOUCHDOWN_COUNT);
				SetMaxWheelbrakeForce(8e5);

				setSwMid(MGID_SW3_GEAR);
			} 
			else if(landingGears->pos == 0) setSwMid(MGID_SW3_GEAR);

			landingGears->sysReset = true;
			if(!gear_mode) StopVesselWave(SFXID, SFX_GEARS);
			break;

		case G422_SYSID_CNRD:
			if (canards->pos == 1) // canards are deployed!
			{
				acsCndrs = CreateControlSurface3(AIRCTRL_ELEVATOR, 5.0, 2.0, _V(0, 2.0, 35.0), AIRCTRL_AXIS_XNEG);
				setSwMid(MGID_SW3_CANARD);
			} 
			else if (canards->pos == 0) setSwMid(MGID_SW3_CANARD);

			canards->sysReset = true;
			StopVesselWave(SFXID, SFX_CNRDS);
			break;


		case G422_SYSID_VISR:
			StopVesselWave(SFXID, SFX_VSRDN);
			StopVesselWave(SFXID, SFX_VSRUP);

			if (visor->pos == 1) { StopVesselWave(SFXID, SFX_WIND_OPEN); setSwMid(MGID_SW3_VISOR); } 
			else if (visor->pos == 0) setSwMid(MGID_SW3_VISOR);
			break;

		case G422_SYSID_RAMX:
			setSwMid(MGID_SW3_RAMX_DOOR);

			// hover engines are assigned to ramcasters upon doors open - this prevents it being "used" by MFD's as if it really was a "hover" engine
			if (ramxDoors->getToggleState()) cueEngines(engRamx, RAMCASTER::ENG_STOP);
			else cueEngines(engRamx, RAMCASTER::ENG_INOP);
			break;

		case G422_SYSID_RCSDR:
			StopVesselWave(SFXID, SFX_RCS_DOORS);

			if (rcsDoors->pos == 1) 
			{ 
				if (rcs.state == RCS::SST_STBY) cueEngines(rcs, RCS::SST_STBY);
				setSwMid(MGID_SW3_RCS);
			} 
			else if (rcsDoors->pos == 0) setSwMid(MGID_SW3_RCS);
			break;

		case G422_SYSID_BAY:
			StopVesselWave(SFXID, SFX_BAY_DOORS);

			if (bayDoors->pos == 1) { ucso->SetSlotDoor(true); setSwMid(MGID_SW3_BAY_OPENCLSE); }
			else if (bayDoors->pos == 0) setSwMid(MGID_SW3_BAY_OPENCLSE);
			break;

		default:
			break;
		}

		return 0;

	default:
		return 0;
	}
}

void G422::setStatusLanded()
{
	VESSELSTATUS2 status;
	memset(&status, 0, sizeof(status));
	status.version = 2;
	GetStatusEx(&status);
	status.status = 1;

	ucso->SetGroundRotation(status, landingGears->pos == 1 ? 5.26 : 4.10324);

	DefSetStateEx(&status);
}