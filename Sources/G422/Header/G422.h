#define STRICT

#ifndef G422_MODULE
#define G422_MODULE

#include "Orbitersdk.h"
#include "UCSO_Vessel.h"

#include "OrbiterSoundSDK40.h"
#include "..\Library\EasyBmp\EasyBMP.h"

#include "resource.h"

#include <math.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>

using namespace std;

extern HINSTANCE thisDLL;

#define LOAD_BMP(id) (LoadBitmap (thisDLL, MAKEINTRESOURCE (id)))

#define ENT << endl

#define _V0 _V(0,0,0)

#define _X _V(1,0,0)
#define _Y _V(0,1,0)
#define _Z _V(0,0,1)

#define _Xn _V(-1,0,0)
#define _Yn _V(0,-1,0)
#define _Zn _V(0,0,-1)

#define SCALAR_ANGLE 0.15915494309189533577

#define NML_AGL_PRS 101.4e3

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

#define ISPMAX_MAIN_GEN  41500
#define ISPMAX_MAIN_AFB  13500

#define ISPMAX_MAIN_RKT  7800
#define ISPMIN_MAIN_RKT  7300

#define ISPMAX_RAMXLO   16700
#define ISPMAX_RAMXHI   32000

#define ISPMAX_RCS 6100
#define ISPMAX_OMS 6800

#define ISPMIN_RCS 1100
#define ISPMIN_OMS 1000

#define MAXFUEL_MAIN_ALL 205e3

#define MAXFUEL_OXY 85000.0
#define MAXFUEL_SYS 16000.0

#define OXYFUEL_RATIO 1.15

#define APU_FUEL_RATE 0.035

#define RCS_FX_LSCALE 2.25
#define RCS_FX_WSCALE 0.20
#define OMS_FX_LSCALE 5.4
#define OMS_FX_WSCALE 0.26

#define SFX_ENGINERUN_MAIN_EXT 1
#define SFX_ENGINERUN_MAIN_INT 2

#define SFX_ENGINERUN_RAMX_LO 3
#define SFX_ENGINERUN_RAMX_HI 4
#define SFX_RAMCASTER_START   5
#define SFX_RAMCASTER_TRST_UP 6

#define SFX_APU_START 7
#define SFX_APU_RUN   8
#define SFX_APU_STOP  9

#define SFX_FUELPUMP_MAIN 10
#define SFX_FUELPUMP_RAMX 12
#define SFX_FUELPUMP_OXY  13
#define SFX_FUELPUMP_RCS  14
#define SFX_FUELPUMP_OMS  15
#define SFX_FUELPUMP_APU  16

#define SFX_GEARS 17
#define SFX_VSRUP 18
#define SFX_VSRDN 19
#define SFX_CNRDS 20
#define SFX_RCS_DOORS 21
#define SFX_BAY_DOORS 22

#define SFX_WIND_OPEN 23
#define SFX_WIND_CLSD 24

#define SFX_VC_POP   25
#define SFX_VC_FLICK 26
#define SFX_VC_TICK  27
#define SFX_VC_BEEP  28
#define SFX_VC_BLIP  29
#define SFX_VC_CLAK  30
#define SFX_VC_AFLIK 31
#define SFX_VC_SLACK 32

#define SFX_WINGMTR 33

#define SFX_ENGINESTART_MAIN_EXT 34
#define SFX_ENGINESTART_MAIN_INT 35

#define SFX_ENGINESTOP_MAIN_EXT 36
#define SFX_ENGINESTOP_MAIN_INT 37

#define SFX_AFTERBURNER 38
#define SFX_JETROAR 39
#define SFX_RCTROAR 40

#define SFX_OMSBURN 41
#define SFX_RAMXBURN 42

#define SFX_BAY_DOORS_CLOSED 43
#define SFX_BAY_SLOT_EMPTY 44
#define SFX_BAY_SLOT_OCCUPIED 45
#define SFX_BAY_RELEASED 46
#define SFX_BAY_RELEASE_FAILED 47
#define SFX_BAY_GRAPPLED 48
#define SFX_BAY_GRAPPLE_NORANGE 49

#define SFX_RCS_NORMAL 50
#define SFX_RCS_DOCKING 51

#define SFX_FUEL_DUMP 52
#define SFX_FUEL_FLOW 53

