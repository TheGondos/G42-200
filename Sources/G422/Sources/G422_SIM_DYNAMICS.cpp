#include "../Headers/G422.h"

//
//             from stock DG - blame dr. Martin
// ==============================================================
// Airfoil coefficient functions
// Return lift, moment and zero-lift drag coefficients as a
// function of angle of attack (alpha or beta)
// ==============================================================

// 1. vertical lift component (wings and body) | alpha
void G422::VLiftCoeff(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd)
{
	int i;

	const int nabsc = 9;
	static const double AOA[nabsc] =      {-PI, -60*RAD, -30*RAD, -2*RAD, 15*RAD, 20*RAD, 25*RAD, 60*RAD, PI};
	for (i = 0; i < nabsc-1 && AOA[i+1] < aoa; i++);
	double f = (aoa-AOA[i]) / (AOA[i+1]-AOA[i]);

	static const double CL_sbsc[nabsc]  = { 0,      0,   -0.4,      0,    0.7,     1,   0.8,     0,      0 };
	static const double CM_sbsc[nabsc]  = { 0,      0,  0.014, 0.0039, -0.006,-0.008,-0.010,     0,      0 };
	
	*cl = CL_sbsc[i] + (CL_sbsc[i+1]-CL_sbsc[i]) * f;  // aoa-dependent lift coefficient
	*cm = CM_sbsc[i] + (CM_sbsc[i+1]-CM_sbsc[i]) * f;  // aoa-dependent moment coefficient
	
	double wingPos = (static_cast<G422*>( v ))->wingTipWvrd->pos;

	if (wingPos > 0.01) *cl = (*cl) * (wingPos * ( 6.0* (M+2.0) )) * WAVERIDER_FACTOR;
	
	wingPos = (static_cast<G422*>( v ))->wingTipFthr->pos;

	if (wingPos > 0.01) *cl = (*cl) * (1 - wingPos * WINGFTHR_LIFTDMP);

	double saoa = sin(aoa);
	double pd = 0.015 + 0.4*saoa*saoa;  // profile drag
	
	double wd = oapiGetWaveDrag (M, 0.75, 1, 1.1, 0.04);
	*cd = pd + oapiGetInducedDrag (*cl, 1.5, 0.7) + wd;
	// profile drag + (lift-)induced drag + transonic/supersonic wave (compressibility) drag
	
	// ground effect lift
	double agl = v->GetAltitude(); // prehaps this could be more precise

	if (agl < MAXHEIG_GROUNDEFFECT && M > 0.1)
	{
		double gndFX = 1-((agl) * MAXHINV_GROUNDEFFECT);
		*cl += ((gndFX * gndFX * saoa) * MAXLIFT_GROUNDEFFECT);
	}
}

// 2. horizontal lift component (vertical stabilisers and body) | beta
void G422::HLiftCoeff(VESSEL* v, double beta, double M, double Re, void* context, double* cl, double* cm, double* cd)
{
	int i;

	const int nabsc = 8;
	static const double BETA[nabsc] = {-PI,-135*RAD,-PI05,-45*RAD,45*RAD,PI05,135*RAD,PI};
	static const double CL[nabsc]   = {       0,    +0.3,      0,   -0.3,  +0.3,     0,   -0.3,      0};

	for (i = 0; i < nabsc-1 && BETA[i+1] < beta; i++);
	*cl = CL[i] + (CL[i+1]-CL[i]) * (beta-BETA[i]) / (BETA[i+1]-BETA[i]);

	*cm = 0;
	*cd = 0.015 + oapiGetInducedDrag (*cl, 1.5, 0.6) + oapiGetWaveDrag (M, 0.75, 1, 1.1, 0.04);
}