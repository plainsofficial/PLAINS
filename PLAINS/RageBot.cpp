
#include "RageBot.h"
#include "DrawManager.h"
#include "Autowall.h"
#include <iostream>
#include "Resolver.h"

#define M_PI_F ((float)(M_PI))

CRagebot ragebot;

void CRagebot::run(CUserCmd *pCmd, bool& bSendPacket)
{
	if (m_pEngine->IsConnected() && m_pEngine->IsInGame())
	{ 
		auto m_local = game::localdata.localplayer();
		if (!m_local || !m_local->IsAlive()) return;

		auto m_weapon = m_local->GetWeapon();
		if (m_weapon)
		{
			aa.Manage(pCmd, bSendPacket);

			if (m_weapon->GetAmmoInClip())
			{
				if (ragebotconfig.bEnabled)
				{
					if (m_local->IsAlive())
					{
						aimbot.aimbotted_in_current_tick = false;
						if (game::functions.can_shoot()) {
							aimbot.DoAimbot(pCmd, bSendPacket);
						}
						if (!aimbot.aimbotted_in_current_tick) aimbot.auto_revolver(pCmd);

						RemoveRecoil(pCmd);
					}
				}
			}
			else
			{
				pCmd->buttons &= ~IN_ATTACK;
				pCmd->buttons |= IN_RELOAD;
			}

			if (game::globals.Target)
			{
				if (ragebotconfig.bAutostop)
				{
					if (m_local->GetVelocity().Length() > 0)
					{
						if (!(ragebotconfig.iAutostopType))
						{
							pCmd->forwardmove = 0.f;
							pCmd->sidemove = 0.f;
						}
						else if (ragebotconfig.iAutostopType == 1)
						{
							Vector velocity = m_local->GetVelocity();
							Vector direction = velocity.Angle();
							float speed = velocity.Length();

							direction.y = pCmd->viewangles.y - direction.y;

							Vector negated_direction = direction.Forward() * -speed;

							pCmd->forwardmove = negated_direction.x;
							pCmd->sidemove = negated_direction.y;
						}
					}

					if ((pCmd->buttons & IN_FORWARD))
						pCmd->buttons &= ~IN_FORWARD;

					if ((pCmd->buttons & IN_LEFT))
						pCmd->buttons &= ~IN_LEFT;

					if ((pCmd->buttons & IN_RIGHT))
						pCmd->buttons &= ~IN_RIGHT;

					if ((pCmd->buttons & IN_BACK))
						pCmd->buttons &= ~IN_BACK;

					if ((pCmd->buttons & IN_JUMP))
						pCmd->buttons &= ~IN_JUMP;
				}
			}
		}
		if (miscconfig.bAntiUntrusted)
			game::math.normalize_vector(pCmd->viewangles);
	}
}
bool CanWallbang(Vector& EndPos, float &Damage)
{

	if (CanHit(EndPos, &Damage))
	{
		return true;
	}
	return false;
}
Vector GetBestPoint(IClientEntity *targetPlayer, Vector &final)
{
	IClientEntity* m_local = game::localdata.localplayer();

	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = targetPlayer;
	ray.Init(final + Vector(0, 0, 10), final);
	m_pTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	final = tr.endpos;
	return final;
}
void VectorAngles1(const Vector& forward, Vector &angles) {
	float	tmp, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / PI);
		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

float hitchance() {
	auto m_local = game::localdata.localplayer();
	auto m_weapon = m_local->GetWeapon();

	float hitchance = 101;
	if (!m_weapon) return 0;
	if (ragebotconfig.bHitchance && ragebotconfig.flHitchanceAmt > 1)
	{
		float inaccuracy = m_weapon->GetCone();
		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;
	}
	return hitchance;
}

void CRagebot::RemoveRecoil(CUserCmd* pCmd)
{
	if (!ragebotconfig.bRemoveRecoil) return;

	IClientEntity* m_local = game::localdata.localplayer();
	if (m_local)
	{
		Vector AimPunch = m_local->localPlayerExclusive()->GetAimPunchAngle();
		if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 150)
		{
			auto weapon_recoil_scale = m_pCVar->FindVar(XorStr("weapon_recoil_scale"));
			*(int*)((DWORD)&weapon_recoil_scale->fnChangeCallback + 0xC) = 0;
			pCmd->viewangles -= AimPunch * weapon_recoil_scale->GetFloat();
			game::math.normalize_vector(pCmd->viewangles);
		}
	}
}

int Aimbot::GetTargetFOV()
{
	auto m_local = game::localdata.localplayer();

	int target = -1;
	float mfov = ragebotconfig.flMaxFov;

	Vector viewoffset = m_local->GetOrigin() + m_local->GetViewOffset();
	Vector view; m_pEngine->GetViewAngles(view);

	for (int i = 0; i < m_pEntityList->GetMaxEntities(); i++)
	{
		IClientEntity* pEntity = m_pEntityList->GetClientEntity(i);

		if (IsViable(pEntity))
		{
			int newhb = HitScan(pEntity);
			if (newhb >= 0)
			{
				float fov = FovToPlayer(viewoffset, view, pEntity, 0);
				CPlayer* Player = plist.get_player(i);
				if (fov < mfov || (Player->Priority && fov <= 180))
				{
					mfov = fov;
					target = i;
				}
			}
		}
	}

	return target;
}

int Aimbot::GetTargetDistance()
{
	auto m_local = game::localdata.localplayer();

	int target = -1;
	int minDist = 99999;

	IClientEntity* pLocal = m_local;
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; m_pEngine->GetViewAngles(View);

	for (int i = 0; i < m_pEntityList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = m_pEntityList->GetClientEntity(i);
		if (IsViable(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				int Distance = Difference.Length();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				CPlayer* Player = plist.get_player(i);
				if ((Distance < minDist || Player->Priority) && fov < ragebotconfig.flMaxFov)
				{
					minDist = Distance;
					target = i;
				}
			}
		}
	}

	return target;
}

int Aimbot::GetTargetHealth()
{
	auto m_local = game::localdata.localplayer();

	int target = -1;
	int minHealth = 101;

	IClientEntity* pLocal = m_local;
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; m_pEngine->GetViewAngles(View);

	for (int i = 0; i < m_pEntityList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = m_pEntityList->GetClientEntity(i);
		if (IsViable(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				CPlayer* Player = plist.get_player(i);
				if ((Health < minHealth || Player->Priority) && fov < ragebotconfig.flMaxFov)
				{
					minHealth = Health;
					target = i;
				}
			}
		}
	}

	return target;
}

