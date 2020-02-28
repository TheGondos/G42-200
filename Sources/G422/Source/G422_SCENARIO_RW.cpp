#include "..\Header\G422.h"
#include "..\Header\G422_DVC.h"
#include "..\Header\G422_MDL_DVC.h"

// this helps make things less ugly
inline bool CFGCheckTag(char* line, const char* tag, int* cfgData)
{
	int len = strlen(tag);
	if (_strnicmp(line, tag, len)) return false;

	sscanf(line + len, "%d", cfgData);
	return true;
}

void G422::clbkLoadStateEx(FILEHANDLE scn, void* status)
{
	char* lineChars;
	int cfgLoadInt;

	while (oapiReadScenario_nextline(scn, lineChars))
	{
		if (CFGCheckTag(lineChars, "GEARS", &cfgLoadInt)) landingGears->loadCfgState(cfgLoadInt);
		else if (CFGCheckTag(lineChars, "CNRDS", &cfgLoadInt)) canards->loadCfgState(cfgLoadInt);
		else if (CFGCheckTag(lineChars, "VISOR", &cfgLoadInt)) visor->loadCfgState(cfgLoadInt);
		else if (CFGCheckTag(lineChars, "INLET_MAIN", &cfgLoadInt)) inltDoors->loadCfgState(cfgLoadInt);
		else if (CFGCheckTag(lineChars, "INLET_RAMX", &cfgLoadInt)) ramxDoors->loadCfgState(cfgLoadInt);
		else if (CFGCheckTag(lineChars, "WING_WVRD", &cfgLoadInt)) wingTipWvrd->loadCfgState(cfgLoadInt);
		else if (CFGCheckTag(lineChars, "WING_FTHR", &cfgLoadInt)) wingTipFthr->loadCfgState(cfgLoadInt);
		else if (CFGCheckTag(lineChars, "CGO_BAY", &cfgLoadInt)) bayDoors->loadCfgState(cfgLoadInt);
		else if (CFGCheckTag(lineChars, "RCSDOORS", &cfgLoadInt)) rcsDoors->loadCfgState(cfgLoadInt);

		else if (CFGCheckTag(lineChars, "MAIN_ENG_MD", &cfgLoadInt)) main_eng_mode = cfgLoadInt;
		else if (CFGCheckTag(lineChars, "RAMX_ENG_MD", &cfgLoadInt)) ramcaster_mode = cfgLoadInt;
		else if (CFGCheckTag(lineChars, "PRK_BRAKE_MD", &cfgLoadInt)) prk_brake_mode = cfgLoadInt;

		else if (!_strnicmp(lineChars, "VC_SWS", 6))
			for (int i = 0; i < VC_SWITCH_COUNT; ++i) VCSwitches[i].setLoadState(lineChars[i + 7], this);
		
		else if (!_strnicmp(lineChars, "VC_KBS", 6))
			for (int i = 0; i < VC_KNOB_COUNT; ++i) VCKnobs[i].setLoadState(lineChars[i + 7], this);

		else if (!_strnicmp(lineChars, "ENG_MAIN_", 9))
		{
			if (lineChars[9] == 'L') // load state for left main engine
				sscanf(lineChars + 10, "%d%lf%lf%lf%lf%lf",
					&engMain_L.state, &engMain_L.thr, &engMain_L.genPct,
					&engMain_L.epr, &engMain_L.fuelPrs, &engMain_L.oxyPrs);
			else // load state for right  main engine
				sscanf(lineChars + 10, "%d%lf%lf%lf%lf%lf",
					&engMain_R.state, &engMain_R.thr, &engMain_R.genPct,
					&engMain_R.epr, &engMain_R.fuelPrs, &engMain_R.oxyPrs);
		}

		else if (!_strnicmp(lineChars, "ENG_RAMX", 8))
			sscanf(lineChars + 8, "%d%lf%lf%lf", &engRamx.state, &engRamx.thr, &engRamx.epr, &engRamx.fuelPrs);

		else if (!_strnicmp(lineChars, "ENG_APU_", 8))
		{
			if (lineChars[8] == 'A')
				sscanf(lineChars + 9, "%d%lf%lf%lf", &apuPackA.state, &apuPackA.pwrPct, &apuPackA.fuelPrs, &apuPackA.fuelFlow);
			else
				sscanf(lineChars + 9, "%d%lf%lf%lf", &apuPackB.state, &apuPackB.pwrPct, &apuPackB.fuelPrs, &apuPackB.fuelFlow);
		}

		else if (!_strnicmp(lineChars, "SYS_HYD_", 8))
		{
			if (lineChars[8] == 'A')
				sscanf(lineChars + 9, "%lf%lf", &hydSysA.hydPrs, &hydSysA.hydFlow);
			else
				sscanf(lineChars + 9, "%lf%lf", &hydSysB.hydPrs, &hydSysB.hydFlow);
		}

		else if (!_strnicmp(lineChars, "ENG_RCS", 7))
			sscanf(lineChars + 7, "%d%lf%lf", &rcs.state, &rcs.fuelPrs, &rcs.fuelFlow);

		else if (!_strnicmp(lineChars, "ENG_OMS", 7))
			sscanf(lineChars + 7, "%d%lf%lf%lf", &oms.state, &oms.fuelPrs, &oms.fuelFlow, &oms.thr);

		else if (!_strnicmp(lineChars, "VC_EICAS", 8))
			sscanf(lineChars + 8, "%d%d%d%d%d%d",
				VC_eicas_screens + 5, VC_eicas_screens + 4, VC_eicas_screens, VC_eicas_screens + 1, VC_eicas_screens + 2, VC_eicas_screens + 3);

		else ParseScenarioLineEx(lineChars, status);
	}
}

