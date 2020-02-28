#include "..\Header\G422.h"
#include "..\Header\G422_MDL_DVC.h"
#include "..\Header\G422_DVC.h"

#define EICAS_W 340
#define EICAS_H 410
#define EICAS_MW 170
#define EICAS_MH 205

inline void txtFltData(oapi::Sketchpad* skp, int x, int y, string st, double& val, int prcsn = 2);
inline void txtFltData(oapi::Sketchpad* skp, int x, int y, double& val, int prcsn = 2, bool symbol = true);

#define BLIT_PTARROW_R 56, 39, 10, 13
#define BLIT_PTARROW_L 56, 75, 10, 13
#define BLIT_PTARROW_U 55, 57, 13, 10

const UINT pnl_mgids[6] = { MGID_EICAS_C1, MGID_EICAS_C2, MGID_EICAS_P1, MGID_EICAS_P2, MGID_EICAS_E1, MGID_EICAS_E2 };

void G422::selEicasMode(int eicasIdx, int mode)
{
	//  note on how this works:
	//		in order to allow saving the current screen more even when the screen is off, we use the negated mode numbers to flag that the screen is to go dark
	//      then, when turning it back on - we can use abs() to restore the last mode displayed -- therefore, we don't need a second set of variables! clever, huh?

	if (mode == 0) return; // there is no mode zero...

	static WORD vIdx[4] = { 0,1,2,3 };
	NTVERTEX vtx[4];

	if (mode >= 0)
	{
		float md_X = ((mode - 1) % 4) * .25f;
		float md_Y = floorf((mode - 1) * .25f) * .3f;

		vtx[0].tu = md_X;       vtx[0].tv = md_Y;
		vtx[1].tu = md_X;       vtx[1].tv = md_Y + .3f;
		vtx[2].tu = md_X + .25f; vtx[2].tv = md_Y + .3f;
		vtx[3].tu = md_X + .25f; vtx[3].tv = md_Y;
	}
	else
	{
		// power OFF -- negative
		vtx[0].tu = .925F;   vtx[0].tv = .910F;
		vtx[1].tu = .925F;   vtx[1].tv = .999F;
		vtx[2].tu = .999F;	 vtx[2].tv = .999F;
		vtx[3].tu = .999F;   vtx[3].tv = .910F;
	}

	GROUPEDITSPEC spc;
	spc.flags = GRPEDIT_VTXTEX;
	spc.Vtx = vtx;
	spc.vIdx = vIdx;
	spc.nVtx = 4;

	oapiEditMeshGroup(GetDevMesh(visual, mdlID_dvc), pnl_mgids[eicasIdx], &spc);

	int lMode = VC_eicas_screens[eicasIdx];
	VC_eicas_screens[eicasIdx] = mode;

	// disable redraw on invisible pages
	drawEicas |= (1 << mode);

	if (lMode < 0) return; // irrelevant when toggling screens on from off state

	for (int i = 0; i < 6; i++) // check for panels still displaying the page that was just closed
		if (VC_eicas_screens[i] == lMode) return; // stop here if any are found

	// otherwise, flag disused page OFF so it doesn't go on redrawing while invisible
	drawEicas &= ~(1 << lMode);
}

void G422::clbkMFDMode(int mfd, int mode) { oapiVCTriggerRedrawArea(-1, VC_AREA_MFDKEYS); }

