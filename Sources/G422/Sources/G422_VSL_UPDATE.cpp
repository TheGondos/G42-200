#include "..\Headers\G422.h"
#include "..\Headers\G422_DVC.h"
#include "..\Headers\G422_MDL_EXT.h"
#include "..\Headers\G422_MDL_DVC.h"

void MovingPart::operate(double& dt)
{
	G422::HYDRAULIC& hydSys = vessel->hydSysA.hydPrs < 2800 ? vessel->hydSysB : vessel->hydSysA;
	
	if (hydSys.hydPrs > 2800 && (mpOldStatus == MP_MOVING || mpOldStatus == MP_REVERSING))
	{
		mpStatus = mpOldStatus;
		mpOldStatus = MP_INOP;
		vessel->clbkGeneric(VMSG_MPSTRT, sysID, this);
	}

	switch (mpStatus)
	{
	case MP_MOVING:
		if ((!vessel->gearMode || sysID != G422::SYSID_LGRS) && hydSys.hydPrs < 2800)
		{
			if (pos > 0) { mpOldStatus = MP_MOVING; mpStatus = MP_INOP; }
			vessel->clbkGeneric(VMSG_MPSTOP, sysID, this);
			vessel->playSound(G422::SFX_WARN_APUOFF);
			break;
		}

		vessel->playSound(soundID);

		pos = pos + (rate * dt);

		if (pos >= 1)
		{
			pos = 1;
			vessel->stopSound(soundID);

			mpStatus = MP_HI_DETENT;
			vessel->clbkGeneric(VMSG_MPSTOP, sysID, this);
		}
		vessel->SetAnimation(animIndex, pos);

		break;

	case MP_REVERSING:
		if (hydSys.hydPrs < 2800)
		{
			if (pos < 1) { mpOldStatus = MP_REVERSING; mpStatus = MP_INOP; }
			vessel->clbkGeneric(VMSG_MPSTOP, sysID, this);
			break;
		}

		float volume = float((sysID == G422::SYSID_LGRS && vessel->gearMode) ? 0.125 : 1);

		vessel->playSound(soundID, volume);

		pos = pos - (rate * dt);

		if (pos <= 0)
		{
			pos = 0;
			vessel->stopSound(soundID);

			mpStatus = MP_LOW_DETENT;
			vessel->clbkGeneric(VMSG_MPSTOP, sysID, this);
		}
		vessel->SetAnimation(animIndex, pos);

		break;
	}
}