#define SFX_WARN_MAINLOW 54
#define SFX_WARN_OXYLOW 55
#define SFX_WARN_ASFLOW 56
#define SFX_WARN_MAINDEP 57
#define SFX_WARN_OXYDEP 58
#define SFX_WARN_ASFDEP 59
#define SFX_WARN_CTRLOFF 60

#define SFX_V1 61
#define SFX_ROTATE 62

enum VCSwitchState
{
	SW3_DOWN, SW3_MID, SW3_UP, // 3-stop switches
	SW2_DOWN, SW2_UP           // 2-stop switches
};

class VCSwitch
{
public:
	UINT mgid;
	UINT anID;
	MGROUP_ROTATE* mgRot;
	VCSwitchState pos;

	bool flick(bool upDn, VESSEL4* vslRef);
	void setPos(VCSwitchState newPos, VESSEL4* vslRef);

	char getSaveState();
	void setLoadState(char& cs, VESSEL4* vsl);
};

typedef struct {  // this allows us to statically define all switches in a list-like fashion, then instantiate
                  // independent VCSwitch structs for new G42-class vessels
	UINT mgid;
	VCSwitchState init_pos;
	VECTOR3 refPos, axisPos, axisDir;
	double initState = 0;
} VCSwitchDef;

#define VC_SWITCH_COUNT 41 // number of switches in VC (so far)
extern VCSwitchDef VC_swDefs[VC_SWITCH_COUNT]; // see DVC SETUP

extern map<UINT, int> VC_swIndexByMGID; // defined by the first vessel created, see VSL_SETUP

enum VCKnobState
{
	KB3_DOWN, KB3_MID, KB3_UP, // 3-stop knobs
	KB2_DOWN, KB2_UP           // 2-stop knobs
};

class VCKnob
{
public:
	UINT mgid;
	UINT anID;
	MGROUP_ROTATE* mgRot;
	VCKnobState pos;
	double middleState;

	bool flick(bool upDn, VESSEL4* vslRef);
	void setPos(VCKnobState newPos, VESSEL4* vslRef);

	char getSaveState();
	void setLoadState(char& cs, VESSEL4* vsl);
};

typedef struct 
{
	UINT mgid;
	VCKnobState init_pos;
	VECTOR3 refPos, axisDir;
	double size;
	float rotAngle;
	double initState, middleState;
} VCKnobDef;

#define VC_KNOB_COUNT 11 // number of knobs in VC
extern VCKnobDef VC_kbDefs[VC_KNOB_COUNT]; // see DVC SETUP

extern map<UINT, int> VC_kbIndexByMGID; // defined by the first vessel created, see VSL_SETUP

enum ButtonID { AP_MASTER_BUTTON, AP_PROGRADE_BUTTON, AP_RETROGRADE_BUTTON, AP_NORMAL_BUTTON, AP_ANTINORMAL_BUTTON, AP_HLEVEL_BUTTON,
	AP_KILLROT_BUTTON, HUD_DOCK_BUTTON, HUD_ORBIT_BUTTON, HUD_SURF_BUTTON, HUD_OFF_BUTTON, HUD_BRT_BUTTON, HUD_CLR_BUTTON,
	MASTER_WARN_BUTTON };

typedef struct
{
	ButtonID bid;
	VECTOR3 refPos;
	double size;
} VCButtonDef;

#define VC_BUTTON_COUNT 14 // number of buttons in VC
extern VCButtonDef VC_btDefs[VC_BUTTON_COUNT]; // see DVC SETUP

enum MP_STATE
{
	MP_MOVING = 1, MP_LOW_DETENT = 2, MP_HI_DETENT = 4, MP_REVERSING = 3, MP_INOP = 0
};

// vessel generic messages
#define VMSG_MPSTRT 1010
#define VMSG_MPSTOP 1011
#define VMSG_ENGRST 1012

class G422;

class MovingPart  // implemented in G422_AUX
{
public:
	double pos;

	bool sysReset; // will be set to false whever a callback is fired, reset to true after updating systems logic
	MP_STATE mp_status, mp_old_status;

	MovingPart(UINT idx, double rt, MP_STATE st, double stPos = 0, G422* vslRef = NULL, int sID = 0);

	inline bool operate(double& dt); // except this one - see SIM UPDATE
	int toggle();
	void toggle(bool b);
	void toDetent(int p);
	bool getToggleState();
	
