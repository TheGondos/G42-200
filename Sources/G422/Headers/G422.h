#pragma once

#include <UCSO\Vessel.h>
#include <XRSound.h>

#include <math.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>

#include "resource.h"

using namespace std;

class G422;

#define _V0 _V(0,0,0)

#define _X _V(1,0,0)
#define _Y _V(0,1,0)
#define _Z _V(0,0,1)

#define _Xn _V(-1,0,0)
#define _Yn _V(0,-1,0)
#define _Zn _V(0,0,-1)

#define SCALAR_ANGLE 0.15915494309189533577

#define EMPTY_MASS 86e3

#define WAVERIDER_FACTOR 0.025
#define WINGFTHR_LIFTDMP 0.525

#define MAXLIFT_GROUNDEFFECT 0.85
#define MAXHEIG_GROUNDEFFECT 35.0
#define MAXHINV_GROUNDEFFECT 2.8571428571429e-002

#define ENGINE_RAMP_SCALAR  0.42
#define ENGINE_IDLE_EXTPCT  0.05
#define ENGINE_IDLE_INTPCT  0.12

// thrust rating for turbine + afterburner is the same as for full rocket mode
#define MAXTHRUST_MAIN_GEN 68e4
#define MAXTHRUST_MAIN_AFB 82e4

#define MAXTHRUST_MAIN_RKT 150e4

#define MAXTHRUST_RAMX_LO 350e4
#define MAXTHRUST_RAMX_HI 365e4

#define MAXTRHUST_RCS 5500.0
#define MAXTHRUST_OMS 65800.0

#define ISPMAX_MAIN_GEN 41500
#define ISPMAX_MAIN_AFB 13500

#define ISPMAX_MAIN_RKT 7800
#define ISPMIN_MAIN_RKT 7300

#define ISPMAX_RAMXLO 16700
#define ISPMAX_RAMXHI 32000

#define ISPMAX_RCS 6100
#define ISPMAX_OMS 6800

#define ISPMIN_RCS 1100
#define ISPMIN_OMS 1000

#define MAXFUEL_MAIN_ALL 205e3
#define MAXFUEL_OXY 85000.0
#define MAXFUEL_ASF 16000.0

#define OXYFUEL_RATIO 1.15

#define APU_FUEL_RATE 0.035

#define RCS_FX_LSCALE 2.25
#define RCS_FX_WSCALE 0.20
#define OMS_FX_LSCALE 5.4
#define OMS_FX_WSCALE 0.26

class VCSwitch
{
public:
	enum Position
	{
		SW3_DOWN, SW3_MID, SW3_UP, // 3-stop switches
		SW2_DOWN, SW2_UP           // 2-stop switches
	};

	UINT mgid;
	UINT anID;
	MGROUP_ROTATE* mgRot;
	Position pos;

	bool flick(bool upDn, VESSEL4* vesselRef);
	void setPos(Position newPos, VESSEL4* vesselRef);

	char getSaveState();
	void setLoadState(char& cs, VESSEL4* vessel);
};

// this allows us to statically define all switches in a list-like fashion, then instantiate independent VCSwitch structs for new G42-class vessels
struct VCSwitchDef 
{ 
	UINT mgid;
	VCSwitch::Position initPos;
	VECTOR3 refPos, axisPos, axisDir;
	double initState = 0;
};

#define VC_SWITCH_COUNT 44 // number of switches in VC (so far)

class VCKnob
{
public:
	enum Position
	{
		KB3_DOWN, KB3_MID, KB3_UP, // 3-stop knobs
		KB2_DOWN, KB2_UP           // 2-stop knobs
	};

	UINT mgid;
	UINT anID;
	MGROUP_ROTATE* mgRot;
	Position pos;
	double middleState;

	bool flick(bool upDn, VESSEL4* vesselRef);
	void setPos(Position newPos, VESSEL4* vesselRef);

	char getSaveState();
	void setLoadState(char& cs, VESSEL4* vessel);
};

