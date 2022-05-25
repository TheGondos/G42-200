#include "../Headers/G422.h"
#include "../Headers/G422_DVC.h"
#include "../Headers/G422_MDL_DVC.h"

void G422::cueEngines(RAMCASTER& eng, RAMCASTER::SIMSTATE sst)
{
	if (eng.state == sst) return;

	switch (sst)
	{
	case RAMCASTER::ENG_START:
		if (eng.state != RAMCASTER::ENG_INOP && eng.state != RAMCASTER::ENG_STOP) return; // don't even try
		if (ramxDoors->pos < 1 || GetEngineLevel(ENGINE_HOVER) < 0.5) return; // doors closed or throttle too low...

		playSound(SFX_RAMCASTER_START);
		cueEngines(eng, RAMCASTER::ENG_RUNLO); // set it off!

		return;

	case RAMCASTER::ENG_RUNLO:
		if (eng.state == RAMCASTER::ENG_RUNHI) // switching from HI back to LO (why? but then, what the hell...)
			stopSound(SFX_ENGINERUN_RAMX_HI);

		ramcasterMode = 1;

		break;

	case RAMCASTER::ENG_RUNHI:
		if (eng.state != RAMCASTER::ENG_RUNLO) return; // switching to HI is only allowed when running LO

		ramcasterMode = 2;
		stopSound(SFX_ENGINERUN_RAMX_LO);
		playSound(SFX_RAMCASTER_TRST_UP);

		break;

	case RAMCASTER::ENG_INOP:
		if (eng.state == RAMCASTER::ENG_STOP) break;

	case RAMCASTER::ENG_STOP:
		stopSound(SFX_ENGINERUN_RAMX_LO);
		stopSound(SFX_ENGINERUN_RAMX_HI);

		ramcasterMode = 0;
		SetThrusterLevel(eng.thRamx, 0);

		break;

	default:
		return;
	}
	eng.state = sst;
}

