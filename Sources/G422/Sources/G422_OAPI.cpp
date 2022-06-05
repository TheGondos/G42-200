#define ORBITER_MODULE

#include "../Headers/G422.h"
#include "../Headers/G422_DVC.h"

DLLCLBK void InitModule(MODULEHANDLE mod) { }

DLLCLBK void ExitModule(MODULEHANDLE mod) { }

DLLCLBK VESSEL* ovcInit(OBJHANDLE vessel_hndl, int fltModel) { return new G422(vessel_hndl, fltModel); }

DLLCLBK void ovcExit(VESSEL* vessel) { if (vessel) delete static_cast<G422*>(vessel); }