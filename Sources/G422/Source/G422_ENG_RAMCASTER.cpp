#include "..\Header\G422.h"
#include "..\Header\G422_MDL_EXT.h"

void G422::cueEngines(RAMCASTER& eng, RAMCASTER::SIMSTATE sst)
{
	if (eng.state == sst) return;

	switch (sst)
	{
	case RAMCASTER::ENG_START:
		if (eng.state != RAMCASTER::ENG_INOP && eng.state != RAMCASTER::ENG_STOP) return; // don't even try
		if (ramxDoors->pos < 1.0 || GetEngineLevel(ENGINE_HOVER) < .5) return; // doors closed or throttle too low...

		PlayVesselWave(SFXID, SFX_RAMCASTER_START);
		cueEngines(eng, RAMCASTER::ENG_RUNLO); // set it off!

		return;

	case RAMCASTER::ENG_RUNLO:
		if (eng.state == RAMCASTER::ENG_RUNHI) // switching from HI back to LO (why? but then, what the hell...)
			StopVesselWave(SFXID, SFX_ENGINERUN_RAMX_HI);

		ramcaster_mode = 1;

		break;

	case RAMCASTER::ENG_RUNHI:
		if (eng.state != RAMCASTER::ENG_RUNLO) return; // switching to HI is only allowed when running LO

		ramcaster_mode = 2;
		StopVesselWave(SFXID, SFX_ENGINERUN_RAMX_LO);
		PlayVesselWave(SFXID, SFX_RAMCASTER_TRST_UP);

		break;

	case RAMCASTER::ENG_INOP:
		if (eng.state == RAMCASTER::ENG_STOP) break;

	case RAMCASTER::ENG_STOP:
		StopVesselWave(SFXID, SFX_ENGINERUN_RAMX_LO);
		StopVesselWave(SFXID, SFX_ENGINERUN_RAMX_HI);

		ramcaster_mode = 0;
		SetThrusterLevel(eng.th_ramx, 0.0);

		break;

	default:
		return;
	}
	eng.state = sst;
}

void G422::simEngines(double& dT, RAMCASTER& eng)
{
	if (eng.state == RAMCASTER::ENG_INOP) return; // no simulation needed

	if (eng.feed & RAMCASTER::FUEL_PUMP && apu.pwrPct > .85) // simulate fuel line pressure from pumps
	{
		if (eng.fuelPrs < 1.0)
		{
			double deltaPrs = .22 * dT;
			eng.fuelPrs = min(eng.fuelPrs + deltaPrs, 1.0);
		}
	}
	else
	{
		if (eng.fuelPrs > 0.0)
		{
			double deltaPrs = -.12 * (1.0 - eng.thr * .5) * dT;
			eng.fuelPrs = max(eng.fuelPrs + deltaPrs, 0.0);
		}
	}

	if (GetPropellantMass(fuel_main_allTanks) < .001)
	{
		eng.fuelPrs = 0.0;
		return cueEngines(eng, RAMCASTER::ENG_INOP);
	}

	switch (eng.state)
	{
	case RAMCASTER::ENG_RUNLO:
	{
		//                       dynP :  0      4      8     12     16    20     24     28    32    36    40     44
		static double engFx_dynP[12] = { 0.00, 0.4, 0.68, 1.0385, 1.0425, 1.022,  .85,   .69,  .58,  .3,  .23,   .19 };
		//                       Mach :  0     .5    1   1.5    2    2.5    3   3.5   4   4.5   5    5.5  
		static double engFx_mach[12] = { 0.0,   0.2,   .33,  .5, .937, 1.048,  1.035,   .97,  .83,  .66,  .47,   .27 };

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

		if (engPerf < .25) return cueEngines(eng, RAMCASTER::ENG_STOP);

		SetThrusterIsp(ramcaster, ISPMAX_RAMXLO * engPerf);
		SetThrusterMax0(ramcaster, MAXTHRUST_RAMX_LO * engPerf);

		eng.thr = GetEngineLevel(ENGINE_HOVER) * eng.fuelPrs;
		SetThrusterLevel(ramcaster, eng.thr);

		eng.epr = eng.thr * engPerf;

		PlayVesselWave(SFXID, SFX_ENGINERUN_RAMX_LO, NOLOOP, int(255 * eng.epr));
		PlayVesselWave(SFXID, SFX_RAMXBURN, NOLOOP, int(255 * eng.thr));
	}
	break;

	case RAMCASTER::ENG_RUNHI:
	{
		//                        dynP :  0     4      8     12     16     20     24     28     32    36    40     44
		static double engFx_dynP[12] = { 0.35, 0.8, 1.015, 1.2315, 1.3155, 1.3553, 1.3581, 1.165, 1.08, 0.9, 0.73,  0.56 };
		//                        Mach :  3     4   5     6     7     8     9      10    11     12      13    14     15      16    17  
		static double engFx_mach[15] = { 0.8, 1.3, 1.31, 1.3, 1.303, 1.3, 1.342, 1.335, 1.321, 1.782, 1.222, 1.172, 1.115, 1.004, 0.555 };

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

		if (engPerf < .25) return cueEngines(eng, RAMCASTER::ENG_STOP);

		SetThrusterIsp(ramcaster, ISPMAX_RAMXHI * engPerf);
		SetThrusterMax0(ramcaster, MAXTHRUST_RAMX_HI * engPerf);

		eng.thr = GetEngineLevel(ENGINE_HOVER) * eng.fuelPrs;
		SetThrusterLevel(ramcaster, eng.thr);

		eng.epr = eng.thr * engPerf;

		PlayVesselWave(SFXID, SFX_ENGINERUN_RAMX_HI, NOLOOP, int(255 * eng.epr));
		PlayVesselWave(SFXID, SFX_RAMXBURN, NOLOOP, int(255 * eng.thr));
	}
	break;

	default:
		return;
	}

	if (eng.thr < .15 || ramxDoors->pos < 1.0) // throttle too low or doors closed
		// engine cuts off below this point
		cueEngines(eng, RAMCASTER::ENG_STOP);
}