void G422::simSystems(double& dT, int sysid)
{
	switch (sysid)
	{
	case G422::SYSID_HYDS:
		if (hydSysA.hydPrs < 2800 && hydSysB.hydPrs < 2800)
		{
			if (GetADCtrlMode() != 0)
			{
				SetADCtrlMode(0);
				playSound(SFX_VC_TICK);
				playSound(SFX_VC_AFLIK);
				vcKnobs[vcKnobIndexByMGID[MGID_KB3_ACS_MODE]].setPos(VCKnob::KB3_DOWN, this);
				playSound(SFX_WARN_APUOFF);
			}

			if (!brakesReset) { SetMaxWheelbrakeForce(0); brakesReset = true; }
		} 
		else if (brakesReset) { SetMaxWheelbrakeForce(8e5); brakesReset = false; }

		if (apuPackA.state != APU::ENG_RUN && apuPackB.state != APU::ENG_RUN && apuPackA.hydFeed == APU::HYD_PUMP)
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_APU_HYD] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
		break;

	case G422::SYSID_MAINPMP:
		if (engMainL.feed & RT66::FUEL_PUMP && (GetPropellantMass(allMainFuel) < 0.001 || !(engMainL.feed & RT66::FUEL_OPEN)))
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW2_MSFEED_L] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);

		if (engMainL.feed & RT66::OXY_PUMP && (GetPropellantMass(oxidizerFuel) < 0.001 || !(engMainL.feed & RT66::OXY_OPEN)))
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_OXYFEED_L] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);

		if (engMainR.feed & RT66::FUEL_PUMP && (GetPropellantMass(allMainFuel) < 0.001 || !(engMainR.feed & RT66::FUEL_OPEN)))
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW2_MSFEED_R] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);

		if (engMainR.feed & RT66::OXY_PUMP && (GetPropellantMass(oxidizerFuel) < 0.001 || !(engMainR.feed & RT66::OXY_OPEN)))
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_OXYFEED_R] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
		break;

	case G422::SYSID_FUELDMP:
		if (fuelDump.masterDump)
		{
			if (fuelDump.dumpLeft || fuelDump.dumpFwd || fuelDump.dumpAft || fuelDump.dumpRight)
			{
				double dumpMass = GetPropellantMass(allMainFuel) - 33 * dT;

				if (dumpMass > 0) SetPropellantMass(allMainFuel, dumpMass);
				else 
				{
					SetPropellantMass(allMainFuel, 0);
					fuelDump.dumpLeft = fuelDump.dumpFwd = fuelDump.dumpAft = fuelDump.dumpRight = false; 
				}
			}

			if (fuelDump.dumpAsf)
			{
				double dumpMass = GetPropellantMass(asfFuel) - 33 * dT;

				if (dumpMass > 0) SetPropellantMass(asfFuel, dumpMass);
				else { SetPropellantMass(asfFuel, 0); fuelDump.dumpAsf = false; }
			}

			if (fuelDump.dumpOxy)
			{
				double dumpMass = GetPropellantMass(oxidizerFuel) - 33 * dT;

				if (dumpMass > 0) SetPropellantMass(oxidizerFuel, dumpMass);
				else { SetPropellantMass(oxidizerFuel, 0); fuelDump.dumpOxy = false; }
			}

			if (fuelDump.dumpLeft || fuelDump.dumpFwd || fuelDump.dumpAft || fuelDump.dumpRight || fuelDump.dumpAsf || fuelDump.dumpOxy)
				playSound(SFX_FUEL_FLOW);
			else fuelDump.masterDump = false;
		}
		else if (fuelDump.dumpingFuel)
		{
			stopSound(SFX_FUEL_FLOW);
			fuelDump.dumpingFuel = false;
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW2_MASTER_DUMP] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
		}
		break;

	case G422::SYSID_DCKSPLY:
		if (dockSupply.supplyFuel)
		{
			if (GetDockStatus(GetDockHandle(0)))
			{
				double supplyMass = GetPropellantMass(allMainFuel) + 17 * dT;

				if (supplyMass < MAXFUEL_MAIN_ALL) SetPropellantMass(allMainFuel, supplyMass);
				else
				{
					SetPropellantMass(allMainFuel, MAXFUEL_MAIN_ALL);
					clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW2_DCKSPL_FUEL] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
				}
			}  else clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW2_DCKSPL_FUEL] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
		}

		if (dockSupply.supplyOxy)
		{
			if (GetDockStatus(GetDockHandle(0)))
			{
				double supplyMass = GetPropellantMass(oxidizerFuel) + 17 * dT;

				if (supplyMass < MAXFUEL_OXY) SetPropellantMass(oxidizerFuel, supplyMass);
				else
				{
					SetPropellantMass(oxidizerFuel, MAXFUEL_OXY);
					clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW2_DCKSPL_OXY] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
				}
			}  else clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW2_DCKSPL_OXY] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
		}

		if (dockSupply.supplyAsf)
		{
			if (GetDockStatus(GetDockHandle(0)))
			{
				double supplyMass = GetPropellantMass(asfFuel) + 17 * dT;

				if (supplyMass < MAXFUEL_ASF) SetPropellantMass(asfFuel, supplyMass);
				else
				{
					SetPropellantMass(asfFuel, MAXFUEL_ASF);
					clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW2_DCKSPL_ASF] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
				}
			}  else clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW2_DCKSPL_ASF] & 0xFFFF), PANEL_MOUSE_RBPRESSED, _V0);
		}

		if (dockSupply.supplyFuel || dockSupply.supplyOxy || dockSupply.supplyAsf) playSound(SFX_FUEL_FLOW);
		else if (dockSupply.supplyingFuel || dockSupply.supplyingOxy || dockSupply.supplyingAsf)
		{
			stopSound(SFX_FUEL_FLOW);
			dockSupply.supplyingFuel = dockSupply.supplyingOxy = dockSupply.supplyingAsf = false;
		}
		break;

	case G422::SYSID_ANTSLP:
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
		else if (rudderSet) 
		{
			SetControlSurfaceLevel(AIRCTRL_RUDDER, 0);
			rudderSet = false;
			playSound(SFX_AP_OFF);
		}
		break;

	case G422::SYSID_WARN:
		if (GroundContact())
		{
			VECTOR3 acc; GetAngularAcc(acc);

			if (acc.z > 0)
			{
				double airspeed = GetAirspeed();

				if (airspeed >= 140 && airspeed < 185) { if (!warningSys.v1) { warningSys.v1 = true; playSound(SFX_INFO_V1); } }
				else if (airspeed >= 185) { if (!warningSys.rotate) { warningSys.rotate = true; playSound(SFX_INFO_ROTATE); } }
				else warningSys.v1 = warningSys.rotate = false;
			}
		}

		if (landingGear->pos != 1 && GetAltitude(ALTMODE_GROUND) <= 300)
		{
			VECTOR3 vel;
			GetGroundspeedVector(FRAME_HORIZON, vel);

			if (vel.y <= 0) if (!warningSys.inhibit) playSound(SFX_WARN_GEARUP);
		}

		double mainMass = GetPropellantMass(allMainFuel);

		if (mainMass == MAXFUEL_MAIN_ALL)
		{
			if (!warningSys.mainFull)
			{
				playSound(SFX_INFO_MAINFULL);
				warningSys.mainFull = true;
			}
		}
		else if (warningSys.mainFull) warningSys.mainFull = false;

		if (mainMass <= 42e3 && mainMass > 0.001)
		{ 
			if (!warningSys.mainLow) { warningSys.mainLow = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) playSound(SFX_WARN_MAINLOW);
		}
		else if (warningSys.mainLow) { warningSys.mainLow = false; stopSound(SFX_WARN_MAINLOW); }

		if (mainMass < 0.001)
		{ 
			if (!warningSys.mainDep) { warningSys.mainDep = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) playSound(SFX_WARN_MAINDEP);
		}
		else if (warningSys.mainDep) { warningSys.mainDep = false; stopSound(SFX_WARN_MAINDEP); }

		double oxyMass = GetPropellantMass(oxidizerFuel);

		if (oxyMass == MAXFUEL_OXY)
		{
			if (!warningSys.oxyFull)
			{
				playSound(SFX_INFO_OXYFULL);
				warningSys.oxyFull = true;
			}
		}
		else if (warningSys.oxyFull) warningSys.oxyFull = false;

		if (oxyMass <= 16e3 && oxyMass > 0.001)
		{ 
			if (!warningSys.oxyLow) { warningSys.oxyLow = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) playSound(SFX_WARN_OXYLOW);
		}
		else if (warningSys.oxyLow) { warningSys.oxyLow = false; stopSound(SFX_WARN_OXYLOW); }

		if (oxyMass < 0.001)
		{ 
			if (!warningSys.oxyDep) { warningSys.oxyDep = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) playSound(SFX_WARN_OXYDEP);
		}
		else if (warningSys.oxyDep) { warningSys.oxyDep = false; stopSound(SFX_WARN_OXYDEP); }

		double asfMass = GetPropellantMass(asfFuel);

		if (asfMass == MAXFUEL_ASF)
		{
			if (!warningSys.asfFull)
			{
				playSound(SFX_INFO_ASFFULL);
				warningSys.asfFull = true;
			}
		}
		else if (warningSys.asfFull) warningSys.asfFull = false;

		if (asfMass <= 3e3 && asfMass > 0.001)
		{ 
			if (!warningSys.asfLow) { warningSys.asfLow = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) playSound(SFX_WARN_ASFLOW);
		}
		else if (warningSys.asfLow) { warningSys.asfLow = false; stopSound(SFX_WARN_ASFLOW); }

		if (asfMass < 0.001)
		{ 
			if (!warningSys.asfDep) { warningSys.asfDep = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) playSound(SFX_WARN_ASFDEP);
		}
		else if (warningSys.asfDep) { warningSys.asfDep = false; stopSound(SFX_WARN_ASFDEP); }

		if (GetADCtrlMode() == 0 && GetDynPressure() > 5e3) 
		{
			if (!warningSys.controlOff) { warningSys.controlOff = true; warningSys.inhibit = false; }
			if (!warningSys.inhibit) playSound(SFX_WARN_CTRLOFF);
		}
		else if (warningSys.controlOff) { warningSys.controlOff = false; stopSound(SFX_WARN_CTRLOFF); }

		break;
	}
}

