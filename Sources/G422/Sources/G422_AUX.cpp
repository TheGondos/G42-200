#include "../Headers/G422.h"
#include "../Headers/G422_DVC.h"

MovingPart::MovingPart(UINT animIndex, double rate, Status status, double pos, G422* vesselRef, int sysID, int soundID) :
	animIndex(animIndex), rate(rate), mpStatus(status), pos(pos), vessel(vesselRef), sysID(sysID), soundID(soundID)
{
	sysReset = true;

	if (vessel) vessel->SetAnimation(animIndex, pos);
}

int MovingPart::toggle()
{
	G422::HYDRAULIC& hydSys = vessel->hydSysA.hydPrs < 2800 ? vessel->hydSysB : vessel->hydSysA;

	if (hydSys.hydPrs > 2800)
	{
		if (mpStatus == MP_REVERSING || mpStatus == MP_LOW_DETENT)
		{
			mpStatus = MP_MOVING;
			if (sysID == G422::SYSID_VISR) soundID = G422::SFX_VSRDN;

			sysReset = false;
			vessel->clbkGeneric(VMSG_MPSTRT, sysID, this);
		}
		else if (mpStatus == MP_MOVING || mpStatus == MP_HI_DETENT)
		{
			mpStatus = MP_REVERSING;
			if (sysID == G422::SYSID_VISR) soundID = G422::SFX_VSRUP;

			sysReset = false;
			vessel->clbkGeneric(VMSG_MPSTRT, sysID, this);
		}
	}

	return mpStatus;
}

bool MovingPart::toggle(bool hiDetent)
{
	G422::HYDRAULIC& hydSys = vessel->hydSysA.hydPrs < 2800 ? vessel->hydSysB : vessel->hydSysA;

	if (hydSys.hydPrs > 2800)
	{
		if (hiDetent && (mpStatus == MP_REVERSING || mpStatus == MP_LOW_DETENT))
		{
			mpStatus = MP_MOVING;
			if (sysID == G422::SYSID_VISR) soundID = G422::SFX_VSRDN;

			sysReset = false;
			vessel->clbkGeneric(VMSG_MPSTRT, sysID, this);
			return true;
		}

		if (!hiDetent && (mpStatus == MP_MOVING || mpStatus == MP_HI_DETENT))
		{
			mpStatus = MP_REVERSING;
			if (sysID == G422::SYSID_VISR) soundID = G422::SFX_VSRUP;

			sysReset = false;
			vessel->clbkGeneric(VMSG_MPSTRT, sysID, this);
			return true;
		}
	}
	else vessel->playSound(G422::SFX_WARN_APUOFF);

	return false;
}

void MovingPart::toDetent(int detent)
{
	if (detent == MP_HI_DETENT)
	{
		mpStatus = MP_HI_DETENT;
		pos = 1;
	}
	else
	{
		mpStatus = MP_LOW_DETENT;
		pos = 0;
	}

	vessel->SetAnimation(animIndex, pos);

	sysReset = false;
	vessel->clbkGeneric(VMSG_MPSTRT, sysID, this);
}

bool MovingPart::getToggleState()
{
	if ((mpStatus == MP_REVERSING || mpStatus == MP_LOW_DETENT)) return false;

	if ((mpStatus == MP_MOVING || mpStatus == MP_HI_DETENT)) return true;

	return false;
}

bool VCSwitch::flick(bool upDn, VESSEL4* vesselRef) // true = "up" | false = "down"
{
	switch (pos)
	{
	case SW2_DOWN:
		if (upDn)
		{
			vesselRef->SetAnimation(anID, 1);
			pos = SW2_UP;
			return true;
		}
		break;

	case SW2_UP:
		if (!upDn)
		{
			vesselRef->SetAnimation(anID, 0);
			pos = SW2_DOWN;
			return true;
		}
		break;

	case SW3_DOWN:
		if (upDn)
		{
			vesselRef->SetAnimation(anID, 0.5);
			pos = SW3_MID;
			return true;
		}
		break;

	case SW3_MID:
		if (upDn)
		{
			vesselRef->SetAnimation(anID, 1);
			pos = SW3_UP;
			return true;
		}
		else
		{
			vesselRef->SetAnimation(anID, 0);
			pos = SW3_DOWN;
			return true;
		}
		break;

	case SW3_UP:
		if (!upDn)
		{
			vesselRef->SetAnimation(anID, 0.5);
			pos = SW3_MID;
			return true;
		}
		break;
	}

	return false; // false is returned when the switch cannot be flipped to that position
}

void VCSwitch::setPos(Position newPos, VESSEL4* vesselRef)
{
	if (pos == newPos) return;

	pos = newPos;
	switch (newPos)
	{
	case SW2_UP:
	case SW3_UP:
		vesselRef->SetAnimation(anID, 1);
		return;

	case SW3_MID:
		vesselRef->SetAnimation(anID, 0.5);
		return;

	case SW2_DOWN:
	case SW3_DOWN:
		vesselRef->SetAnimation(anID, 0);
		return;
	}
}

void G422::setSwMid(int mgid)
{
	playSound(SFX_VC_AFLIK);
	vcSwitches[vcSwitchIndexByMGID[mgid]].setPos(VCSwitch::SW3_MID, this);
}

bool VCKnob::flick(bool upDn, VESSEL4* vesselRef) // true = "up" | false = "down"
{
	switch (pos)
	{
	case KB2_DOWN:
		if (upDn)
		{
			vesselRef->SetAnimation(anID, 1);
			pos = KB2_UP;
			return true;
		}
		break;

	case KB2_UP:
		if (!upDn)
		{
			vesselRef->SetAnimation(anID, 0);
			pos = KB2_DOWN;
			return true;
		}
		break;

	case KB3_DOWN:
		if (upDn)
		{
			vesselRef->SetAnimation(anID, middleState);
			pos = KB3_MID;
			return true;
		}
		break;

	case KB3_MID:
		if (upDn)
		{
			vesselRef->SetAnimation(anID, 1);
			pos = KB3_UP;
			return true;
		}
		else
		{
			vesselRef->SetAnimation(anID, 0);
			pos = KB3_DOWN;
			return true;
		}
		break;

	case KB3_UP:
		if (!upDn)
		{
			vesselRef->SetAnimation(anID, middleState);
			pos = KB3_MID;
			return true;
		}
		break;
	}

	return false;
}

void VCKnob::setPos(Position newPos, VESSEL4* vesselRef)
{
	if (pos == newPos) return;

	pos = newPos;
	switch (pos)
	{
	case KB2_UP:
	case KB3_UP:
		vesselRef->SetAnimation(anID, 1);
		return;

	case KB3_MID:
		vesselRef->SetAnimation(anID, middleState);
		return;

	case KB2_DOWN:
	case KB3_DOWN:
		vesselRef->SetAnimation(anID, 0);
		return;
	}
}

void G422::playSound(const int soundID, const bool bLoop, const float volume) 
{
	if (xrSound) xrSound->PlayWav(soundID, bLoop, volume);
	else pendingSounds.push_back({ soundID, bLoop });
}

void G422::stopSound(const int soundID) { if (xrSound) xrSound->StopWav(soundID); }