bool Aimbot::IsViable(IClientEntity* pEntity)
{
	auto m_local = game::localdata.localplayer();
	if (pEntity && pEntity->IsDormant() == false && pEntity->IsAlive() && pEntity->GetIndex() != m_local->GetIndex())
	{
		ClientClass *pClientClass = pEntity->GetClientClass();
		player_info_t pinfo;
		if (pClientClass->m_ClassID == (int)CSGOClassID::CCSPlayer && m_pEngine->GetPlayerInfo(pEntity->GetIndex(), &pinfo))
		{
			if (pEntity->GetTeamNum() != m_local->GetTeamNum() || ragebotconfig.bFriendlyFire)
			{
				if (!pEntity->m_bGunGameImmunity())
				{
					CPlayer* Player = plist.get_player(pEntity->GetIndex());
					if (!Player->Friendly)
						return true;
				}
			}
		}
	}

	return false;
}

void Aimbot::auto_revolver(CUserCmd* m_pcmd) {
	if (ragebotconfig.iAutoFireRevolverMode == 1) {
		auto m_local = game::localdata.localplayer();
		auto m_weapon = m_local->GetWeapon();

		if (m_weapon) {
			if (*m_weapon->GetItemDefinitionIndex() == WEAPON_REVOLVER) {
				m_pcmd->buttons |= IN_ATTACK;
				float flPostponeFireReady = m_weapon->GetFireReadyTime();
				if (flPostponeFireReady > 0 && flPostponeFireReady - 1 < m_pGlobals->curtime) {
					m_pcmd->buttons &= ~IN_ATTACK;
				}
			}
		}
	}
}

float Aimbot::FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int aHitBox)
{
	CONST FLOAT MaxDegrees = 180.0f;
	Vector Angles = View;
	Vector Origin = ViewOffSet;
	Vector Delta(0, 0, 0);
	Vector Forward(0, 0, 0);
	game::math.angle_vectors(Angles, Forward);
	Vector AimPos = game::functions.get_hitbox_location(pEntity, aHitBox);
	game::math.vector_subtract(AimPos, Origin, Delta);
	game::math.normalize(Delta, Delta);
	FLOAT DotProduct = Forward.Dot(Delta);
	return (acos(DotProduct) * (MaxDegrees / PI));
}

int Aimbot::HitScan(IClientEntity* m_entity)
{
	IClientEntity* m_local = game::localdata.localplayer();
	CPlayer* m_player = plist.get_player( m_entity->GetIndex());
	std::vector<int> hitboxes;
	std::vector<int> baim_hitboxes;
	bool AWall = ragebotconfig.bAutoWall;
	int hbox = ragebotconfig.iAutoFireHitbox;

	bool bRevert = true;

	for (int i = 0; i < hitscanconfig.hitboxes.size(); i++)
	{
		if (hitscanconfig.hitboxes[i].bselected)
			bRevert = false;
	}

	std::vector<MultiBoxItem> custom_hitbones = hitscanconfig.hitboxes;

	baim_hitboxes.push_back((int)CSGOHitboxID::UpperChest);
	baim_hitboxes.push_back((int)CSGOHitboxID::Chest);
	baim_hitboxes.push_back((int)CSGOHitboxID::LowerChest);
	baim_hitboxes.push_back((int)CSGOHitboxID::Stomach);

	bool canseebody = false;
	float bodydmg;

	for (auto HitBoxID : baim_hitboxes)
	{
		Vector Point = game::functions.get_hitbox_location(m_entity, HitBoxID);
		if (CanHit(Point, &bodydmg))
		{
			canseebody = true;
		}
	}

	if (bRevert)
	{
		switch (hbox)
		{
		case 0: hitboxes.push_back((int)CSGOHitboxID::Head);
		case 1: hitboxes.push_back((int)CSGOHitboxID::Neck);
		case 2: hitboxes.push_back((int)CSGOHitboxID::Chest);
		case 3: hitboxes.push_back((int)CSGOHitboxID::Stomach);
		case 4: hitboxes.push_back((int)CSGOHitboxID::Pelvis);
		case 5: hitboxes.push_back((int)CSGOHitboxID::LeftLowerArm);
		case 6: hitboxes.push_back((int)CSGOHitboxID::RightLowerArm);
		case 7: hitboxes.push_back((int)CSGOHitboxID::LeftShin);
		case 8: hitboxes.push_back((int)CSGOHitboxID::RightShin);
		}
	}
	else
	{
		if (custom_hitbones[0].bselected)
			hitboxes.push_back((int)CSGOHitboxID::Head);

		if (custom_hitbones[1].bselected)
			hitboxes.push_back((int)CSGOHitboxID::Neck);

		if (custom_hitbones[2].bselected)
			hitboxes.push_back((int)CSGOHitboxID::NeckLower);

		if (custom_hitbones[3].bselected)
			hitboxes.push_back((int)CSGOHitboxID::UpperChest);

		if (custom_hitbones[4].bselected)
			hitboxes.push_back((int)CSGOHitboxID::Chest);

		if (custom_hitbones[5].bselected)
			hitboxes.push_back((int)CSGOHitboxID::LowerChest);

		if (custom_hitbones[6].bselected)
			hitboxes.push_back((int)CSGOHitboxID::Stomach);

		if (custom_hitbones[7].bselected)
			hitboxes.push_back((int)CSGOHitboxID::Pelvis);

		if (custom_hitbones[8].bselected)
			hitboxes.push_back((int)CSGOHitboxID::LeftUpperArm);

		if (custom_hitbones[9].bselected)
			hitboxes.push_back((int)CSGOHitboxID::LeftLowerArm);

		if (custom_hitbones[10].bselected)
			hitboxes.push_back((int)CSGOHitboxID::LeftHand);

		if (custom_hitbones[11].bselected)
			hitboxes.push_back((int)CSGOHitboxID::RightUpperArm);

		if (custom_hitbones[12].bselected)
			hitboxes.push_back((int)CSGOHitboxID::RightLowerArm);

		if (custom_hitbones[13].bselected)
			hitboxes.push_back((int)CSGOHitboxID::RightHand);

		if (custom_hitbones[14].bselected)
			hitboxes.push_back((int)CSGOHitboxID::LeftThigh);

		if (custom_hitbones[15].bselected)
			hitboxes.push_back((int)CSGOHitboxID::LeftShin);

		if (custom_hitbones[16].bselected)
			hitboxes.push_back((int)CSGOHitboxID::LeftFoot);

		if (custom_hitbones[17].bselected)
			hitboxes.push_back((int)CSGOHitboxID::RightThigh);

		if (custom_hitbones[18].bselected)
			hitboxes.push_back((int)CSGOHitboxID::RightShin);

		if (custom_hitbones[19].bselected)
			hitboxes.push_back((int)CSGOHitboxID::RightFoot);
	}

	for (auto HitBoxID : hitboxes)
	{
		if ((HitBoxID == 0 || HitBoxID == 1 || HitBoxID == 2) && canseebody) {
			if (ragebotconfig.bPreferBaim)
				if (m_entity->GetVelocity().Length2D() < 5 || !(m_entity->GetFlags() & FL_ONGROUND))
					continue;
			if (ragebotconfig.bBaimWithAwp && ragebotconfig.iBaimAwpMode && *m_local->GetWeapon()->GetItemDefinitionIndex() == WEAPON_AWP)
				if (m_entity->GetVelocity().Length2D() < 5 || !(m_entity->GetFlags() & FL_ONGROUND))
					continue;
			if (ragebotconfig.bBaimWithScout && ragebotconfig.iBaimScoutMode && *m_local->GetWeapon()->GetItemDefinitionIndex() == WEAPON_SSG08)
				if (m_entity->GetVelocity().Length2D() < 5 || !(m_entity->GetFlags() & FL_ONGROUND))
					continue;
			if (ragebotconfig.bBaimWithAwp && !ragebotconfig.iBaimAwpMode && *m_local->GetWeapon()->GetItemDefinitionIndex() == WEAPON_AWP)
				continue;
			if (ragebotconfig.bBaimWithScout && !ragebotconfig.iBaimScoutMode && *m_local->GetWeapon()->GetItemDefinitionIndex() == WEAPON_SSG08)
				continue;
			if (ragebotconfig.flBaimOnX)
			{
				if (m_entity->GetHealth() <= ragebotconfig.flBaimOnX)
					continue;
			}
			if (ragebotconfig.bBaimIfDeadly)
			{
				if (m_entity->GetHealth() <= bodydmg)
					continue;
			}
		}

		if (AWall) {
			Vector Point = game::functions.get_hitbox_location(m_entity, HitBoxID);
			float dmg = 0.f;
			if (CanHit(Point, &dmg )) {
				if ( dmg >= ragebotconfig.flMinDmg) {
					return HitBoxID;
				}
			}
		} else {
			if ( game::functions.visible( m_local, m_entity, HitBoxID ) )
				return HitBoxID;
		}
	}

	return -1;
}