	// and these two - see scenario RW
	void loadCfgState(int state);
	int getCfgState();

private:
	double rate;
	int sysID;    // id for use in callbacks
	UINT anim_idx;
	G422* vsl;
};

// engine shit...

extern BMP thrFX_map;

struct FXMapRef
{
	double channelRed;
	double channelGreen;
	double channelBlue;
};

// this is all defined in SIM_DYNAMICS (for now)
void   FXMapAll(FXMapRef &fxr, BMP &tex, double u, double v);
double FXMapRedChannel(BMP &tex, double u, double v);
double FXMapGreenChannel(BMP &tex, double u, double v);
double FXMapBlueChannel(BMP &tex, double u, double v);

extern struct G422_DRAWRES
{
	bool def;
	
	SURFHANDLE bmpRes[1];
	
	oapi::Font *mfdLabelsFont;

	oapi::Brush *brSet[8];
	oapi::Pen   *spSet[8];
} G422DrawRes;

//   SYSTEM ID's

// some of these may have cue specifiers to work with... 
#define G422_SYSID_LGRS  1
#define G422_SYSID_CNRD  2
#define G422_SYSID_VISR  3
#define G422_SYSID_BAY   4
#define G422_SYSID_RAMX  5
#define G422_SYSID_INLT  6
#define G422_SYSID_RCSDR 7

#define G422_SYSID_HYDRAULICS 8
#define G422_SYSID_MAINPUMPS 9
#define G422_SYSID_FUELDUMP 10
#define G422_SYSID_DCKSUPPLY 11
#define G422_SYSID_ANTISLIP 12
#define G422_SYSID_WARNING 13

/////////////////////////////////////////////// G422 VESSEL4 CLASS //////////////////////////////////////////////////////////
class G422 : public VESSEL4
{
public:
	G422 (OBJHANDLE vsl_hndl, int fltModel);
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
	bool clbkVCMouseEvent(int id, int ev, VECTOR3& p);
	int clbkConsumeBufferedKey(DWORD key, bool down, char* kstate);

	// VESSEL UPDATE
	void clbkPreStep(double simt, double simdt, double mjd);
	int clbkGeneric(int msgid, int prm, void* context);

	struct RAMCASTER
	{
		enum FEED_CFG { FUEL_OPEN = 1, FUEL_PUMP = 2 }; int feed = 0;
		enum SIMSTATE { ENG_INOP, ENG_START, ENG_RUNLO, ENG_UPCAST, ENG_RUNHI, ENG_DOWNCAST, ENG_STOP, ENG_FAIL }; int state = ENG_INOP;
		double thr = 0;
		double fuelPrs = 0;

		double spr = 0, epr = 0, ett = 0, idt = 0;
		THRUSTER_HANDLE th_ramx;
	} engRamx;

	MovingPart* wingTipWvrd;
	MovingPart* wingTipFthr;
	
private:
	static int vslCount; 

	UCSO::Vessel* ucso;
	bool ucsoInstalled;
	int cargoIndex = -1, slotIndex = -1, fuelIndex = -1; // 0: main, 1: ASF, 2: oxidizer
	bool displayCargoInfo = false;
	const char* message;
	char buffer[256];
	double timer = 0;
	bool apSet = false, rudderSet = false;
	
	MESHHANDLE mdl_ext;
	MESHHANDLE mdl_dvc;
	MESHHANDLE mdl_vc_R;
	MESHHANDLE mdl_vc_L;
	
	UINT mdlID_ext, mdlID_dvc, mdlID_dvcR, mdlID_dvcL;

	SURFHANDLE vcTex_mfds;
	SURFHANDLE vcTex_eicas;
	
	SURFHANDLE vcRes_eicasSrc;
	
	PROPELLANT_HANDLE fuel_main_allTanks;
	PROPELLANT_HANDLE fuel_main_L, fuel_main_F, fuel_main_A, fuel_main_R;
	PROPELLANT_HANDLE fuel_sys, fuel_oxy, dummyFuel; 
	
	THRUSTER_HANDLE dummyThruster, dummyHover, dummyThrAirbrakes;
	
	THRUSTER_HANDLE RT66_gasGen_thgr[2];
	THRUSTER_HANDLE RT66_burner_thgr[2];
	THRUSTER_HANDLE RT66_rocket_thgr[2];
	THRUSTER_HANDLE ramcaster;
	THRUSTER_HANDLE oms_thgr[2];