void G422::clbkPreStep(double simt, double dT, double mjd)
{
	if (timer < 5) timer += dT;

	// get flight envelope coords for engine reference lookup table
	xThrFX = GetMachNumber() * 0.05;
	yThrFX = 1 - (GetAltitude() * 12.5e-6);

	simSystems(dT, G422::SYSID_HYDS);
	simSystems(dT, G422::SYSID_MAINPMP);
	simSystems(dT, G422::SYSID_FUELDMP);
	simSystems(dT, G422::SYSID_DCKSPLY);
	simSystems(dT, G422::SYSID_ANTSLP);
	simSystems(dT, G422::SYSID_WARN);

	simEngines(dT, apuPackA);
	simEngines(dT, apuPackB);

	simEngines(dT, engMainL);

	simEngines(dT, engMainR);

	simEngines(dT, engRamx);

	simEngines(dT, rcs);
	simEngines(dT, oms);

	SetAnimation(dvcThrottleAnim, GetEngineLevel(ENGINE_MAIN));
	SetAnimation(dvcRamxAnim, GetEngineLevel(ENGINE_HOVER));

	if (GetADCtrlMode())
	{
		SetAnimation(dvcStickPitchAnim, (GetControlSurfaceLevel(AIRCTRL_ELEVATOR) * -.5) + .5);
		SetAnimation(dvcStickRollAnim,  (GetControlSurfaceLevel(AIRCTRL_AILERON)  *  .5) + .5);
	} 
	else
	{
		VECTOR3 rotCtrl;
		GetAttitudeRotLevel(rotCtrl);
		SetAnimation(dvcStickPitchAnim, (rotCtrl.x * -.5) + .5);
		SetAnimation(dvcStickRollAnim,  (rotCtrl.z *  .5) + .5);
	}

	SetAnimation(canardAnim, (canards->pos == 1) ? ((GetControlSurfaceLevel(AIRCTRL_ELEVATOR) * -0.5) + .5) : .5);

	SetAnimation(elevatorAnim, (GetControlSurfaceLevel(AIRCTRL_ELEVATOR) * -0.5) + .5);

	SetAnimation(rudderAnim, (GetControlSurfaceLevel(AIRCTRL_RUDDER) * -0.5) + .5);

	SetAnimation(aileronAnim, (GetControlSurfaceLevel(AIRCTRL_AILERON) * -0.5) + .5);

	// operate moving parts
	landingGear->operate(dT);
	visor->operate(dT);
	canards->operate(dT);
	engMainL.inltDoor->operate(dT);
	engMainR.inltDoor->operate(dT);
	ramxDoors->operate(dT);
	bayDoors->operate(dT);
	rcsDoors->operate(dT);

	// wings can only pivot one way if the other is at low-detent
	if (wingTipFthr->pos <= 0) wingTipWvrd->operate(dT);
	if (wingTipWvrd->pos <= 0) wingTipFthr->operate(dT);

	// light positions must update accordingly as well...
	*navRed.pos = wingTipLightsL[0];
	*leftStrobe.pos = wingTipLightsL[1];
	*navGreen.pos = wingTipLightsR[0];
	*rightStrobe.pos = wingTipLightsR[1];

	SetAnimation(dvcAdiAnimA, 0.5 + (GetBank() * SCALAR_ANGLE));
	SetAnimation(dvcAdiAnimB, 0.5 + (GetSlipAngle() * SCALAR_ANGLE));
	SetAnimation(dvcAdiAnimC, 0.5 + (GetPitch() * SCALAR_ANGLE));

	if (GroundContact())
	{
		double steer = GetControlSurfaceLevel(AIRCTRL_RUDDER);
		double speed = GetGroundspeed();

		SetAnimation(dvcTillerAnim, 0.5 - (steer * 0.5));

		if (speed < 250) AddForce({ ((250 - speed) / 250) * steer * 3e5, 0, 0 }, TOUCHDOWN_GEARDOWN[0].pos);

		if (parkingBrakeMode && !brakesReset)
		{
			if (speed > 0.2) SetWheelbrakeLevel(1, 0, false);
			else if (!(GetFlightStatus() & 1)) setStatusLanded();
		}
	}
}