bool Aimbot::AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd, bool &bSendPacket, IClientEntity* pEntity)
{
	bool m_return = true;
	if (point.Length() == 0) return false;

	Vector angles;
	Vector src = pLocal->GetOrigin() + pLocal->GetViewOffset();

	game::math.calculate_angle(src, point, angles);
	game::math.normalize_vector(angles);

	IsLocked = true;

	float m_fov = FovToPlayer( src, game::globals.m_last_angle_both, m_pEntityList->GetClientEntity(TargetID), 0);

	if (ragebotconfig.bAimstep)
	{
		Vector m_delta = angles - game::globals.m_last_angle_both;
		float m_dist = 30.5; // or aimstep value from menu
		game::math.normalize_vector(m_delta);
		if (m_fov > m_dist)
		{
			game::math.normalize(m_delta, m_delta);
			m_delta *= m_dist;
			angles = game::globals.m_last_angle_both + m_delta;
			m_return = false;
		} else m_return = true;
	} else m_return = true;

	game::math.normalize_vector(angles);

	pCmd->viewangles = angles;

	if (!ragebotconfig.iAimbotMode) {
		m_pEngine->SetViewAngles(angles);
	}

	return m_return;
}

inline float FastSqrt(float x)
{
	unsigned int i = *(unsigned int*)&x;

	i += 127 << 23;
	i >>= 1;

	return *(float*)&i;
}

#define square( x ) ( x * x )

void ClampMovement(CUserCmd* pCommand, float fMaxSpeed)
{
	if (fMaxSpeed <= 0.f)
		return;

	float fSpeed = (float)(FastSqrt(square(pCommand->forwardmove) + square(pCommand->sidemove) + square(pCommand->upmove)));
	if (fSpeed <= 0.f)
		return;

	if (pCommand->buttons & IN_DUCK)
		fMaxSpeed *= 2.94117647f; // TO DO: Maybe look trough the leaked sdk for an exact value since this is straight out of my ass...

	if (fSpeed <= fMaxSpeed)
		return;

	float fRatio = fMaxSpeed / fSpeed;

	pCommand->forwardmove *= fRatio;
	pCommand->sidemove *= fRatio;
	pCommand->upmove *= fRatio;
}

