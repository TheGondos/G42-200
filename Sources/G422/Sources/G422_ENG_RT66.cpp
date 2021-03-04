#include "..\Headers\G422.h"
#include "..\Headers\G422_MDL_EXT.h"

void G422::cueEngines(RT66& eng, RT66::SIMSTATE sst)
{
	if (eng.state == sst) return;

	if (eng.state == RT66::SST_RUN_EXT) eng.contrailLevel = 0;

	switch (sst)
	{
	case RT66::ENG_SPOOLUP:
		if (eng.state != RT66::SST_CUT && eng.state != RT66::SST_INOP) return; // yeah, can't start if not stopped first...

		if (eng.inltDoor->getToggleState()) // int mode!
			return cueEngines(eng, RT66::SST_STARTRCKT); // start as rocket!

	case RT66::SST_STARTGEN:
		eng.state = RT66::SST_STARTGEN;

		return;

	case RT66::ENG_SPOOLDOWN:
		if (eng.state == RT66::SST_RUN_INT) return cueEngines(eng, RT66::SST_STOPRCKT); // stop as rocket when running as such....
		if (eng.state != RT66::SST_RUN_EXT && eng.state != RT66::SST_STARTGEN) return; // otherwise, still can't stop what's not running

	case RT66::SST_STOPGEN:
		eng.state = RT66::SST_STOPGEN;
		eng.epr = 0;
		eng.thr = 0;

		SetThrusterLevel(eng.burnerThr, 0);
		SetThrusterLevel(eng.gasGenThr, 0);

		stopSound(eng.jetRoarSFX);
		stopSound(eng.burnerSFX);
		stopSound(eng.extStartSFX);

		return;

	case RT66::SST_RUN_EXT:
		stopSound(eng.extStartSFX);

		break;

	case RT66::SST_CUT:
		eng.thr = 0;
		eng.epr = 0;
		eng.genPct = 0;

		SetThrusterLevel(eng.burnerThr, 0);
		SetThrusterLevel(eng.gasGenThr, 0);
		SetThrusterLevel(eng.rocketThr, 0);

		break;

	case RT66::ENG_INLT_CLSE:
		switch (eng.state)
		{
		default: return; // not allowed if engines are running (or staring)
		case RT66::SST_CUT:
		case RT66::SST_FAIL:
		case RT66::SST_INOP:
		case RT66::SST_STOPGEN:
		case RT66::SST_STOPRCKT:
			eng.inltDoor->toggle(true);
		}
		return;

	case RT66::ENG_INLT_OPEN:
		switch (eng.state)
		{
		default: return;
		case RT66::SST_CUT:  
		case RT66::SST_FAIL: 
		case RT66::SST_INOP:
		case RT66::SST_STOPGEN: 
		case RT66::SST_STOPRCKT:
			eng.inltDoor->toggle(false);
		}

		return;

	case RT66::SST_RUN_INT:
		stopSound(eng.intStartSFX);
		break;

	case RT66::SST_STOPRCKT:
		if (eng.state != RT66::SST_RUN_INT) return; // likewise, can't stop what's not running
		eng.epr = 0;

		stopSound(eng.intRunSFX);
		stopSound(eng.intStartSFX);
		stopSound(eng.rctRoarSFX);

		SetThrusterLevel(eng.rocketThr, 0);

		break;

	default:
		break;

	}
	eng.state = sst;
}

