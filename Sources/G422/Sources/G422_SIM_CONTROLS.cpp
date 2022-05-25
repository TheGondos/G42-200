#include "../Headers/G422.h"
#include "../Headers/G422_MDL_DVC.h"
#include "../Headers/G422_DVC.h"
#include <cstring>

void G422::clbkRCSMode(int mode)
{
	VCKnob& rcsKnob = vcKnobs[vcKnobIndexByMGID[MGID_KB3_RCS_MODE]];

	switch (mode)
	{
	case RCS_NONE:
		rcsKnob.setPos(VCKnob::KB3_DOWN, this);
		break;

	case RCS_ROT:
		rcsKnob.setPos(VCKnob::KB3_MID, this);
		break;

	case RCS_LIN:
		rcsKnob.setPos(VCKnob::KB3_UP, this);
		break;
	}
}

void G422::clbkNavMode(int mode, bool active) { if (active) apSet = false; }

bool G422::clbkVCMouseEvent(int id, int ev, VECTOR3& p)
{
	int ctrlSet = (id>>16) & 0xffff;
	int ctrlID = (id) & 0xffff;

	switch (ctrlSet)
	{
	case VC_CTRLSET_MFDK:
	{
		bool op = oapiProcessMFDButton(ctrlID / 12, ctrlID % 12, ev);
		static bool hold = false;
		if (!hold)
		{
			playSound(SFX_VC_POP);
			hold = true;
		}
		else if (ev & PANEL_MOUSE_UP) hold = false;
		return true;
	}	

	case VC_CTRLSET_MFDC:
		switch (ctrlID % 3)
		{
		case 0:  // select keys...
			playSound(SFX_VC_POP);
			oapiSendMFDKey(ctrlID / 3, OAPI_KEY_F1);
			return true;

		case 1: // power buttons...
			playSound(SFX_VC_POP);
			oapiSendMFDKey(ctrlID / 3, OAPI_KEY_ESCAPE);
			return true;

		case 2: // menu keys...
			playSound(SFX_VC_POP);
			oapiSendMFDKey(ctrlID / 3, OAPI_KEY_GRAVE);
			return true;
		}

	case VC_CTRLSET_EICAS_C1:
	case VC_CTRLSET_EICAS_C2:
	{
		UINT btn = (UINT)(p.x * 6.0) + ((p.y > .5) ? 7 : 1);

		playSound(SFX_VC_POP);
		selEicasMode((ctrlSet == VC_CTRLSET_EICAS_C1) ? 0 : 1, btn);

		return true;
	}

	case VC_CTRLSET_EICAS_P1L:
	case VC_CTRLSET_EICAS_P1R:
	{
		UINT btn = (UINT)(p.y * 6.0) + ((ctrlSet == VC_CTRLSET_EICAS_P1L) ? 1 : 7);

		playSound(SFX_VC_POP);
		selEicasMode(2, btn);

		return true;
	}
	case VC_CTRLSET_EICAS_P2L:
	case VC_CTRLSET_EICAS_P2R:
	{
		UINT btn = (UINT)(p.y * 6.0) + ((ctrlSet == VC_CTRLSET_EICAS_P2L) ? 1 : 7);

		playSound(SFX_VC_POP);
		selEicasMode(3, btn);

		return true;
	}

	case VC_CTRLSET_EICAS_E1L:
	case VC_CTRLSET_EICAS_E1R:
	{
		UINT btn = (UINT)(p.y * 6.0) + ((ctrlSet == VC_CTRLSET_EICAS_E1R) ? 1 : 7);

		playSound(SFX_VC_POP);
		selEicasMode(4, btn);

		return true;
	}
	case VC_CTRLSET_EICAS_E2L:
	case VC_CTRLSET_EICAS_E2R:
	{
		UINT btn = (UINT)(p.y * 6.0) + ((ctrlSet == VC_CTRLSET_EICAS_E2R) ? 1 : 7);

		playSound(SFX_VC_POP);
		selEicasMode(5, btn);

		return true;
	}

	case VC_CTRLSET_EICAS_PWR:
		playSound(SFX_VC_POP);
		selEicasMode(ctrlID, (ev == PANEL_MOUSE_LBDOWN) ? abs(vcEicasScreens[ctrlID]) : -abs(vcEicasScreens[ctrlID]));
		return true;

	case VC_CTRLSET_SWITCHES:
	{
		VCSwitch& vcSwitch = vcSwitches[ctrlID];

		if (ev != PANEL_MOUSE_IGNORE)
		{
			if (ev & (PANEL_MOUSE_LBUP | PANEL_MOUSE_RBUP)) // spring-loaded switches revert when released
			{
				switch (vcSwitch.mgid)
				{
				case MGID_SW3_RAMX_IGN:
				case MGID_SW3_EMAINL_IGN:
				case MGID_SW3_EMAINR_IGN:
				case MGID_SW3_APU_PACK_A:
				case MGID_SW3_APU_PACK_B:
					// the above switches are spring-loaded only on top position...
					if (vcSwitch.pos != VCSwitch::SW3_UP) return false;

				case MGID_SW3_EMAINL_MODE:
				case MGID_SW3_EMAINR_MODE:
				case MGID_SW3_RAMX_MODE:
					// 
					vcSwitch.setPos(VCSwitch::SW3_MID, this);
					playSound(SFX_VC_TICK);
					return true;
				}

				return false;
			}
			else if (ev & PANEL_MOUSE_RBPRESSED)
			{
				if (!vcSwitch.flick(false, this)) return false;

				playSound(SFX_VC_TICK);
				playSound(SFX_VC_AFLIK);
			}
			else 
			{
				if (!vcSwitch.flick(ev == PANEL_MOUSE_LBDOWN, this))
				{
					playSound(SFX_VC_POP);
					return false;
				}

				playSound(SFX_VC_FLICK);
			}
		}

		switch (vcSwitch.mgid)
		{
		case MGID_SW3_GEAR:
		{
			bool toggled = false;
			if (vcSwitch.pos == VCSwitch::SW3_UP && !gearMode) toggled = landingGear->toggle(false); // gears are backwards (false means retract)
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN && !gearMode) toggled = landingGear->toggle(true);

			if (!toggled) setSwMid(MGID_SW3_GEAR);
			return true;
		}
		case MGID_SW3_CANARD:
		{
			bool toggled = false;
			if (vcSwitch.pos == VCSwitch::SW3_UP) toggled = canards->toggle(true);
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN) toggled = canards->toggle(false);

			if (!toggled) setSwMid(MGID_SW3_CANARD);
			return true;
		}
		case MGID_SW3_VISOR:
		{
			bool toggled = false;
			if (vcSwitch.pos == VCSwitch::SW3_UP) toggled = visor->toggle(true);
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN) toggled = visor->toggle(false);

			if (!toggled) setSwMid(MGID_SW3_VISOR);
			return true;
		}
		case MGID_SW3_RCS:
		{
			bool toggled = false;
			if (vcSwitch.pos == VCSwitch::SW3_UP) toggled = rcsDoors->toggle(true);
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN) toggled = rcsDoors->toggle(false);

			if (!toggled) setSwMid(MGID_SW3_RCS);
			return true;
		}
		case MGID_SW3_BAY_OPENCLSE:
		{
			bool toggled = false;
			if (vcSwitch.pos == VCSwitch::SW3_UP) toggled = bayDoors->toggle(true);
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN) toggled = bayDoors->toggle(false);

			if (!toggled) setSwMid(MGID_SW3_BAY_OPENCLSE);
			return true;
		}
		case MGID_SW3_EMAINL_MODE: // mode switch
			if (vcSwitch.pos == VCSwitch::SW3_UP) cueEngines(engMainL, RT66::ENG_INLT_OPEN);
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN) cueEngines(engMainL, RT66::ENG_INLT_CLSE);
			return true;

		case MGID_SW3_EMAINR_MODE: // mode switch
			if (vcSwitch.pos == VCSwitch::SW3_UP) cueEngines(engMainR, RT66::ENG_INLT_OPEN);
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN) cueEngines(engMainR, RT66::ENG_INLT_CLSE);
			return true;

		case MGID_SW3_EMAINL_REHEAT:
			if (vcSwitch.pos == VCSwitch::SW2_UP) engMainL.burnerToggle = 1;
			else engMainL.burnerToggle = 0;
			return true;

		case MGID_SW3_EMAINR_REHEAT:
			if (vcSwitch.pos == VCSwitch::SW2_UP) engMainR.burnerToggle = 1;
			else engMainR.burnerToggle = 0;
			return true;

		// this is a bit kludge-ish... the ideal thing would be to simulate turbine RPM or something
		// but for now this should kinda do the trick....
		case MGID_SW3_EMAINL_IGN:
			if (vcSwitch.pos == VCSwitch::SW3_UP) cueEngines(engMainL, RT66::ENG_SPOOLUP);
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN) cueEngines(engMainL, RT66::ENG_SPOOLDOWN);
			return true;

		case MGID_SW3_EMAINR_IGN:
			if (vcSwitch.pos == VCSwitch::SW3_UP) cueEngines(engMainR, RT66::ENG_SPOOLUP);
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN) cueEngines(engMainR, RT66::ENG_SPOOLDOWN);
			return true;

		case MGID_SW3_RAMX_IGN:
			if (vcSwitch.pos == VCSwitch::SW3_UP) cueEngines(engRamx, RAMCASTER::ENG_START);
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN) cueEngines(engRamx, RAMCASTER::ENG_STOP);
			return true;

		case MGID_SW3_RAMX_DOOR:
		{
			bool toggled = false;
			if (vcSwitch.pos == VCSwitch::SW3_UP) toggled = ramxDoors->toggle(true);
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN) toggled = ramxDoors->toggle(false);

			if (!toggled) setSwMid(MGID_SW3_RAMX_DOOR);
			return true;
		}

		case MGID_SW3_RAMX_MODE:
			if (!ramcasterMode) return true;  // off...

			if (vcSwitch.pos == VCSwitch::SW3_UP) cueEngines(engRamx, RAMCASTER::ENG_RUNHI);
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN) cueEngines(engRamx, RAMCASTER::ENG_RUNLO);
			return true;

		case MGID_SW2_THR_AUTH:
			if (vcSwitch.pos == VCSwitch::SW2_UP) thrAuthority = 0;
			else thrAuthority = 1;
			return true;

		case MGID_SW2_MSFEED_R:
			if (vcSwitch.pos == VCSwitch::SW2_UP) { engMainR.feed |= RT66::FUEL_PUMP; playSound(SFX_FUELPUMP_MAIN, true); }
			else
			{
				engMainR.feed &= ~RT66::FUEL_PUMP;
				if (vcSwitches[vcSwitchIndexByMGID[MGID_SW2_MSFEED_L]].pos == VCSwitch::SW2_DOWN) stopSound(SFX_FUELPUMP_MAIN);
			}
			return true;

		case MGID_SW2_MSFEED_L:
			if (vcSwitch.pos == VCSwitch::SW2_UP) { engMainL.feed |= RT66::FUEL_PUMP; playSound(SFX_FUELPUMP_MAIN, true); }
			else
			{
				engMainL.feed &= ~RT66::FUEL_PUMP;
				if (vcSwitches[vcSwitchIndexByMGID[MGID_SW2_MSFEED_R]].pos == VCSwitch::SW2_DOWN) stopSound(SFX_FUELPUMP_MAIN);
			}
			return true;

		case MGID_SW2_MSFEED_C:
			if (vcSwitch.pos == VCSwitch::SW2_UP) playSound(SFX_FUELPUMP_RAMX, true);
			else stopSound(SFX_FUELPUMP_RAMX);
			return true;

		case MGID_SW3_OXYFEED_R:
			if (vcSwitch.pos == VCSwitch::SW2_UP) { engMainR.feed |= RT66::OXY_PUMP; playSound(SFX_FUELPUMP_OXY, true); }
			else
			{
				engMainR.feed &= ~RT66::OXY_PUMP;
				if (vcSwitches[vcSwitchIndexByMGID[MGID_SW3_OXYFEED_L]].pos == VCSwitch::SW2_DOWN) stopSound(SFX_FUELPUMP_OXY);
			}
			return true;

		case MGID_SW3_OXYFEED_L:
			if (vcSwitch.pos == VCSwitch::SW2_UP) { engMainL.feed |= RT66::OXY_PUMP; playSound(SFX_FUELPUMP_OXY, true); }
			else
			{
				engMainL.feed &= ~RT66::OXY_PUMP;
				if (vcSwitches[vcSwitchIndexByMGID[MGID_SW3_OXYFEED_R]].pos == VCSwitch::SW2_DOWN) stopSound(SFX_FUELPUMP_OXY);
			}
			return true;

		case MGID_SW2_SYSFEED_APU:
			if (vcSwitch.pos == VCSwitch::SW2_UP)
			{
				apuPackA.feed |= APU::FUEL_PUMP;
				apuPackB.feed |= APU::FUEL_PUMP;
				playSound(SFX_FUELPUMP_APU, true);
			}
			else
			{
				apuPackA.feed &= ~APU::FUEL_PUMP;
				apuPackB.feed &= ~APU::FUEL_PUMP;
				stopSound(SFX_FUELPUMP_APU);
			}
			return true;

		case MGID_SW2_SYSFEED_RCS:
			if (vcSwitch.pos == VCSwitch::SW2_UP) { rcs.feed |= RCS::FUEL_PUMP; playSound(SFX_FUELPUMP_RCS, true); }
			else { rcs.feed &= ~RCS::FUEL_PUMP; stopSound(SFX_FUELPUMP_RCS); }
			return true;

		case MGID_SW2_SYSFEED_OMS:
			if (vcSwitch.pos == VCSwitch::SW2_UP) { oms.feed |= OMS::FUEL_PUMP; playSound(SFX_FUELPUMP_OMS, true);}
			else { oms.feed &= ~OMS::FUEL_PUMP; stopSound(SFX_FUELPUMP_OMS); }
			return true;

		case MGID_SW3_APU_PACK_A:
			if (vcSwitch.pos == VCSwitch::SW3_UP) cueEngines(apuPackA, APU::ENG_START);
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN) cueEngines(apuPackA, APU::ENG_STOP);
			return true;

		case MGID_SW3_APU_PACK_B:
			if (vcSwitch.pos == VCSwitch::SW3_UP) cueEngines(apuPackB, APU::ENG_START);
			else if (vcSwitch.pos == VCSwitch::SW3_DOWN) cueEngines(apuPackB, APU::ENG_STOP);
			return true;

		case MGID_SW3_APU_HYD:
			if (vcSwitch.pos == VCSwitch::SW3_UP) { apuPackA.hydFeed = apuPackB.hydFeed = APU::HYD_PUMP; }
			else if (vcSwitch.pos == VCSwitch::SW3_MID) { apuPackA.hydFeed = apuPackB.hydFeed = APU::HYD_AUTO; }
			else { apuPackA.hydFeed = apuPackB.hydFeed = APU::HYD_OFF; }
			return true;

		case MGID_SW3_STBYIGN_RCS:
			if (vcSwitch.pos == VCSwitch::SW2_UP) cueEngines(rcs, RCS::SST_STBY);
			else cueEngines(rcs, RCS::SST_INOP);
			return true;

		case MGID_SW3_STBYIGN_OMS:
			if (vcSwitch.pos == VCSwitch::SW2_UP) cueEngines(oms, OMS::SST_STBY);
			else cueEngines(oms, OMS::SST_INOP);
			return true;

		case MGID_SW2_APU_RAMX:
			if (vcSwitch.pos == VCSwitch::SW2_UP) engRamx.feed |= RAMCASTER::FUEL_PUMP;
			else engRamx.feed &= ~RAMCASTER::FUEL_PUMP;
			return true;

		case MGID_SW2_LGT_NAV:
			navWhite.active = navRed.active = navGreen.active = vcSwitch.pos == VCSwitch::SW2_UP;
			return true;

		case MGID_SW2_LGT_STB:
			tailStrobe.active = leftStrobe.active = rightStrobe.active = vcSwitch.pos == VCSwitch::SW2_UP;
			return true;

		case MGID_SW2_LGT_BCN:
			beacon1.active = beacon2.active = vcSwitch.pos == VCSwitch::SW2_UP;
			return true;

		case MGID_SW2_LGT_LND:
			landingLight->Activate(vcSwitch.pos == VCSwitch::SW2_UP);
			return true;

		case MGID_SW2_LGT_TAX:
			taxiLight->Activate(vcSwitch.pos == VCSwitch::SW2_UP);
			return true;

		case MGID_SW2_LGT_DCK:
			dockingLight->Activate(vcSwitch.pos == VCSwitch::SW2_UP);
			return true;

		case MGID_SW2_LEFT_DUMP:
			fuelDump.dumpLeft = vcSwitch.pos == VCSwitch::SW2_UP;
			return true;

		case MGID_SW2_FWD_DUMP:
			fuelDump.dumpFwd = vcSwitch.pos == VCSwitch::SW2_UP;
			return true;

		case MGID_SW2_AFT_DUMP:
			fuelDump.dumpAft = vcSwitch.pos == VCSwitch::SW2_UP;
			return true;

		case MGID_SW2_RIGHT_DUMP:
			fuelDump.dumpRight = vcSwitch.pos == VCSwitch::SW2_UP;
			return true;

		case MGID_SW2_ASF_DUMP:
			fuelDump.dumpAsf = vcSwitch.pos == VCSwitch::SW2_UP;
			return true;

		case MGID_SW2_OXY_DUMP:
			fuelDump.dumpOxy = vcSwitch.pos == VCSwitch::SW2_UP;
			return true;

		case MGID_SW2_MASTER_DUMP:
			if (vcSwitch.pos == VCSwitch::SW2_UP) { fuelDump.dumpingFuel = true; fuelDump.masterDump = true; playSound(SFX_FUEL_DUMP); }
			else fuelDump.masterDump = false;
			return true;

		case MGID_SW2_DCKSPL_FUEL:
			if (vcSwitch.pos == VCSwitch::SW2_UP) { dockSupply.supplyingFuel = true; dockSupply.supplyFuel = true; }
			else dockSupply.supplyFuel = false;
			return true;

		case MGID_SW2_DCKSPL_OXY:
			if (vcSwitch.pos == VCSwitch::SW2_UP) { dockSupply.supplyingOxy = true; dockSupply.supplyOxy = true; }
			else dockSupply.supplyOxy = false;
			return true;

		case MGID_SW2_DCKSPL_ASF:
			if (vcSwitch.pos == VCSwitch::SW2_UP) { dockSupply.supplyingAsf = true; dockSupply.supplyAsf = true; }
			else dockSupply.supplyAsf = false;
			return true;
		}

		return true; // whatever else, just flick it! (faux switch)
	}
	case VC_CTRLSET_KNOBS:
	{
		VCKnob& vcKnob = vcKnobs[ctrlID];

		if (ev != PANEL_MOUSE_IGNORE)
		{
			if (!vcKnob.flick(ev == PANEL_MOUSE_LBDOWN, this)) { playSound(SFX_VC_POP); return false; }
			playSound(SFX_VC_FLICK);
		}

		switch (vcKnob.mgid)
		{
		case MGID_KB3_RCS_MODE:
			switch (vcKnob.pos)
			{
			case VCKnob::KB3_UP:
				SetAttitudeMode(RCS_LIN);
				break;
			case VCKnob::KB3_MID:
				SetAttitudeMode(RCS_ROT);
				break;
			case VCKnob::KB3_DOWN:
				SetAttitudeMode(RCS_NONE);
				break;
			}
			break;

		case MGID_KB3_ACS_MODE:
			switch (vcKnob.pos)
			{
			case VCKnob::KB3_UP:
				SetADCtrlMode(7);
				break;
			case VCKnob::KB3_MID:
				SetADCtrlMode(1);
				break;
			case VCKnob::KB3_DOWN:
				SetADCtrlMode(0);
				break;
			}
			break;

		case MGID_KB2_COORD_REF:
			switch (vcKnob.pos)
			{
			case VCKnob::KB2_UP:
				rcs.mode = true;
				if (rcs.state == RCS::SST_STBY) { cueEngines(rcs, RCS::SST_STBY);  if (xrSound) playSound(SFX_RCS_DOCKING); }
				break;
			case VCKnob::KB2_DOWN:
				rcs.mode = false;
				if (rcs.state == RCS::SST_STBY) { cueEngines(rcs, RCS::SST_STBY); if (xrSound) playSound(SFX_RCS_NORMAL); }
				break;
			}
			break;

		case MGID_KB2_LENG_FEED:
			switch (vcKnob.pos)
			{
			case VCKnob::KB2_UP:
				engMainL.feed |= RT66::FUEL_OPEN;
				break;
			case VCKnob::KB2_DOWN:
				engMainL.feed &= ~RT66::FUEL_OPEN;
				break;
			}
			break;

		case MGID_KB2_RAMX_FEED:
			switch (vcKnob.pos)
			{
			case VCKnob::KB2_UP:
				engRamx.feed |= RAMCASTER::FUEL_OPEN;
				break;
			case VCKnob::KB2_DOWN:
				engRamx.feed &= ~RAMCASTER::FUEL_OPEN;
				break;
			}
			break;

		case MGID_KB2_RENG_FEED:
			switch (vcKnob.pos)
			{
			case VCKnob::KB2_UP:
				engMainR.feed |= RT66::FUEL_OPEN;
				break;
			case VCKnob::KB2_DOWN:
				engMainR.feed &= ~RT66::FUEL_OPEN;
				break;
			}
			break;

		case MGID_KB2_RCS_FEED:
			switch (vcKnob.pos)
			{
			case VCKnob::KB2_UP:
				rcs.feed |= RCS::FUEL_OPEN;
				break;
			case VCKnob::KB2_DOWN:
				rcs.feed &= ~RCS::FUEL_OPEN;
				break;
			}
			break;

		case MGID_KB2_OMS_FEED:
			switch (vcKnob.pos)
			{
			case VCKnob::KB2_UP:
				oms.feed |= OMS::FUEL_OPEN;
				break;
			case VCKnob::KB2_DOWN:
				oms.feed &= ~OMS::FUEL_OPEN;
				break;
			}
			break;

		case MGID_KB2_APU_FEED:
			switch (vcKnob.pos)
			{
			case VCKnob::KB2_UP:
				apuPackA.feed |= APU::FUEL_OPEN;
				apuPackB.feed |= APU::FUEL_OPEN;
				break;
			case VCKnob::KB2_DOWN:
				apuPackA.feed &= ~APU::FUEL_OPEN;
				apuPackB.feed &= ~APU::FUEL_OPEN;
				break;
			}
			break;

		case MGID_KB2_LOXY_FEED:
			switch (vcKnob.pos)
			{
			case VCKnob::KB2_UP:
				engMainL.feed |= RT66::OXY_OPEN;
				break;
			case VCKnob::KB2_DOWN:
				engMainL.feed &= ~RT66::OXY_OPEN;
				break;
			}
			break;

		case MGID_KB2_ROXY_FEED:
			switch (vcKnob.pos)
			{
			case VCKnob::KB2_UP:
				engMainR.feed |= RT66::OXY_OPEN;
				break;
			case VCKnob::KB2_DOWN:
				engMainR.feed &= ~RT66::OXY_OPEN;
				break;
			}
			break;
		}

		return true;
	}
	case VC_CTRLSET_BUTTONS:
	{
		playSound(SFX_VC_POP);

		switch (vcButtonDefs[ctrlID].bid)
		{
		case AP_MASTER_BUTTON:
			apSet = !apSet;

			if (apSet) 
			{ 
				playSound(SFX_AP_ON);
				
				for (int mode = NAVMODE_KILLROT; mode <= NAVMODE_HOLDALT; mode++) DeactivateNavmode(mode);
			}
			break;

		case AP_PROGRADE_BUTTON:
			ToggleNavmode(NAVMODE_PROGRADE);
			break;
		case AP_RETROGRADE_BUTTON:
			ToggleNavmode(NAVMODE_RETROGRADE);
			break;
		case AP_NORMAL_BUTTON:
			ToggleNavmode(NAVMODE_NORMAL);
			break;
		case AP_ANTINORMAL_BUTTON:
			ToggleNavmode(NAVMODE_ANTINORMAL);
			break;
		case AP_HLEVEL_BUTTON:
			ToggleNavmode(NAVMODE_HLEVEL);
			break;
		case AP_KILLROT_BUTTON:
			ToggleNavmode(NAVMODE_KILLROT);
			break;

		case HUD_DOCK_BUTTON:
			oapiSetHUDMode(HUD_DOCKING);
			break;
		case HUD_ORBIT_BUTTON:
			oapiSetHUDMode(HUD_ORBIT);
			break;
		case HUD_SURF_BUTTON:
			oapiSetHUDMode(HUD_SURFACE);
			break;
		case HUD_OFF_BUTTON:
			oapiSetHUDMode(HUD_NONE);
			break;

		case HUD_BRT_BUTTON:
			if(ev & PANEL_MOUSE_RBDOWN) oapiSetHUDIntensity(min(oapiGetHUDIntensity() + 0.1, 1.0));
			else oapiSetHUDIntensity(max(oapiGetHUDIntensity() - 0.1, 0.0));
			break;
		case HUD_CLR_BUTTON:
			oapiToggleHUDColour();
			break;

		case MASTER_WARN_BUTTON:
			warningSys.inhibit = true;
			playSound(SFX_WARN_RESET);
			break;
		}

		return true;
	}
	case VC_CTRLSET_RAMXLVR:
	{
		double level = GetEngineLevel(ENGINE_HOVER);

		if (ev == PANEL_MOUSE_LBDOWN)
		{
			if (level < 0.5)
			{
				playSound(SFX_VC_CLAK);

				SetEngineLevel(ENGINE_HOVER, 0.5);

				oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_RAMXLVR << 16), V3_VC_RAMX_ENG, .035);
			}
			else if (level < 1)
			{
				playSound(SFX_VC_CLAK);

				SetEngineLevel(ENGINE_HOVER, 1);

				oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_RAMXLVR << 16), V3_VC_RAMX_FULL, .035);
			}
		}
		else
		{
			if (level == 1)
			{
				playSound(SFX_VC_CLAK);

				SetEngineLevel(ENGINE_HOVER, 0.5);

				oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_RAMXLVR << 16), V3_VC_RAMX_ENG, .035);
			}
			else if (level >= 0.5)
			{
				playSound(SFX_VC_CLAK);

				SetEngineLevel(ENGINE_HOVER, 0);

				oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_RAMXLVR << 16), V3_VC_RAMX_IDLE, .035);
			}
		}
		return true;
	}
	case VC_CTRLSET_WPOSLVR:
	{
		playSound(SFX_VC_CLAK);

		if ((ev == PANEL_MOUSE_LBDOWN && wingPos >= 1) || (ev == PANEL_MOUSE_RBDOWN && wingPos <= -1)) return true;

		G422::HYDRAULIC& hydSys = hydSysA.hydPrs < 2800 ? hydSysB : hydSysA;

		if (hydSys.hydPrs < 2800)
		{
			playSound(SFX_WARN_APUOFF);
			return true;
		}

		ev == PANEL_MOUSE_LBDOWN ? ++wingPos : --wingPos;

		switch (wingPos)
		{
		case 0:
			wingTipWvrd->toggle(false);
			wingTipFthr->toggle(false);
			SetAnimation(dvcWingPosAnim, 0.5);
			oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_WPOSLVR << 16), V3_VC_WPOS_MID, .035);
			break;
		case -1:
			wingTipWvrd->toggle(true);
			SetAnimation(dvcWingPosAnim, 0);
			oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_WPOSLVR << 16), V3_VC_WPOS_AFT, .035);
			break;
		case 1:
			wingTipFthr->toggle(true);
			SetAnimation(dvcWingPosAnim, 1);
			oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_WPOSLVR << 16), V3_VC_WPOS_FWD, .035);
			break;
		}

		return true;
	}
	case VC_CTRLSET_PRK_BRK_HNDL:
		if (ev == PANEL_MOUSE_LBDOWN && !parkingBrakeMode)
		{
			playSound(SFX_VC_CLAK);
			parkingBrakeMode = true;
			SetAnimation(dvcParkingAnim, 1);
		}
		else if (ev == PANEL_MOUSE_RBDOWN && parkingBrakeMode)
		{
			playSound(SFX_VC_CLAK);
			parkingBrakeMode = false;
			SetAnimation(dvcParkingAnim, 0);
		}
		return true;

	case VC_CTRLSET_GEAR_EMER_HNDL:
		if (!gearMode)
		{
			playSound(SFX_VC_CLAK);
			gearMode = true;
			SetAnimation(dvcGearAnim, 1);
			if (landingGear->pos < 1) landingGear->toggle();
		}
		return true;

	case VC_CTRLSET_UNDOCK_HNDL:
		if (ev & PANEL_MOUSE_LBUP) { SetAnimation(dvcUndockAnim, 0); return true; }

		playSound(SFX_VC_CLAK);
		SetAnimation(dvcUndockAnim, 1);
		Undock(0);
		return true;

	case VC_CTRLSET_BURNENG:
		if (ev == PANEL_MOUSE_LBDOWN)
		{
			if (burnerEngMode == 0) { SetAnimation(dvcBurnerAnim, 0.28); burnerEngMode = 0.25; }
			else if (burnerEngMode == 0.25) { SetAnimation(dvcBurnerAnim, 0.5); burnerEngMode = 0.5; }
			else if (burnerEngMode == 0.5) { SetAnimation(dvcBurnerAnim, 0.73); burnerEngMode = 0.75; }
			else if (burnerEngMode == 0.75) { SetAnimation(dvcBurnerAnim, 1); burnerEngMode = 1; }
			else { playSound(SFX_VC_POP); return false; }
		}
		else
		{
			if (burnerEngMode == 1) { SetAnimation(dvcBurnerAnim, 0.73); burnerEngMode = 0.75; }
			else if (burnerEngMode == 0.75) { SetAnimation(dvcBurnerAnim, 0.5); burnerEngMode = 0.5; }
			else if (burnerEngMode == 0.5) { SetAnimation(dvcBurnerAnim, 0.28); burnerEngMode = 0.25; }
			else if (burnerEngMode == 0.25) { SetAnimation(dvcBurnerAnim, 0); burnerEngMode = 0; }
			else { playSound(SFX_VC_POP); return false; }
		}

		playSound(SFX_VC_FLICK);
		return true;

	case VC_CTRLSET_MOVESEAT:
		if (vcPovStation == 0)
		{
			vcPovStation = 1;  //											engineer side

			SetMeshVisibilityMode(dvcModelIdR, MESHVIS_NEVER);
			SetMeshVisibilityMode(dvcModelIdL, MESHVIS_VC);

			//                   LEAN FORWARD (FMC)                     LEAN LEFT (WINDOW)                  LEAN RIGHT (OVERHEAD)
			SetCameraMovement(_V(0.1, -0.15, 0.1), -12 * RAD, -10 * RAD, _V(-0.15, -0.02, 0), 35 * RAD, 0 * RAD, _V(0.22, -0.17, .1), -15 * RAD, 60 * RAD);
			//
			SetCameraOffset(V3_VC_POV_FO);

			VECTOR3 vDir = _V(0, -0.2, 1); normalise(vDir);
			SetCameraDefaultDirection(vDir);
			oapiCameraSetCockpitDir(0, -5 * RAD);

			oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_MOVESEAT << 16), _V(1, 2.25, 38.7), .6);
		}
		else
		{
			vcPovStation = 0; //											 pilot side
			SetMeshVisibilityMode(dvcModelIdR, MESHVIS_VC);
			SetMeshVisibilityMode(dvcModelIdL, MESHVIS_NEVER);

			SetCameraMovement(_V(0, -0.05, 0.1), 0, -20 * RAD, _V(-0.20, -0.1, 0), 15 * RAD, 45 * RAD, _V(0.15, -0.02, 0), -35 * RAD, 0);

			SetCameraOffset(V3_VC_POV_PILOT);

			oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_MOVESEAT << 16), _V(-1, 2.25, 38.7), .6);

			VECTOR3 vDir = _V(0, -0.15, 1); normalise(vDir);
			SetCameraDefaultDirection(vDir);
			oapiCameraSetCockpitDir(0, 0);
		}
		return true;

	default:
		return false;
	}
}