struct VCKnobDef
{
	UINT mgid;
	VCKnob::Position initPos;
	VECTOR3 refPos, axisDir;
	double size;
	float rotAngle;
	double initState, middleState;
};

#define VC_KNOB_COUNT 11 // number of knobs in VC

enum ButtonID 
{
	AP_MASTER_BUTTON, AP_PROGRADE_BUTTON, AP_RETROGRADE_BUTTON, AP_NORMAL_BUTTON, AP_ANTINORMAL_BUTTON, AP_HLEVEL_BUTTON,
	AP_KILLROT_BUTTON, HUD_DOCK_BUTTON, HUD_ORBIT_BUTTON, HUD_SURF_BUTTON, HUD_OFF_BUTTON, HUD_BRT_BUTTON, HUD_CLR_BUTTON,
	MASTER_WARN_BUTTON 
};

struct VCButtonDef
{
	ButtonID bid;
	VECTOR3 refPos;
	double size;
};

#define VC_BUTTON_COUNT 14 // number of buttons in VC

// vessel generic messages
#define VMSG_MPSTRT 1010
#define VMSG_MPSTOP 1011

class MovingPart  // implemented in G422_AUX
{
public:
	enum Status
	{
		MP_INOP, MP_MOVING, MP_LOW_DETENT, MP_REVERSING, MP_HI_DETENT,
	};

	double pos;

	bool sysReset; // will be set to false whever a callback is fired, reset to true after updating systems logic
	Status mpStatus, mpOldStatus;

	MovingPart(UINT animIndex, double rate, Status status, double pos = 0, G422* vesselRef = NULL, int sysId = 0, int soundID = 0);

	void operate(double& dt); // except this one - see SIM UPDATE
	int toggle();
	bool toggle(bool hiDetent);
	void toDetent(int detent);
	bool getToggleState();
	
	// and these two - see scenario RW
	void loadCfgState(int state);
	int getCfgState();

private:
	UINT animIndex;
	double rate;
	G422* vessel;
	int sysID; // id for use in callbacks
	int soundID;	
};

/////////////////////////////////////////////// G422 VESSEL4 CLASS //////////////////////////////////////////////////////////
class G422 : public VESSEL4
{
public:
	static HINSTANCE thisDLL;
	static map<UINT, int> vcSwitchIndexByMGID;
	static map<UINT, int> vcKnobIndexByMGID;

	enum SoundID
	{
		SFX_ENGINESTART_LMAIN_EXT = 1,
		SFX_ENGINESTART_RMAIN_EXT,
		SFX_ENGINESTART_LMAIN_INT,
		SFX_ENGINESTART_RMAIN_INT,

		SFX_ENGINERUN_LMAIN_EXT,
		SFX_ENGINERUN_RMAIN_EXT,
		SFX_ENGINERUN_LMAIN_INT,
		SFX_ENGINERUN_RMAIN_INT,

		SFX_LMAIN_JETROAR,
		SFX_RMAIN_JETROAR,
		SFX_LMAIN_BURNER,
		SFX_RMAIN_BURNER,
		SFX_LMAIN_RCTROAR,
		SFX_RMAIN_RCTROAR,

		SFX_ENGINESTOP_LMAIN_EXT,
		SFX_ENGINESTOP_RMAIN_EXT,
		SFX_ENGINESTOP_LMAIN_INT,
		SFX_ENGINESTOP_RMAIN_INT,

		SFX_RAMCASTER_START,
		SFX_ENGINERUN_RAMX_LO,
		SFX_RAMCASTER_TRST_UP,
		SFX_ENGINERUN_RAMX_HI,

		SFX_RAMXBURN,
		SFX_OMSBURN,

		SFX_LAPU_START,
		SFX_LAPU_RUN,
		SFX_LAPU_STOP,
		SFX_RAPU_START,
		SFX_RAPU_RUN,
		SFX_RAPU_STOP,