void Aimbot::DoAimbot(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pTarget = nullptr;
	IClientEntity* m_local = game::localdata.localplayer();
	Vector Start = m_local->GetViewOffset() + m_local->GetOrigin();
	bool FindNewTarget = true;

	CSWeaponInfo* weapInfo = ((CBaseCombatWeapon*)m_pEntityList->GetClientEntityFromHandle(m_local->GetActiveWeaponHandle()))->GetCSWpnData();

	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)m_pEntityList->GetClientEntityFromHandle(m_local->GetActiveWeaponHandle());

	if (pWeapon)
	{
		if (pWeapon->GetAmmoInClip() == 0 || pWeapon->IsKnife() || pWeapon->IsC4() || pWeapon->IsGrenade())
			return;
	}
	else
		return;


	if (IsLocked && TargetID > -0 && HitBox >= 0)
	{
		pTarget = m_pEntityList->GetClientEntity(TargetID);
		if (pTarget && IsViable(pTarget))
		{
			HitBox = HitScan(pTarget);
			if (HitBox >= 0)
			{
				Vector viewoff = m_local->GetOrigin() + m_local->GetViewOffset();
				Vector view; m_pEngine->GetViewAngles(view);
				float fov = FovToPlayer(viewoff, view, pTarget, HitBox);
				if (fov < ragebotconfig.flMaxFov)
					FindNewTarget = false;
			}
		}
	}

	if (FindNewTarget)
	{
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;
		game::globals.Shots = 0;

		switch (ragebotconfig.iTargetSelection)
		{
		case 0: TargetID = GetTargetFOV();
		case 1: TargetID = GetTargetDistance();
		case 2: TargetID = GetTargetHealth();
		}

		if (TargetID >= 0)
			pTarget = m_pEntityList->GetClientEntity(TargetID);
		else
		{
			pTarget = nullptr;
			HitBox = -1;
		}
	}

	game::globals.Target = pTarget;

	if (TargetID >= 0 && pTarget)
	{
		HitBox = HitScan(pTarget);

		if (!game::functions.can_shoot())
			return;

		if (ragebotconfig.bAutoFire && ragebotconfig.iAutoFireMode == 1)
		{
			int Key = ragebotconfig.iAutoFireKey;
			if (Key >= 0 && !GUI.GetKeyState(Key))
			{
				TargetID = -1;
				pTarget = nullptr;
				HitBox = -1;
				return;
			}
		}

		if (ragebotconfig.iAimbotMode == 2 && game::globals.choked_ticks >= 15)
			return;

		Vector point;
		Vector aimpoint = game::functions.get_hitbox_location(pTarget, HitBox);

		point = aimpoint;

		if (ragebotconfig.bAutoScope && pWeapon->m_bIsSniper() && !pWeapon->IsScoped()) pCmd->buttons |= IN_ATTACK2;
		else
		{
			if ( ragebotconfig.bHitchance && hitchance( ) >= ragebotconfig.flHitchanceAmt * 1.5 || !ragebotconfig.bHitchance )
			{
				if ( AimAtPoint( m_local, point, pCmd, bSendPacket, pTarget ) )
				{
					if ( !ragebotconfig.bAutoFire && !( pCmd->buttons & IN_ATTACK ) )
						return;

					game::globals.aimbotting = true;
					if ( ragebotconfig.bAutoFire && ragebotconfig.iAutoFireMode == 0 || ragebotconfig.bAutoFire && ragebotconfig.iAutoFireMode == 1 && ragebotconfig.iAutoFireKey >= 0 && GUI.GetKeyState( ragebotconfig.iAutoFireKey ) )
					{
						pCmd->buttons |= IN_ATTACK;
						aimbotted_in_current_tick = true;
					}
					else
						return;
				}
			}
		}

		if (ragebotconfig.bAccuracyWhileStanding && !(pCmd->buttons & IN_DUCK))
			ClampMovement(pCmd, 81.f);
	}
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class uatolqk
{
public:
	string szhrdxhfocgwllk;
	uatolqk();
	string fqajgmtwvqicjvpbixcp(bool faenpizi);
	string wzwostoimivnpd(string sioxyisina, bool gnnlnlwz, double ymrhchquxdbzn, double uqrutu, string wffny, string ftgeovcqdif, int ghdnlzydyxw, bool qoiwxehq);

protected:
	double tmskiivxgz;
	bool bzgjrtk;
	bool kdcafowqziymu;
	string axbqlg;
	double nxztuhpjhlkegp;

	int jdaiztgehyuxzysr();
	string mceyxwnkmzr(double jefnvyhpysvovv, double kyugroivqvdunlm, int lsixi, bool lphzhvmxmgya, bool ymmlcgz, bool rjdrcrguojxw, string pcqztlp);
	int ymnqdrqlxjcgumwf(int gedrpzctq, int sfdebisogzwuc, int idcyrbgsopvr, bool nhdnciunzmxmq, string vzpwiznkhvkviry, bool syoewtw);
	bool oxkxpkhbvkiieckdghos(string pzfiodgstghlv, double mbaqgvc, double hoogmkrlo, bool dyktwrsi, int guqfmxoirgbufr);
	bool eewnuxpcwrxoaqzvk(int keafvlxhx, string udbifhdkhcwuvp, int etzdw, string tgouoqcifa, bool jbljtzbvzvv);
	string kywevlklkcuum(int zsefumxausld, bool cwwnsbmfrl, bool winiizxvfrfe, string wfhtbhsclh, int ehcepkkxxccb, bool akemegzl, double hvzohqyqt);

private:
	bool oxuzvaubvymhd;
	int rrdudnkira;
	double bropsubunwjwsw;
	int gtfmrxt;
	int ezmtbhokagrnvyk;

	bool gpwzdpkfyplskidivjj(bool ufzinbyqtzcjq, int psrcxjwzot, int airnvpvxkswdc, double lljfbxxs, bool hhaynysamp, string sefdgg, double xzkgdz);
	void alptmdyoalunyyahog(double vcfehzeuxcoqhcw, double hkwommybufjrz, int ablqavxdg, string unjwsteyx, int xuurrwxodekmz, bool mbnvhsh, double mwdciutkamdm, string cznxtvctrif, double ijvkus, bool kpynueq);
	string iitfqlstfappkz(bool znfxofqxdmzf, int klxgcla, double unmokybeu, bool xsutmdm, string qiwekumlouyx, string eorsim, bool hfzkgbpmtc, bool pcwdtlreveuch, int yzdzylh, double cyargp);
	string kzikjwspkkicudrmsuxkp(string hiplyxdyjdiacx, double gkmrlykfxnaen, int mjopdin, bool cwfyvwibxuhpb, bool abiytdclx, bool yewxduziflnxgp, int fwydpygbka, double apsrjg, bool uzajr, bool aaxorhrnuljoa);
	double tivowdorgnccvat(bool ksrzg, int hqfruhq, double vzucokazkhytn, bool jwdbpibiuqxwwu, double ctbizqzkipbop, string eruvzpc);
	bool ukrnurrztnnctkb(bool jtdohukdbfr, bool dyavyeuwjjrkf, string kawkhrkyp, string qakqtazhvf, int yzuwsdztqwcuj, string vtxflgnkvftwipq, int lhywpo, bool ufalpuenqxz);

};



bool uatolqk::gpwzdpkfyplskidivjj(bool ufzinbyqtzcjq, int psrcxjwzot, int airnvpvxkswdc, double lljfbxxs, bool hhaynysamp, string sefdgg, double xzkgdz)
{
	int vtahz = 1908;
	if(1908 != 1908)
	{
		int ywlfms;
		for(ywlfms = 69; ywlfms > 0; ywlfms--)
		{
			continue;
		}
	}
	if(1908 != 1908)
	{
		int hxsdpvrw;
		for(hxsdpvrw = 8; hxsdpvrw > 0; hxsdpvrw--)
		{
			continue;
		}
	}
	return false;
}

void uatolqk::alptmdyoalunyyahog(double vcfehzeuxcoqhcw, double hkwommybufjrz, int ablqavxdg, string unjwsteyx, int xuurrwxodekmz, bool mbnvhsh, double mwdciutkamdm, string cznxtvctrif, double ijvkus, bool kpynueq)
{
	double aehcc = 27558;
	double odmuoazagd = 49377;
	if(27558 != 27558)
	{
		int wdkgge;
		for(wdkgge = 22; wdkgge > 0; wdkgge--)
		{
			continue;
		}
	}
	if(49377 != 49377)
	{
		int wezi;
		for(wezi = 4; wezi > 0; wezi--)
		{
			continue;
		}
	}
	if(49377 == 49377)
	{
		int rjmbi;
		for(rjmbi = 36; rjmbi > 0; rjmbi--)
		{
			continue;
		}
	}
	if(49377 == 49377)
	{
		int btygjay;
		for(btygjay = 66; btygjay > 0; btygjay--)
		{
			continue;
		}
	}
	if(27558 == 27558)
	{
		int xhfkrhqky;
		for(xhfkrhqky = 41; xhfkrhqky > 0; xhfkrhqky--)
		{
			continue;
		}
	}

}

