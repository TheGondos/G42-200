#include "..\Header\G422.h"
#include "..\Header\G422_MDL_DVC.h"
#include "..\Header\G422_DVC.h"

void G422::clbkRCSMode(int mode)
{
	VCKnob& rcsKnob = VCKnobs[VC_kbIndexByMGID[MGID_KB3_RCS_MODE]];

	switch (mode)
	{
	case RCS_NONE:
		rcsKnob.setPos(KB3_DOWN, this);
		break;
	case RCS_ROT:
		rcsKnob.setPos(KB3_MID, this);
		break;
	case RCS_LIN:
		rcsKnob.setPos(KB3_UP, this);
		break;
	default:
		break;
	}
}

bool G422::clbkVCMouseEvent(int id, int ev, VECTOR3& p)
{
	int ctrlSet = HIWORD(id);
	int ctrlID = LOWORD(id);

	switch (ctrlSet)
	{
	case VC_CTRLSET_MFDK:
	{
		bool op = oapiProcessMFDButton(ctrlID / 12, ctrlID % 12, ev);
		static bool hold = false;
		if (!hold)
		{
			PlayVesselWave(SFXID, SFX_VC_POP);
			hold = true;
		}
		else if (ev & PANEL_MOUSE_UP) hold = false;
	}
	return true;

	case VC_CTRLSET_MFDC:
		switch (ctrlID % 3)
		{
		case 0:  // select keys...
			PlayVesselWave(SFXID, SFX_VC_POP);
			oapiSendMFDKey(ctrlID / 3, OAPI_KEY_F1);
			return true;

		case 1: // power buttons...
			PlayVesselWave(SFXID, SFX_VC_POP);
			oapiSendMFDKey(ctrlID / 3, OAPI_KEY_ESCAPE);
			return true;

		case 2: // menu keys...
			PlayVesselWave(SFXID, SFX_VC_POP);
			oapiSendMFDKey(ctrlID / 3, OAPI_KEY_GRAVE);
			return true;
		}

	case VC_CTRLSET_EICAS_C1:
	case VC_CTRLSET_EICAS_C2:
	{
		UINT btn = (UINT)(p.x * 6.0) + ((p.y > .5) ? 7 : 1);

		PlayVesselWave(SFXID, SFX_VC_POP);
		selEicasMode((ctrlSet == VC_CTRLSET_EICAS_C1) ? 0 : 1, btn);

		return true;
	}

	case VC_CTRLSET_EICAS_P1L:
	case VC_CTRLSET_EICAS_P1R:
	{
		UINT btn = (UINT)(p.y * 6.0) + ((ctrlSet == VC_CTRLSET_EICAS_P1L) ? 1 : 7);

		PlayVesselWave(SFXID, SFX_VC_POP);
		selEicasMode(2, btn);

		return true;
	}
	case VC_CTRLSET_EICAS_P2L:
	case VC_CTRLSET_EICAS_P2R:
	{
		UINT btn = (UINT)(p.y * 6.0) + ((ctrlSet == VC_CTRLSET_EICAS_P2L) ? 1 : 7);

		PlayVesselWave(SFXID, SFX_VC_POP);
		selEicasMode(3, btn);

		return true;
	}

	case VC_CTRLSET_EICAS_E1L:
	case VC_CTRLSET_EICAS_E1R:
	{
		UINT btn = (UINT)(p.y * 6.0) + ((ctrlSet == VC_CTRLSET_EICAS_E1R) ? 1 : 7);

		PlayVesselWave(SFXID, SFX_VC_POP);
		selEicasMode(4, btn);

		return true;
	}
	case VC_CTRLSET_EICAS_E2L:
	case VC_CTRLSET_EICAS_E2R:
	{
		UINT btn = (UINT)(p.y * 6.0) + ((ctrlSet == VC_CTRLSET_EICAS_E2R) ? 1 : 7);

		PlayVesselWave(SFXID, SFX_VC_POP);
		selEicasMode(5, btn);

		return true;
	}

	case VC_CTRLSET_EICAS_PWR:
	{
		PlayVesselWave(SFXID, SFX_VC_POP);
		selEicasMode(ctrlID, (ev == PANEL_MOUSE_LBDOWN) ? abs(VC_eicas_screens[ctrlID]) : -abs(VC_eicas_screens[ctrlID]));
		return true;

	}

	case VC_CTRLSET_SWITCHES:
	{
		VCSwitch& sw = VCSwitches[ctrlID];

		if (ev != PANEL_MOUSE_IGNORE)
		{
			if (ev & (PANEL_MOUSE_LBUP | PANEL_MOUSE_RBUP)) // spring-loaded switches revert when released
			{
				switch (sw.mgid)
				{
				case MGID_SW3_RAMX_IGN:
				case MGID_SW3_EMAIN_IGN:
				case MGID_SW3_APU_PACK_A:
				case MGID_SW3_APU_PACK_B:
					// the above switches are spring-loaded only on top position...
					if (sw.pos != SW3_UP) return false;

				case MGID_SW3_EMAIN_MODE:
				case MGID_SW3_RAMX_MODE:
					// 
					sw.setPos(SW3_MID, this);
					PlayVesselWave(SFXID, SFX_VC_TICK);
					return true;
				}

				return false;
			}
			else if (ev & PANEL_MOUSE_RBPRESSED)
			{
				if (!sw.flick(false, this)) return false;

				PlayVesselWave(SFXID, SFX_VC_TICK);
				PlayVesselWave(SFXID, SFX_VC_AFLIK);
			}
			else 
			{
				if (!sw.flick(ev == PANEL_MOUSE_LBDOWN, this))
				{
					PlayVesselWave(SFXID, SFX_VC_POP);
					return false;
				}

				PlayVesselWave(SFXID, SFX_VC_FLICK);
			}
		}

		switch (sw.mgid)
		{
		case MGID_SW3_GEAR:
		{
			if (sw.pos == SW3_UP && !gear_mode) landingGears->toggle(false); // gears are backwards (false means retract)
			else if (sw.pos == SW3_DOWN && !gear_mode) landingGears->toggle(true);
			return true;
		}
		case MGID_SW3_CANARD:
		{
			if (sw.pos == SW3_UP) canards->toggle(true);
			else if (sw.pos == SW3_DOWN) canards->toggle(false);
			return true;
		}
		case MGID_SW3_VISOR:
		{
			if (sw.pos == SW3_UP) visor->toggle(true);
			else if (sw.pos == SW3_DOWN) visor->toggle(false);
			return true;
		}
		case MGID_SW3_RCS:
		{
			if (sw.pos == SW3_UP) rcsDoors->toggle(true);
			else if (sw.pos == SW3_DOWN) rcsDoors->toggle(false);
			return true;
		}
		case MGID_SW3_BAY_OPENCLSE:
		{
			if (sw.pos == SW3_UP) bayDoors->toggle(true);
			else if (sw.pos == SW3_DOWN) bayDoors->toggle(false);
			return true;
		}
		case MGID_SW3_EMAIN_MODE: // mode switch
			if (sw.pos == SW3_UP) { cueEngines(engMain_L, RT66::ENG_INLT_OPEN); cueEngines(engMain_R, RT66::ENG_INLT_OPEN); }
			else if (sw.pos == SW3_DOWN) { cueEngines(engMain_L, RT66::ENG_INLT_CLSE); cueEngines(engMain_R, RT66::ENG_INLT_CLSE); }
			return true;

		case MGID_SW3_EMAIN_REHEAT:
			if (sw.pos == SW2_UP) burner_toggle = 1;
			else burner_toggle = 0;
			return true;

			// this is a bit kludge-ish... the ideal thing would be to simulate turbine RPM or something
			// but for now this should kinda do the trick....
		case MGID_SW3_EMAIN_IGN:
			if (sw.pos == SW3_UP) { cueEngines(engMain_L, RT66::ENG_SPOOLUP); cueEngines(engMain_R, RT66::ENG_SPOOLUP); }
			else if (sw.pos == SW3_DOWN) { cueEngines(engMain_L, RT66::ENG_SPOOLDOWN); cueEngines(engMain_R, RT66::ENG_SPOOLDOWN); }
			return true;

		case MGID_SW3_RAMX_IGN:
			if (sw.pos == SW3_UP) cueEngines(engRamx, RAMCASTER::ENG_START);
			else if (sw.pos == SW3_DOWN) cueEngines(engRamx, RAMCASTER::ENG_STOP);
			return true;

		case MGID_SW3_RAMX_DOOR:
			if (sw.pos == SW3_UP) ramxDoors->toggle(true);
			else if (sw.pos == SW3_DOWN) ramxDoors->toggle(false);
			return true;

		case MGID_SW3_RAMX_MODE:
			if (!ramcaster_mode) return true;  // off...

			if (sw.pos == SW3_UP) cueEngines(engRamx, RAMCASTER::ENG_RUNHI);
			else if (sw.pos == SW3_DOWN) cueEngines(engRamx, RAMCASTER::ENG_RUNLO);
			return true;

		case MGID_SW2_THR_AUTH:
			if (sw.pos == SW2_UP) thr_authority = 0;
			else thr_authority = 1;
			return true;

		case MGID_SW2_MSFEED_R:
			if (sw.pos == SW2_UP) { engMain_R.feed |= RT66::FUEL_PUMP; PlayVesselWave(SFXID, SFX_FUELPUMP_MAIN, LOOP); }
			else
			{
				engMain_R.feed &= ~RT66::FUEL_PUMP;
				if (VCSwitches[VC_swIndexByMGID[MGID_SW2_MSFEED_L]].pos == SW2_DOWN)
					StopVesselWave(SFXID, SFX_FUELPUMP_MAIN);
			}
			return true;

		case MGID_SW2_MSFEED_L:
			if (sw.pos == SW2_UP) { engMain_L.feed |= RT66::FUEL_PUMP; PlayVesselWave(SFXID, SFX_FUELPUMP_MAIN, LOOP); }
			else
			{
				engMain_L.feed &= ~RT66::FUEL_PUMP;
				if (VCSwitches[VC_swIndexByMGID[MGID_SW2_MSFEED_R]].pos == SW2_DOWN) StopVesselWave(SFXID, SFX_FUELPUMP_MAIN);
			}
			return true;

		case MGID_SW2_MSFEED_C:
			if (sw.pos == SW2_UP) PlayVesselWave(SFXID, SFX_FUELPUMP_RAMX, LOOP);
			else StopVesselWave(SFXID, SFX_FUELPUMP_RAMX);
			return true;

		case MGID_SW3_OXYFEED_R:
			if (sw.pos == SW2_UP) { engMain_R.feed |= RT66::OXY_PUMP; PlayVesselWave(SFXID, SFX_FUELPUMP_OXY, LOOP); }
			else
			{
				engMain_R.feed &= ~RT66::OXY_PUMP;
				if (VCSwitches[VC_swIndexByMGID[MGID_SW3_OXYFEED_L]].pos == SW2_DOWN) StopVesselWave(SFXID, SFX_FUELPUMP_OXY);
			}
			return true;

		case MGID_SW3_OXYFEED_L:
			if (sw.pos == SW2_UP) { engMain_L.feed |= RT66::OXY_PUMP; PlayVesselWave(SFXID, SFX_FUELPUMP_OXY, LOOP); }
			else
			{
				engMain_L.feed &= ~RT66::OXY_PUMP;
				if (VCSwitches[VC_swIndexByMGID[MGID_SW3_OXYFEED_R]].pos == SW2_DOWN) StopVesselWave(SFXID, SFX_FUELPUMP_OXY);
			}
			return true;

		case MGID_SW2_SYSFEED_APU:
			if (sw.pos == SW2_UP)
			{
				apuPackA.feed |= APU::FUEL_PUMP;
				apuPackB.feed |= APU::FUEL_PUMP;
				PlayVesselWave(SFXID, SFX_FUELPUMP_APU, LOOP);
			}
			else
			{
				apuPackA.feed &= ~APU::FUEL_PUMP;
				apuPackB.feed &= ~APU::FUEL_PUMP;
				StopVesselWave(SFXID, SFX_FUELPUMP_APU);
			}
			return true;

		case MGID_SW2_SYSFEED_RCS:
			if (sw.pos == SW2_UP) { rcs.feed |= RCS::FUEL_PUMP; PlayVesselWave(SFXID, SFX_FUELPUMP_RCS, LOOP); }
			else { rcs.feed &= ~RCS::FUEL_PUMP; StopVesselWave(SFXID, SFX_FUELPUMP_RCS); }
			return true;

		case MGID_SW2_SYSFEED_OMS:
			if (sw.pos == SW2_UP) { oms.feed |= OMS::FUEL_PUMP; PlayVesselWave(SFXID, SFX_FUELPUMP_OMS, LOOP);}
			else { oms.feed &= ~OMS::FUEL_PUMP; StopVesselWave(SFXID, SFX_FUELPUMP_OMS); }
			return true;

		case MGID_SW3_APU_PACK_A:
			if (sw.pos == SW3_UP) cueEngines(apuPackA, APU::ENG_START);
			else if (sw.pos == SW3_DOWN) cueEngines(apuPackA, APU::ENG_STOP);
			return true;

		case MGID_SW3_APU_PACK_B:
			if (sw.pos == SW3_UP) cueEngines(apuPackB, APU::ENG_START);
			else if (sw.pos == SW3_DOWN) cueEngines(apuPackB, APU::ENG_STOP);
			return true;

		case MGID_SW3_APU_HYD:
			if (sw.pos == SW3_UP) { apuPackA.hydFeed = apuPackB.hydFeed = APU::HYD_PUMP; }
			else if (sw.pos == SW3_MID) { apuPackA.hydFeed = apuPackB.hydFeed = APU::HYD_AUTO; }
			else { apuPackA.hydFeed = apuPackB.hydFeed = APU::HYD_OFF; }
			return true;

		case MGID_SW3_STBYIGN_RCS:
			if (sw.pos == SW2_UP) cueEngines(rcs, RCS::SST_STBY);
			else cueEngines(rcs, RCS::SST_INOP);
			return true;

		case MGID_SW3_STBYIGN_OMS:
			if (sw.pos == SW2_UP) cueEngines(oms, OMS::SST_STBY);
			else cueEngines(oms, OMS::SST_INOP);
			return true;

		case MGID_SW2_APU_RAMX:
			if (sw.pos == SW2_UP) engRamx.feed |= RAMCASTER::FUEL_PUMP;
			else engRamx.feed &= ~RAMCASTER::FUEL_PUMP;
			return true;

		case MGID_SW2_LGT_NAV:
			nav_white.active = nav_red.active = nav_green.active = sw.pos == SW2_UP;
			return true;

		case MGID_SW2_LGT_STB:
			stb_t.active = stb_l.active = stb_r.active = sw.pos == SW2_UP;
			return true;

		case MGID_SW2_LGT_BCN:
			bcn1.active = bcn2.active = sw.pos == SW2_UP;
			return true;

		case MGID_SW2_LGT_LND:
			landingLight->Activate(sw.pos == SW2_UP);
			return true;

		case MGID_SW2_LGT_TAX:
			taxiLight->Activate(sw.pos == SW2_UP);
			return true;

		case MGID_SW2_LGT_DCK:
			dockingLight->Activate(sw.pos == SW2_UP);
			return true;

		case MGID_SW2_LEFT_DUMP:
			fuelDump.dumpLeft = sw.pos == SW2_UP;
			return true;

		case MGID_SW2_FWD_DUMP:
			fuelDump.dumpFwd = sw.pos == SW2_UP;
			return true;

		case MGID_SW2_AFT_DUMP:
			fuelDump.dumpAft = sw.pos == SW2_UP;
			return true;

		case MGID_SW2_RIGHT_DUMP:
			fuelDump.dumpRight = sw.pos == SW2_UP;
			return true;

		case MGID_SW2_ASF_DUMP:
			fuelDump.dumpAsf = sw.pos == SW2_UP;
			return true;

		case MGID_SW2_OXY_DUMP:
			fuelDump.dumpOxy = sw.pos == SW2_UP;
			return true;

		case MGID_SW2_MASTER_DUMP:
			if (sw.pos == SW2_UP) { fuelDump.dumpingFuel = true; fuelDump.masterDump = true; PlayVesselWave(SFXID, SFX_FUEL_DUMP); }
			else fuelDump.masterDump = false;
			return true;

		case MGID_SW2_DCKSPL_FUEL:
			if (sw.pos == SW2_UP) { dockSupply.supplyingFuel = true; dockSupply.supplyFuel = true; }
			else dockSupply.supplyFuel = false;
			return true;

		case MGID_SW2_DCKSPL_OXY:
			if (sw.pos == SW2_UP) { dockSupply.supplyingOxy = true; dockSupply.supplyOxy = true; }
			else dockSupply.supplyOxy = false;
			return true;

		case MGID_SW2_DCKSPL_ASF:
			if (sw.pos == SW2_UP) { dockSupply.supplyingAsf = true; dockSupply.supplyAsf = true; }
			else dockSupply.supplyAsf = false;
			return true;
		}

		return true; // whatever else, just flick it! (faux switch)
	}
	case VC_CTRLSET_KNOBS:
	{
		VCKnob& kb = VCKnobs[ctrlID];

		if (ev != PANEL_MOUSE_IGNORE)
		{
			if (!kb.flick(ev == PANEL_MOUSE_LBDOWN, this))
			{
				PlayVesselWave(SFXID, SFX_VC_POP);
				return false;
			}

			PlayVesselWave(SFXID, SFX_VC_FLICK);
		}

		switch (kb.mgid)
		{
		case MGID_KB3_RCS_MODE:
			switch (kb.pos)
			{
			case KB3_UP:
				SetAttitudeMode(RCS_LIN);
				break;
			case KB3_MID:
				SetAttitudeMode(RCS_ROT);
				break;
			case KB3_DOWN:
				SetAttitudeMode(RCS_NONE);
				break;
			}
			break;

		case MGID_KB3_ACS_MODE:
			if (hydSysA.hydPrs < 2800 && hydSysB.hydPrs < 2800) { PlayVesselWave(SFXID, SFX_VC_POP); kb.setPos(KB3_DOWN, this); break; }
			switch (kb.pos)
			{
			case KB3_UP:
				SetADCtrlMode(7);
				break;
			case KB3_MID:
				SetADCtrlMode(1);
				break;
			case KB3_DOWN:
				SetADCtrlMode(0);
				break;
			}
			break;

		case MGID_KB2_COORD_REF:
			switch (kb.pos)
			{
			case KB2_UP:
				rcs.mode = true;
				if (rcs.state == RCS::SST_STBY) { cueEngines(rcs, RCS::SST_STBY); PlayVesselWave(SFXID, SFX_RCS_DOCKING); }
				break;
			case KB2_DOWN:
				rcs.mode = false;
				if (rcs.state == RCS::SST_STBY) { cueEngines(rcs, RCS::SST_STBY); PlayVesselWave(SFXID, SFX_RCS_NORMAL); }
				break;
			}
			break;

		case MGID_KB2_LENG_FEED:
			switch (kb.pos)
			{
			case KB2_UP:
				engMain_L.feed |= RT66::FUEL_OPEN;
				break;
			case KB2_DOWN:
				engMain_L.feed &= ~RT66::FUEL_OPEN;
				break;
			}
			break;

		case MGID_KB2_RAMX_FEED:
			switch (kb.pos)
			{
			case KB2_UP:
				engRamx.feed |= RAMCASTER::FUEL_OPEN;
				break;
			case KB2_DOWN:
				engRamx.feed &= ~RAMCASTER::FUEL_OPEN;
				break;
			}
			break;

		case MGID_KB2_RENG_FEED:
			switch (kb.pos)
			{
			case KB2_UP:
				engMain_R.feed |= RT66::FUEL_OPEN;
				break;
			case KB2_DOWN:
				engMain_R.feed &= ~RT66::FUEL_OPEN;
				break;
			}
			break;

		case MGID_KB2_RCS_FEED:
			switch (kb.pos)
			{
			case KB2_UP:
				rcs.feed |= RCS::FUEL_OPEN;
				break;
			case KB2_DOWN:
				rcs.feed &= ~RCS::FUEL_OPEN;
				break;
			}
			break;

		case MGID_KB2_OMS_FEED:
			switch (kb.pos)
			{
			case KB2_UP:
				oms.feed |= OMS::FUEL_OPEN;
				break;
			case KB2_DOWN:
				oms.feed &= ~OMS::FUEL_OPEN;
				break;
			}
			break;

		case MGID_KB2_APU_FEED:
			switch (kb.pos)
			{
			case KB2_UP:
				apuPackA.feed |= APU::FUEL_OPEN;
				apuPackB.feed |= APU::FUEL_OPEN;
				break;
			case KB2_DOWN:
				apuPackA.feed &= ~APU::FUEL_OPEN;
				apuPackB.feed &= ~APU::FUEL_OPEN;
				break;
			}
			break;

		case MGID_KB2_LOXY_FEED:
			switch (kb.pos)
			{
			case KB2_UP:
				engMain_L.feed |= RT66::OXY_OPEN;
				break;
			case KB2_DOWN:
				engMain_L.feed &= ~RT66::OXY_OPEN;
				break;
			}
			break;

		case MGID_KB2_ROXY_FEED:
			switch (kb.pos)
			{
			case KB2_UP:
				engMain_R.feed |= RT66::OXY_OPEN;
				break;
			case KB2_DOWN:
				engMain_R.feed &= ~RT66::OXY_OPEN;
				break;
			}
			break;
		}

		return true;
	}
	case VC_CTRLSET_BUTTONS:
	{
		PlayVesselWave(SFXID, SFX_VC_POP);

		switch (VC_btDefs[ctrlID].bid)
		{
		case AP_MASTER_BUTTON:
			apSet = !apSet;
			break;
		case AP_PROGRADE_BUTTON:
			apSet = false;
			ToggleNavmode(NAVMODE_PROGRADE);
			break;
		case AP_RETROGRADE_BUTTON:
			apSet = false;
			ToggleNavmode(NAVMODE_RETROGRADE);
			break;
		case AP_NORMAL_BUTTON:
			apSet = false;
			ToggleNavmode(NAVMODE_NORMAL);
			break;
		case AP_ANTINORMAL_BUTTON:
			apSet = false;
			ToggleNavmode(NAVMODE_ANTINORMAL);
			break;
		case AP_HLEVEL_BUTTON:
			apSet = false;
			ToggleNavmode(NAVMODE_HLEVEL);
			break;
		case AP_KILLROT_BUTTON:
			apSet = false;
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
			if(ev & PANEL_MOUSE_RBDOWN) oapiSetHUDIntensity(min(oapiGetHUDIntensity() + 0.1, 1));
			else oapiSetHUDIntensity(max(oapiGetHUDIntensity() - 0.1, 0));
			break;
		case HUD_CLR_BUTTON:
			oapiToggleHUDColour();
			break;

		case MASTER_WARN_BUTTON:
			warningSys.inhibit = true;
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
				PlayVesselWave(SFXID, SFX_VC_CLAK);

				SetEngineLevel(ENGINE_HOVER, 0.5);

				oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_RAMXLVR << 16), V3_VC_RAMX_ENG, .035);
			}
			else if (level < 1)
			{
				PlayVesselWave(SFXID, SFX_VC_CLAK);

				SetEngineLevel(ENGINE_HOVER, 1);

				oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_RAMXLVR << 16), V3_VC_RAMX_FULL, .035);
			}
		}
		else
		{
			if (level == 1)
			{
				PlayVesselWave(SFXID, SFX_VC_CLAK);

				SetEngineLevel(ENGINE_HOVER, 0.5);

				oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_RAMXLVR << 16), V3_VC_RAMX_ENG, .035);
			}
			else if (level >= 0.5)
			{
				PlayVesselWave(SFXID, SFX_VC_CLAK);

				SetEngineLevel(ENGINE_HOVER, 0);

				oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_RAMXLVR << 16), V3_VC_RAMX_IDLE, .035);
			}
		}
		return true;
	}
	case VC_CTRLSET_WPOSLVR:
		if (ev == PANEL_MOUSE_LBDOWN) { if (wingPos < 1) ++wingPos; }
		else { if (wingPos > -1) --wingPos; }

		PlayVesselWave(SFXID, SFX_VC_CLAK);
		StopVesselWave(SFXID, SFX_WINGMTR);

		switch (wingPos)
		{
		case 0:
			wingTipWvrd->toggle(false);
			wingTipFthr->toggle(false);
			SetAnimation(an_dvc_wpos, 0.5);
			oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_WPOSLVR << 16), V3_VC_WPOS_MID, .035);
			PlayVesselWave(SFXID, SFX_WINGMTR);
			break;
		case -1:
			wingTipWvrd->toggle(true);
			SetAnimation(an_dvc_wpos, 0);
			oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_WPOSLVR << 16), V3_VC_WPOS_AFT, .035);
			PlayVesselWave(SFXID, SFX_WINGMTR);
			break;
		case 1:
			wingTipFthr->toggle(true);
			SetAnimation(an_dvc_wpos, 1);
			oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_WPOSLVR << 16), V3_VC_WPOS_FWD, .035);
			PlayVesselWave(SFXID, SFX_WINGMTR);
			break;
		}
		return true;

	case VC_CTRLSET_PRK_BRK_HNDL:
		if (ev == PANEL_MOUSE_LBDOWN && prk_brake_mode)
		{
			PlayVesselWave(SFXID, SFX_VC_CLAK);
			prk_brake_mode = false;
			SetAnimation(an_dvc_prk, 0);
		}
		else if (ev == PANEL_MOUSE_RBDOWN && !prk_brake_mode)
		{
			PlayVesselWave(SFXID, SFX_VC_CLAK);
			prk_brake_mode = true;
			SetAnimation(an_dvc_prk, 1);
		}
		return true;

	case VC_CTRLSET_GEAR_EMER_HNDL:
		if (!gear_mode)
		{
			PlayVesselWave(SFXID, SFX_VC_CLAK);
			gear_mode = true;
			SetAnimation(an_dvc_gear, 1);
			if(landingGears->pos < 1) landingGears->toggle();
		}
		return true;

	case VC_CTRLSET_UNDOCK_HNDL:
		if (ev & PANEL_MOUSE_LBUP) { SetAnimation(an_dvc_undock, 0); return true; }

		PlayVesselWave(SFXID, SFX_VC_CLAK);
		SetAnimation(an_dvc_undock, 1);
		Undock(0);
		return true;

	case VC_CTRLSET_MOVESEAT:
		clbkConsumeBufferedKey(OAPI_KEY_TAB, true, 0);
		return true;

	default:
		return false;
	}
}

