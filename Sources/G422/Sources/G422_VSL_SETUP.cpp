#include "../Headers/G422.h"
#include "../Headers/G422_DVC.h"
#include "../Headers/G422_MDL_EXT.h"
#include <cstring>

int G422::vesselCount = 0;
G422::DrawRes G422::drawRes;
map<UINT, int> G422::vcSwitchIndexByMGID;
map<UINT, int> G422::vcKnobIndexByMGID;

G422::G422(OBJHANDLE hVessel, int fmodel) : VESSEL4 (hVessel, fmodel)
{
	ucso = UCSO::Vessel::CreateInstance(this);

	const char* ucsoVersion = ucso->GetUCSOVersion();
	ucsoInstalled = ucsoVersion;

	if (ucsoInstalled)
	{
		sprintf(buffer, "UCSO version: %s", ucsoVersion);
		message = strdup(buffer);

		ucso->SetUnpackingRange(30);
	}

	apuPackA.hydSys = &hydSysA;
	apuPackA.startSFX = SFX_LAPU_START;
	apuPackA.runSFX = SFX_LAPU_RUN;
	apuPackA.stopSFX = SFX_LAPU_STOP;	

	apuPackB.hydSys = &hydSysB;
	apuPackB.startSFX = SFX_RAPU_START;
	apuPackB.runSFX = SFX_RAPU_RUN;
	apuPackB.stopSFX = SFX_RAPU_STOP;
	
	vcEicasScreens[0] = -1;
	vcEicasScreens[1] = -1;
	vcEicasScreens[2] = -1;
	vcEicasScreens[3] = -1;
	vcEicasScreens[4] = -1;
	vcEicasScreens[5] = -1;
	
	if (!vesselCount)
	{
		drawRes.mfdLabelsFont = oapiCreateFont(12, true, "FixedSys", FONT_BOLD);

		drawRes.brSet[0] = oapiCreateBrush(0x000000);
		drawRes.brSet[1] = oapiCreateBrush(0x0000FF);
		drawRes.brSet[2] = oapiCreateBrush(0x200000);
		drawRes.brSet[3] = oapiCreateBrush(0xFFFFFF);

		drawRes.spSet[0] = oapiCreatePen(1, 2, 0x00FF00);
		drawRes.spSet[1] = oapiCreatePen(1, 2, 0xFFCC00);
		drawRes.spSet[2] = oapiCreatePen(1, 2, 0x0000FF);
		drawRes.spSet[3] = oapiCreatePen(1, 2, 0x00CCFF);
		drawRes.spSet[4] = oapiCreatePen(1, 2, 0xFFFFFF);
		drawRes.spSet[5] = oapiCreatePen(1, 2, 0x200000);

		// map the array indices of the switches by their mesh IDs... makes them easier to find
		for (int i = 0; i < VC_SWITCH_COUNT; ++i) vcSwitchIndexByMGID[(vcSwitchDefs[i].mgid)] = i;

		for (int i = 0; i < VC_KNOB_COUNT; ++i) vcKnobIndexByMGID[(vcKnobDefs[i].mgid)] = i;
	}
	
	++vesselCount;
}

G422::~G422()
{
	clearAnimations();
	oapiDestroySurface(vcEicasRes);

	delete ucso;
	delete xrSound;

	--vesselCount;

	if (!vesselCount)
	{
		oapiReleaseFont(drawRes.mfdLabelsFont);
		
		oapiReleasePen(drawRes.spSet[0]);
		oapiReleasePen(drawRes.spSet[1]);
		oapiReleasePen(drawRes.spSet[2]);
		oapiReleasePen(drawRes.spSet[3]);
		oapiReleasePen(drawRes.spSet[4]);
		oapiReleasePen(drawRes.spSet[5]);
		
		oapiReleaseBrush(drawRes.brSet[0]);
		oapiReleaseBrush(drawRes.brSet[1]);
		oapiReleaseBrush(drawRes.brSet[2]);
		oapiReleaseBrush(drawRes.brSet[3]);
		
		oapiDestroySurface(drawRes.bmpRes[0]);
	}
}