int G422::clbkConsumeDirectKey(char* kstate)
{
	if (ignoreDirectKey < 15)
	{
		ignoreDirectKey++;
		return false;
	}

	if (KEYDOWN(kstate, OAPI_KEY_MINUS)) 
	{ 
		directCall = true;
		return clbkConsumeBufferedKey(OAPI_KEY_MINUS, true, kstate); 
	}
	else if (KEYDOWN(kstate, OAPI_KEY_EQUALS)) 
	{
		directCall = true;
		return clbkConsumeBufferedKey(OAPI_KEY_EQUALS, true, kstate);
	}

	return false;
}

int G422::clbkConsumeBufferedKey(int key, bool down, char* kstate)
{
	if (!down) return 0;

	if (KEYMOD_SHIFT(kstate)) // SHIFT KEYS
	{
		if (cargoMode)
		{
			switch (key)
			{
			case OAPI_KEY_S:
				// Reset the index if reached the cargo count, otherwise increase the index
				cargoIndex + 1 < ucso->GetAvailableCargoCount() ? cargoIndex++ : cargoIndex = 0;
				return 1;

			case OAPI_KEY_A:
				switch (ucso->AddCargo(cargoIndex, slotIndex))
				{
				case UCSO::Vessel::GRAPPLE_SUCCEEDED:
					message = "Success: The cargo was added.";
					playSound(SFX_BAY_GRAPPLED);
					break;

				case UCSO::Vessel::GRAPPLE_SLOT_CLOSED:
					message = "Error: The payload bay doors are closed.";
					playSound(SFX_BAY_DOORS_CLOSED);
					break;

				case UCSO::Vessel::GRAPPLE_SLOT_OCCUPIED:
					message = "Error: The slot is occupied.";
					playSound(SFX_BAY_SLOT_OCCUPIED);
					break;

				case UCSO::Vessel::GRAPPLE_FAILED:
					message = "Error: Couldn't add the cargo.";
					break;

				default: break;
				}

				timer = 0;
				return 1;

			case OAPI_KEY_G:
				switch (ucso->GrappleCargo(slotIndex))
				{
				case UCSO::Vessel::GRAPPLE_SUCCEEDED:
					message = "Success: The cargo was grappled.";
					playSound(SFX_BAY_GRAPPLED);
					break;

				case UCSO::Vessel::NO_CARGO_IN_RANGE:
					message = "Error: No grappleable cargo in range.";
					playSound(SFX_BAY_GRAPPLE_NORANGE);
					break;

				case UCSO::Vessel::GRAPPLE_SLOT_CLOSED:
					message = "Error: The payload bay doors are closed.";
					playSound(SFX_BAY_DOORS_CLOSED);
					break;

				case UCSO::Vessel::GRAPPLE_SLOT_OCCUPIED:
					message = "Error: The slot is occupied.";
					playSound(SFX_BAY_SLOT_OCCUPIED);
					break;

				case UCSO::Vessel::GRAPPLE_FAILED:
					message = "Error: Couldn't grapple any cargo.";
					playSound(SFX_BAY_GRAPPLE_FAILED);
					break;

				default: break;
				}

				timer = 0;
				return 1;

			case OAPI_KEY_R:
				switch (ucso->ReleaseCargo(slotIndex))
				{
				case UCSO::Vessel::RELEASE_SUCCEEDED:
					message = "Success: The cargo was released.";
					playSound(SFX_BAY_RELEASED);
					break;

				case UCSO::Vessel::NO_EMPTY_POSITION:
					message = "Error: No empty position nearby.";
					break;

				case UCSO::Vessel::RELEASE_SLOT_CLOSED:
					message = "Error: The payload bay doors are closed.";
					playSound(SFX_BAY_DOORS_CLOSED);
					break;

				case UCSO::Vessel::RELEASE_SLOT_EMPTY:
					message = "Error: The slot is empty.";
					playSound(SFX_BAY_SLOT_EMPTY);
					break;

				case UCSO::Vessel::RELEASE_FAILED:
					message = "Error: Couldn't release the cargo.";
					playSound(SFX_BAY_RELEASE_FAILED);
					break;

				default: break;
				}

				timer = 0;
				return 1;

			case OAPI_KEY_P:
				if (ucso->PackCargo()) message = "Success: The nearest cargo was packed.";
				else message = "Error: No packable cargo in range.";

				timer = 0;
				return 1;

			case OAPI_KEY_U:
				if (ucso->UnpackCargo()) message = "Success: The nearest cargo was unpacked.";
				else message = "Error: No unpackable cargo in range.";

				timer = 0;
				return 1;

			case OAPI_KEY_F:
			{
				const char* requiredResource = fuelIndex == 2 ? "oxygen" : "fuel";

				PROPELLANT_HANDLE tankHandle = fuelIndex == 0 ? allMainFuel : fuelIndex == 1 ? asfFuel : oxidizerFuel;

				double requiredMass = GetPropellantMaxMass(tankHandle) - GetPropellantMass(tankHandle);

				if (requiredMass == 0)
				{
					message = "Error: The selected tank is full.";
					timer = 0;
					return 1;
				}

				// Drain the required mass to fill the tank
				double drainedMass = ucso->DrainCargoResource(requiredResource, requiredMass);

				// If no resource cargo is available, drain from the nearest station or unpacked resource
				if (drainedMass == 0) drainedMass = ucso->DrainStationOrUnpackedResource(requiredResource, requiredMass);
				else SetPropellantMass(tankHandle, GetPropellantMass(tankHandle) + drainedMass);

				if (drainedMass > 0)
				{
					sprintf(buffer, "Success: Drained %d kilograms of fuel.", int(round(drainedMass)));
					message = strdup(buffer);
				}
				else message = "Error: Couldn't drain fuel.";

				timer = 0;
				return 1;
			}
			case OAPI_KEY_D:
				switch (ucso->DeleteCargo(slotIndex))
				{
				case UCSO::Vessel::RELEASE_SUCCEEDED:
					message = "Success: The cargo was deleted.";
					break;

				case UCSO::Vessel::RELEASE_SLOT_EMPTY:
					message = "Error: The slot is empty.";
					playSound(SFX_BAY_SLOT_EMPTY);
					break;

				case UCSO::Vessel::RELEASE_FAILED:
					message = "Error: The deletion failed.";
					break;

				default: break;
				}

				timer = 0;
				return 1;

			default:
				return 0;
			}
		}
		else
		{
		VECTOR3 V0 = _V0;
		switch (key)
		{
		case OAPI_KEY_E:
		{
			int leftIgnIndex = vcSwitchIndexByMGID[MGID_SW3_EMAINL_IGN];
			int rightIgnIndex = vcSwitchIndexByMGID[MGID_SW3_EMAINR_IGN];

			if (engMainL.state == RT66::SST_RUN_EXT || engMainL.state == RT66::SST_RUN_INT)
			{
				clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (leftIgnIndex & 0xFFFF), PANEL_MOUSE_RBDOWN, V0);
				clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (leftIgnIndex & 0xFFFF), PANEL_MOUSE_RBDOWN, V0);
			}
			else
			{
				clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (leftIgnIndex & 0xFFFF), PANEL_MOUSE_LBDOWN, V0);
				clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (leftIgnIndex & 0xFFFF), PANEL_MOUSE_LBDOWN, V0);
				clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (leftIgnIndex & 0xFFFF), PANEL_MOUSE_LBUP, V0);
			}

			if (engMainR.state == RT66::SST_RUN_EXT || engMainR.state == RT66::SST_RUN_INT)
			{
				clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (rightIgnIndex & 0xFFFF), PANEL_MOUSE_RBDOWN, V0);
				clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (rightIgnIndex & 0xFFFF), PANEL_MOUSE_RBDOWN, V0);
			}
			else
			{
				clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (rightIgnIndex & 0xFFFF), PANEL_MOUSE_LBDOWN, V0);
				clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (rightIgnIndex & 0xFFFF), PANEL_MOUSE_LBDOWN, V0);
				clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (rightIgnIndex & 0xFFFF), PANEL_MOUSE_LBUP, V0);
			}

			return 1;
		}
		case OAPI_KEY_B:
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_EMAINL_REHEAT] & 0xFFFF), engMainL.burnerToggle ? PANEL_MOUSE_RBDOWN : PANEL_MOUSE_LBDOWN, V0);
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_EMAINR_REHEAT] & 0xFFFF), engMainR.burnerToggle ? PANEL_MOUSE_RBDOWN : PANEL_MOUSE_LBDOWN, V0);

			return 1;

		case OAPI_KEY_P:
			clbkVCMouseEvent((VC_CTRLSET_PRK_BRK_HNDL << 16), parkingBrakeMode ? PANEL_MOUSE_RBDOWN : PANEL_MOUSE_LBDOWN, V0);
			return 1;

		case OAPI_KEY_C:
			if (ucsoInstalled) cargoMode = !cargoMode;
			else timer = 0;

			return 1;

		default:
			return 0;
		}
		}
	}
	else
	{
		if (key == OAPI_KEY_MINUS || key == OAPI_KEY_EQUALS)
		{
			if (directCall) directCall = false;
			else if (ignoreDirectKey)
			{
				if (ignoreDirectKey >= 15)
				{
					ignoreDirectKey = 0;
					return 1;
				}

				ignoreDirectKey = 0;
			}

			if (hydSysA.hydPrs < 2800 && hydSysB.hydPrs < 2800)
			{
				playSound(SFX_WARN_APUOFF);
				return 1;
			}
		}

		VECTOR3 V0 = _V0;
		switch (key)
		{
		case OAPI_KEY_G:
		{
			// Landing gear is reversed
			int mouseEvent = landingGear->getToggleState() ? PANEL_MOUSE_LBDOWN : PANEL_MOUSE_RBDOWN;
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_GEAR] & 0xFFFF), mouseEvent, V0);
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_GEAR] & 0xFFFF), mouseEvent, V0);
			return 1;
		}
		case OAPI_KEY_V:
		{
			int mouseEvent = visor->getToggleState() ? PANEL_MOUSE_RBDOWN : PANEL_MOUSE_LBDOWN;
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_VISOR] & 0xFFFF), mouseEvent, V0);
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_VISOR] & 0xFFFF), mouseEvent, V0);
			return 1;
		}
		case OAPI_KEY_N:
		{
			int mouseEvent = canards->getToggleState() ? PANEL_MOUSE_RBDOWN : PANEL_MOUSE_LBDOWN;
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_CANARD] & 0xFFFF), mouseEvent, V0);
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_CANARD] & 0xFFFF), mouseEvent, V0);
			return 1;
		}
		case OAPI_KEY_B:
		{
			int mouseEvent = bayDoors->getToggleState() ? PANEL_MOUSE_RBDOWN : PANEL_MOUSE_LBDOWN;
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_BAY_OPENCLSE] & 0xFFFF), mouseEvent, V0);
			clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW3_BAY_OPENCLSE] & 0xFFFF), mouseEvent, V0);
			return 1;
		}
		case OAPI_KEY_TAB:
			if (vcPovStation == 0)
			{
				vcPovStation = 1;  //											engineer side

				SetMeshVisibilityMode(dvcModelIdR, MESHVIS_NEVER);
				SetMeshVisibilityMode(dvcModelIdL, MESHVIS_VC);

				//                   LEAN FORWARD (FMC)                     LEAN LEFT (WINDOW)                  LEAN RIGHT (OVERHEAD)
				SetCameraMovement(_V(0.1, -0.15, 0.1), -12 * RAD, -10 * RAD, _V(-0.15, -0.02, 0), 35 * RAD, 0 * RAD, _V(0.22, -0.17, .1), -15 * RAD, 60 * RAD);
				//
				SetCameraOffset(V3_VC_POV_FO);

				VECTOR3 vDir = _V(0, -0.2, 1); normalise(vDir);
				SetCameraDefaultDirection(vDir);
				oapiCameraSetCockpitDir(0, -5 * RAD);

				oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_MOVESEAT << 16), _V(1, 2.25, 38.7), .6);
			}
			else
			{
				vcPovStation = 0; //											 pilot side
				SetMeshVisibilityMode(dvcModelIdR, MESHVIS_VC);
				SetMeshVisibilityMode(dvcModelIdL, MESHVIS_NEVER);

				SetCameraMovement(_V(0, -0.05, 0.1), 0, -20 * RAD, _V(-0.20, -0.1, 0), 15 * RAD, 45 * RAD, _V(0.15, -0.02, 0), -35 * RAD, 0);

				SetCameraOffset(V3_VC_POV_PILOT);

				oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_MOVESEAT << 16), _V(-1, 2.25, 38.7), .6);

				VECTOR3 vDir = _V(0, -0.15, 1); normalise(vDir);
				SetCameraDefaultDirection(vDir);
				oapiCameraSetCockpitDir(0, 0);
			}

			return 1;

		case OAPI_KEY_MINUS:
			if (cgPos > -4)
			{
				cgPos -= 0.1;
				EditAirfoil(wingLift, 0x01, _V(0, 0, -cgPos), NULL, 0, 0, 0);
			}

			return 1;

		case OAPI_KEY_EQUALS:
			if (cgPos < 4)
			{
				cgPos += 0.1;
				EditAirfoil(wingLift, 0x01, _V(0, 0, -cgPos), NULL, 0, 0, 0);
			}

			return 1;

		case OAPI_KEY_BACK:
			if (hydSysA.hydPrs < 2800 && hydSysB.hydPrs < 2800)
			{
				playSound(SFX_WARN_APUOFF);
				return 1;
			}

			cgPos = 0;
			EditAirfoil(wingLift, 0x01, _V(0, 0, 0), NULL, 0, 0, 0);

			return 1;

		default:
			break;
		}

		if (cargoMode)
		{
			switch (key)
			{
			case OAPI_KEY_S:
				slotIndex < 7 ? slotIndex++ : slotIndex = 0;
				return 1;

			case OAPI_KEY_F:
				fuelIndex < 2 ? fuelIndex++ : fuelIndex = 0;
				return 1;

			default:
				break;
			}
		}
	}

	return 0;
}