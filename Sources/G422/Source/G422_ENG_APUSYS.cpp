#include "..\Header\G422.h"
#include "..\Header\G422_MDL_DVC.h"

void G422::cueEngines(APU& eng, APU::SIMSTATE sst)
{
	if (eng.state == sst) return;

	switch (sst)
	{
	case APU::ENG_START:
		if (eng.state == APU::ENG_RUN) return; // can't set a running engine to start again
		PlayVesselWave(SFXID, SFX_APU_START);

		break;
	case APU::ENG_RUN:
		if (eng.state == APU::ENG_STOP) return; // engine doesn't go just from stop to run
		StopVesselWave(SFXID, SFX_APU_START);
		StopVesselWave(SFXID, SFX_APU_STOP);

		break;
	case APU::ENG_STOP:
		if (eng.state == APU::ENG_STOP) return; // can't stop what's not running
		StopVesselWave(SFXID, SFX_APU_RUN);
		StopVesselWave(SFXID, SFX_APU_START);
		if (eng.state == APU::ENG_RUN) PlayVesselWave(SFXID, SFX_APU_STOP); // only makes sound after it's running (should be set by rpm, really)

		break;
	}

	eng.state = sst;
}

void G422::simEngines(double& dT, APU& eng)
{
	if (eng.state == APU::ENG_INOP) return;

	if (eng.feed & APU::FUEL_PUMP) // simulate fuel line pressure from pumps
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
			double deltaPrs = -.12 * (1.0 - eng.pwrPct * .5) * dT;
			eng.fuelPrs = max(eng.fuelPrs + deltaPrs, 0.0);
		}
	}

	double fuelLvl = GetPropellantMass(fuel_sys);

	if (fuelLvl < .001)
	{
		eng.fuelPrs = 0.0;
		return cueEngines(eng, APU::ENG_STOP);
	}

	switch (eng.state)
	{
	case APU::ENG_RUN:
	{
		if (eng.fuelPrs == 0) return cueEngines(eng, APU::ENG_STOP);

		eng.fuelFlow = APU_FUEL_RATE;
		eng.exhaustTemp = 527;

		// ?? whatever apu does... (just noise for now...)
		double usedFuel = APU_FUEL_RATE * dT;
		eng.usedFuel += usedFuel;
		SetPropellantMass(fuel_sys, fuelLvl - usedFuel);

		PlayVesselWave(SFXID, SFX_APU_RUN);

		return;
	}
	case APU::ENG_START:
	{
		if (eng.fuelPrs == 0) return cueEngines(eng, APU::ENG_STOP);

		eng.pwrPct += 0.28 * dT;
		eng.fuelFlow = APU_FUEL_RATE * eng.pwrPct;
		eng.exhaustTemp = 527 * eng.pwrPct;

		double usedFuel = eng.fuelFlow * dT;
		eng.usedFuel += usedFuel;
		SetPropellantMass(fuel_sys, fuelLvl - usedFuel);

		if (eng.pwrPct >= 1.0)
		{
			eng.pwrPct = 1.0;
			cueEngines(apu, APU::ENG_RUN);

			switch (VCKnobs[VC_kbIndexByMGID[MGID_ACS_KNOB]].pos)
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
		}
		return;
	}
	case APU::ENG_STOP:
	{
		if (GetADCtrlMode() != 0) SetADCtrlMode(0);

		eng.pwrPct -= 0.12 * dT;
		eng.fuelFlow = APU_FUEL_RATE * eng.pwrPct;
		eng.exhaustTemp = 527 * eng.pwrPct;

		double usedFuel = eng.fuelFlow * dT;
		eng.usedFuel += usedFuel;
		SetPropellantMass(fuel_sys, fuelLvl - usedFuel);

		if (eng.pwrPct <= 0.0)
		{
			eng.pwrPct = 0.0;
			eng.fuelFlow = 0;
			eng.exhaustTemp = 0;
			cueEngines(apu, APU::ENG_INOP);
		}
		return;
	}
	}
}

