#define ORBITER_MODULE

#include "..\Header\G422.h"
#include "..\Header\G422_DVC.h"

HINSTANCE thisDLL;

DLLCLBK void InitModule(HINSTANCE mod) { thisDLL = mod; }

DLLCLBK void ExitModule(HINSTANCE mod) { thisDLL = NULL; }

DLLCLBK VESSEL* ovcInit(OBJHANDLE vsl_hndl, int fltModel) { return new G422(vsl_hndl, fltModel); }

DLLCLBK void ovcExit(VESSEL* vsl) { if (vsl) delete static_cast<G422*>(vsl); }

// utility functions for reading from the CFG file
void StringExplode(string str, string separator, vector<string>* results)
{
	int found;
	found = str.find_first_of(separator);

	while (found != string::npos)
	{
		if (found > 0) results->push_back(str.substr(0, found));
		str = str.substr(found + 1);
		found = str.find_first_of(separator);
	}

	if (str.length() > 0) results->push_back(str);
}

void StringTrimWS(string& str)
{
	str.erase(0, str.find_first_not_of(' '));
	str.erase(0, str.find_first_not_of('\t'));
	str.erase(str.find_last_not_of(' ') + 1);
	str.erase(str.find_last_not_of('\t') + 1);
}

void StringToLower(string& str)
{
	for (size_t i = 0; i < str.length(); i++)
		if (str[i] >= 0x41 && str[i] <= 0x5A) str[i] = str[i] + 0x20;
}

int parseInt(string& str)
{
	int val;
	stringstream ssVal(str);
	ssVal >> val;

	return val;
}

double parseDouble(string& str)
{
	double val;
	stringstream ssVal(str);
	ssVal >> val;

	return val;
}