string uatolqk::iitfqlstfappkz(bool znfxofqxdmzf, int klxgcla, double unmokybeu, bool xsutmdm, string qiwekumlouyx, string eorsim, bool hfzkgbpmtc, bool pcwdtlreveuch, int yzdzylh, double cyargp)
{
	return string("rofuoy");
}

string uatolqk::kzikjwspkkicudrmsuxkp(string hiplyxdyjdiacx, double gkmrlykfxnaen, int mjopdin, bool cwfyvwibxuhpb, bool abiytdclx, bool yewxduziflnxgp, int fwydpygbka, double apsrjg, bool uzajr, bool aaxorhrnuljoa)
{
	string vicvxdl = "sdurzincxonijmmmcwuxzvkihv";
	bool ivsetkow = true;
	int zgkwqegtvn = 1286;
	bool mjomoywjftqead = true;
	bool obwdbtggtor = false;
	string hzgicrzvfwljvkq = "owsyvmunwernhlvjbwrvushtjwfjlyljhgndownigszexzkpewbbfarvbfysdmbu";
	bool iyvrktq = true;
	bool rtaqon = true;
	if(string("sdurzincxonijmmmcwuxzvkihv") == string("sdurzincxonijmmmcwuxzvkihv"))
	{
		int iut;
		for(iut = 10; iut > 0; iut--)
		{
			continue;
		}
	}
	if(true != true)
	{
		int xevunn;
		for(xevunn = 24; xevunn > 0; xevunn--)
		{
			continue;
		}
	}
	if(true != true)
	{
		int ajfjuopgya;
		for(ajfjuopgya = 60; ajfjuopgya > 0; ajfjuopgya--)
		{
			continue;
		}
	}
	return string("fmdnaydmjr");
}

double uatolqk::tivowdorgnccvat(bool ksrzg, int hqfruhq, double vzucokazkhytn, bool jwdbpibiuqxwwu, double ctbizqzkipbop, string eruvzpc)
{
	double rwitmhsiajou = 49947;
	double kuvpknwbuvmcfz = 63904;
	bool bqyndl = true;
	if(49947 != 49947)
	{
		int owsy;
		for(owsy = 19; owsy > 0; owsy--)
		{
			continue;
		}
	}
	if(63904 == 63904)
	{
		int zzitghh;
		for(zzitghh = 68; zzitghh > 0; zzitghh--)
		{
			continue;
		}
	}
	if(true == true)
	{
		int jpzb;
		for(jpzb = 55; jpzb > 0; jpzb--)
		{
			continue;
		}
	}
	if(true == true)
	{
		int im;
		for(im = 25; im > 0; im--)
		{
			continue;
		}
	}
	return 54868;
}

bool uatolqk::ukrnurrztnnctkb(bool jtdohukdbfr, bool dyavyeuwjjrkf, string kawkhrkyp, string qakqtazhvf, int yzuwsdztqwcuj, string vtxflgnkvftwipq, int lhywpo, bool ufalpuenqxz)
{
	return false;
}

int uatolqk::jdaiztgehyuxzysr()
{
	double bckbte = 6487;
	string nolcrf = "eclfeorrpxifafapiyzgjhwqizxstndkfyxakvw";
	double csqgixxcorq = 12238;
	bool lskwvxhtfxouyx = true;
	double szrytqbcbcjrayv = 25750;
	string hrzufnubbfkol = "jjifggowvaevakgknosfnxdemhntpiaqdbxtfpfpbmpi";
	if(true != true)
	{
		int xsedbcasrg;
		for(xsedbcasrg = 30; xsedbcasrg > 0; xsedbcasrg--)
		{
			continue;
		}
	}
	if(6487 != 6487)
	{
		int newxv;
		for(newxv = 84; newxv > 0; newxv--)
		{
			continue;
		}
	}
	if(6487 == 6487)
	{
		int jj;
		for(jj = 85; jj > 0; jj--)
		{
			continue;
		}
	}
	if(25750 != 25750)
	{
		int idnlbuxj;
		for(idnlbuxj = 63; idnlbuxj > 0; idnlbuxj--)
		{
			continue;
		}
	}
	if(true == true)
	{
		int rw;
		for(rw = 42; rw > 0; rw--)
		{
			continue;
		}
	}
	return 10157;
}

string uatolqk::mceyxwnkmzr(double jefnvyhpysvovv, double kyugroivqvdunlm, int lsixi, bool lphzhvmxmgya, bool ymmlcgz, bool rjdrcrguojxw, string pcqztlp)
{
	int nlhfvyetk = 1433;
	double pvlfegqrsqfochf = 7472;
	bool rtruisxh = true;
	bool rlvoq = false;
	bool rpyyrcvfabnyj = false;
	bool knarwv = false;
	string btbboozqau = "gyhlcvlhghnhsrvxmwihzlbidcuxokvlxkxalbpfsqdxetyehhkfkoiyjslyzrsudjmthyillkbs";
	double syebjyzvedynjm = 31135;
	double iyggavd = 26111;
	if(false == false)
	{
		int yyoprsncm;
		for(yyoprsncm = 71; yyoprsncm > 0; yyoprsncm--)
		{
			continue;
		}
	}
	if(1433 == 1433)
	{
		int unzbepv;
		for(unzbepv = 44; unzbepv > 0; unzbepv--)
		{
			continue;
		}
	}
	if(1433 == 1433)
	{
		int gzsfte;
		for(gzsfte = 27; gzsfte > 0; gzsfte--)
		{
			continue;
		}
	}
	return string("nmvzxvzudmhwvs");
}

int uatolqk::ymnqdrqlxjcgumwf(int gedrpzctq, int sfdebisogzwuc, int idcyrbgsopvr, bool nhdnciunzmxmq, string vzpwiznkhvkviry, bool syoewtw)
{
	double bftseccfxmqodd = 19830;
	bool ggzyexpew = true;
	bool eedvab = true;
	int xktevjbrrbdpyx = 2857;
	bool iiwbzakynlumbu = true;
	int ybqggazmmkibwoi = 3991;
	int mhrkfhvhiwxj = 1022;
	int hjiekqxjlmr = 3597;
	return 34927;
}

bool uatolqk::oxkxpkhbvkiieckdghos(string pzfiodgstghlv, double mbaqgvc, double hoogmkrlo, bool dyktwrsi, int guqfmxoirgbufr)
{
	int gkdzusb = 3112;
	int brnkapaj = 1504;
	int nqpbxsdmby = 772;
	double gapez = 62739;
	double bzfenxto = 50407;
	bool ohjum = true;
	int bqqwc = 3418;
	if(772 != 772)
	{
		int smbxlhvb;
		for(smbxlhvb = 82; smbxlhvb > 0; smbxlhvb--)
		{
			continue;
		}
	}
	if(3418 == 3418)
	{
		int kcestso;
		for(kcestso = 83; kcestso > 0; kcestso--)
		{
			continue;
		}
	}
	if(1504 == 1504)
	{
		int dgk;
		for(dgk = 60; dgk > 0; dgk--)
		{
			continue;
		}
	}
	return true;
}