		SFX_FUELPUMP_MAIN,
		SFX_FUELPUMP_OXY,
		SFX_FUELPUMP_RAMX,
		SFX_FUELPUMP_APU,
		SFX_FUELPUMP_RCS,
		SFX_FUELPUMP_OMS,

		SFX_LDG_GEAR,
		SFX_CNRDS,
		SFX_VSRUP,
		SFX_VSRDN,
		SFX_HYDRAULICS,
		SFX_WINGMTR,

		SFX_VC_POP,
		SFX_VC_FLICK,
		SFX_VC_AFLIK,
		SFX_VC_BEEP,
		SFX_VC_BLIP,
		SFX_VC_CLAK,
		SFX_VC_TICK,
		SFX_VC_SLACK,

		SFX_AP_ON,
		SFX_AP_OFF,
		SFX_RCS_NORMAL,
		SFX_RCS_DOCKING,
		SFX_FUEL_DUMP,
		SFX_FUEL_FLOW,

		SFX_WARN_MAINLOW,
		SFX_WARN_OXYLOW,
		SFX_WARN_ASFLOW,
		SFX_WARN_MAINDEP,
		SFX_WARN_OXYDEP,
		SFX_WARN_ASFDEP,
		SFX_WARN_APUOFF,
		SFX_WARN_CTRLOFF,
		SFX_WARN_GEARUP,
		SFX_WARN_RESET,

		SFX_INFO_MAINFULL,
		SFX_INFO_OXYFULL,
		SFX_INFO_ASFFULL,
		SFX_INFO_V1,
		SFX_INFO_ROTATE,
		SFX_INFO_GEARUP,
		SFX_INFO_GEARUPLOCK,
		SFX_INFO_GEARDN,
		SFX_INFO_GEARDNLOCK,

		SFX_BAY_DOORS_CLOSED,
		SFX_BAY_SLOT_EMPTY,
		SFX_BAY_SLOT_OCCUPIED,
		SFX_BAY_RELEASED,
		SFX_BAY_RELEASE_FAILED,
		SFX_BAY_GRAPPLED,
		SFX_BAY_GRAPPLE_NORANGE,
		SFX_BAY_GRAPPLE_FAILED
	};

	enum SystemID
	{
		SYSID_LGRS = 1,
		SYSID_CNRD,
		SYSID_VISR,
		SYSID_BAY,
		SYSID_RAMX,
		SYSID_RCSDR,
		SYSID_HYDS,
		SYSID_MAINPMP,
		SYSID_FUELDMP,
		SYSID_DCKSPLY,
		SYSID_ANTSLP,
		SYSID_WARN
	};

	G422 (OBJHANDLE hVessel, int fmodel);
	~G422 ();

	void clbkSetClassCaps(FILEHANDLE cfg);
	void clbkPostCreation();

	void clbkVisualCreated(VISHANDLE vis, int refcount);

	// SCENARIO READ/WRITE
	void clbkLoadStateEx(FILEHANDLE scn, void* status);
	void clbkSaveState(FILEHANDLE scn);

	// DVC SETUP
	bool clbkLoadVC(int id);

	// DVC DISPLAYS
	void clbkMFDMode(int mfd, int mode);

	bool clbkVCRedrawEvent(int id, int ev, SURFHANDLE surf);
	bool clbkDrawHUD(int mode, const HUDPAINTSPEC* hps, oapi::Sketchpad* skp);

	// SIM CONTROLS
	void clbkRCSMode(int mode);
	void clbkNavMode(int mode, bool active);
	bool clbkVCMouseEvent(int id, int ev, VECTOR3& p);
	int clbkConsumeDirectKey(char* kstate);
	int clbkConsumeBufferedKey(DWORD key, bool down, char* kstate);

	// VESSEL UPDATE
	void clbkPreStep(double simt, double simdt, double mjd);
	int clbkGeneric(int msgid, int prm, void* context);