void G422::simEngines(double& dT, RT66& eng)
{
	if (eng.feed & RT66::FUEL_PUMP) // simulate fuel line pressure from pumps
	{
		if (eng.fuelPrs < 1)
		{
			double deltaPrs = 0.22 * dT;
			eng.fuelPrs = min(eng.fuelPrs + deltaPrs, 1);
		}
	}
	else
	{
		if (eng.fuelPrs > 0)
		{
			double deltaPrs = -0.12 * (1 - eng.genPct * 0.5) * dT;
			eng.fuelPrs = max(eng.fuelPrs + deltaPrs, 0);
		}
	}

	if (GetPropellantMass(allMainFuel) < 0.001) eng.fuelPrs = 0;

	if (eng.feed & RT66::OXY_PUMP) // simulate oxidizer line pressure from pumps
	{
		if (eng.oxyPrs < 1)
		{
			double deltaPrs = 0.22 * dT;
			eng.oxyPrs = min(eng.oxyPrs + deltaPrs, 1);
		}
	}
	else
	{
		if (eng.oxyPrs > 0)
		{
			double deltaPrs = -0.12 * (1 - eng.genPct * .5) * dT;
			eng.oxyPrs = max(eng.oxyPrs + deltaPrs, 0);
		}
	}

	if (GetPropellantMass(oxidizerFuel) < 0.001) eng.oxyPrs = 0;

	if (eng.state == RT66::SST_INOP) return; // no simulation needed

	switch (eng.state)
	{
	default:
	case RT66::SST_CUT:
	case RT66::SST_FAIL:
		return;

	case RT66::SST_STARTGEN:
		if (eng.fuelPrs == 0) return cueEngines(eng, RT66::SST_CUT);

		playSound(eng.extStartSFX, false, 0.86f);
		eng.genPct += dT * 0.04854368932038834951 * ENGINE_IDLE_EXTPCT; //   1 / 20.6 secs -- engine spoolup time must match sound fx length

		if (eng.genPct >= ENGINE_IDLE_EXTPCT) cueEngines(eng, RT66::SST_RUN_EXT); // running!
		return;

	case RT66::SST_RUN_EXT:
	{
		//                       dynP :   0     4     8      12      16     20     24      28     32     36     40    44
		static double engFx_subs[12] = { 0.92, 0.94, 0.97, 0.9825, 0.9992, 0.995, 0.952,  0.83,  0.68,  0.54,  0.39, 0.32 };
		static double engFx_sprs[12] = { 0.50, 0.65, 0.83, 0.9885, 0.9998,   1,   0.9987, 0.884, 0.753, 0.425, 0.28, 0.14 };
		double* engFx = (GetMachNumber() >= 1) ? engFx_sprs : engFx_subs; // very blunt, yet effective sonic transition

		//                       Mach :   0     0.5     1   1.5   2     2.5     3    3.5 
		static double engFx_mach[8] = { 0.974, 0.995, 0.998, 1, 0.975, 0.822, 0.585, 0.12 };

		double DpFx = GetDynPressure() * 2.27272727e-5;

		int perfRefLo = int(floor(DpFx * 11.0));   perfRefLo = (perfRefLo > 11) ? 11 : (perfRefLo < 0) ? 0 : perfRefLo;
		int perfRefHi = int(ceil(DpFx * 11.0));   perfRefHi = (perfRefHi > 11) ? 11 : (perfRefHi < 0) ? 0 : perfRefHi;

		double delta = engFx[perfRefHi] - engFx[perfRefLo];
		double engPerf = engFx[perfRefLo] + delta * ((DpFx * 11.0 - (double)perfRefLo) * 0.0909090909);

		double MnFx = GetMachNumber() * 0.285714286;

		perfRefLo = int(floor(MnFx * 7.0));  perfRefLo = (perfRefLo > 7) ? 7 : (perfRefLo < 0) ? 0 : perfRefLo;
		perfRefHi = int(ceil(MnFx * 7.0));  perfRefHi = (perfRefHi > 7) ? 7 : (perfRefHi < 0) ? 0 : perfRefHi;

		delta = engFx_mach[perfRefHi] - engFx_mach[perfRefLo];
		engPerf *= engFx_mach[perfRefLo] + delta * ((MnFx * 7.0 - (double)perfRefLo) * 0.142857143);

		if (eng.burnerToggle && burnerEngMode > 0)
		{
			eng.thr = ENGINE_IDLE_EXTPCT + (GetEngineLevel(ENGINE_MAIN) * (1.2 - ENGINE_IDLE_EXTPCT) * eng.fuelPrs);

			if (eng.thr > 1) // burners, engage!
			{
				double rhtLvl = max(0, (eng.thr - 1) * 5.0);
				SetThrusterLevel(eng.burnerThr, rhtLvl);
				playSound(eng.burnerSFX, true, float(rhtLvl * burnerEngMode));

				eng.thr = 1;
			}
			else
			{
				stopSound(eng.burnerSFX);
				SetThrusterLevel(eng.burnerThr, 0);
			}
		}
		else
		{
			eng.thr = ENGINE_IDLE_EXTPCT + (GetEngineLevel(ENGINE_MAIN) * (1 - ENGINE_IDLE_EXTPCT) * eng.fuelPrs);

			stopSound(eng.burnerSFX);
			SetThrusterLevel(eng.burnerThr, 0);
		}

		eng.genPct += ((eng.thr - eng.genPct) * ENGINE_RAMP_SCALAR * dT);
		eng.epr += (eng.genPct * engPerf - eng.epr) * dT * ENGINE_RAMP_SCALAR;

		if (eng.fuelPrs <= 0.01) return cueEngines(eng, RT66::SST_STOPGEN); // dry stall

		SetThrusterIsp(eng.gasGenThr, ISPMAX_MAIN_GEN * eng.epr);
		SetThrusterMax0(eng.gasGenThr, MAXTHRUST_MAIN_GEN * eng.epr);
		SetThrusterLevel(eng.gasGenThr, eng.genPct);

		playSound(eng.extRunSFX, true, float(220 + int(eng.epr * 25)) / 255);
		playSound(eng.jetRoarSFX, true, float(eng.epr));

		SetThrusterIsp(eng.burnerThr, ISPMAX_MAIN_AFB * eng.epr * burnerEngMode);
		SetThrusterMax0(eng.burnerThr, MAXTHRUST_MAIN_AFB * eng.epr * burnerEngMode);

		// not quite sure how or why this works.... got to it by tweaking around with plot-o-matic...
		double alt = GetAltitude() * .00001;
		eng.contrailLevel = max(0, ((alt - 0.028) * alt) * ((0.18 - alt) * 300.0) * eng.epr);
	}
	return;

	case RT66::SST_STOPGEN:
		if (eng.genPct > ENGINE_IDLE_EXTPCT)
		{
			eng.genPct -= eng.genPct * ENGINE_RAMP_SCALAR * dT;

			playSound(eng.extRunSFX, false, 0.86f);
		}
		else
		{
			playSound(eng.extStopSFX, false, 0.86f);
			eng.genPct -= dT * 0.07407407407407407407 * ENGINE_IDLE_EXTPCT; // 1 / 13.5 secs -- same deal...

			if (eng.genPct <= 0) cueEngines(eng, RT66::SST_CUT); // off!
		}

		return;

	case RT66::SST_STARTRCKT:
		if (eng.fuelPrs == 0 || eng.oxyPrs == 0) return cueEngines(eng, RT66::SST_CUT);

		playSound(eng.intStartSFX, false, 0.86f);
		eng.genPct += dT * 0.04444444444444444444 * ENGINE_IDLE_INTPCT; //   1 / 22.5 secs -- engine spoolup time must match sound fx length

		if (eng.genPct >= ENGINE_IDLE_INTPCT)
			cueEngines(eng, RT66::SST_RUN_INT); // running!
		return;

	case RT66::SST_RUN_INT:
	{
		// engines are shut if throttle gets fully closed	(or when they run out of fuel)
		if (eng.fuelPrs * eng.oxyPrs <= 0.01 || GetEngineLevel(ENGINE_MAIN) < 0.001) return cueEngines(eng, RT66::SST_STOPRCKT);

		eng.thr = ENGINE_IDLE_INTPCT + (GetEngineLevel(ENGINE_MAIN) * (1 - ENGINE_IDLE_INTPCT) * eng.fuelPrs * eng.oxyPrs);
		eng.genPct += ((eng.thr - eng.genPct) * 0.52 * dT);
		eng.epr = eng.genPct * eng.thr;

		SetThrusterLevel(eng.rocketThr, eng.epr);

		playSound(eng.intRunSFX, true, float(220 + int(eng.epr * 15)) / 255);
		playSound(eng.rctRoarSFX, true, float(eng.thr));

		double flowRate = 0;

		if (engMainL.state == RT66::SST_RUN_INT && engMainR.state == RT66::SST_RUN_INT) flowRate = GetPropellantFlowrate(allMainFuel) * 0.5;
		else flowRate = GetPropellantFlowrate(allMainFuel);

		// we've halved the propellant flowrate (wrt the legacy model) since now this bit runs for each of the two engines
		SetPropellantMass(oxidizerFuel, max(0, GetPropellantMass(oxidizerFuel) - (flowRate * 0.5 * dT * OXYFUEL_RATIO)));
	}
	return;

	case RT66::SST_STOPRCKT:
		if (eng.genPct > ENGINE_IDLE_INTPCT)
		{
			eng.genPct -= eng.genPct * ENGINE_RAMP_SCALAR * dT;
			playSound(eng.intRunSFX, false, 0.86f);
		}
		else
		{
			playSound(eng.intStopSFX, false, 0.86f);
			eng.genPct -= dT * .1 * ENGINE_IDLE_INTPCT; // 1 / 10 secs -- same deal...

			if (eng.genPct <= 0) cueEngines(eng, RT66::SST_CUT); // off!
		}
		return;
	}
}