bool G422::clbkVCRedrawEvent(int id, int ev, SURFHANDLE surf)
{
	oapi::Sketchpad* sp;
	static const int mfdUpdateLineup[4] = { MFD_LEFT, MFD_USER1, MFD_RIGHT, MFD_USER2 };

	switch (id)
	{
	case VC_AREA_MFDKEYS:
	{
		sp = oapiGetSketchpad(surf);

		if (!sp) return false;

		sp->SetFont(G422DrawRes.mfdLabelsFont);
		sp->SetTextColor(0xFFCC00);
		sp->SetTextAlign(oapi::Sketchpad::CENTER, oapi::Sketchpad::BASELINE);

		int x = 0;

		for (UINT i = 0; i < 4; ++i)
		{
			for (UINT j = 0; j < 6; ++j)
			{
				const char* label = oapiMFDButtonLabel(mfdUpdateLineup[i], j);

				if (!label) continue;

				sp->Text(25 + x, 32 + (j * 50), label, strlen(label));
			}

			x += 50;

			for (UINT j = 6; j < 12; ++j)
			{
				const char* label = oapiMFDButtonLabel(mfdUpdateLineup[i], j);

				if (!label) continue;

				sp->Text(25 + x, 32 + ((j - 6) * 50), label, strlen(label));
			}

			x += 50;
		}

		x = 0;
		int y = 300;

		for (UINT i = 0; i < 12; ++i)
		{
			const char* label = oapiMFDButtonLabel(MFD_USER3, i);

			if (label) sp->Text(25 + x, 32 + y, label, strlen(label));

			y += 50;

			if (y > 450) { x += 50; y = 300; }
		}

		for (UINT i = 0; i < 12; ++i)
		{
			const char* label = oapiMFDButtonLabel(MFD_USER4, i);

			if (label) sp->Text(25 + x, 32 + y, label, strlen(label));

			y += 50;

			if (y > 450) { x += 50; y = 300; }
		}

		oapiReleaseSketchpad(sp);
	}
	return true;

	case VC_AREA_EICAS_ALL:
	{
		sp = oapiGetSketchpad(surf);

		if (!sp) return false;

		sp->SetFont(MFDS_FONT);
		sp->SetTextColor(0xFFFFFF);
		sp->SetTextAlign(oapi::Sketchpad::LEFT, oapi::Sketchpad::BASELINE);

		sp->SetPen(PEN_AMBER);

		//     ramcaster flight-envelope display
		if (drawEicas & (1 << 7))
		{
			if (thrFX_Y > 0.0001 && thrFX_Y < 1)
			{
				if (thrFX_X > 0.0001 && thrFX_X < 1)
				{
					sp->MoveTo(RDW_RAMXFPR_X + int(thrFX_X * RDW_RAMXFPR_W), RDW_RAMXFPR_Y);
					sp->LineTo(RDW_RAMXFPR_X + int(thrFX_X * RDW_RAMXFPR_W), RDW_RAMXFPR_Y + RDW_RAMXFPR_H);
				}
				sp->MoveTo(RDW_RAMXFPR_X, RDW_RAMXFPR_Y + int(thrFX_Y * RDW_RAMXFPR_H));
				sp->LineTo(RDW_RAMXFPR_X + RDW_RAMXFPR_W, RDW_RAMXFPR_Y + int(thrFX_Y * RDW_RAMXFPR_H));
			}
		}

		// GEN PCT gauges...
		if (drawEicas & (1 << 5))
		{
			double fdat = engMain_L.genPct * 120.0;
			txtFltData(sp, 32, 372, fdat, 1);

			fdat = engMain_R.genPct * 120.0;
			txtFltData(sp, 108, 372, fdat, 1);

			double thr = engMain_L.genPct * PI * 1.25;
			double dx = cos(thr); double dy = sin(thr);
			sp->MoveTo(RDW_GENPCT_XL + int(dx * RDW_GENPCT_R1), RDW_GENPCT_Y + int(dy * RDW_GENPCT_R1));
			sp->LineTo(RDW_GENPCT_XL + int(dx * RDW_GENPCT_R2), RDW_GENPCT_Y + int(dy * RDW_GENPCT_R2));

			thr = engMain_R.genPct * PI * 1.25;
			dx = cos(thr); dy = sin(thr);
			sp->MoveTo(RDW_GENPCT_XR + int(dx * RDW_GENPCT_R1), RDW_GENPCT_Y + int(dy * RDW_GENPCT_R1));
			sp->LineTo(RDW_GENPCT_XR + int(dx * RDW_GENPCT_R2), RDW_GENPCT_Y + int(dy * RDW_GENPCT_R2));

			// EPR gauges
			fdat = engMain_L.epr * PI + PI * .75;
			dx = cos(fdat); dy = sin(fdat);
			sp->MoveTo(202 + int(dx * 12), 360 + int(dy * 12));
			sp->LineTo(202 + int(dx * 18), 360 + int(dy * 18));

			fdat = engMain_R.epr * PI + PI * .75;
			dx = cos(fdat); dy = sin(fdat);
			sp->MoveTo(202 + int(dx * 12), 398 + int(dy * 12));
			sp->LineTo(202 + int(dx * 18), 398 + int(dy * 18));

			fdat = engMain_L.epr * 155.0;
			txtFltData(sp, 205, 370, fdat, 1);

			fdat = engMain_R.epr * 155.0;
			txtFltData(sp, 205, 407, fdat, 1);

			// thr indicators
			thr = engMain_L.thr * PI * 1.25;
			dx = cos(thr); dy = sin(thr);
			sp->SetPen(PEN_WHITE);
			sp->MoveTo(RDW_GENPCT_XL + int(dx * RDW_GENPCT_R2), RDW_GENPCT_Y + int(dy * RDW_GENPCT_R2));
			sp->LineTo(RDW_GENPCT_XL + int(dx * RDW_GENPCT_R3), RDW_GENPCT_Y + int(dy * RDW_GENPCT_R3));

			thr = engMain_R.thr * PI * 1.25;
			dx = cos(thr); dy = sin(thr);
			sp->MoveTo(RDW_GENPCT_XR + int(dx * RDW_GENPCT_R2), RDW_GENPCT_Y + int(dy * RDW_GENPCT_R2));
			sp->LineTo(RDW_GENPCT_XR + int(dx * RDW_GENPCT_R3), RDW_GENPCT_Y + int(dy * RDW_GENPCT_R3));

			// fuel flow
			fdat = GetPropellantFlowrate(fuel_main_allTanks) * .5;
			txtFltData(sp, 28, 448, fdat, 1); txtFltData(sp, 104, 448, fdat, 1); // FFL

			// fuel pressure
			fdat = engMain_L.fuelPrs * 100.0; txtFltData(sp, 28, 482, fdat, 1); // FSP
			fdat = engMain_R.fuelPrs * 100.0; txtFltData(sp, 105, 482, fdat, 1);

			fdat = engMain_L.oxyPrs * 100.0; txtFltData(sp, 197, 480, fdat, 1);
			fdat = engMain_R.oxyPrs * 100.0; txtFltData(sp, 199, 595, fdat, 1);

			fdat = GetPropellantFlowrate(fuel_oxy) * .5;
			txtFltData(sp, 197, 443, fdat, 1); txtFltData(sp, 199, 558, fdat, 1);

			if (engMain_L.state == RT66::SST_RUN_EXT) //   turbine+afterburner mode
				fdat = GetThrusterLevel(RT66_gasGen_thgr[0]) * GetThrusterMax(RT66_gasGen_thgr[0]) +
				GetThrusterLevel(RT66_burner_thgr[0]) * GetThrusterMax(RT66_burner_thgr[0]);
			else if (engMain_L.state == RT66::SST_RUN_INT) //      rocket mode
				fdat = GetThrusterLevel(RT66_rocket_thgr[0]) * GetThrusterMax(RT66_rocket_thgr[0]);

			txtFltData(sp, 28, 595, fdat, 1);

			if (engMain_R.state == RT66::SST_RUN_EXT) //   turbine+afterburner mode
				fdat = GetThrusterLevel(RT66_gasGen_thgr[1]) * GetThrusterMax(RT66_gasGen_thgr[1]) +
				GetThrusterLevel(RT66_burner_thgr[1]) * GetThrusterMax(RT66_burner_thgr[1]);
			else if (engMain_R.state == RT66::SST_RUN_INT) //      rocket mode
				fdat = GetThrusterLevel(RT66_rocket_thgr[1]) * GetThrusterMax(RT66_rocket_thgr[1]);
	
			txtFltData(sp, 104, 595, fdat, 1); // TTR
		}

		if (drawEicas & (1 << 7))
		{
			// RAMCASTER DISPLAY
			double fdat = GetThrusterLevel(ramcaster) * GetThrusterMax(ramcaster);
			txtFltData(sp, 591, 597, fdat, 1);

			fdat = GetPropellantFlowrate(fuel_main_allTanks);
			txtFltData(sp, 529, 569, fdat, 1);

			fdat = engRamx.fuelPrs * 100.0;
			txtFltData(sp, 529, 597, fdat, 1);

			fdat = engRamx.epr * 225.0;
			txtFltData(sp, 715, 597, fdat, 1);

			switch (engRamx.state)
			{
			case RAMCASTER::ENG_RUNLO:
				sp->Text(690, 339, "MODE: LO", 8);
				break;
			case RAMCASTER::ENG_RUNHI:
				sp->Text(690, 339, "MODE: HI", 8);
				break;
			default:
				sp->Text(700, 339, "INOP", 4);
				break;
			}
		}

		// APU gauges
		if (drawEicas & (1 << 8))
		{
			sp->SetTextAlign(oapi::Sketchpad::CENTER, oapi::Sketchpad::BASELINE);

			sp->SetTextColor(0x00FF00);

			double fdat = apuPackA.pwrPct * 100.0;
			txtFltData(sp, 812, 382, fdat, 0);
			fdat = apuPackB.pwrPct * 100.0;
			txtFltData(sp, 875, 382, fdat, 0);

			txtFltData(sp, 812, 427, apuPackA.exhaustTemp, 0);
			txtFltData(sp, 875, 427, apuPackB.exhaustTemp, 0);

			sp->SetPen(PEN_AMBER);
			fdat = apuPackA.pwrPct * -PI;
			double dx = cos(fdat); double dy = sin(fdat);
			sp->MoveTo(812 - int(dx * 20), 379 + int(dy * 20));
			sp->LineTo(812 - int(dx * 27), 379 + int(dy * 27));

			fdat = apuPackB.pwrPct * -PI;
			dx = cos(fdat); dy = sin(fdat);
			sp->MoveTo(875 - int(dx * 20), 379 + int(dy * 20));
			sp->LineTo(875 - int(dx * 27), 379 + int(dy * 27));

			sp->SetTextColor(0xFFFFFF);
			fdat = (apuPackA.exhaustTemp / 527) * -105 * RAD;
			dx = cos(fdat); dy = sin(fdat);
			sp->MoveTo(812 - int(dx * 20), 423 + int(dy * 20));
			sp->LineTo(812 - int(dx * 27), 423 + int(dy * 27));

			fdat = (apuPackB.exhaustTemp / 527) * -105 * RAD;
			dx = cos(fdat); dy = sin(fdat);
			sp->MoveTo(875 - int(dx * 20), 423 + int(dy * 20));
			sp->LineTo(875 - int(dx * 27), 423 + int(dy * 27));

			fdat = (GetPropellantMass(fuel_sys) / GetPropellantMaxMass(fuel_sys)) * 100;
			txtFltData(sp, 1000, 377, fdat, 1);

			fdat = apuPackA.usedFuel + apuPackB.usedFuel;
			txtFltData(sp, 1000, 388, fdat, 1);

			fdat = apuPackA.fuelFlow + apuPackB.fuelFlow;
			txtFltData(sp, 1000, 404, fdat, 3);

			if (apuPackA.state == APU::ENG_RUN && apuPackB.state == APU::ENG_RUN)
			{
				fdat = GetPropellantMass(fuel_sys) / ((apuPackA.fuelFlow + apuPackB.fuelFlow) * 60 * 60);
				string endr = to_string(int(fdat));
				endr += ":";
				endr += to_string(int((fdat - int(fdat)) * 60));

				sp->Text(987, 420, endr.c_str(), endr.length());
			}
		}

		// Hydraulics
		if (drawEicas & (1 << 2))
		{
			double fdat = hydSysA.hydPrs;
			sp->SetTextColor(fdat >= 2800 ? 0x00FF00 : 0x0000FF);
			txtFltData(sp, 300, 261, fdat, 0, false);

			fdat = hydSysB.hydPrs;
			sp->SetTextColor(fdat >= 2800 ? 0x00FF00 : 0x0000FF);
			txtFltData(sp, 474, 261, fdat, 0, false);
		}

		sp->SetPen(PEN_WHITE);
		sp->SetTextColor(0xFFFFFF);
		sp->SetTextAlign(oapi::Sketchpad::LEFT, oapi::Sketchpad::BASELINE);

		// FUEL RESERVES PANEL

		// main
		double flLvl = GetPropellantMass(fuel_main_allTanks);
		txtFltData(sp, FLD_FUEL_MAIN_X, FLD_FUEL_MAIN_Y, " ", flLvl);

		double flMax = GetPropellantMaxMass(fuel_main_allTanks);
		double flPct = (flLvl / flMax) * PI * 1.45;

		double dX = cos(flPct); double dY = sin(flPct);
		sp->SetPen((flPct > PI * .3) ? PEN_AMBER : PEN_RED);
		sp->MoveTo(114 + int(dX * 20), 978 + int(dY * 20));
		sp->LineTo(114 + int(dX * 25), 978 + int(dY * 25));

		// oxidizer
		flLvl = GetPropellantMass(fuel_oxy);
		txtFltData(sp, FLD_FUEL_OXY_X, FLD_FUEL_OXY_Y, " ", flLvl);

		flMax = GetPropellantMaxMass(fuel_oxy);
		flPct = (flLvl / flMax) * PI * 1.45;
		dX = cos(flPct); dY = sin(flPct);
		sp->SetPen((flPct > PI * .3) ? PEN_AMBER : PEN_RED);
		sp->MoveTo(40 + int(dX * 20), 978 + int(dY * 20));
		sp->LineTo(40 + int(dX * 25), 978 + int(dY * 25));

		// systems monopropellant
		flLvl = GetPropellantMass(fuel_sys);
		txtFltData(sp, FLD_FUEL_ASF_X, FLD_FUEL_ASF_Y, " ", flLvl);

		flMax = GetPropellantMaxMass(fuel_sys);
		flPct = (flLvl / flMax) * PI * 1.45;
		dX = cos(flPct); dY = sin(flPct);
		sp->SetPen((flPct > PI * .3) ? PEN_AMBER : PEN_RED);
		sp->MoveTo(185 + int(dX * 20), 978 + int(dY * 20));
		sp->LineTo(185 + int(dX * 25), 978 + int(dY * 25));

		//	ACS CONTROLS (wings position indicator)
		if (drawEicas & (1 << 9))
		{
			if ((wingTipFthr->mp_status | wingTipWvrd->mp_status) & MP_MOVING) // any of the two hinge actuators moving
			{
				sp->SetPen(PEN_RED);

				flPct = (wingTipWvrd->pos - wingTipFthr->pos) * PI * .5;
				dX = cos(flPct); dY = sin(flPct);

				sp->MoveTo(54 - int(dX * 5), 680 + int(dY * 5));
				sp->LineTo(54 - int(dX * 18), 680 + int(dY * 18));
				sp->MoveTo(76 + int(dX * 5), 680 + int(dY * 5));
				sp->LineTo(76 + int(dX * 18), 680 + int(dY * 18));
			}
			else
			{
				sp->SetPen(PEN_WHITE);

				if (wingTipFthr->pos + wingTipWvrd->pos == 0) { dX = 1; dY = 0; }
				else
				{
					dX = 0;
					dY = (wingTipFthr->pos > wingTipWvrd->pos) ? -1 : 1;
				}

				sp->MoveTo(54 - int(dX * 5), 680 + int(dY * 5));
				sp->LineTo(54 - int(dX * 18), 680 + int(dY * 18));
				sp->MoveTo(76 + int(dX * 5), 680 + int(dY * 5));
				sp->LineTo(76 + int(dX * 18), 680 + int(dY * 18));
			}
		}

		oapiReleaseSketchpad(sp);

		if (drawEicas & (1 << 5))
		{
			switch (inltDoors->mp_status)
			{
			case MP_LOW_DETENT:
				oapiBlt(surf, vcRes_eicasSrc, 24, 333, 70, 27, 48, 23);
				oapiBlt(surf, vcRes_eicasSrc, 100, 333, 70, 27, 48, 23);
				break;
			case MP_HI_DETENT:
				oapiBlt(surf, vcRes_eicasSrc, 24, 333, 70, 3, 48, 23);
				oapiBlt(surf, vcRes_eicasSrc, 100, 333, 70, 3, 48, 23);
				break;
			default:
				oapiBlt(surf, vcRes_eicasSrc, 24, 333, 70, 49, 48, 23);
				oapiBlt(surf, vcRes_eicasSrc, 100, 333, 70, 49, 48, 23);
				break;
			}

			if (engMain_L.state == RT66::SST_RUN_INT)
			{
				int engLvl = int(GetEngineLevel(ENGINE_MAIN) * 164);

				if (engLvl < 82)
				{
					oapiBlt(surf, vcRes_eicasSrc, 169, 595 - engLvl, 13, 90 - engLvl, 8, engLvl);
					oapiBlt(surf, vcRes_eicasSrc, 180, 595 - engLvl, 34, 90 - engLvl, 8, engLvl);
				}
				else
				{
					oapiBlt(surf, vcRes_eicasSrc, 169, 513, 13, 8, 8, 82);
					oapiBlt(surf, vcRes_eicasSrc, 180, 513, 34, 8, 8, 82);

					engLvl -= 82;
					oapiBlt(surf, vcRes_eicasSrc, 169, 513 - engLvl, 21, 90 - engLvl, 8, engLvl);
					oapiBlt(surf, vcRes_eicasSrc, 180, 513 - engLvl, 42, 90 - engLvl, 8, engLvl);
				}
			}
			else if (burner_toggle)
			{
				int engLvl = int((GetEngineLevel(ENGINE_MAIN) - .8)) * 85 * 5;

				oapiBlt(surf, vcRes_eicasSrc, 10, 495 - engLvl, 0, 90 - engLvl, 2, engLvl);
				oapiBlt(surf, vcRes_eicasSrc, 86, 495 - engLvl, 0, 90 - engLvl, 2, engLvl);

				int rhtLvl = int(GetThrusterLevel(RT66_burner_thgr[0])) * 85;

				oapiBlt(surf, vcRes_eicasSrc, 12, 495 - rhtLvl, 2, 90 - rhtLvl, 8, rhtLvl);
				oapiBlt(surf, vcRes_eicasSrc, 88, 495 - rhtLvl, 2, 90 - rhtLvl, 8, rhtLvl);
			}
		}

		if (drawEicas & (1 << 9))
		{
			oapiBlt(surf, vcRes_eicasSrc, 209, 700 + int((60 * GetControlSurfaceLevel(AIRCTRL_ELEVATORTRIM))), BLIT_PTARROW_R);

			if (canards->pos >= 1)
			{
				oapiBlt(surf, vcRes_eicasSrc, 115, 676 + int((26 * GetControlSurfaceLevel(AIRCTRL_ELEVATOR))), BLIT_PTARROW_R);
				oapiBlt(surf, vcRes_eicasSrc, 182, 676 + int((26 * GetControlSurfaceLevel(AIRCTRL_ELEVATOR))), BLIT_PTARROW_L);
			}

			oapiBlt(surf, vcRes_eicasSrc, 88, 857 + int((26 * GetControlSurfaceLevel(AIRCTRL_ELEVATOR))), BLIT_PTARROW_R);
			oapiBlt(surf, vcRes_eicasSrc, 209, 857 + int((26 * GetControlSurfaceLevel(AIRCTRL_ELEVATOR))), BLIT_PTARROW_L);

			oapiBlt(surf, vcRes_eicasSrc, 132, 850 - int((26 * GetControlSurfaceLevel(AIRCTRL_AILERON))), BLIT_PTARROW_R);
			oapiBlt(surf, vcRes_eicasSrc, 166, 850 + int((26 * GetControlSurfaceLevel(AIRCTRL_AILERON))), BLIT_PTARROW_L);

			oapiBlt(surf, vcRes_eicasSrc, 149 + int((26 * GetControlSurfaceLevel(AIRCTRL_RUDDER))), 895, BLIT_PTARROW_U);

			switch (landingGears->mp_status) // gear indicators
			{
			case MP_LOW_DETENT:
				oapiBlt(surf, vcRes_eicasSrc, 20, 718, 162, 0, 94, 39);

				break;
			case MP_HI_DETENT:
				oapiBlt(surf, vcRes_eicasSrc, 20, 718, 162, 76, 94, 39);

				break;
			default:
				oapiBlt(surf, vcRes_eicasSrc, 20, 718, 162, 38, 94, 39);
			}
		}
	}

	return true;
	}

	return false;
}