	MovingPart* wingTipWvrd;
	MovingPart* wingTipFthr;
	
private:
	static int vesselCount;

	struct DrawRes
	{
		bool def;

		SURFHANDLE bmpRes[1];

		oapi::Font* mfdLabelsFont;

		oapi::Brush* brSet[8];
		oapi::Pen* spSet[8];
	} static drawRes;

	static VCSwitchDef vcSwitchDefs[VC_SWITCH_COUNT];
	VCSwitch vcSwitches[VC_SWITCH_COUNT];

	static VCKnobDef vcKnobDefs[VC_KNOB_COUNT];
	VCKnob vcKnobs[VC_KNOB_COUNT];

	static VCButtonDef vcButtonDefs[VC_BUTTON_COUNT];	

	UCSO::Vessel* ucso;
	bool ucsoInstalled;
	bool cargoMode = false;
	bool clearMessage = false;

	int cargoIndex = 0, slotIndex = 0;
	int fuelIndex = 0; // 0: main, 1: ASF, 2: oxidizer
	const char* message;
	char buffer[256];
	double timer = 0;

	bool apSet = false, rudderSet = false, brakesReset = false;
	DOCKHANDLE dockHandle = nullptr;

	XRSound* xrSound = nullptr;
	vector<pair<int, bool>> pendingSounds;
	
	MESHHANDLE extModel;
	MESHHANDLE dvcModel;
	MESHHANDLE rightVcModel;
	MESHHANDLE leftVcModel;
	
	UINT extModelId, dvcModelId, dvcModelIdR, dvcModelIdL;

	SURFHANDLE vcMfdTex;
	SURFHANDLE vcEicasTex;
	
	SURFHANDLE vcEicasRes;
	
	PROPELLANT_HANDLE allMainFuel;
	PROPELLANT_HANDLE asfFuel, oxidizerFuel, dummyFuel; 
	
	THRUSTER_HANDLE dummyThruster, dummyHover, dummyThrAirbrakes;
	
	THRUSTER_HANDLE mainGasGenGroup[2];
	THRUSTER_HANDLE mainBurnerGroup[2];
	THRUSTER_HANDLE mainRocketGroup[2];
	THRUSTER_HANDLE ramcaster;
	THRUSTER_HANDLE omsGroup[2];

	THRUSTER_HANDLE rcsPitchUp[2], rcsPitchDown[2],  rcsTransUp[2], rcsTransDown[2];
	THRUSTER_HANDLE rcsYawLeft[2], rcsYawRight[2], rcsTransLeft[2], rcsTransRight[2];
	THRUSTER_HANDLE rcsForward[1], rcsBackward[1];
	THRUSTER_HANDLE rcsBankRight[2], rcsBankLeft[2];

	THGROUP_HANDLE masterGroup, controllerGroup;
	THGROUP_HANDLE rcsForwardGroup, rcsR1Group, rcsR2Group;
	
	AIRFOILHANDLE wingLift;
	
	CTRLSURFHANDLE acsCndrs;
	
	BEACONLIGHTSPEC navGreen, navRed, navWhite, leftStrobe, rightStrobe, tailStrobe, beacon1, beacon2;
	SpotLight* landingLight;
	SpotLight* taxiLight;
	SpotLight* dockingLight;
	
	MovingPart* landingGear;
	MovingPart* visor;
	MovingPart* canards;
	MovingPart* rcsDoors;
	MovingPart* ramxDoors;
	MovingPart* bayDoors;
	
	MGROUP_ROTATE* wingTipRotL,* wingTipRotR,* wingTipLightsRotL,* wingTipLightsRotR; 
	MGROUP_ROTATE* elevatorRotL,* elevatorRotR; 	
	
	VECTOR3 wingTipLightsL[2];
	VECTOR3 wingTipLightsR[2];	
	
	int vcEicasScreens[6];
	int drawEicas = (1 << 5) | (1 << 7);
	