void G422::clbkSaveState(FILEHANDLE scn)
{
	VESSEL4::clbkSaveState(scn);

	// alright, first the moving parts... those should be mostly readable...
	oapiWriteScenario_int(scn, "GEARS", landingGears->getCfgState());
	oapiWriteScenario_int(scn, "CNRDS", canards->getCfgState());
	oapiWriteScenario_int(scn, "VISOR", visor->getCfgState());
	oapiWriteScenario_int(scn, "RCSDOORS", rcsDoors->getCfgState());

	oapiWriteScenario_int(scn, "INLET_MAIN", inltDoors->getCfgState());
	oapiWriteScenario_int(scn, "INLET_RAMX", ramxDoors->getCfgState());

	oapiWriteScenario_int(scn, "WING_WVRD", wingTipWvrd->getCfgState());
	oapiWriteScenario_int(scn, "WING_FTHR", wingTipFthr->getCfgState());

	oapiWriteScenario_int(scn, "CGO_BAY", bayDoors->getCfgState());

	// now, simulation things....
	oapiWriteScenario_int(scn, "MAIN_ENG_MD", main_eng_mode);
	oapiWriteScenario_int(scn, "RAMX_ENG_MD", ramcaster_mode);
	oapiWriteScenario_int(scn, "PRK_BRAKE_MD", prk_brake_mode);

	// engines crap...
	char scnData[256];

	sprintf(scnData, "%d %0.4f %0.4f %0.4f %0.4f %0.4f",
		engMain_L.state, engMain_L.thr, engMain_L.genPct, engMain_L.epr, engMain_L.fuelPrs, engMain_L.oxyPrs);
	oapiWriteScenario_string(scn, "ENG_MAIN_L", scnData); // left main engine

	sprintf(scnData, "%d %0.4f %0.4f %0.4f %0.4f %0.4f",
		engMain_R.state, engMain_R.thr, engMain_R.genPct, engMain_R.epr, engMain_R.fuelPrs, engMain_R.oxyPrs);
	oapiWriteScenario_string(scn, "ENG_MAIN_R", scnData); // right main engine

	sprintf(scnData, "%d %0.4f %0.4f %0.4f",
		engRamx.state, engRamx.thr, engRamx.epr, engRamx.fuelPrs);
	oapiWriteScenario_string(scn, "ENG_RAMX", scnData); // ramcaster

	sprintf(scnData, "%d %0.4f %0.4f %0.4f", apuPackA.state, apuPackA.pwrPct, apuPackA.fuelPrs, apuPackA.fuelFlow);
	oapiWriteScenario_string(scn, "ENG_APU_A", scnData); // APU pack A

	sprintf(scnData, "%d %0.4f %0.4f %0.4f", apuPackB.state, apuPackB.pwrPct, apuPackB.fuelPrs, apuPackB.fuelFlow);
	oapiWriteScenario_string(scn, "ENG_APU_B", scnData); // APU pack B

	sprintf(scnData, "%0.4f %0.4f", hydSysA.hydPrs, hydSysA.hydFlow);
	oapiWriteScenario_string(scn, "SYS_HYD_A", scnData); // Hydraulic A

	sprintf(scnData, "%0.4f %0.4f", hydSysB.hydPrs, hydSysB.hydFlow);
	oapiWriteScenario_string(scn, "SYS_HYD_B", scnData); // Hydraulic B

	sprintf(scnData, "%d %0.4f %0.4f", rcs.state, rcs.fuelPrs, rcs.fuelFlow);
	oapiWriteScenario_string(scn, "ENG_RCS", scnData); // RCS

	sprintf(scnData, "%d %0.4f %0.4f %0.4f", oms.state, oms.fuelPrs, oms.fuelFlow, oms.thr);
	oapiWriteScenario_string(scn, "ENG_OMS", scnData); // OMS

	// ok... now onto VC stuff
	sprintf(scnData, "%d %d %d %d %d %d",
		VC_eicas_screens[5], VC_eicas_screens[4], VC_eicas_screens[0], VC_eicas_screens[1], VC_eicas_screens[2], VC_eicas_screens[3]);
	oapiWriteScenario_string(scn, "VC_EICAS", scnData); // EICAS SCREENS

	// this might come out a little unreadable now... but we gotta save the state for ALL those switches
	char vcSwsState[VC_SWITCH_COUNT + 1];
	for (int i = 0; i < VC_SWITCH_COUNT; ++i) vcSwsState[i] = VCSwitches[i].getSaveState();
	vcSwsState[VC_SWITCH_COUNT] = '\x00'; // set null terminator
	oapiWriteScenario_string(scn, "VC_SWS", vcSwsState);

	char vcKbsState[VC_KNOB_COUNT + 1];
	for (int i = 0; i < VC_KNOB_COUNT; ++i) vcKbsState[i] = VCKnobs[i].getSaveState();
	vcKbsState[VC_KNOB_COUNT] = '\x00'; // set null terminator
	oapiWriteScenario_string(scn, "VC_KBS", vcKbsState);
}

