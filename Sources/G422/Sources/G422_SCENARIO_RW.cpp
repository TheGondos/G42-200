#include "..\Headers\G422.h"
#include "..\Headers\G422_DVC.h"
#include "..\Headers\G422_MDL_DVC.h"

void G422::clbkLoadStateEx(FILEHANDLE scn, void* status)
{
	char* lineChars;
	int cfgLoadInt;

	while (oapiReadScenario_nextline(scn, lineChars))
	{
		if (checkCfgTag(lineChars, "LDG_GEAR", &cfgLoadInt)) landingGear->loadCfgState(cfgLoadInt);
		else if (checkCfgTag(lineChars, "CNRDS", &cfgLoadInt)) canards->loadCfgState(cfgLoadInt);
		else if (checkCfgTag(lineChars, "VISOR", &cfgLoadInt)) visor->loadCfgState(cfgLoadInt);
		else if (checkCfgTag(lineChars, "INLET_LMAIN", &cfgLoadInt)) engMainL.inltDoor->loadCfgState(cfgLoadInt);
		else if (checkCfgTag(lineChars, "INLET_RMAIN", &cfgLoadInt)) engMainR.inltDoor->loadCfgState(cfgLoadInt);
		else if (checkCfgTag(lineChars, "INLET_RAMX", &cfgLoadInt)) ramxDoors->loadCfgState(cfgLoadInt);
		else if (checkCfgTag(lineChars, "WING_WVRD", &cfgLoadInt)) wingTipWvrd->loadCfgState(cfgLoadInt);
		else if (checkCfgTag(lineChars, "WING_FTHR", &cfgLoadInt)) wingTipFthr->loadCfgState(cfgLoadInt);
		else if (checkCfgTag(lineChars, "CGO_BAY", &cfgLoadInt)) bayDoors->loadCfgState(cfgLoadInt);
		else if (checkCfgTag(lineChars, "RCSDOORS", &cfgLoadInt)) rcsDoors->loadCfgState(cfgLoadInt);

		else if (checkCfgTag(lineChars, "BURNER_ENG_MD", &cfgLoadInt)) burnerEngMode = cfgLoadInt / 4.0;
		else if (checkCfgTag(lineChars, "RAMX_ENG_MD", &cfgLoadInt)) ramcasterMode = cfgLoadInt;
		else if (checkCfgTag(lineChars, "PRK_BRAKE_MD", &cfgLoadInt)) parkingBrakeMode = cfgLoadInt;
		else if (checkCfgTag(lineChars, "VC_SEAT", &cfgLoadInt)) vcPovStation = cfgLoadInt;

		else if (!_strnicmp(lineChars, "VC_SWS", 6))
			for (int i = 0; i < VC_SWITCH_COUNT; ++i) vcSwitches[i].setLoadState(lineChars[i + 7], this);
		
		else if (!_strnicmp(lineChars, "VC_KBS", 6))
			for (int i = 0; i < VC_KNOB_COUNT; ++i) vcKnobs[i].setLoadState(lineChars[i + 7], this);

		else if (!_strnicmp(lineChars, "ENG_MAIN_", 9))
		{
			if (lineChars[9] == 'L') // load state for left main engine
				sscanf(lineChars + 10, "%d%d%lf%lf%lf%lf%lf",
					&engMainL.state, &engMainL.mode, &engMainL.thr, &engMainL.genPct,
					&engMainL.epr, &engMainL.fuelPrs, &engMainL.oxyPrs);
			else // load state for right  main engine
				sscanf(lineChars + 10, "%d%d%lf%lf%lf%lf%lf",
					&engMainR.state, &engMainR.mode, &engMainR.thr, &engMainR.genPct,
					&engMainR.epr, &engMainR.fuelPrs, &engMainR.oxyPrs);
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

		else if (!_strnicmp(lineChars, "ENG_RCS", 7)) sscanf(lineChars + 7, "%d%lf%lf", &rcs.state, &rcs.fuelPrs, &rcs.fuelFlow);

		else if (!_strnicmp(lineChars, "ENG_OMS", 7)) sscanf(lineChars + 7, "%d%lf%lf%lf", &oms.state, &oms.fuelPrs, &oms.fuelFlow, &oms.thr);

		else if (!_strnicmp(lineChars, "VC_EICAS", 8))
			sscanf(lineChars + 8, "%d%d%d%d%d%d",
				vcEicasScreens + 5, vcEicasScreens + 4, vcEicasScreens, vcEicasScreens + 1, vcEicasScreens + 2, vcEicasScreens + 3);

		else if (!_strnicmp(lineChars, "CG_POS", 6)) sscanf(lineChars + 6, "%lf", &cgPos);

		else ParseScenarioLineEx(lineChars, status);
	}
}

void G422::clbkSaveState(FILEHANDLE scn)
{
	VESSEL4::clbkSaveState(scn);

	// alright, first the moving parts... those should be mostly readable...
	oapiWriteScenario_int(scn, "LDG_GEAR", landingGear->getCfgState());
	oapiWriteScenario_int(scn, "CNRDS", canards->getCfgState());
	oapiWriteScenario_int(scn, "VISOR", visor->getCfgState());
	oapiWriteScenario_int(scn, "RCSDOORS", rcsDoors->getCfgState());

	oapiWriteScenario_int(scn, "INLET_LMAIN", engMainL.inltDoor->getCfgState());
	oapiWriteScenario_int(scn, "INLET_RMAIN", engMainR.inltDoor->getCfgState());
	oapiWriteScenario_int(scn, "INLET_RAMX", ramxDoors->getCfgState());

	oapiWriteScenario_int(scn, "WING_WVRD", wingTipWvrd->getCfgState());
	oapiWriteScenario_int(scn, "WING_FTHR", wingTipFthr->getCfgState());

	oapiWriteScenario_int(scn, "CGO_BAY", bayDoors->getCfgState());

	// now, simulation things....
	oapiWriteScenario_int(scn, "BURNER_ENG_MD", int(burnerEngMode * 4));
	oapiWriteScenario_int(scn, "RAMX_ENG_MD", ramcasterMode);
	oapiWriteScenario_int(scn, "PRK_BRAKE_MD", parkingBrakeMode);
	oapiWriteScenario_int(scn, "VC_SEAT", vcPovStation);

	// engines crap...
	char scnData[256];

	sprintf(scnData, "%d %d %0.4f %0.4f %0.4f %0.4f %0.4f",
		engMainL.state, engMainL.mode, engMainL.thr, engMainL.genPct, engMainL.epr, engMainL.fuelPrs, engMainL.oxyPrs);
	oapiWriteScenario_string(scn, "ENG_MAIN_L", scnData); // left main engine

	sprintf(scnData, "%d %d %0.4f %0.4f %0.4f %0.4f %0.4f",
		engMainR.state, engMainR.mode, engMainR.thr, engMainR.genPct, engMainR.epr, engMainR.fuelPrs, engMainR.oxyPrs);
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
	// this might come out a little unreadable now... but we gotta save the state for ALL those switches
	char vcSwsState[VC_SWITCH_COUNT + 1];
	for (int index = 0; index < VC_SWITCH_COUNT; ++index) vcSwsState[index] = vcSwitches[index].getSaveState();
	vcSwsState[VC_SWITCH_COUNT] = '\x00'; // set null terminator
	oapiWriteScenario_string(scn, "VC_SWS", vcSwsState);

	char vcKbsState[VC_KNOB_COUNT + 1];
	for (int index = 0; index < VC_KNOB_COUNT; ++index) vcKbsState[index] = vcKnobs[index].getSaveState();
	vcKbsState[VC_KNOB_COUNT] = '\x00'; // set null terminator
	oapiWriteScenario_string(scn, "VC_KBS", vcKbsState);

	sprintf(scnData, "%d %d %d %d %d %d",
		vcEicasScreens[5], vcEicasScreens[4], vcEicasScreens[0], vcEicasScreens[1], vcEicasScreens[2], vcEicasScreens[3]);
	oapiWriteScenario_string(scn, "VC_EICAS", scnData); // EICAS SCREENS

	oapiWriteScenario_float(scn, "CG_POS", cgPos);
}

int MovingPart::getCfgState()
{
	if ((mpStatus == MovingPart::MP_REVERSING || mpStatus == MovingPart::MP_LOW_DETENT)) return 1;

	if ((mpStatus == MovingPart::MP_MOVING || mpStatus == MovingPart::MP_HI_DETENT)) return 2;

	return 0;
}

void MovingPart::loadCfgState(int state)
{
	if (state == 2)
	{
		mpStatus = MovingPart::MP_HI_DETENT;
		pos = 1;
		vessel->SetAnimation(animIndex, pos);
	}

	else if (state == 1)
	{
		mpStatus = MovingPart::MP_LOW_DETENT;
		pos = 0;
		vessel->SetAnimation(animIndex, pos);
		return;
	}

	sysReset = false;
	vessel->clbkGeneric(VMSG_MPSTOP, sysID, nullptr);
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

void VCSwitch::setLoadState(char& cs, VESSEL4* vessel)
{
	switch (cs)
	{
	case '-': setPos(SW3_DOWN, vessel);  break;
	case '=': setPos(SW3_MID, vessel); return; // Don't continue if the switch is in the middle
	case '+': setPos(SW3_UP, vessel); break;
	case '^': setPos(SW2_UP, vessel); break;
	case 'v': setPos(SW2_DOWN, vessel); break;
	}

	vessel->clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (G422::vcSwitchIndexByMGID[mgid] & 0xFFFF), PANEL_MOUSE_IGNORE, _V0);
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

void VCKnob::setLoadState(char& cs, VESSEL4* vessel)
{
	switch (cs)
	{
	case '-': setPos(KB3_DOWN, vessel);  break;
	case '=': setPos(KB3_MID, vessel); break;
	case '+': setPos(KB3_UP, vessel); break;
	case '^': setPos(KB2_UP, vessel); break;
	case 'v': setPos(KB2_DOWN, vessel); break;
	}

	vessel->clbkVCMouseEvent((VC_CTRLSET_KNOBS << 16) | (G422::vcKnobIndexByMGID[mgid] & 0xFFFF), PANEL_MOUSE_IGNORE, _V0);
}

// this helps make things less ugly
bool G422::checkCfgTag(char* line, const char* tag, int* cfgData)
{
	int len = strlen(tag);
	if (_strnicmp(line, tag, len)) return false;

	sscanf(line + len, "%d", cfgData);
	return true;
}