int G422::clbkGeneric(int msgid, int prm, void* context)
{
	switch (msgid)
	{
	case VMSG_MPSTRT:  // moving part started moving
		switch (prm)
		{
		case G422::SYSID_LGRS:
			if (landingGear->pos == 1)
			{
				SetTouchdownPoints(TOUCHDOWN_GEARUP, TOUCHDOWN_COUNT);
				SetMaxWheelbrakeForce(0);
				playSound(SFX_INFO_GEARUP);
			}
			else playSound(SFX_INFO_GEARDN);

			landingGear->sysReset = true;
			return 1;

		case G422::SYSID_CNRD:
			// canards retracted or retracting...
			if (canards->pos == 1) DelControlSurface(acsCndrs);

			canards->sysReset = true;
			return 1;

		case G422::SYSID_BAY:
			if (bayDoors->pos == 1) 
			{
				DelDock(dockHandle);
				ucso->SetSlotDoor(false); 
			}
			return 1;

		case G422::SYSID_RCSDR:
			if (rcsDoors->pos == 1 && rcs.state == RCS::SST_STBY) cueEngines(rcs, RCS::SST_STBY);
			return 1;

		default:
			return 0;
		}

	case VMSG_MPSTOP:  // moving part stopped moving
		switch (prm)
		{
		case G422::SYSID_LGRS:
			if (context)
			{
				if (landingGear->pos == 1) // gears are down and locked
				{
					SetTouchdownPoints(TOUCHDOWN_GEARDOWN, TOUCHDOWN_COUNT);
					SetMaxWheelbrakeForce(8e5);

					setSwMid(MGID_SW3_GEAR);
					playSound(SFX_INFO_GEARDNLOCK);
				}
				else if (landingGear->pos == 0) 
				{
					setSwMid(MGID_SW3_GEAR); 
					playSound(SFX_INFO_GEARUPLOCK);
				}

				landingGear->sysReset = true;
			}
			else if (landingGear->pos == 1) // gears are down and locked
			{
				SetTouchdownPoints(TOUCHDOWN_GEARDOWN, TOUCHDOWN_COUNT);
				SetMaxWheelbrakeForce(8e5);
			}
			
			return 1;

		case G422::SYSID_CNRD:
			if (context)
			{
				if (canards->pos == 1) // canards are deployed!
				{
					acsCndrs = CreateControlSurface3(AIRCTRL_ELEVATOR, 5.0, 2.0, _V(0, 2.0, 35.0), AIRCTRL_AXIS_XNEG);
					setSwMid(MGID_SW3_CANARD);
				}
				else if (canards->pos == 0) setSwMid(MGID_SW3_CANARD);

				canards->sysReset = true;
			}
			else if (canards->pos == 1) acsCndrs = CreateControlSurface3(AIRCTRL_ELEVATOR, 5.0, 2.0, _V(0, 2.0, 35.0), AIRCTRL_AXIS_XNEG);

			return 1;

		case G422::SYSID_VISR:
			if (context)
			{
				if (visor->pos == 1) { xrSound->LoadWav(XRSound::FlightWind, "Sound/G422/wind_vsr_up.wav", XRSound::BothViewClose); setSwMid(MGID_SW3_VISOR); }
				else if (visor->pos == 0) { xrSound->LoadWav(XRSound::FlightWind, "Sound/G422/wind_vsr_down.wav", XRSound::BothViewClose); setSwMid(MGID_SW3_VISOR); }
			}

			return 1;

		case G422::SYSID_RAMX:
			if (context) setSwMid(MGID_SW3_RAMX_DOOR);

			// hover engines are assigned to ramcasters upon doors open - this prevents it being "used" by MFD's as if it really was a "hover" engine
			if (ramxDoors->getToggleState()) cueEngines(engRamx, RAMCASTER::ENG_STOP);
			else cueEngines(engRamx, RAMCASTER::ENG_INOP);

			return 1;

		case G422::SYSID_RCSDR:
			if (context)
			{
				if (rcsDoors->pos == 1)
				{
					if (rcs.state == RCS::SST_STBY) cueEngines(rcs, RCS::SST_STBY);
					setSwMid(MGID_SW3_RCS);
				}
				else if (rcsDoors->pos == 0) setSwMid(MGID_SW3_RCS);
			}
			else if (rcsDoors->pos == 1 && rcs.state == RCS::SST_STBY) cueEngines(rcs, RCS::SST_STBY);

			return 1;

		case G422::SYSID_BAY:
			if (context)
			{
				if (bayDoors->pos == 1) setSwMid(MGID_SW3_BAY_OPENCLSE);
				else if (bayDoors->pos == 0) setSwMid(MGID_SW3_BAY_OPENCLSE);
			} 

			if (bayDoors->pos == 1) 
			{
				dockHandle = CreateDock(V3_DOCKING_PORT, _Y, _Zn);
				ucso->SetSlotDoor(true);
			}

			return 1;

		default:
			return 0;
		}

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

	ucso->SetGroundRotation(status, landingGear->pos == 1 ? 5.26 : 4.10324);

	DefSetStateEx(&status);
}