	int vcPovStation = 0; // 0: pilot 1: F/0 (for now...)
	
	VISHANDLE visual;
	
	// individual animations for VC (not simple switches)
	UINT dvcRamxAnim;
	UINT dvcWingPosAnim;
	UINT dvcThrottleAnim;
	UINT dvcStickPitchAnim;
	UINT dvcStickRollAnim;
	UINT dvcTillerAnim;
	UINT dvcParkingAnim;
	UINT dvcGearAnim;
	UINT dvcUndockAnim;
	UINT dvcBurnerAnim;
	
	// aerodynamic control surfaces
	UINT canardAnim;
	UINT rudderAnim;
	UINT aileronAnim;
	UINT elevatorAnim;

	// gimbals (innermost out)
	UINT dvcAdiAnimC, dvcAdiAnimB, dvcAdiAnimA;
	MGROUP_ROTATE* adiRotG1,* adiRotG2;
	
	double thrPos = 0; // gauged throttle position from controller dummy
	
	double xThrFX = 0, yThrFX = 0; // scanline positions for thruster performance lookup table
	
	double burnerEngMode = 0;
	
	int ramcasterMode = 0; // ramx mode 0: off 1:low 2:hi
	
	int thrAuthority = 0; // 0: main engines 1: OMS
	
	bool parkingBrakeMode = false; // false: off, true: set

	bool gearMode = false; // false: normal, true: emergency release
	
	double cgPos = 0;

	int ignoreDirectKey = 0;
	bool directCall = false;
	
	int wingPos = 0; // -1 = fuly down 0 = flat  1 = fully up
	
	struct RT66
	{
		enum FEED_CFG {FUEL_OPEN = 1, FUEL_PUMP = 2, OXY_OPEN = 4, OXY_PUMP = 8}; int feed = 0;
		enum SIMSTATE {SST_INOP, SST_CUT, ENG_SPOOLUP, SST_STARTGEN, SST_STARTRCKT, SST_RUN_EXT, ENG_INLT_CLSE,
					   ENG_INLT_OPEN, SST_RUN_INT, ENG_SPOOLDOWN, SST_STOPGEN, SST_STOPRCKT, SST_FAIL}; int state = SST_INOP;
		
		int mode = 0; // main engine mode selector 0: off 1:ext 2: int 
		int burnerToggle = 0; // main engine afterburner 0: off 1: on

		double fuelPrs = 0, oxyPrs = 0; // fuel and oxy lines pressure
		
		double thr = 0; // throttle command position
		double genPct = 0; // general percent compressor/turbopump RPM
		
		double spr = 0, epr = 0, mct = 0; // stream pressure rating, effective pressure rating,  mean compressor temperature
		
		THRUSTER_HANDLE gasGenThr, burnerThr, rocketThr;
		MovingPart* inltDoor;

		int extStartSFX, intStartSFX, extRunSFX, intRunSFX, jetRoarSFX, burnerSFX, rctRoarSFX, extStopSFX, intStopSFX;
		
		double contrailLevel = 0;
	} engMainL, engMainR;

	struct RAMCASTER
	{
		enum FEED_CFG { FUEL_OPEN = 1, FUEL_PUMP = 2 }; int feed = 0;
		enum SIMSTATE { ENG_INOP, ENG_START, ENG_RUNLO, ENG_UPCAST, ENG_RUNHI, ENG_DOWNCAST, ENG_STOP, ENG_FAIL }; int state = ENG_INOP;
		double thr = 0;
		double fuelPrs = 0;

		double spr = 0, epr = 0, ett = 0, idt = 0;
		THRUSTER_HANDLE thRamx;
	} engRamx;

	struct RCS
	{
		enum FEED_CFG { FUEL_OPEN = 1, FUEL_PUMP = 2 }; int feed = 0;
		enum SIMSTATE { SST_INOP, SST_STBY }; int state = SST_INOP;

		double fuelPrs = 0, fuelFlow = 0;

