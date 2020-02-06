#include "..\Header\G422.h"
#include "..\Header\G422_DVC.h"
#include "..\Header\G422_MDL_EXT.h"
#include "..\Header\G422_MDL_DVC.h"

inline bool MovingPart::operate(double& dt)
{
	switch (mp_status)
	{
	case MP_MOVING:
		pos = pos + (rate * dt);

		if (pos >= 1.0)
		{
			pos = 1.0;
			mp_status = MP_HI_DETENT;
			vsl->clbkGeneric(VMSG_MPSTOP, sysID, this);
		}
		vsl->SetAnimation(anim_idx, pos);

		return true;

	case MP_REVERSING:
		pos = pos - (rate * dt);

		if (pos <= 0.0)
		{
			pos = 0.0;
			mp_status = MP_LOW_DETENT;
			vsl->clbkGeneric(VMSG_MPSTOP, sysID, this);
		}
		vsl->SetAnimation(anim_idx, pos);

		return true;
	}
	return false;
}

VECTOR3 vF, vW, vA; // define some static stuff for use below...

void G422::clbkPreStep(double simt, double dT, double mjd)
{
	if (timer < 5) timer += dT;
	// get flight envelope coords for engine reference lookup table
	thrFX_X = GetMachNumber() * .05;
	thrFX_Y = 1.0 - (GetAltitude() * 12.5e-6);

	simEngines(dT, apu);

	simEngines(dT, engMain_L);
	simEngines(dT, engMain_R);

	simEngines(dT, engRamx);

	simEngines(dT, rcoms);

	if (engMain_L.state == RT66::SST_RUN_EXT)
	{
		// not quite sure how or why this works.... got to it by tweaking around with plot-o-matic...
		double alt = GetAltitude() * .00001;
		fx_contrailLvl = max(0, ((alt - .028) * alt) * ((.18 - alt) * 300.0) * engMain_L.epr);
		fx_exhaustLvl = engMain_L.epr;
	}
	else
	{
		fx_contrailLvl = 0.0;
		fx_exhaustLvl = 0.0;
	}

	SetAnimation(an_dvc_thr, GetEngineLevel(ENGINE_MAIN));
	SetAnimation(an_dvc_ramx, GetEngineLevel(ENGINE_HOVER));

	SetAnimation(an_dvc_csPitch, (GetControlSurfaceLevel(AIRCTRL_ELEVATOR) * -.5) + .5);
	SetAnimation(an_dvc_csRoll, (GetControlSurfaceLevel(AIRCTRL_AILERON) * .5) + .5);

	SetAnimation(an_acs_cnrd, (canards->pos >= 1.0) ? ((GetControlSurfaceLevel(AIRCTRL_ELEVATOR) * -.5) + .5) : .5);

	SetAnimation(an_acs_elvr, (GetControlSurfaceLevel(AIRCTRL_ELEVATOR) * -.5) + .5);

	SetAnimation(an_acs_rddr, (GetControlSurfaceLevel(AIRCTRL_RUDDER) * -.5) + .5);

	SetAnimation(an_acs_alrn, (GetControlSurfaceLevel(AIRCTRL_AILERON) * -.5) + .5);

	if (visor->pos < 1.0) // visor open (down)
	{
		double dprs = GetDynPressure();

		if (dprs > .1) PlayVesselWave(SFXID, SFX_WIND_OPEN, LOOP, min(240, int(dprs * 11.0e-3)));
	}

	// operate moving parts
	landingGears->operate(dT);
	visor->operate(dT);
	canards->operate(dT);
	inltDoors->operate(dT);
	ramxDoors->operate(dT);
	bayDoors->operate(dT);
	rcs->operate(dT);

	// wings can only pivot one way if the other is at low-detent
	if (wingTipFthr->pos <= 0.0) wingTipWvrd->operate(dT);
	if (wingTipWvrd->pos <= 0.0) wingTipFthr->operate(dT);

	// light positions must update accordingly as well...
	if (wingTipFthr->pos > 0.0 && wingTipFthr->pos < 1.0 // when necessary...
		|| wingTipWvrd->pos > 0.0 && wingTipWvrd->pos < 1.0)
	{
		*nav_red.pos = wingTipLightsL[0];
		*stb_l.pos = wingTipLightsL[1];
		*nav_green.pos = wingTipLightsR[0];
		*stb_r.pos = wingTipLightsR[1];
	}

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
		SetThrusterLevel(ramcaster, 0.0);
		SetThrusterLevel(oms_thgr[0], 0.0);
		SetThrusterLevel(oms_thgr[1], 0.0);

		SetThrusterLevel(RT66_gasGen_thgr[0], 0.0);
		SetThrusterLevel(RT66_gasGen_thgr[1], 0.0);

		SetThrusterLevel(RT66_burner_thgr[0], 0.0);
		SetThrusterLevel(RT66_burner_thgr[1], 0.0);

		SetThrusterLevel(RT66_rocket_thgr[0], 0.0);
		SetThrusterLevel(RT66_rocket_thgr[1], 0.0);
		return 0;

	case VMSG_MPSTRT:  // moving part started moving
		switch (prm)
		{
		case G422_SYSID_LGRS:
			if (landingGears->pos >= 1.0)
			{
				SetTouchdownPoints(TOUCHDOWN_GEARUP, TOUCHDOWN_COUNT);
				SetMaxWheelbrakeForce(0.0);
			}

			PlayVesselWave(SFXID, SFX_GEARS);
			landingGears->sysReset = true;

			break;

		case G422_SYSID_CNRD:
			if (canards->pos >= 1.0) // canards retracted or retracting...
				DelControlSurface(acsCndrs);

			PlayVesselWave(SFXID, SFX_CNRDS);
			canards->sysReset = true;

			break;

		case G422_SYSID_VISR:
			if (visor->pos <= 1.0) PlayVesselWave(SFXID, SFX_VSRDN);
			else                   PlayVesselWave(SFXID, SFX_VSRUP);

			break;

		case G422_SYSID_BAY:
			PlayVesselWave(SFXID, SFX_BAY_DOORS);
			if (bayDoors->pos <= 1.0) ucso->SetSlotDoor(false);
			break;
		default:
			break;
		}
		return 0;

	case VMSG_MPSTOP:  // moving part stopped moving
		switch (prm)
		{
		case G422_SYSID_LGRS:
			if (landingGears->pos >= 1.0) // gears are down and locked
			{
				SetTouchdownPoints(TOUCHDOWN_GEARDOWN, TOUCHDOWN_COUNT);
				SetMaxWheelbrakeForce(8e5);

				VCSwitches[VC_swIndexByMGID[MGID_SW3_GEAR]].setPos(SW3_MID, this);
			}
			landingGears->sysReset = true;
			StopVesselWave(SFXID, SFX_GEARS);
			break;

		case G422_SYSID_CNRD:
			if (canards->pos >= 1.0) // canards are deployed!
			{
				acsCndrs = CreateControlSurface3(AIRCTRL_ELEVATOR, 5.0, 2.0, _V(0.0, 2.0, 35.0), AIRCTRL_AXIS_XNEG);

				VCSwitches[VC_swIndexByMGID[MGID_SW3_CANARD]].setPos(SW3_MID, this);
			}
			canards->sysReset = true;
			StopVesselWave(SFXID, SFX_CNRDS);
			break;


		case G422_SYSID_VISR:
			VCSwitches[VC_swIndexByMGID[MGID_SW3_VISOR]].setPos(SW3_MID, this);
			StopVesselWave(SFXID, SFX_VSRDN);
			StopVesselWave(SFXID, SFX_VSRUP);
			if (visor->pos >= 1.0) StopVesselWave(SFXID, SFX_WIND_OPEN);
			break;

		case G422_SYSID_RAMX:
			PlayVesselWave(SFXID, SFX_VC_AFLIK);
			VCSwitches[VC_swIndexByMGID[MGID_SW3_RAMX_DOOR]].setPos(SW3_MID, this);
			// hover engines are assigned to ramcasters upon doors open - this prevents it being "used" by MFD's as if it really was a "hover" engine
			if (ramxDoors->getToggleState()) cueEngines(engRamx, RAMCASTER::ENG_STOP);
			else cueEngines(engRamx, RAMCASTER::ENG_INOP);
			break;

		case G422_SYSID_BAY:
			StopVesselWave(SFXID, SFX_BAY_DOORS);
			if (bayDoors->pos >= 1.0) ucso->SetSlotDoor(true);
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

	ucso->SetGroundRotation(status, landingGears->pos >= 1.0 ? 5.26 : 4.10324);

	DefSetStateEx(&status);
}