void G422::simEngines(double& dT, RAMCASTER& eng)
{
	double fuelMass = GetPropellantMass(allMainFuel);

	if (eng.feed & RAMCASTER::FUEL_PUMP && (fuelMass < 0.001 || !(eng.feed & RAMCASTER::FUEL_OPEN) || 
		(apuPackA.pwrPct < 0.85 && apuPackB.pwrPct < 0.85))) {
		VECTOR3 V0 = _V0;
		clbkVCMouseEvent((VC_CTRLSET_SWITCHES << 16) | (vcSwitchIndexByMGID[MGID_SW2_APU_RAMX] & 0xFFFF), PANEL_MOUSE_RBPRESSED, V0);
	}

	if (eng.feed & RAMCASTER::FUEL_PUMP) // simulate fuel line pressure from pumps
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
			double deltaPrs = -0.12 * (1 - eng.thr * 0.5) * dT;
			eng.fuelPrs = max(eng.fuelPrs + deltaPrs, 0.0);
		}
	}

	if (fuelMass < 0.001) eng.fuelPrs = 0;

	if (eng.state == RAMCASTER::ENG_INOP) return; // no simulation needed
	
	if (eng.fuelPrs == 0) return cueEngines(eng, RAMCASTER::ENG_INOP); 
	
	switch (eng.state)
	{
	case RAMCASTER::ENG_RUNLO:
	{
		//                       dynP :  0   4    8      12     16      20     24     28    32   36     40    44
		static double engFx_dynP[12] = { 0, 0.5, 0.78, 1.1385, 1.1425, 1.122, 0.95,  0.79, 0.68, 0.4,  0.33, 0.29 };
		//                       Mach :  0  0.5   1     1.5      2      2.5     3     3.5    4   4.5     5    5.5  
		static double engFx_mach[12] = { 0, 0.3, 0.43,  0.6,   1.037,  1.148, 1.135, 1.07, 0.93, 0.76, 0.57, 0.37 };

		double DpFx = GetDynPressure() * 2.27272727e-5;

		int perfRefLo = int(floor(DpFx * 11.0));   perfRefLo = (perfRefLo > 11) ? 11 : (perfRefLo < 0) ? 0 : perfRefLo;
		int perfRefHi = int(ceil(DpFx * 11.0));   perfRefHi = (perfRefHi > 11) ? 11 : (perfRefHi < 0) ? 0 : perfRefHi;

		double delta = engFx_dynP[perfRefHi] - engFx_dynP[perfRefLo];
		double engPerf = engFx_dynP[perfRefLo] + delta * ((DpFx * 11.0 - (double)perfRefLo) * 0.0909090909);

		double MnFx = GetMachNumber() * 0.181818182;

		perfRefLo = int(floor(MnFx * 11.0));  perfRefLo = (perfRefLo > 11) ? 11 : (perfRefLo < 0) ? 0 : perfRefLo;
		perfRefHi = int(ceil(MnFx * 11.0));  perfRefHi = (perfRefHi > 11) ? 11 : (perfRefHi < 0) ? 0 : perfRefHi;

		delta = engFx_mach[perfRefHi] - engFx_mach[perfRefLo];
		engPerf *= engFx_mach[perfRefLo] + delta * ((MnFx * 11.0 - (double)perfRefLo) * 0.0909090909);

		if (engPerf < 0.25) return cueEngines(eng, RAMCASTER::ENG_STOP);

		SetThrusterIsp(ramcaster, ISPMAX_RAMXLO * engPerf);
		SetThrusterMax0(ramcaster, MAXTHRUST_RAMX_LO * engPerf);

		eng.thr = GetEngineLevel(ENGINE_HOVER) * eng.fuelPrs;
		SetThrusterLevel(ramcaster, eng.thr);

		eng.epr = eng.thr * engPerf;

		playSound(SFX_ENGINERUN_RAMX_LO, false, float(eng.epr));
		playSound(SFX_RAMXBURN, false, float(eng.thr));
	}
	break;

	case RAMCASTER::ENG_RUNHI:
	{
		//                        dynP :  0     4     8      12      16      20      24     28     32    36      40     44
		static double engFx_dynP[12] = { 0.35, 0.8, 1.015, 1.2315, 1.3155, 1.3553, 1.3581, 1.165, 1.08,  0.9,   0.73,  0.56 };
		//                        Mach :  3    4    5     6     7     8     9      10     11     12     13     14     15      16    17  
		static double engFx_mach[15] = { 0.8, 1.3, 1.31, 1.3, 1.303, 1.3, 1.342,  1.335, 1.321, 1.782, 1.222, 1.172, 1.115, 1.004, 0.555 };

		if (GetMachNumber() < 3.0) return cueEngines(eng, RAMCASTER::ENG_STOP);

		double DpFx = GetDynPressure() * 2.27272727e-5;

		int perfRefLo = int(floor(DpFx * 11.0));   perfRefLo = (perfRefLo > 11) ? 11 : (perfRefLo < 0) ? 0 : perfRefLo;
		int perfRefHi = int(ceil(DpFx * 11.0));   perfRefHi = (perfRefHi > 11) ? 11 : (perfRefHi < 0) ? 0 : perfRefHi;

		double delta = engFx_dynP[perfRefHi] - engFx_dynP[perfRefLo];
		double engPerf = engFx_dynP[perfRefLo] + delta * ((DpFx * 11.0 - (double)perfRefLo) * 0.0909090909);

		double MnFx = (GetMachNumber() - 3.0) * 0.0714285714;

		perfRefLo = int(floor(MnFx * 14.0));  perfRefLo = (perfRefLo > 14) ? 14 : (perfRefLo < 0) ? 0 : perfRefLo;
		perfRefHi = int(ceil(MnFx * 14.0));  perfRefHi = (perfRefHi > 14) ? 14 : (perfRefHi < 0) ? 0 : perfRefHi;

		delta = engFx_mach[perfRefHi] - engFx_mach[perfRefLo];
		engPerf *= engFx_mach[perfRefLo] + delta * ((MnFx * 14.0 - (double)perfRefLo) * 0.0714285714);

		if (engPerf < 0.25) return cueEngines(eng, RAMCASTER::ENG_STOP);

		SetThrusterIsp(ramcaster, ISPMAX_RAMXHI * engPerf);
		SetThrusterMax0(ramcaster, MAXTHRUST_RAMX_HI * engPerf);

		eng.thr = GetEngineLevel(ENGINE_HOVER) * eng.fuelPrs;
		SetThrusterLevel(ramcaster, eng.thr);

		eng.epr = eng.thr * engPerf;

		playSound(SFX_ENGINERUN_RAMX_HI, false, float(eng.epr));
		playSound(SFX_RAMXBURN, false, float(eng.thr));
	}
	break;

	default:
		return;
	}

	if (eng.thr < 0.15 || ramxDoors->pos < 1) // throttle too low or doors closed
		// engine cuts off below this point
		cueEngines(eng, RAMCASTER::ENG_STOP);
}