// --------------------------------------------------------------
// Process buffered key events
// --------------------------------------------------------------
int G422::clbkConsumeBufferedKey(DWORD key, bool down, char* kstate)
{
	if (!down) return 0;

	if (kstate)
	{
		if (KEYMOD_SHIFT(kstate)) // SHIFT KEYS
		{
			switch (key)
			{
			case OAPI_KEY_B:
				if (burner_toggle)
				{
					VCSwitches[VC_swIndexByMGID[MGID_SW3_EMAIN_REHEAT]].setPos(SW2_DOWN, this);
					burner_toggle = 0;
				}
				else
				{
					VCSwitches[VC_swIndexByMGID[MGID_SW3_EMAIN_REHEAT]].setPos(SW2_UP, this);
					burner_toggle = 1;
				}
				return 1;

			case OAPI_KEY_P:
				SetAnimation(an_dvc_prk, prk_brake_mode);
				prk_brake_mode = !prk_brake_mode;
				return 1;

			case OAPI_KEY_S:
				if (!ucsoInstalled) { timer = 0; return 1; }

				// Reset the index if reached the cargo count, otherwise increase the index
				cargoIndex < ucso->GetAvailableCargoCount() - 1 ? cargoIndex++ : cargoIndex = 0;

				sprintf(buffer, "Selected cargo to add: %s", ucso->GetAvailableCargoName(cargoIndex));
				message = _strdup(buffer);

				timer = 0;
				return 1;

			case OAPI_KEY_A:
				if (!ucsoInstalled) { timer = 0; return 1; }

				switch (ucso->AddCargo(cargoIndex, slotIndex))
				{
				case UCSO::Vessel::GRAPPLE_SUCCEEDED:
					message = "Success: The cargo is added.";
					PlayVesselWave(SFXID, SFX_BAY_GRAPPLED);
					break;
				case UCSO::Vessel::GRAPPLE_SLOT_CLOSED:
					message = "Error: The slot is closed.";
					PlayVesselWave(SFXID, SFX_BAY_DOORS_CLOSED);
					break;
				case UCSO::Vessel::GRAPPLE_SLOT_OCCUPIED:
					message = "Error: The slot is occupied.";
					PlayVesselWave(SFXID, SFX_BAY_SLOT_OCCUPIED);
					break;
				case UCSO::Vessel::GRAPPLE_FAILED:
					message = "Error: Couldn't add the cargo.";
					break;
				default: break;
				}
				timer = 0;
				return 1;

			case OAPI_KEY_G:
				if (!ucsoInstalled) { timer = 0; return 1; }

				switch (ucso->GrappleCargo(slotIndex))
				{
				case UCSO::Vessel::GRAPPLE_SUCCEEDED:
					message = "Success: The cargo is grappled.";
					PlayVesselWave(SFXID, SFX_BAY_GRAPPLED);
					break;
				case UCSO::Vessel::NO_CARGO_IN_RANGE:
					message = "Error: No grappleable cargo in range.";
					PlayVesselWave(SFXID, SFX_BAY_GRAPPLE_NORANGE);
					break;
				case UCSO::Vessel::GRAPPLE_SLOT_CLOSED:
					message = "Error: The slot is closed.";
					PlayVesselWave(SFXID, SFX_BAY_DOORS_CLOSED);
					break;
				case UCSO::Vessel::GRAPPLE_SLOT_OCCUPIED:
					message = "Error: The slot is occupied.";
					PlayVesselWave(SFXID, SFX_BAY_SLOT_OCCUPIED);
					break;
				case UCSO::Vessel::GRAPPLE_FAILED:
					message = "Error: Couldn't grapple any cargo.";
					break;
				default: break;
				}
				timer = 0;
				return 1;

			case OAPI_KEY_R:
				if (!ucsoInstalled) { timer = 0; return 1; }

				switch (ucso->ReleaseCargo(slotIndex))
				{
				case UCSO::Vessel::RELEASE_SUCCEEDED:
					message = "Success: The cargo is released.";
					PlayVesselWave(SFXID, SFX_BAY_RELEASED);
					break;
				case UCSO::Vessel::NO_EMPTY_POSITION:
					message = "Error: No empty position nearby.";
					break;
				case UCSO::Vessel::RELEASE_SLOT_CLOSED:
					message = "Error: The slot is closed.";
					PlayVesselWave(SFXID, SFX_BAY_DOORS_CLOSED);
					break;
				case UCSO::Vessel::RELEASE_SLOT_EMPTY:
					message = "Error: The slot is empty.";
					PlayVesselWave(SFXID, SFX_BAY_SLOT_EMPTY);
					break;
				case UCSO::Vessel::RELEASE_FAILED:
					message = "Error: Couldn't release the cargo.";
					PlayVesselWave(SFXID, SFX_BAY_RELEASE_FAILED);
					break;
				default: break;
				}
				timer = 0;
				return 1;

			case OAPI_KEY_F:
			{
				if (!ucsoInstalled) { timer = 0; return 1; }

				const char* requiredResource = fuelIndex == 2 ? "oxygen" : "fuel";

				PROPELLANT_HANDLE tankHandle = fuelIndex == 0 ? fuel_main_allTanks : fuelIndex == 1 ? fuel_sys : fuel_oxy;

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
				if (drainedMass == 0) drainedMass = ucso->DrainStationOrUnpackedResource(requiredResource,requiredMass);
				else SetPropellantMass(tankHandle, GetPropellantMass(tankHandle) + drainedMass);

				if (drainedMass > 0) 
				{
					sprintf(buffer, "Success: Drained %d kilograms of fuel.", int(round(drainedMass)));
					message = _strdup(buffer);
				}
				else message = "Error: Couldn't drain fuel.";
				timer = 0;
				return 1;
			}
			case OAPI_KEY_D:
				if (!ucsoInstalled) { timer = 0; return 1; }

				switch (ucso->DeleteCargo(slotIndex))
				{
				case UCSO::Vessel::RELEASE_SUCCEEDED:
					message = "Success: The cargo is deleted.";
					break;
				case UCSO::Vessel::RELEASE_SLOT_EMPTY:
					message = "Error: The slot is empty.";
					break;
				case UCSO::Vessel::RELEASE_FAILED:
					message = "Error: The deletion failed.";
					break;
				default: break;
				}

				timer = 0;
				return 1;

			default: break;
			}

			return 0;
		}
	}

	switch (key)
	{
	case OAPI_KEY_S:
		if (!ucsoInstalled) { timer = 0; return 1; }

		slotIndex < 7 ? slotIndex++ : slotIndex = 0;
		sprintf(buffer, "Selected slot: %d", slotIndex + 1);
		message = _strdup(buffer);

		timer = 0;
		return 1;

	case OAPI_KEY_F:
		if (!ucsoInstalled) { timer = 0; return 1; }

		fuelIndex < 2 ? fuelIndex++ : fuelIndex = 0;
		sprintf(buffer, "Selected tank: %s", fuelIndex == 0 ? "Main" : fuelIndex == 1 ? "ASF" : "Oxidizer");
		message = _strdup(buffer);

		timer = 0;
		return 1;

	case OAPI_KEY_D:
		if (!ucsoInstalled) { timer = 0; return 1; }

		displayCargoInfo = !displayCargoInfo;
		return 1;

	case OAPI_KEY_G:
		if(!gear_mode) landingGears->toggle();
		VCSwitches[VC_swIndexByMGID[MGID_SW3_GEAR]].setPos(landingGears->getToggleState() ? SW3_DOWN : SW2_UP, this);
		return 1;

	case OAPI_KEY_V:
		visor->toggle();
		VCSwitches[VC_swIndexByMGID[MGID_SW3_VISOR]].setPos(visor->getToggleState() ? SW2_UP : SW3_DOWN, this);
		return 1;

	case OAPI_KEY_N:
		canards->toggle();
		VCSwitches[VC_swIndexByMGID[MGID_SW3_CANARD]].setPos(canards->getToggleState() ? SW2_UP : SW3_DOWN, this);
		return 1;

	case OAPI_KEY_B:
		bayDoors->toggle();
		return 1;

	case OAPI_KEY_TAB:
	{
		if (VC_PoV_station == 0)
		{
			VC_PoV_station = 1;  //											engineer side

			SetMeshVisibilityMode(mdlID_dvcR, MESHVIS_NEVER);
			SetMeshVisibilityMode(mdlID_dvcL, MESHVIS_VC);

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
			VC_PoV_station = 0; //											 pilot side
			SetMeshVisibilityMode(mdlID_dvcR, MESHVIS_VC);
			SetMeshVisibilityMode(mdlID_dvcL, MESHVIS_NEVER);

			SetCameraMovement(_V(0, -0.05, 0.1), 0, -20 * RAD, _V(-0.20, -0.1, 0), 15 * RAD, 45 * RAD, _V(0.15, -0.02, 0), -35 * RAD, 0);

			SetCameraOffset(V3_VC_POV_PILOT);

			oapiVCSetAreaClickmode_Spherical((VC_CTRLSET_MOVESEAT << 16), _V(-1, 2.25, 38.7), .6);

			VECTOR3 vDir = _V(0, -0.15, 1); normalise(vDir);
			SetCameraDefaultDirection(vDir);
			oapiCameraSetCockpitDir(0, 0);
		};
	}

	break;

	/*
	case OAPI_KEY_MINUS:
		cgShiftRef += 0.1;
		EditAirfoil(wingLift, 0x01, _V(0, 0, cgShiftRef), NULL, 0, 0, 0);

		break;

	case OAPI_KEY_EQUALS:
		cgShiftRef -= 0.1;
		EditAirfoil(wingLift, 0x01, _V(0, 0, cgShiftRef), NULL, 0, 0, 0);

		break;
		*/

	default: return 0;
	}

	return 0;
}