	THRUSTER_HANDLE th_rcs_pitchUp[2], th_rcs_pitchDn[2],  th_rcs_vertUp[2], th_rcs_vertDown[2];
	THRUSTER_HANDLE th_rcs_yawLeft[2], th_rcs_yawRight[2], th_rcs_horLeft[2], th_rcs_horRight[2];
	THRUSTER_HANDLE th_rcs_fwd[1], th_rcs_back[1];
	THRUSTER_HANDLE th_rcs_bankRight[2], th_rcs_bankLeft[2];

	THGROUP_HANDLE master_thgr, controller_thgr;
	THGROUP_HANDLE rcs_fwd_thgr, rcs_r1_thgr, rcs_r2_thgr;
	
	AIRFOILHANDLE wingLift;
	
	CTRLSURFHANDLE acsCndrs;

	double fx_contrailLvl = 0;
	double fx_exhaustLvl = 0;
	
	BEACONLIGHTSPEC nav_green, nav_red, nav_white, stb_l, stb_r, stb_t, bcn1, bcn2;
	SpotLight* landingLight;
	SpotLight* taxiLight;
	SpotLight* dockingLight;
	
	MovingPart *landingGears;
	MovingPart *visor;
	MovingPart *canards;
	MovingPart *rcsDoors;

	const double* gearsPos;
	const double* canardsPos;
	const double* inltPos;
	const double* ramxPos;
	
	MovingPart *inltDoors;
	MovingPart *ramxDoors;
	
	MGROUP_ROTATE *mgRot_wtL, *mgRot_wtR, *mgRot_wtLgtsL, *mgRot_wtLgtsR; 
	MGROUP_ROTATE *mgRot_acs_evL, *mgRot_acs_evR; 	
	
	VECTOR3 wingTipLightsL[2];
	VECTOR3 wingTipLightsR[2];
	
	MovingPart *bayDoors;
	MovingPart *radiators;
	
	int VC_eicas_screens[6];
	int drawEicas = (1 << 5) | (1 << 7);
	
	int VC_PoV_station = 0; // 0: pilot 1: F/0 (for now...)
	
	VCSwitch VCSwitches[VC_SWITCH_COUNT]; // that's right, load 'em up...

	VCKnob VCKnobs[VC_KNOB_COUNT]; // that's right, load 'em up...
	
	VISHANDLE visual;
	
	// individual animations for VC (not simple switches)
	UINT an_dvc_ramx;
	UINT an_dvc_wpos;
	UINT an_dvc_thr;
	UINT an_dvc_csPitch;
	UINT an_dvc_csRoll;
	UINT an_dvc_tiller;
	UINT an_dvc_prk;
	UINT an_dvc_gear;
	UINT an_dvc_undock;
	
	// aerodynamic control surfaces
	UINT an_acs_cnrd;
	UINT an_acs_rddr;
	UINT an_acs_alrn;
	UINT an_acs_elvr;

	// gimbals (innermost out)
	UINT an_dvc_adi_C, an_dvc_adi_B, an_dvc_adi_A;
	MGROUP_ROTATE* mgRot_ADI_G1, * mgRot_ADI_G2;
	
	double thrPos = 0; // gauged throttle position from controller dummy
	
	double thrFX_X = 0, thrFX_Y = 0; // scanline positions for thruster performance lookup table

	int SFXID; // for use with orbiter-sound
	
	int main_eng_mode = 0; // main engine mode selector 0: off 1:ext 2: int 
	int burner_toggle = 0; // main engine afterburners 0: off 1: on (both engines)
	
	int ramcaster_mode = 0; // ramx mode 0: off 1:low 2:hi
	
	int thr_authority = 0; // 0: main engines 1: OMS
	
	bool prk_brake_mode = false; // false: off, true: set

	bool gear_mode = false; // false: normal, true: emergency release
	
	// mind you - we're not really dealing with ACTUAL CG_shifting here - instead, we just throw off the center-of-lift
	//            to the opposite side, as that's the only effect a pilot would perceive (after PBW and FMC sort out everything else)
	//               plus, the ShiftCG functions are bugged to boot -- too much fuss for a non-apparent aspect of the sim...
	//  then we can cut a few corners and get down to what matters only...
	//
	// "ref" is where you want the CG to be, 
	// "pos" is where it really is,
	// "dyn" is how your flight-envelope throws it off...
	double cgShiftRef = 0, cgShiftPos = 0, cgShiftDyn = 0;
	