void G422::clbkSetClassCaps(FILEHANDLE cfg)
{
	SetEmptyMass(EMPTY_MASS);
	SetSize(43);
	
	extModel = oapiLoadMeshGlobal("G422/G422_ext");
	dvcModel = oapiLoadMeshGlobal("G422/G422_dvc");	
	leftVcModel = oapiLoadMeshGlobal("G422/G422_dvc_L");	
	rightVcModel = oapiLoadMeshGlobal("G422/G422_dvc_R");	
	
	extModelId  =  AddMesh(extModel);     SetMeshVisibilityMode (extModelId,  MESHVIS_EXTERNAL); 
	dvcModelId  =  AddMesh(dvcModel);     SetMeshVisibilityMode (dvcModelId,  MESHVIS_VC);	    
	dvcModelIdR =  AddMesh(rightVcModel); SetMeshVisibilityMode (dvcModelIdR, MESHVIS_VC);	 
	dvcModelIdL =  AddMesh(leftVcModel);  SetMeshVisibilityMode (dvcModelIdL, MESHVIS_NEVER);	 

	static SURFHANDLE rcsTex = oapiRegisterExhaustTexture("exhaust_atrcs");
	static SURFHANDLE exhaustTex[4];
	exhaustTex[0] = oapiRegisterExhaustTexture("G422/exhaust_main");
	exhaustTex[1] = oapiRegisterExhaustTexture("G422/exhaust_ramx");
	exhaustTex[2] = oapiRegisterExhaustTexture("G422/exhaust_oxy");
	exhaustTex[3] = oapiRegisterExhaustTexture("G422/exhaust_oms");

	SetTouchdownPoints(TOUCHDOWN_GEARDOWN, TOUCHDOWN_COUNT);

	SetGravityGradientDamping (20.0);
	SetCrossSections (_V(575, 1505, 90));
	SetMaxWheelbrakeForce (8e5);
	SetPMI (_V(85.0, 105.0, 45.0));
	SetRotDrag (_V(0.85, 0.82, 0.75));
	
	EnableTransponder(true);

	ucso->SetSlotAttachment(0, CreateAttachment(false, V3_BAY_ATTACH_001, _Y, _Z, "BAY1"));
	ucso->SetSlotAttachment(1, CreateAttachment(false, V3_BAY_ATTACH_002, _Y, _Z, "BAY2"));
	ucso->SetSlotAttachment(2, CreateAttachment(false, V3_BAY_ATTACH_003, _Y, _Z, "BAY3"));
	ucso->SetSlotAttachment(3, CreateAttachment(false, V3_BAY_ATTACH_004, _Y, _Z, "BAY4"));
	ucso->SetSlotAttachment(4, CreateAttachment(false, V3_BAY_ATTACH_005, _Y, _Z, "BAY5"));
	ucso->SetSlotAttachment(5, CreateAttachment(false, V3_BAY_ATTACH_006, _Y, _Z, "BAY6"));
	ucso->SetSlotAttachment(6, CreateAttachment(false, V3_BAY_ATTACH_007, _Y, _Z, "BAY7"));
	ucso->SetSlotAttachment(7, CreateAttachment(false, V3_BAY_ATTACH_008, _Y, _Z, "BAY8"));
	
	static VECTOR3 navLightsPos[7] =
	{
		 V3_LGT_NAV_GREEN, V3_LGT_NAV_RED, V3_LGT_STROBE_L, V3_LGT_STROBE_R, V3_LGT_STROBE_T, V3_LGT_BCN_1, V3_LGT_BCN_2
	};
	static VECTOR3 navLightsColor[3] = {{0.5, 1, 0.5}, {1, 0.5, 0.5}, {1, 1, 1}};
	
	navGreen.shape = BEACONSHAPE_DIFFUSE;      navRed.shape = BEACONSHAPE_DIFFUSE;    navWhite.shape = BEACONSHAPE_DIFFUSE;
	navGreen.pos = navLightsPos;			   navRed.pos = navLightsPos +1;		  navWhite.pos = navLightsPos +4;
	navGreen.col = navLightsColor;			   navRed.col = navLightsColor+1;		  navWhite.col =  navLightsColor+2;
	navGreen.size = .25;					   navRed.size = .25;					  navWhite.size = .25;
	navGreen.falloff = .2;				       navRed.falloff = .2;				      navWhite.falloff = .2;
	navGreen.period = 0;				       navRed.period = 0;				      navWhite.period = 0;	
	navGreen.active = false;				   navRed.active = false;				  navWhite.active = false;
	AddBeacon(&navGreen);					   AddBeacon(&navRed);					  AddBeacon(&navWhite);
	
	leftStrobe.shape = BEACONSHAPE_STAR;       rightStrobe.shape = BEACONSHAPE_STAR;  tailStrobe.shape = BEACONSHAPE_STAR;
	leftStrobe.pos = navLightsPos+2;	       rightStrobe.pos = navLightsPos+3;	  tailStrobe.pos = navLightsPos+4;
	leftStrobe.col = navLightsColor+2;		   rightStrobe.col = navLightsColor+2;	  tailStrobe.col = navLightsColor+2;
	leftStrobe.size = 0.5;					   rightStrobe.size = 0.5;				  tailStrobe.size = 0.5;
	leftStrobe.falloff = .6;                   rightStrobe.falloff = .6;			  tailStrobe.falloff = .6;
	leftStrobe.period = 1;                     rightStrobe.period = 1;				  tailStrobe.period = 1;
	leftStrobe.duration = .05;				   rightStrobe.duration = .05;			  tailStrobe.duration = .05;
	leftStrobe.tofs = 0;                       rightStrobe.tofs = 0;				  tailStrobe.tofs = 0.2;
	leftStrobe.active = false;				   rightStrobe.active = false;			  tailStrobe.active = false;
	AddBeacon(&leftStrobe);					   AddBeacon(&rightStrobe);				  AddBeacon(&tailStrobe);

	beacon1.shape = BEACONSHAPE_DIFFUSE;      beacon2.shape = BEACONSHAPE_DIFFUSE;
	beacon1.pos = navLightsPos+5;			  beacon2.pos = navLightsPos+6;
	beacon1.col = navLightsColor+1;			  beacon2.col = navLightsColor+1;
	beacon1.size = .3;						  beacon2.size = .3;
	beacon1.falloff = .2;					  beacon2.falloff = .2;
	beacon1.period = 1.5;					  beacon2.period = 1.5;
	beacon1.duration = .2;					  beacon2.duration = .2;
	beacon1.tofs = 0.5;						  beacon2.tofs = 1;
	beacon1.active = false;					  beacon2.active = false;
	AddBeacon(&beacon1);					  AddBeacon(&beacon2);

	double tilt = -10.0 * RAD; double tiltY = sin(tilt); double tiltZ = cos(tilt);

	landingLight = static_cast<SpotLight*>(AddSpotLight(_V(0, 1.5, 39.81), _V(0, tiltY, tiltZ),
		5000, 1e-3, 1e-5, 2e-7, RAD * 25, RAD * 40, { 1,1,1,0 }, { 1,1,1,0 }, { 0,0,0,0 }));
	landingLight->Activate(false);

	taxiLight = static_cast<SpotLight*>(AddSpotLight(_V(0, 1.5, 39.81), _V(0, tiltY, tiltZ),
		150, 1e-3, 0, 1e-4, RAD * 25, RAD * 40, { 1,1,1,0 }, { 1,1,1,0 }, { 0,0,0,0 }));
	taxiLight->Activate(false);

	dockingLight = static_cast<SpotLight*>(AddSpotLight(_V(-0.553, 2.77473, 32.0663), _Y,
		150, 1e-3, 0, 1e-3, RAD * 30, RAD * 60, { 1,1,1,0 }, { 1,1,1,0 }, { 0,0,0,0 }));
	dockingLight->Activate(false);

	allMainFuel = CreatePropellantResource(MAXFUEL_MAIN_ALL);
	oxidizerFuel = CreatePropellantResource(MAXFUEL_OXY);
	asfFuel = CreatePropellantResource(MAXFUEL_ASF);
	
	dummyFuel = CreatePropellantResource(1);
	dummyThruster     = CreateThruster(_V0, _Z, 0, dummyFuel, 1, 1, 1);
	dummyThrAirbrakes = CreateThruster(_V0, _Z, 0, dummyFuel, 1, 1, 1);
	dummyHover        = CreateThruster(_V0, _Z, 0, dummyFuel, 1, 1, 1);

	controllerGroup = CreateThrusterGroup(&dummyThruster, 1, THGROUP_MAIN); // this will serve to read the inputs from the throttle

	CreateThrusterGroup(&dummyThrAirbrakes, 1, THGROUP_RETRO);
	CreateThrusterGroup(&dummyHover, 1,	THGROUP_HOVER);
	
	// main engines!
	mainGasGenGroup[0] = engMainL.gasGenThr = CreateThruster({ -10.2083, 0, -21.6084 }, _Z, MAXTHRUST_MAIN_GEN, allMainFuel);
	mainGasGenGroup[1] = engMainR.gasGenThr =  CreateThruster({ 10.2083, 0, -21.6084 }, _Z, MAXTHRUST_MAIN_GEN, allMainFuel);
	
	mainBurnerGroup[0] = engMainL.burnerThr =  CreateThruster({ -10.2083, 0, -21.6084 }, _Z, MAXTHRUST_MAIN_AFB, allMainFuel);
	mainBurnerGroup[1] = engMainR.burnerThr =  CreateThruster({ 10.2083, 0, -21.6084 }, _Z, MAXTHRUST_MAIN_AFB, allMainFuel);
	
	SetEngineLevel(ENGINE_MAIN, 0);
	thrPos = 0;

	AddExhaust(mainBurnerGroup[0], 15, 1.5, V3_EXHAUST_MAIN_L, _Zn, exhaustTex[0]);
	AddExhaust(mainBurnerGroup[1], 15, 1.5, V3_EXHAUST_MAIN_R, _Zn, exhaustTex[0]);

	static PARTICLESTREAMSPEC mainContrail =
	{
		0, 1, .85, 120, 0.05, 30.0, 8, 3.0, PARTICLESTREAMSPEC::EMISSIVE,
		PARTICLESTREAMSPEC::LVL_LIN, 0, 1,
		PARTICLESTREAMSPEC::ATM_FLAT, 1, 1, NULL
	};

	AddParticleStream(&mainContrail, _V(-10, 0.0, -50), _Zn, &engMainL.contrailLevel);
	AddParticleStream(&mainContrail, _V(10, 0.0, -50), _Zn, &engMainR.contrailLevel);

	// rocket mode!
	mainRocketGroup[0] = engMainL.rocketThr =  CreateThruster({ -10.2083, 0, -21.6084 }, _Z, MAXTHRUST_MAIN_RKT, allMainFuel, ISPMAX_MAIN_RKT, ISPMIN_MAIN_RKT);
	mainRocketGroup[1] = engMainR.rocketThr =  CreateThruster({ 10.2083, 0, -21.6084 }, _Z, MAXTHRUST_MAIN_RKT, allMainFuel, ISPMAX_MAIN_RKT, ISPMIN_MAIN_RKT);
	AddExhaust(mainRocketGroup[0], 18, 1.2, V3_EXHAUST_MAIN_L, _Zn, exhaustTex[2]);
	AddExhaust(mainRocketGroup[1], 18, 1.2, V3_EXHAUST_MAIN_R, _Zn, exhaustTex[2]);
	CreateThrusterGroup(mainRocketGroup, 2, THGROUP_USER);

	engMainL.extStartSFX = SFX_ENGINESTART_LMAIN_EXT;
	engMainR.extStartSFX = SFX_ENGINESTART_RMAIN_EXT;
	engMainL.intStartSFX = SFX_ENGINESTART_LMAIN_INT;
	engMainR.intStartSFX = SFX_ENGINESTART_RMAIN_INT;

	engMainL.extRunSFX = SFX_ENGINERUN_LMAIN_EXT;
	engMainR.extRunSFX = SFX_ENGINERUN_RMAIN_EXT;
	engMainL.intRunSFX = SFX_ENGINERUN_LMAIN_INT;
	engMainR.intRunSFX = SFX_ENGINERUN_RMAIN_INT;

	engMainL.jetRoarSFX = SFX_LMAIN_JETROAR;
	engMainR.jetRoarSFX = SFX_RMAIN_JETROAR;
	engMainL.burnerSFX = SFX_LMAIN_BURNER;
	engMainR.burnerSFX = SFX_RMAIN_BURNER;
	engMainL.rctRoarSFX = SFX_LMAIN_RCTROAR;
	engMainR.rctRoarSFX = SFX_RMAIN_RCTROAR;

	engMainL.extStopSFX = SFX_ENGINESTOP_LMAIN_EXT;
	engMainR.extStopSFX = SFX_ENGINESTOP_RMAIN_EXT;
	engMainL.intStopSFX = SFX_ENGINESTOP_LMAIN_INT;
	engMainR.intStopSFX = SFX_ENGINESTOP_RMAIN_INT;
	
	// ramcaster!
	ramcaster = engRamx.thRamx = CreateThruster({ 0, 0, -14.5855 }, _Z, 1, allMainFuel);
	
	AddExhaust(ramcaster, 16, 1.5, V3_RAMX_EXHAUST_001, _Zn, exhaustTex[1]);
	AddExhaust(ramcaster, 18, 1.6, V3_RAMX_EXHAUST_002, _Zn, exhaustTex[1]);
	AddExhaust(ramcaster, 16, 1.5, V3_RAMX_EXHAUST_003, _Zn, exhaustTex[1]);
	AddExhaust(ramcaster, 15, 1.4, V3_RAMX_EXHAUST_004, _Zn, exhaustTex[1]);
	AddExhaust(ramcaster, 15, 1.4, V3_RAMX_EXHAUST_005, _Zn, exhaustTex[1]);

	static PARTICLESTREAMSPEC contrail_ramx =
	{
		0, 2.5, .95, 120, 0.03, 10.0, 5, 3.0, PARTICLESTREAMSPEC::EMISSIVE,
		PARTICLESTREAMSPEC::LVL_PLIN, -1.0, 25.0,
		PARTICLESTREAMSPEC::ATM_PLIN, 0.15, 0.05
	};

	AddExhaustStream(ramcaster, _V(0, -1, -30), &contrail_ramx);
	
	// OMS!
	omsGroup[0] = CreateThruster({ -1.59957, 0, -22.5133 }, _Z, MAXTHRUST_OMS, asfFuel, ISPMAX_OMS, ISPMIN_OMS);
	omsGroup[1] = CreateThruster({ 1.59957, 0, -22.5133 }, _Z, MAXTHRUST_OMS, asfFuel, ISPMAX_OMS,  ISPMIN_OMS);
	AddExhaust(omsGroup[0], 10, .6, V3_EXHAUST_OMS_L, _Zn, exhaustTex[3]);
	AddExhaust(omsGroup[1], 10, .6, V3_EXHAUST_OMS_R, _Zn, exhaustTex[3]);
		
	wingLift = CreateAirfoil3(LIFT_VERTICAL, _V0, VLiftCoeff, 0, 32, 850, 2.1);

	CreateAirfoil3(LIFT_HORIZONTAL, _V(0,0,-22), HLiftCoeff, 0, 12, 180, 2.2);

	CreateControlSurface(AIRCTRL_ELEVATOR,     4.0, 1.5, _V(0,0,-25.0), AIRCTRL_AXIS_AUTO);
	CreateControlSurface(AIRCTRL_ELEVATORTRIM, 4.0, 1.5, _V(0,0,-25.0), AIRCTRL_AXIS_AUTO);

	CreateControlSurface(AIRCTRL_AILERON, 4.0, 1.5, _V( 22.5,0,-25), AIRCTRL_AXIS_AUTO);
	CreateControlSurface(AIRCTRL_AILERON, 4.0, 1.5, _V(-22.5,0,-25), AIRCTRL_AXIS_AUTO);
	
	CreateControlSurface(AIRCTRL_RUDDER, 4.5, 1.5, _V(0,0,-30.0), AIRCTRL_AXIS_AUTO);
	
	// and now, the RCS thrusters
	rcsPitchUp[0] = rcsTransUp[0]   = CreateThruster(_V(0,0, 40), _Y, MAXTRHUST_RCS, asfFuel, ISPMAX_RCS); // raise bow 
	rcsPitchUp[1] = rcsTransDown[1] = CreateThruster(_V(0,0,-40), _Yn, MAXTRHUST_RCS, asfFuel, ISPMAX_RCS); // drop stern

	rcsPitchDown[0] = rcsTransDown[0] = CreateThruster(_V(0,0, 40), _Yn, MAXTRHUST_RCS, asfFuel, ISPMAX_RCS); // drop bow
	rcsPitchDown[1] = rcsTransUp[1]   = CreateThruster(_V(0,0,-40), _Y, MAXTRHUST_RCS, asfFuel, ISPMAX_RCS); // raise stern

	AddExhaust(rcsPitchUp[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_DN_1, _V(-0.1,-1,0), rcsTex); // raise bow
	AddExhaust(rcsPitchUp[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_DN_2, _V(-0.1,-1,0), rcsTex);
	AddExhaust(rcsPitchUp[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_DN_3, _V(-0.1,-1,0), rcsTex);
	AddExhaust(rcsPitchUp[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_DN_4, _V( .1,-1,0), rcsTex);
	AddExhaust(rcsPitchUp[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_DN_5, _V( .1,-1,0), rcsTex);
	AddExhaust(rcsPitchUp[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_DN_6, _V( .1,-1,0), rcsTex);
	
	AddExhaust(rcsPitchUp[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_UP_1, _Y, rcsTex); // drop stern
	AddExhaust(rcsPitchUp[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_UP_2, _Y, rcsTex);
	AddExhaust(rcsPitchUp[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_UP_3, _Y, rcsTex);
	AddExhaust(rcsPitchUp[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_UP_4, _Y, rcsTex);
	AddExhaust(rcsPitchUp[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_UP_5, _Y, rcsTex);
	AddExhaust(rcsPitchUp[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_UP_6, _Y, rcsTex);
	AddExhaust(rcsPitchUp[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_UP_7, _Y, rcsTex);
	AddExhaust(rcsPitchUp[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_UP_8, _Y, rcsTex);
	
	AddExhaust(rcsPitchDown[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_DN_1, _V(-0.1,-1,0), rcsTex); // raise stern
	AddExhaust(rcsPitchDown[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_DN_2, _V(-0.1,-1,0), rcsTex);
	AddExhaust(rcsPitchDown[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_DN_3, _V(-0.1,-1,0), rcsTex);
	AddExhaust(rcsPitchDown[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_DN_4, _V( .1,-1,0), rcsTex);
	AddExhaust(rcsPitchDown[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_DN_5, _V( .1,-1,0), rcsTex);
	AddExhaust(rcsPitchDown[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_DN_6, _V( .1,-1,0), rcsTex);
	
	AddExhaust(rcsPitchDown[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_UP_1, _Y, rcsTex); // drop bow
	AddExhaust(rcsPitchDown[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_UP_2, _Y, rcsTex);
	AddExhaust(rcsPitchDown[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_UP_3, _Y, rcsTex);
	AddExhaust(rcsPitchDown[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_UP_4, _Y, rcsTex);
	
	rcsYawLeft[0]  = rcsTransLeft[0]  = CreateThruster(_V(0,0, 40), _Xn, MAXTRHUST_RCS * .75, asfFuel, ISPMAX_RCS);  // bow left
	rcsYawLeft[1]  = rcsTransRight[1] = CreateThruster(_V(0,0,-40), _X, MAXTRHUST_RCS * .75, asfFuel, ISPMAX_RCS);  // stern right
																									  
	rcsYawRight[0] = rcsTransRight[0] = CreateThruster(_V(0,0, 40), _X, MAXTRHUST_RCS * .75, asfFuel, ISPMAX_RCS);  // bow right
	rcsYawRight[1] = rcsTransLeft[1]  = CreateThruster(_V(0,0,-40), _Xn, MAXTRHUST_RCS * .75, asfFuel, ISPMAX_RCS);  // stern left
	
	AddExhaust(rcsYawLeft[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_RT_1, _X, rcsTex); // bow left
	AddExhaust(rcsYawLeft[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_RT_2, _X, rcsTex);
	AddExhaust(rcsYawLeft[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_RT_3, _X, rcsTex);
	
	AddExhaust(rcsYawLeft[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_LF_1, _Xn, rcsTex); // stern right
	AddExhaust(rcsYawLeft[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_LF_2, _Xn, rcsTex);
	AddExhaust(rcsYawLeft[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_LF_3, _Xn, rcsTex);
	
	AddExhaust(rcsYawRight[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_LF_1, _Xn, rcsTex); // bow right
	AddExhaust(rcsYawRight[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_LF_2, _Xn, rcsTex);
	AddExhaust(rcsYawRight[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSF_LF_3, _Xn, rcsTex);
	
	AddExhaust(rcsYawRight[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_RT_1, _X, rcsTex); // stern left
	AddExhaust(rcsYawRight[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_RT_2, _X, rcsTex);
	AddExhaust(rcsYawRight[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSA_RT_3, _X, rcsTex);
	
	rcsForward[0]  = CreateThruster(_V0,  _Z, MAXTRHUST_RCS * 2.5, asfFuel, ISPMAX_RCS); // fore-aft RCS is intentionally overpowered wrt the other sets...
	rcsBackward[0] = CreateThruster(_V0,  _Zn, MAXTRHUST_RCS * 2.5, asfFuel, ISPMAX_RCS); // this is to make "braking" easier, since there aren't any beefy retro thrusters aboard

	AddExhaust(rcsBackward[0], 2.75, 0.5, V3_RCSF_RFW_1, _Z, rcsTex);
	AddExhaust(rcsBackward[0], 2.75, 0.5, V3_RCSF_RFW_2, _Z, rcsTex);
	AddExhaust(rcsBackward[0], 2.75, 0.5, V3_RCSF_RFW_3, _Z, rcsTex);
	AddExhaust(rcsBackward[0], 2.75, 0.5, V3_RCSF_LFW_1, _Z, rcsTex);
	AddExhaust(rcsBackward[0], 2.75, 0.5, V3_RCSF_LFW_2, _Z, rcsTex);
	AddExhaust(rcsBackward[0], 2.75, 0.5, V3_RCSF_LFW_3, _Z, rcsTex);

	AddExhaust(rcsForward[0], 2.75, 0.5, V3_RCSA_BK_1, _Zn, rcsTex);
	AddExhaust(rcsForward[0], 2.75, 0.5, V3_RCSA_BK_2, _Zn, rcsTex);
	AddExhaust(rcsForward[0], 2.75, 0.5, V3_RCSA_BK_3, _Zn, rcsTex);
	AddExhaust(rcsForward[0], 2.75, 0.5, V3_RCSA_BK_4, _Zn, rcsTex);
	AddExhaust(rcsForward[0], 2.75, 0.5, V3_RCSA_BK_5, _Zn, rcsTex);
	AddExhaust(rcsForward[0], 2.75, 0.5, V3_RCSA_BK_6, _Zn, rcsTex);

	rcsBankRight[0] = CreateThruster(_V( 10,0,0), _Yn,  MAXTRHUST_RCS  * 2.0, asfFuel, ISPMAX_RCS);   // drop port
	rcsBankRight[1] = CreateThruster(_V(-10,0,0), _Y,  MAXTRHUST_RCS  * 2.0, asfFuel, ISPMAX_RCS);   // raise starboard

	rcsBankLeft[0] =  CreateThruster(_V(10, 0, 0), _Y, MAXTRHUST_RCS * 2.0, asfFuel, ISPMAX_RCS);   // raise port
	rcsBankLeft[1] =  CreateThruster(_V(-10, 0, 0), _Yn, MAXTRHUST_RCS * 2.0, asfFuel, ISPMAX_RCS);   // drop starboard

	AddExhaust(rcsBankRight[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSR_UP_001, _Y, rcsTex); // drop right
	AddExhaust(rcsBankRight[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSR_UP_002, _Y, rcsTex);
	AddExhaust(rcsBankRight[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSR_UP_003, _Y, rcsTex);
	AddExhaust(rcsBankRight[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSR_UP_004, _Y, rcsTex);

	AddExhaust(rcsBankRight[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSL_DN_001, _Yn, rcsTex); // raise left
	AddExhaust(rcsBankRight[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSL_DN_002, _Yn, rcsTex);
	AddExhaust(rcsBankRight[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSL_DN_003, _Yn, rcsTex);
	AddExhaust(rcsBankRight[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSL_DN_004, _Yn, rcsTex);
	
	AddExhaust(rcsBankLeft[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSR_DN_001, _Yn, rcsTex); // raise right
	AddExhaust(rcsBankLeft[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSR_DN_002, _Yn, rcsTex);
	AddExhaust(rcsBankLeft[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSR_DN_003, _Yn, rcsTex);
	AddExhaust(rcsBankLeft[0], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSR_DN_004, _Yn, rcsTex);
				
	AddExhaust(rcsBankLeft[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSL_UP_001, _Y, rcsTex); // drop left
	AddExhaust(rcsBankLeft[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSL_UP_002, _Y, rcsTex);
	AddExhaust(rcsBankLeft[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSL_UP_003, _Y, rcsTex);
	AddExhaust(rcsBankLeft[1], RCS_FX_LSCALE, RCS_FX_WSCALE, V3_RCSL_UP_004, _Y, rcsTex);

	setupAnimations();
}

void G422::clbkPostCreation()
{
	if (GetADCtrlMode() && hydSysA.hydPrs < 2800 && hydSysB.hydPrs < 2800) SetADCtrlMode(0);

	xrSound = XRSound::CreateInstance(this);

	xrSound->SetDefaultSoundEnabled(XRSound::MainEngines, false);
	xrSound->SetDefaultSoundEnabled(XRSound::HoverEngines, false);
	xrSound->SetDefaultSoundEnabled(XRSound::RetroEngines, false);
	xrSound->LoadWav(XRSound::AirConditioning, "Sound/G422/amb.wav", XRSound::PlaybackType::InternalOnly);

	xrSound->LoadWav(XRSound::RCSAttackPlusX, "Sound/G422/RCSfire.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(XRSound::RCSAttackPlusY, "Sound/G422/RCSfire.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(XRSound::RCSAttackPlusZ, "Sound/G422/RCSfire.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(XRSound::RCSAttackMinusX, "Sound/G422/RCSfire.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(XRSound::RCSAttackMinusY, "Sound/G422/RCSfire.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(XRSound::RCSAttackMinusZ, "Sound/G422/RCSfire.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(XRSound::RCSSustain, "Sound/G422/RCSsustain.wav", XRSound::PlaybackType::BothViewClose); 
	xrSound->LoadWav(XRSound::FlightWind, (visor->pos == 1 ? "Sound/G422/wind_vsr_up.wav" : "Sound/G422/wind_vsr_down.wav"), XRSound::PlaybackType::BothViewClose);

	xrSound->LoadWav(SFX_ENGINESTART_LMAIN_EXT, "Sound/G422/main_start_gen.wav", XRSound::PlaybackType::BothViewMedium);
	xrSound->LoadWav(SFX_ENGINESTART_RMAIN_EXT, "Sound/G422/main_start_gen.wav", XRSound::PlaybackType::BothViewMedium);
	xrSound->LoadWav(SFX_ENGINESTART_LMAIN_INT, "Sound/G422/main_start_rct.wav", XRSound::PlaybackType::BothViewMedium);
	xrSound->LoadWav(SFX_ENGINESTART_RMAIN_INT, "Sound/G422/main_start_rct.wav", XRSound::PlaybackType::BothViewMedium);

	xrSound->LoadWav(SFX_ENGINERUN_LMAIN_EXT, "Sound/G422/main_run_gen.wav", XRSound::PlaybackType::BothViewMedium);
	xrSound->LoadWav(SFX_ENGINERUN_RMAIN_EXT, "Sound/G422/main_run_gen.wav", XRSound::PlaybackType::BothViewMedium);
	xrSound->LoadWav(SFX_ENGINERUN_LMAIN_INT, "Sound/G422/main_run_rct.wav", XRSound::PlaybackType::BothViewMedium);
	xrSound->LoadWav(SFX_ENGINERUN_RMAIN_INT, "Sound/G422/main_run_rct.wav", XRSound::PlaybackType::BothViewMedium);

	xrSound->LoadWav(SFX_LMAIN_JETROAR, "Sound/G422/roar.wav", XRSound::PlaybackType::BothViewFar);
	xrSound->LoadWav(SFX_RMAIN_JETROAR, "Sound/G422/roar.wav", XRSound::PlaybackType::BothViewFar);
	xrSound->LoadWav(SFX_LMAIN_BURNER, "Sound/G422/afterburner.wav", XRSound::PlaybackType::BothViewFar);
	xrSound->LoadWav(SFX_RMAIN_BURNER, "Sound/G422/afterburner.wav", XRSound::PlaybackType::BothViewFar);
	xrSound->LoadWav(SFX_LMAIN_RCTROAR, "Sound/G422/main_burn_rct.wav", XRSound::PlaybackType::BothViewFar);
	xrSound->LoadWav(SFX_RMAIN_RCTROAR, "Sound/G422/main_burn_rct.wav", XRSound::PlaybackType::BothViewFar);

	xrSound->LoadWav(SFX_ENGINESTOP_LMAIN_EXT, "Sound/G422/main_stop_gen.wav", XRSound::PlaybackType::BothViewMedium);
	xrSound->LoadWav(SFX_ENGINESTOP_RMAIN_EXT, "Sound/G422/main_stop_gen.wav", XRSound::PlaybackType::BothViewMedium);
	xrSound->LoadWav(SFX_ENGINESTOP_LMAIN_INT, "Sound/G422/main_stop_rct.wav", XRSound::PlaybackType::BothViewMedium);
	xrSound->LoadWav(SFX_ENGINESTOP_RMAIN_INT, "Sound/G422/main_stop_rct.wav", XRSound::PlaybackType::BothViewMedium);

	xrSound->LoadWav(SFX_RAMCASTER_START, "Sound/G422/ramcaster_engage_nmnl.wav", XRSound::PlaybackType::BothViewMedium);	
	xrSound->LoadWav(SFX_ENGINERUN_RAMX_LO, "Sound/G422/ramcaster_run_lo.wav", XRSound::PlaybackType::BothViewMedium);
	xrSound->LoadWav(SFX_RAMCASTER_TRST_UP, "Sound/G422/ramcaster_switch_up.wav", XRSound::PlaybackType::BothViewMedium);
	xrSound->LoadWav(SFX_ENGINERUN_RAMX_HI, "Sound/G422/ramcaster_run_hi.wav", XRSound::PlaybackType::BothViewMedium);	

	xrSound->LoadWav(SFX_RAMXBURN, "Sound/G422/ramcaster_dry.wav", XRSound::PlaybackType::BothViewMedium);
	xrSound->LoadWav(SFX_OMSBURN, "Sound/G422/oms_burn.wav", XRSound::PlaybackType::BothViewFar);

	xrSound->LoadWav(SFX_LAPU_START, "Sound/G422/apu_start.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(SFX_LAPU_RUN, "Sound/G422/apu_run.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(SFX_LAPU_STOP, "Sound/G422/apu_stop.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(SFX_RAPU_START, "Sound/G422/apu_start.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(SFX_RAPU_RUN, "Sound/G422/apu_run.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(SFX_RAPU_STOP, "Sound/G422/apu_stop.wav", XRSound::PlaybackType::BothViewClose);

	xrSound->LoadWav(SFX_FUELPUMP_MAIN, "Sound/G422/fuelpump_main.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_FUELPUMP_OXY, "Sound/G422/fuelpump_oxy.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_FUELPUMP_RAMX, "Sound/G422/fuelpump_ramx.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_FUELPUMP_APU, "Sound/G422/fuelpump_apu.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_FUELPUMP_RCS, "Sound/G422/fuelpump_rcs.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_FUELPUMP_OMS, "Sound/G422/fuelpump_oms.wav", XRSound::PlaybackType::InternalOnly);	

	xrSound->LoadWav(SFX_LDG_GEAR, "Sound/G422/landing_gear.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(SFX_CNRDS, "Sound/G422/canards.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(SFX_VSRUP, "Sound/G422/visor_raise.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(SFX_VSRDN, "Sound/G422/visor_lower.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(SFX_HYDRAULICS, "XRSound/Default/Hydraulics1.wav", XRSound::PlaybackType::BothViewClose);
	xrSound->LoadWav(SFX_WINGMTR, "Sound/G422/wing_motor.wav", XRSound::PlaybackType::BothViewClose);

	xrSound->LoadWav(SFX_VC_POP, "Sound/G422/cockpit/pop.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_VC_FLICK, "Sound/G422/cockpit/flick.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_VC_AFLIK, "Sound/G422/cockpit/auto_flipback.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_VC_BEEP, "Sound/G422/cockpit/beep.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_VC_BLIP, "Sound/G422/cockpit/blip.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_VC_CLAK, "Sound/G422/cockpit/clamp.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_VC_TICK, "Sound/G422/cockpit/tick.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_VC_SLACK, "Sound/G422/cockpit/shclack.wav", XRSound::PlaybackType::InternalOnly);	

	xrSound->LoadWav(SFX_AP_ON, "XRSound/Default/Autopilot On.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_AP_OFF, "XRSound/Default/Autopilot Off.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_RCS_NORMAL, "XRSound/Default/RCS Config Normal.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_RCS_DOCKING, "XRSound/Default/RCS Config Docking.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_FUEL_DUMP, "XRSound/Default/Warning Fuel Dump.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_FUEL_FLOW, "XRSound/Default/Fuel Flow.wav", XRSound::PlaybackType::InternalOnly);

	xrSound->LoadWav(SFX_WARN_MAINLOW, "XRSound/Default/Warning Main Fuel Low.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_WARN_OXYLOW, "XRSound/Default/Warning Oxygen Low.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_WARN_ASFLOW, "XRSound/Default/Warning APU Fuel Low.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_WARN_MAINDEP, "XRSound/Default/Warning Main Fuel Depleted.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_WARN_OXYDEP, "XRSound/Default/Warning Oxygen Depleted.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_WARN_ASFDEP, "XRSound/Default/Warning APU Fuel Depleted No Hydraulic Pressure.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_WARN_APUOFF, "XRSound/Default/APU Offline.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_WARN_CTRLOFF, "XRSound/Default/Warning AF Ctrl Surfaces Off.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_WARN_GEARUP, "XRSound/Default/Warning Gear is Up.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_WARN_RESET, "XRSound/Default/System Reset.wav", XRSound::PlaybackType::InternalOnly);

	xrSound->LoadWav(SFX_INFO_MAINFULL, "XRSound/Default/Main Fuel Tanks Full.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_INFO_OXYFULL, "XRSound/Default/LOX Tanks Full.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_INFO_ASFFULL, "XRSound/Default/APU Fuel Tanks Full.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_INFO_V1, "XRSound/Default/V1.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_INFO_ROTATE, "XRSound/Default/Rotate.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_INFO_GEARUP, "XRSound/Default/Gear Up.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_INFO_GEARUPLOCK, "XRSound/Default/Gear Up and Locked.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_INFO_GEARDN, "XRSound/Default/Gear Down.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_INFO_GEARDNLOCK, "XRSound/Default/Gear Down and Locked.wav", XRSound::PlaybackType::InternalOnly);

	xrSound->LoadWav(SFX_BAY_DOORS_CLOSED, "XRSound/Default/Bay Doors Are Closed.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_BAY_SLOT_EMPTY, "XRSound/Default/Slot is Empty.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_BAY_SLOT_OCCUPIED, "XRSound/Default/Slot Is Full.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_BAY_RELEASED, "XRSound/Default/Cargo Deployed.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_BAY_RELEASE_FAILED, "XRSound/Default/Cargo Deployment Failed.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_BAY_GRAPPLED, "XRSound/Default/Cargo Latched In Bay.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_BAY_GRAPPLE_NORANGE, "XRSound/Default/No Cargo in Grapple Range.wav", XRSound::PlaybackType::InternalOnly);
	xrSound->LoadWav(SFX_BAY_GRAPPLE_FAILED, "XRSound/Default/Auto-Grapple Failed.wav", XRSound::PlaybackType::InternalOnly);
	
	oapiVCTriggerRedrawArea(-1, VC_AREA_MFDKEYS);
	oapiVCTriggerRedrawArea(-1, VC_AREA_EICAS_ALL);
	oapiVCTriggerRedrawArea(-1, VC_AREA_CGPOS);

	EditAirfoil(wingLift, 0x01, _V(0, 0, -cgPos), NULL, 0, 0, 0);

	wingPos = wingTipFthr->getToggleState() ? 1 : wingTipWvrd->getToggleState() ? -1 : 0;

	SetAnimation(dvcWingPosAnim, wingTipFthr->getToggleState() ? 1 : wingTipWvrd->getToggleState() ? 0 : 0.5);
	SetAnimation(dvcParkingAnim, parkingBrakeMode);

	if (burnerEngMode == 0) SetAnimation(dvcBurnerAnim, 0);
	else if (burnerEngMode == 0.25) SetAnimation(dvcBurnerAnim, 0.28);
	else if (burnerEngMode == 0.5) SetAnimation(dvcBurnerAnim, 0.5);
	else if (burnerEngMode == 0.75) SetAnimation(dvcBurnerAnim, 0.73);
	else SetAnimation(dvcBurnerAnim, 1);

	CreateVariableDragElement(&landingGear->pos,       8.5, _V(0,   -3.8,  0));
	CreateVariableDragElement(&canards->pos,           1.6, _V(0,    2.0,  35.0));
	CreateVariableDragElement(&engMainL.inltDoor->pos, 1.5, _V(-10, -0.8, -2.0));
	CreateVariableDragElement(&engMainR.inltDoor->pos, 1.5, _V(10,  -0.8, -2.0));
	CreateVariableDragElement(&ramxDoors->pos,         2.5, _V(0,   -2.2,  2.0));

	if (bayDoors->pos == 1) dockHandle = CreateDock(V3_DOCKING_PORT, _Y, _Zn);
	ucso->SetSlotDoor(bayDoors->pos == 1);

	for (const auto& pair : pendingSounds) xrSound->PlayWav(pair.first, pair.second);
	pendingSounds.clear();

	warningSys.mainFull = GetPropellantMass(allMainFuel) == MAXFUEL_MAIN_ALL;
	warningSys.oxyFull = GetPropellantMass(oxidizerFuel) == MAXFUEL_OXY;
	warningSys.asfFull = GetPropellantMass(asfFuel) == MAXFUEL_ASF;
}

void G422::clbkVisualCreated(VISHANDLE vis, int refcount)
{
	visual = vis;
	for (int i = 0; i < 6; i++) selEicasMode(i, vcEicasScreens[i]);
}