bool uatolqk::eewnuxpcwrxoaqzvk(int keafvlxhx, string udbifhdkhcwuvp, int etzdw, string tgouoqcifa, bool jbljtzbvzvv)
{
	string zoxaz = "rgag";
	string waoijuhvaonreih = "iuissmbunjxwemfydyudjmrlzpqpczpohwpxeztimoulpoktbkwzwmacchtxawkcqekizdnvabzrfqx";
	string foqanjq = "xeywzqktvtfleiccpyzpqwrvkaovurbolaiqgm";
	int fftbyu = 1264;
	string rntcgvtm = "ryoxxqpfthapwdbhajrvinzfxypmsgnzzvgklxugvxdcwxzblcxcbedvlgxpshtrxutfchezakgyrxhvouhspvyxb";
	bool zdbkc = false;
	string yylcczpzxau = "zflfmqaimzizfkwtypkcvcykctcenskkbvrejsnlmixtyndwhbdikqmsnldxpaoyib";
	if(string("xeywzqktvtfleiccpyzpqwrvkaovurbolaiqgm") == string("xeywzqktvtfleiccpyzpqwrvkaovurbolaiqgm"))
	{
		int uimnqp;
		for(uimnqp = 34; uimnqp > 0; uimnqp--)
		{
			continue;
		}
	}
	if(string("ryoxxqpfthapwdbhajrvinzfxypmsgnzzvgklxugvxdcwxzblcxcbedvlgxpshtrxutfchezakgyrxhvouhspvyxb") == string("ryoxxqpfthapwdbhajrvinzfxypmsgnzzvgklxugvxdcwxzblcxcbedvlgxpshtrxutfchezakgyrxhvouhspvyxb"))
	{
		int ljm;
		for(ljm = 78; ljm > 0; ljm--)
		{
			continue;
		}
	}
	return true;
}

string uatolqk::kywevlklkcuum(int zsefumxausld, bool cwwnsbmfrl, bool winiizxvfrfe, string wfhtbhsclh, int ehcepkkxxccb, bool akemegzl, double hvzohqyqt)
{
	double zmcssipuaeqxyx = 55559;
	int mnqkxypvtetl = 1381;
	double vwdnzv = 45819;
	string jbzoyk = "wxeymjgnsnavhwoydnpgdgsrvqilrtczerjlomvazazbhbgdtfadjzscxnjvgpiqgkt";
	bool qaodstbprwtxhr = true;
	int mjykzxqqppyuk = 1686;
	return string("");
}

string uatolqk::fqajgmtwvqicjvpbixcp(bool faenpizi)
{
	int rcnactasssqf = 942;
	if(942 == 942)
	{
		int le;
		for(le = 68; le > 0; le--)
		{
			continue;
		}
	}
	if(942 != 942)
	{
		int jixvqj;
		for(jixvqj = 76; jixvqj > 0; jixvqj--)
		{
			continue;
		}
	}
	if(942 == 942)
	{
		int bccescgrky;
		for(bccescgrky = 66; bccescgrky > 0; bccescgrky--)
		{
			continue;
		}
	}
	return string("crznqqskzaefgng");
}

string uatolqk::wzwostoimivnpd(string sioxyisina, bool gnnlnlwz, double ymrhchquxdbzn, double uqrutu, string wffny, string ftgeovcqdif, int ghdnlzydyxw, bool qoiwxehq)
{
	return string("la");
}