int MovingPart::getCfgState()
{
	if ((mp_status == MP_REVERSING || mp_status == MP_LOW_DETENT)) return 1;

	if ((mp_status == MP_MOVING || mp_status == MP_HI_DETENT)) return 2;

	return 0;
}

void MovingPart::loadCfgState(int state)
{
	if (state == 2)
	{
		mp_status = MP_HI_DETENT;
		pos = 1;
		vsl->SetAnimation(anim_idx, pos);
	}

	else if (state == 1)
	{
		mp_status = MP_LOW_DETENT;
		pos = 0;
		vsl->SetAnimation(anim_idx, pos);
		return;
	}

	sysReset = false;
	vsl->clbkGeneric(VMSG_MPSTOP, sysID, this);
}

char VCSwitch::getSaveState()
{
	switch (pos)
	{
	case SW3_DOWN: return '-';
	case SW3_MID:  return '=';
	case SW3_UP:   return '+';
	case SW2_UP:   return '^';
	case SW2_DOWN: return 'v';
	}

	return ' ';
}

void VCSwitch::setLoadState(char& cs, VESSEL4* vsl)
{
	switch (cs)
	{
	case '-': setPos(SW3_DOWN, vsl);  break;
	case '=': setPos(SW3_MID, vsl); return; // Don't continue if the switch is in the middle
	case '+': setPos(SW3_UP, vsl); break;
	case '^': setPos(SW2_UP, vsl); break;
	case 'v': setPos(SW2_DOWN, vsl); break;
	}

	vsl->clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (VC_swIndexByMGID[mgid] & 0xFFFF), PANEL_MOUSE_IGNORE, _V0);
}

char VCKnob::getSaveState()
{
	switch (pos)
	{
	case KB3_DOWN: return '-';
	case KB3_MID:  return '=';
	case KB3_UP:   return '+';
	case KB2_UP:   return '^';
	case KB2_DOWN: return 'v';
	}

	return ' ';
}

void VCKnob::setLoadState(char& cs, VESSEL4* vsl)
{
	switch (cs)
	{
	case '-': setPos(KB3_DOWN, vsl);  break;
	case '=': setPos(KB3_MID, vsl); break;
	case '+': setPos(KB3_UP, vsl); break;
	case '^': setPos(KB2_UP, vsl); break;
	case 'v': setPos(KB2_DOWN, vsl); break;
	}

	vsl->clbkVCMouseEvent((VC_CTRLSET_KNOBS << 16) | (VC_kbIndexByMGID[mgid] & 0xFFFF), PANEL_MOUSE_IGNORE, _V0);
}