		bool mode = false; // false: normal, true: docking
	} rcs;
	
	struct OMS
	{
		enum FEED_CFG { FUEL_OPEN = 1, FUEL_PUMP = 2 }; int feed = 0;
		enum SIMSTATE { SST_INOP, SST_STBY }; int state = SST_INOP;

		double fuelPrs = 0, fuelFlow = 0;
		double thr = 0;
	} oms;

	struct FUELDUMP
	{
		bool dumpLeft = false, dumpFwd = false, dumpAft = false, dumpRight = false, dumpAsf = false, dumpOxy = false;
		bool masterDump = false, dumpingFuel = false;
	} fuelDump;

	struct DOCKSUPPLY
	{
		bool supplyFuel = false, supplyOxy = false, supplyAsf = false;
		bool supplyingFuel = false, supplyingOxy = false, supplyingAsf = false;
	} dockSupply;

	struct WARNING
	{
		bool inhibit = false;
		bool mainFull = false, oxyFull = false, asfFull = false;
		bool mainLow = false, oxyLow = false, asfLow = false;
		bool mainDep = false, oxyDep = false, asfDep = false;
		bool controlOff = false;
		bool v1 = false, rotate = false;
	} warningSys;

	friend class MovingPart;

	struct HYDRAULIC
	{
		double hydPrs = 0, hydFlow = 0;
	} hydSysA, hydSysB;

	struct APU
	{
		enum FEED_CFG { FUEL_OPEN = 1, FUEL_PUMP = 2 }; int feed = 0;
		enum SIMSTATE { ENG_INOP, ENG_RUN, ENG_START, ENG_STOP }; int state = ENG_INOP;

		double pwrPct = 0;
		double fuelPrs = 0, fuelFlow = 0, usedFuel = 0;
		double exhaustTemp = 0;

		int startSFX, runSFX, stopSFX;

		HYDRAULIC* hydSys;
		enum HYD_CFG { HYD_OFF, HYD_AUTO, HYD_PUMP }; int hydFeed = HYD_AUTO;
	} apuPackA, apuPackB;

	// functions and whatnot -- definitions are found in files respectively marked
	// VESSEL SETUP
	void setupAnimations();
	void clearAnimations();

	// SCENARIO READ/WRITE
	bool checkCfgTag(char* line, const char* tag, int* cfgData);

	// ENGINE
	void cueEngines(RT66& eng, RT66::SIMSTATE sst);
	void simEngines(double& dT, RT66& eng);

	void cueEngines(RAMCASTER& eng, RAMCASTER::SIMSTATE sst);
	void simEngines(double& dT, RAMCASTER& eng);

	void cueEngines(APU& eng,  APU::SIMSTATE sst);
	void simEngines(double& dT, APU& eng);

	void cueEngines(RCS& eng, RCS::SIMSTATE sst);
	void simEngines(double& dT, RCS& eng);

	void cueEngines(OMS& eng, OMS::SIMSTATE sst);	
	void simEngines(double& dT, OMS& eng);

	// VESSEL UPDATE
	void simSystems(double& dT, int sysid);
	void setStatusLanded();

	// DVC DISPLAYS
	void selEicasMode(int pnlID, int mode);

	void txtFltData(oapi::Sketchpad* skp, int x, int y, string st, double& val, int prcsn = 2);
	void txtFltData(oapi::Sketchpad* skp, int x, int y, double& val, int prcsn = 2, bool symbol = true);
	void txtFltData(oapi::Sketchpad* skp, int x, int y, double& val, int prcsn, char symbol);

	// AUX
	void setSwMid(int mgid);
	void playSound(const int soundID, const bool bLoop = false, const float volume = 1.0);
	void stopSound(const int soundID);

	// this is all defined in SIM_DYNAMICS
	static void VLiftCoeff(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void HLiftCoeff(VESSEL* v, double beta, double M, double Re, void* context, double* cl, double* cm, double* cd);
};