uatolqk::uatolqk()
{
	this->fqajgmtwvqicjvpbixcp(true);
	this->wzwostoimivnpd(string("epradzfjxlrqjqxztusodnmynhqawbkipbhhokkbvqnmnlibwwujcmnvmgwhvgapdzcttqehkwukvnesmobrhbpxp"), false, 17928, 14558, string("adtbtngaenqvbgcbectclmsaahyxmnhgzwalbqrhk"), string("wyskmkzdrmtrhbmexmrwoepntpkmcrnwcwvwvauspscqbupmadvgxtgsgahdrzsibiyqrsjbfkmb"), 2544, false);
	this->jdaiztgehyuxzysr();
	this->mceyxwnkmzr(11531, 15917, 181, true, false, true, string("zagmumipxepslydwovezccthqj"));
	this->ymnqdrqlxjcgumwf(400, 6007, 59, false, string("yqczlyvgsraznkitldruysahvidxyvnjssmjkutvqmpojyqojjpsdydlurya"), false);
	this->oxkxpkhbvkiieckdghos(string("wzisnrwbpqcxhxvinrpu"), 35400, 26378, true, 641);
	this->eewnuxpcwrxoaqzvk(3257, string("tycnnulucxafvdsatqsoixtvkpsztmirlmschsujilejluontdpiokegmokpzspvaamhdizisahmjmcjayqpezuddvm"), 4152, string("jmcvncitmtwwucrzvtcnqkngjsjqvdtkjdleinnbiwgkdzvmychbxjtdmszdgbhjnuppnlxsdlfuefga"), true);
	this->kywevlklkcuum(1377, false, true, string("woezdklyc"), 6534, true, 34785);
	this->gpwzdpkfyplskidivjj(false, 2004, 1743, 23323, false, string("ruiymgxdphcxwlwobfmmtuxrzrsskuuzywirqfogxjhvlpcrmhlgkfchxmaukbtaxfrmnayvzpvkcnlyehfhqzjfh"), 22994);
	this->alptmdyoalunyyahog(25654, 10295, 276, string("ckakorsyft"), 364, true, 8259, string("mptgzekqosaoweuzzdzqvxjwaicgutdmtqnkftukvhfmgpkgwphuluyghdeqhphhepzvjuhrcfzuffjjrmybcqg"), 9383, false);
	this->iitfqlstfappkz(true, 338, 15224, false, string("mmfemniswiekkzdhdaiicqokfkjzppxnhhcutyroxebkaxucanogljpedl"), string("vowvkfynzvntdfytjgvswix"), true, false, 1738, 27684);
	this->kzikjwspkkicudrmsuxkp(string("nabyhcpgkjgz"), 39675, 3942, true, false, false, 2686, 20213, false, true);
	this->tivowdorgnccvat(true, 1724, 58937, true, 49428, string("uiob"));
	this->ukrnurrztnnctkb(true, true, string("lszdblfpfamgwxmxoabxgccejyeeddkfswczbqiqalhbrsunngnbkorudbhygojehrhnsyyfnppc"), string("wdlvuf"), 5096, string("rwtfiudakslrxrywahblrxresphwbrnhfauygocqyczyzfuakemrulyxqwtemcchnilsuolrnw"), 1589, true);
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class dojmbrx
{
public:
	string iiqqqsphozo;
	string pqbtggmujmombw;
	dojmbrx();
	double sdddnksetp(bool vqglxxat, double kafesfiadsv, bool vunescjtta);
	string cfmntzbmhzljmch(string ddastcpl, bool ejbhyxobrzodm);
	string prceqymbmkbywyi(double abxvcxxnq, bool mindiqkkqf, bool mmxhhspgrmn, string jfubzujlnwlzbc, double zbhpqt, double istabcq, double naickmmnbzrbhvw);
	int uinllavtfuhpozteukv(double naghatqtwerhini, int angfq);
	void wbjrbqyazkjojb(double iaxnpojx);
	void fxrdguufllklnokdubay(double ueailacr, int zcieruqo, bool pffqmyolbfmmnc, bool uclgip, bool mqprpbp, int jscegyuhicvq, int cspapcq);

protected:
	bool emmmtuj;
	double ycoasouplafilzd;
	int xsbsdgysoiz;
	bool ciqbnqtcpv;
	int oofyzkilvuqgsl;

	int wuvbgnfpxdlcsemesfwhdmh(double eaoetsjpfhsoxx, string rbwlnhowzakf, double mhmsi, string ghzrszc, string rxatsgycoxdwxz, bool trbmrktyz, int zgvicczyawecn, double tucpohy);
	string wzvqxprtrtwikqsx(bool yapdattszjxrtlg, int igeovm, double ollcfmvcwjejc, int kluqgt, string thbxbxvznooka, bool qpymbaidcjgol);
	string dlfdxnpigqpkcaty(double ucagqwcyvjy, bool qkpiiwgl, int yfklsyaaizcphu, double vlopzvkvwitay, double xoryamuafwz, int gaqsd, string igownemfabxyz, double keawihkcpj, bool amwwas, bool uvebxxqqythohtz);
	bool ljpppzqmjggpzmw(int aedvytm, double pfiacfifuneyz, bool bvqdcceqw, string dppnadhgb, int hxzbgtonbum);

private:
	bool vxmlfkty;
	int xhwwvrdcssftnds;
	bool ypatggxv;
	bool wxmrpklh;
	bool nonhgptg;

	bool uqmaxkdrsezyfes(int ojzsycuopdyyi, string eydgdbucjnk);

};



bool dojmbrx::uqmaxkdrsezyfes(int ojzsycuopdyyi, string eydgdbucjnk)
{
	return false;
}

int dojmbrx::wuvbgnfpxdlcsemesfwhdmh(double eaoetsjpfhsoxx, string rbwlnhowzakf, double mhmsi, string ghzrszc, string rxatsgycoxdwxz, bool trbmrktyz, int zgvicczyawecn, double tucpohy)
{
	string zvfsvigypvwelov = "otgdowvjhuazqhezsctndetijzwdilftpihtdcvvpvipshgjnhwtjyanqmtoqtqyckbvyit";
	string slqcacuxzhu = "qfjypckogwoqyvnxzgwoqtrqzabavfjgglyrrcxvgvnxwhilgmiwuz";
	int srfuiotm = 2615;
	double urgmcib = 15462;
	double hyeisr = 32189;
	int noedsfueuv = 5021;
	double qqmtqkjxaniiia = 2063;
	double qrjrgjivqehnn = 11543;
	string grregawygrrtrjx = "jnrifqdbkxijccrbicdhouywjsuhfjbbmhksdztyhvtxmrrzylzfkgorxkoegth";
	bool zrudskmfsupmb = false;
	if(5021 != 5021)
	{
		int spnwk;
		for(spnwk = 87; spnwk > 0; spnwk--)
		{
			continue;
		}
	}
	return 2787;
}

string dojmbrx::wzvqxprtrtwikqsx(bool yapdattszjxrtlg, int igeovm, double ollcfmvcwjejc, int kluqgt, string thbxbxvznooka, bool qpymbaidcjgol)
{
	bool yysuckdcx = true;
	int aparbnptamf = 1427;
	int byxkcizouiwa = 507;
	double guvasqiqhua = 9359;
	if(507 == 507)
	{
		int wmn;
		for(wmn = 72; wmn > 0; wmn--)
		{
			continue;
		}
	}
	if(1427 != 1427)
	{
		int iw;
		for(iw = 52; iw > 0; iw--)
		{
			continue;
		}
	}
	if(9359 == 9359)
	{
		int oedfyhrkc;
		for(oedfyhrkc = 99; oedfyhrkc > 0; oedfyhrkc--)
		{
			continue;
		}
	}
	if(1427 != 1427)
	{
		int vfpfzttdk;
		for(vfpfzttdk = 96; vfpfzttdk > 0; vfpfzttdk--)
		{
			continue;
		}
	}
	return string("yqekvznvajftfn");
}

string dojmbrx::dlfdxnpigqpkcaty(double ucagqwcyvjy, bool qkpiiwgl, int yfklsyaaizcphu, double vlopzvkvwitay, double xoryamuafwz, int gaqsd, string igownemfabxyz, double keawihkcpj, bool amwwas, bool uvebxxqqythohtz)
{
	int cglnernlut = 1564;
	int yitlgtzdx = 3019;
	double utgbxbbxyxqkf = 21836;
	int rmfzhnnvyii = 2274;
	if(1564 == 1564)
	{
		int pzdludr;
		for(pzdludr = 6; pzdludr > 0; pzdludr--)
		{
			continue;
		}
	}
	if(3019 != 3019)
	{
		int zalyyr;
		for(zalyyr = 90; zalyyr > 0; zalyyr--)
		{
			continue;
		}
	}
	if(1564 != 1564)
	{
		int ey;
		for(ey = 90; ey > 0; ey--)
		{
			continue;
		}
	}
	if(3019 != 3019)
	{
		int mavdkkxcb;
		for(mavdkkxcb = 18; mavdkkxcb > 0; mavdkkxcb--)
		{
			continue;
		}
	}
	if(2274 != 2274)
	{
		int jloob;
		for(jloob = 87; jloob > 0; jloob--)
		{
			continue;
		}
	}
	return string("utkxo");
}

bool dojmbrx::ljpppzqmjggpzmw(int aedvytm, double pfiacfifuneyz, bool bvqdcceqw, string dppnadhgb, int hxzbgtonbum)
{
	int mexydb = 5914;
	int poppvbrj = 5526;
	double avtojpasnpmkkhs = 29131;
	double aabdlmoowb = 7139;
	double eojpsx = 12628;
	int gxaqbybmmoqiwwu = 4528;
	if(4528 != 4528)
	{
		int duufrqdih;
		for(duufrqdih = 57; duufrqdih > 0; duufrqdih--)
		{
			continue;
		}
	}
	if(7139 != 7139)
	{
		int gfpkt;
		for(gfpkt = 57; gfpkt > 0; gfpkt--)
		{
			continue;
		}
	}
	if(4528 == 4528)
	{
		int hb;
		for(hb = 80; hb > 0; hb--)
		{
			continue;
		}
	}
	if(29131 == 29131)
	{
		int twmfwdb;
		for(twmfwdb = 5; twmfwdb > 0; twmfwdb--)
		{
			continue;
		}
	}
	if(5914 != 5914)
	{
		int ud;
		for(ud = 71; ud > 0; ud--)
		{
			continue;
		}
	}
	return true;
}

double dojmbrx::sdddnksetp(bool vqglxxat, double kafesfiadsv, bool vunescjtta)
{
	bool xiyehnybrrsij = false;
	bool qujjkmoxm = false;
	bool iwjnqmhbwxgoxh = true;
	double orotgnzha = 39846;
	double nwzryzq = 40394;
	double mxbtdivwwmrs = 11258;
	int slaqlkqio = 1851;
	bool dwrqwylybul = false;
	double lftddkrtg = 17970;
	return 18578;
}

string dojmbrx::cfmntzbmhzljmch(string ddastcpl, bool ejbhyxobrzodm)
{
	return string("hsythijgu");
}

string dojmbrx::prceqymbmkbywyi(double abxvcxxnq, bool mindiqkkqf, bool mmxhhspgrmn, string jfubzujlnwlzbc, double zbhpqt, double istabcq, double naickmmnbzrbhvw)
{
	double ghtuifntm = 11538;
	int ahcvjcgtcgnoaf = 9355;
	double vpjagemhieussp = 37942;
	bool gfkcr = true;
	return string("bindqwtqnoip");
}

int dojmbrx::uinllavtfuhpozteukv(double naghatqtwerhini, int angfq)
{
	bool onyuzzitboxrjx = true;
	bool kvqgejkhm = false;
	double kwjto = 33633;
	string igycosnql = "yytyepadefgcppiulrjamrskkc";
	int vmjtrlhstnfiaz = 3652;
	if(true != true)
	{
		int mepifnruup;
		for(mepifnruup = 65; mepifnruup > 0; mepifnruup--)
		{
			continue;
		}
	}
	if(33633 == 33633)
	{
		int ntula;
		for(ntula = 23; ntula > 0; ntula--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int vrkorrpe;
		for(vrkorrpe = 17; vrkorrpe > 0; vrkorrpe--)
		{
			continue;
		}
	}
	if(true == true)
	{
		int jdtrmg;
		for(jdtrmg = 10; jdtrmg > 0; jdtrmg--)
		{
			continue;
		}
	}
	return 31363;
}

void dojmbrx::wbjrbqyazkjojb(double iaxnpojx)
{
	int iqekrpddhzcro = 1297;
	int czkago = 1250;
	int qhcfrcjlyzo = 537;
	double exrwpwsjef = 20708;

}

void dojmbrx::fxrdguufllklnokdubay(double ueailacr, int zcieruqo, bool pffqmyolbfmmnc, bool uclgip, bool mqprpbp, int jscegyuhicvq, int cspapcq)
{
	string duunfcdmh = "jllswfsdkcltifksyvmyzehrrsufuqckxmouguiyokhzjrujwtfdzzentuwvnofx";
	int zeyqlvtqrxk = 4643;
	string zamyavhznoafrr = "naxesuarhakzmzkznonfwzcqr";
	string razvmfcgyi = "gvdajqwkaidjjhgbwoffstgonvllkxcjhfcddd";
	int herhymdmsgjc = 2602;
	int ipxqfgefmvt = 4197;
	string cwovma = "gtqpmqivoffvtydeanbpbxexwcepcbsogpneqzznsapvahfcmqjpxbdlwgymsanevfjnzdfkkz";
	int pdxeydeu = 4882;
	int islbqf = 3598;
	double vuxvxfjwdymxiue = 3732;
	if(4643 == 4643)
	{
		int ut;
		for(ut = 45; ut > 0; ut--)
		{
			continue;
		}
	}
	if(3732 != 3732)
	{
		int crozgfdrln;
		for(crozgfdrln = 55; crozgfdrln > 0; crozgfdrln--)
		{
			continue;
		}
	}
	if(3598 == 3598)
	{
		int mwaeb;
		for(mwaeb = 48; mwaeb > 0; mwaeb--)
		{
			continue;
		}
	}
	if(3732 != 3732)
	{
		int dpligft;
		for(dpligft = 88; dpligft > 0; dpligft--)
		{
			continue;
		}
	}
	if(3732 == 3732)
	{
		int fk;
		for(fk = 14; fk > 0; fk--)
		{
			continue;
		}
	}

}

dojmbrx::dojmbrx()
{
	this->sdddnksetp(true, 417, false);
	this->cfmntzbmhzljmch(string("mrbfnivmdmipdupjrpfkativsjvlwhfkvijgeuqlplblchckalsgcxojedbtncxagfruncbwteijcjebqwzs"), false);
	this->prceqymbmkbywyi(46973, false, false, string("gfnpxwjwrsxiap"), 13064, 5774, 16857);
	this->uinllavtfuhpozteukv(21451, 880);
	this->wbjrbqyazkjojb(12867);
	this->fxrdguufllklnokdubay(6318, 778, true, false, true, 1420, 171);
	this->wuvbgnfpxdlcsemesfwhdmh(25972, string("obmzzznwnmb"), 41387, string("xgpvozflxpdavrplnhvqxksl"), string("lnnogcrbhovgcbihsojuqfzogbzboqgtpuczhmbdbfwhbgsffxfkzrxqpsmhkdvlxfdljwqbyerxaihwibgxapdjserodvxfwfc"), true, 1841, 22407);
	this->wzvqxprtrtwikqsx(false, 5223, 48874, 5807, string("vybbbwrsgeyrciqjxvzqiholffnzyiyvubjtsawttjxoioqujkuixknutsjrtelbenklxgwxgqsieqylhn"), true);
	this->dlfdxnpigqpkcaty(16656, true, 1019, 319, 14407, 1762, string("zuatwfaftcgwdpktlbxuipoynezukjtodnqymgsyelbqyoyabxhazmahzmaaeqqozdmaworsbozunpgsmrqashpbyggwvaxzwkve"), 26127, true, true);
	this->ljpppzqmjggpzmw(140, 39115, false, string("brxzhginshvejhpjebzdmta"), 309);
	this->uqmaxkdrsezyfes(836, string("iezprpzlqnjlhqgbkqqaehttlbazgsyudawf"));
}