	int wingPos = 0; // -1 = fuly down 0 = flat  1 = fully up
	
	struct RT66
	{
		enum FEED_CFG {FUEL_OPEN = 1, FUEL_PUMP = 2, OXY_OPEN = 4, OXY_PUMP = 8}; int feed = 0;
		enum SIMSTATE {SST_INOP, SST_CUT, ENG_SPOOLUP, SST_STARTGEN, SST_STARTRCKT, SST_RUN_EXT, ENG_INLT_CLSE,
					   ENG_INLT_OPEN, SST_RUN_INT, ENG_SPOOLDOWN, SST_STOPGEN, SST_STOPRCKT, SST_FAIL}; int state = SST_INOP;
		
		double fuelPrs = 0, oxyPrs = 0; // fuel and oxy lines pressure
		
		double thr = 0; // throttle command position
		double genPct = 0; // general percent compressor/turbopump RPM
		
		double spr = 0, epr = 0, mct = 0; // stream pressure rating, effective pressure rating,  mean compressor temperature
		
		THRUSTER_HANDLE th_gasGen, th_burner, th_rocket;
	} engMain_L, engMain_R;

	friend class MovingPart;

	struct HYDRAULIC
	{
		double hydPrs = 0, hydFlow = 0;
	} hydSysA, hydSysB;
	
	struct APU
	{
		enum FEED_CFG {FUEL_OPEN = 1, FUEL_PUMP = 2}; int feed = 0;
		enum SIMSTATE {ENG_INOP, ENG_RUN, ENG_START, ENG_STOP}; int state = ENG_INOP;
		double pwrPct = 0;
		double fuelPrs = 0, fuelFlow = 0, usedFuel = 0;
		double exhaustTemp = 0;

		HYDRAULIC* hydSys;
		enum HYD_CFG { HYD_OFF, HYD_AUTO, HYD_PUMP }; int hydFeed = HYD_AUTO;
	} apuPackA, apuPackB;

	struct RCS
	{
		enum FEED_CFG { FUEL_OPEN = 1, FUEL_PUMP = 2 }; int feed = 0;
		enum SIMSTATE { SST_INOP, SST_STBY }; int state = SST_INOP;

		double fuelPrs = 0, fuelFlow = 0;

		bool mode = false; // false: normal, true: docking
	} rcs;
	
	struct OMS // reaction control and orbital maneuvering systems
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
		bool mainLow = false, oxyLow = false, asfLow = false;
		bool mainDep = false, oxyDep = false, asfDep = false;
		bool controlOff = false;
		bool v1 = false, rotate = false;
	} warningSys;

	// functions and whatnot -- definitions are found in files respectively marked
	// VESSEL SETUP
	void setupAnimations();
	void clearAnimations();

	void selEicasMode(int pnlID, int mode);

	void setStatusLanded();

	// SIM CUES
	void cueEngines(RT66 &eng, RT66::SIMSTATE sst);
	void cueEngines(RAMCASTER &eng, RAMCASTER::SIMSTATE sst);
	void cueEngines(APU &eng,  APU::SIMSTATE sst);
	void cueEngines(RCS& eng, RCS::SIMSTATE sst);
	void cueEngines(OMS &eng, OMS::SIMSTATE sst);
	
	// SIM DYNAMICS
	void simEngines(double &dT, RT66 &eng);
	void simEngines(double &dT, RAMCASTER &eng);
	void simEngines(double &dT, APU &eng);
	void simEngines(double &dT, RCS &eng);
	void simEngines(double &dT, OMS &eng);

	void simSystems(double &dT, int sysid);

	void setSwMid(int mgid);
};

// global lift functions (also in SIM DYNAMICS)
void VLiftCoeff (VESSEL *v, double aoa, double M, double Re, void *context, double *cl, double *cm, double *cd);
void HLiftCoeff (VESSEL *v, double beta, double M, double Re, void *context, double *cl, double *cm, double *cd);

// utility things
#define clamp(mn, x, mx) ( max((mn), min((mx), (x))) )

void StringExplode(string str, string separator, vector<string>* results);
void StringTrimWS(string &str);
void StringToLower(string &str);
int parseInt(string &str);

inline double sdRandom(int x)
{
	x = (x<<13) ^ x;
	return ( 1 - ( (x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

inline void filter(double &crt, double &tgt, double smth) { crt += ((tgt - crt) * smth); }

#endif