bool G422::clbkDrawHUD(int mode, const HUDPAINTSPEC* hps, oapi::Sketchpad* skp)
{
	if (VC_PoV_station) return true; // anything other than pilot seat gets no HUD

	// draw the default HUD
	VESSEL4::clbkDrawHUD(mode, hps, skp);

	double surfRds = oapiGetSize(GetSurfaceRef());
	double mjdref = oapiGetSimMJD();

	ELEMENTS elms;
	GetElements(elms, mjdref);

	if (oapiCockpitMode() == COCKPIT_VIRTUAL)
	{
		double fltData;

		switch (mode)
		{
		case HUD_SURFACE:
			fltData = GetMachNumber(); txtFltData(skp, 5, 90, "MACH ", fltData);
			fltData = GetDynPressure(); txtFltData(skp, 5, 110, "DynP ", fltData);
			fltData = GetAtmPressure(); txtFltData(skp, hps->W - 150, 90, "StaP ", fltData);

			fltData = GetSlipAngle() * DEG;  txtFltData(skp, hps->W - 120, hps->H - 60, "SLIP ", fltData);
			fltData = GetAOA() * DEG;        txtFltData(skp, hps->W - 120, hps->H - 90, "AoA  ", fltData);

			break;

		case HUD_DOCKING:
			break;

		case HUD_ORBIT:
			GetApDist(fltData); fltData -= surfRds;    txtFltData(skp, 5, 90, "ApA ", fltData);
			GetPeDist(fltData); fltData -= surfRds;    txtFltData(skp, 5, 110, "PeA ", fltData);

			txtFltData(skp, 5, 130, "Ecc ", elms.e);
			txtFltData(skp, 5, 150, "SmA ", elms.a);

			txtFltData(skp, hps->W - 100, 90, "Inc ", elms.i);
			txtFltData(skp, hps->W - 100, 110, "LAN ", elms.theta);

			break;
		}

		if (apSet)
			skp->Text(hps->W - 180, hps->H - 30, "|   ANTISLP   |", 15);

		if (GetNavmodeState(NAVMODE_KILLROT))
			skp->Text(hps->W - 180, hps->H - 30, "|   KILLROT   |", 15);

		if (GetNavmodeState(NAVMODE_PROGRADE))
			skp->Text(hps->W - 180, hps->H - 30, "|   PROGRDE   |", 15);

		if (GetNavmodeState(NAVMODE_RETROGRADE))
			skp->Text(hps->W - 180, hps->H - 30, "|   RETROGD   |", 15);

		if (GetNavmodeState(NAVMODE_NORMAL))
			skp->Text(hps->W - 180, hps->H - 30, "|   NORMAL+   |", 15);

		if (GetNavmodeState(NAVMODE_ANTINORMAL))
			skp->Text(hps->W - 180, hps->H - 30, "|   NORMAL-   |", 15);

		if (GetNavmodeState(NAVMODE_HLEVEL))
			skp->Text(hps->W - 180, hps->H - 30, "|   WINGLVL   |", 15);

		if (GetNavmodeState(NAVMODE_HOLDALT))
			skp->Text(hps->W - 180, hps->H - 30, "|   HOLDALT   |", 15);

		// show RCS / OMS mode
		if (rcs.state == RCS::SST_STBY)
		{
			switch (GetAttitudeMode())
			{
			case RCS_ROT:
				if (!rcs.mode) skp->Text(5, hps->H - 60, "RCS ROT", 7);
				else skp->Text(5, hps->H - 60, "DOCK RCS ROT", 12);
				break;
			case RCS_LIN:
				if (!rcs.mode) skp->Text(5, hps->H - 60, "RCS LIN", 7);
				else skp->Text(5, hps->H - 60, "DOCK RCS LIN", 12);
				break;
			}
		}

		if (oms.state == OMS::SST_STBY && thr_authority) skp->Text(120, hps->H - 30, "|  OMS  |", 9);

		if (prk_brake_mode) skp->Text(5, hps->H - 45, "PK BRAKES", 9);

		// show main throttle position
		fltData = GetThrusterGroupLevel(THGROUP_MAIN) * 100;

		if (burner_toggle) txtFltData(skp, 5, hps->H - 30, (fltData > 80) ? "Th*" : "Th ", fltData, 1);
		else txtFltData(skp, 5, hps->H - 30, "Th ", fltData, 1);

		if (timer < 5) { skp->Text(5, hps->H - 120, message, strlen(message)); }

		if (!displayCargoInfo) return true;

		UCSO::Vessel::CargoInfo cargoInfo = ucso->GetCargoInfo(slotIndex);
		if (!cargoInfo.valid) return true;

		int y = 265;

		skp->Text(5, y, "Selected slot cargo information", 31);
		y += 40;

		sprintf(buffer, "Name: %s", cargoInfo.name);
		skp->Text(5, y, buffer, strlen(buffer));
		y += 20;

		sprintf(buffer, "Mass: %dkg", int(round(cargoInfo.mass)));
		skp->Text(5, y, buffer, strlen(buffer));

		switch (cargoInfo.type)
		{
		case UCSO::Vessel::STATIC:
			skp->Text(5, y - 40, "Type: Static", 12);
			break;

		case UCSO::Vessel::RESOURCE:
			skp->Text(5, y - 40, "Type: Resource", 14);
			y += 20;

			sprintf(buffer, "Resource : %s", cargoInfo.resource);
			skp->Text(5, y, buffer, strlen(buffer));
			y += 20;

			sprintf(buffer, "Resource mass: %d", int(round(cargoInfo.resourceMass)));
			skp->Text(5, y, buffer, strlen(buffer));
			break;

		case UCSO::Vessel::UNPACKABLE_ONLY:
			skp->Text(5, y - 40, "Type: Unpackable only", 21);
			y += 20;

			sprintf(buffer, "Unpacked spawn count: %d cargo(es)", cargoInfo.spawnCount);
			skp->Text(5, y, buffer, strlen(buffer));
			y += 20;

		case UCSO::Vessel::PACKABLE_UNPACKABLE:
			if (cargoInfo.type == UCSO::Vessel::PACKABLE_UNPACKABLE)
			{
				skp->Text(5, y - 40, "Type: Packable and unpackable", 29);
				y += 20;
			}

			switch (cargoInfo.unpackingType)
			{
			case UCSO::Vessel::UCSO_RESOURCE:
				skp->Text(5, y, "Unpacking type: UCSO Resource", 29);
				y += 20;
				break;

			case UCSO::Vessel::UCSO_MODULE:
				skp->Text(5, y, "Unpacking type: UCSO Module", 27);
				y += 20;

				sprintf(buffer, "Breathable: %s", cargoInfo.breathable ? "Yes" : "No");
				skp->Text(5, y, buffer, strlen(buffer));
				break;

			case UCSO::Vessel::ORBITER_VESSEL:
				skp->Text(5, y, "Unpacking type: Orbiter vessel", 30);
				y += 20;

				sprintf(buffer, "Spawn module: %s", cargoInfo.spawnModule);
				skp->Text(5, y, buffer, strlen(buffer));
				y += 20;

				switch (cargoInfo.unpackingMode)
				{
				case UCSO::Vessel::LANDING:
					skp->Text(5, y, "Unpacking mode: Landing", 23);
					break;

				case UCSO::Vessel::DELAYING:
					skp->Text(5, y, "Unpacking mode: Delaying", 24);
					y += 20;

					sprintf(buffer, "Unpacking delay: %is", cargoInfo.unpackingDelay);
					skp->Text(5, y, buffer, strlen(buffer));
					break;

				case UCSO::Vessel::MANUAL:
					skp->Text(5, y, "Unpacking mode: Manual", 22);
					break;
				}
				break;

			case UCSO::Vessel::CUSTOM_CARGO:
				skp->Text(5, y, "Unpacking type: Custom cargo", 28);
				y += 20;

				sprintf(buffer, "Breathable: %s", cargoInfo.breathable ? "Yes" : "No");
				skp->Text(5, y, buffer, strlen(buffer));
				break;
			}
			break;
		}
	}

	return true;
}

inline void txtFltData(oapi::Sketchpad* skp, int x, int y, string st, double& val, int prcsn)
{
	ostringstream strm;

	if (abs(val) < 1000) strm << st << fixed << setprecision(prcsn) << val;
	else if (abs(val) < 1000000) strm << st << fixed << setprecision(prcsn) << (val / 1000) << "K";
	else strm << st << fixed << setprecision(prcsn) << (val / 1000000) << "M";

	st = strm.str();

	skp->Text(x, y, st.c_str(), st.length());
}

inline void txtFltData(oapi::Sketchpad* skp, int x, int y, double& val, int prcsn, bool symbol)
{
	ostringstream strm;

	if (abs(val) < 1000 || !symbol) strm << fixed << setprecision(prcsn) << val;
	else if (abs(val) < 1000000) strm << fixed << setprecision(prcsn) << (val / 1000) << "K";
	else strm << fixed << setprecision(prcsn) << (val / 1000000) << "M";

	string st = strm.str();

	skp->Text(x, y, st.c_str(), st.length());
}



