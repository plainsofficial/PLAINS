#include "SkinChanger.h"

SkinChanger skinchanger;

std::unordered_map<int, SkinStruct>  g_SkinCFG;
std::unordered_map<int, const char*> g_ViewModelCFG;
std::unordered_map<const char*, const char*> g_KillIconCfg;

void SkinChanger::update_settings()
{
	if (m_pEngine->IsInGame() && m_pEngine->IsConnected())
	{
		auto m_local = game::localdata.localplayer();

		if (!m_local)
			return;

		if (!m_local->IsAlive())
			return;

		if (!m_local->GetWeapon())
			return;

		static int last_model;
		static int last_paintkit;
		static float last_wear;
		static int last_quality;

		if (last_paintkit != skinconfig.iKnifePaintkit)
		{
			g_SkinCFG[WEAPON_KNIFE_CT].nFallbackPaintKit = skinconfig.iKnifePaintkit;
			g_SkinCFG[WEAPON_KNIFE_T].nFallbackPaintKit = skinconfig.iKnifePaintkit;

			last_paintkit = skinconfig.iKnifePaintkit;
		}

		if (skinconfig.bOverrideKnife && last_model != skinconfig.iKnife)
		{
			std::vector<char*> icons;
			icons.push_back("bayonet");
			icons.push_back("knife_m9_bayonet");
			icons.push_back("knife_butterfly");
			icons.push_back("knife_flip");
			icons.push_back("knife_gut");
			icons.push_back("knife_karambit");
			icons.push_back("knife_tactical");
			icons.push_back("knife_falchion");
			icons.push_back("knife_push");
			icons.push_back("knife_survival_bowie");

			g_KillIconCfg["knife_default_ct"] = icons[skinconfig.iKnife - 1];
			g_KillIconCfg["knife_t"] = icons[skinconfig.iKnife - 1];

			int nOriginalKnifeCT = m_pModelInfo->GetModelIndex("models/weapons/v_knife_default_ct.mdl");
			int nOriginalKnifeT = m_pModelInfo->GetModelIndex("models/weapons/v_knife_default_t.mdl");

			std::vector<char*> models;
			models.push_back("models/weapons/v_knife_bayonet.mdl");
			models.push_back("models/weapons/v_knife_m9_bay.mdl");
			models.push_back("models/weapons/v_knife_butterfly.mdl");
			models.push_back("models/weapons/v_knife_flip.mdl");
			models.push_back("models/weapons/v_knife_gut.mdl");
			models.push_back("models/weapons/v_knife_karam.mdl");
			models.push_back("models/weapons/v_knife_tactical.mdl");
			models.push_back("models/weapons/v_knife_falchion_advanced.mdl");
			models.push_back("models/weapons/v_knife_push.mdl");
			models.push_back("models/weapons/v_knife_survival_bowie.mdl");

			g_ViewModelCFG[nOriginalKnifeCT] = models[skinconfig.iKnife - 1];
			g_ViewModelCFG[nOriginalKnifeT] = models[skinconfig.iKnife - 1];

			if (skinconfig.iKnife == 1)
			{
				g_SkinCFG[WEAPON_KNIFE_CT].nItemDefinitionIndex = WEAPON_KNIFE_BAYONET;
				g_SkinCFG[WEAPON_KNIFE_T].nItemDefinitionIndex = WEAPON_KNIFE_BAYONET;
			}
			else if (skinconfig.iKnife == 2)
			{
				g_SkinCFG[WEAPON_KNIFE_CT].nItemDefinitionIndex = WEAPON_KNIFE_M9_BAYONET;
				g_SkinCFG[WEAPON_KNIFE_T].nItemDefinitionIndex = WEAPON_KNIFE_M9_BAYONET;
			}
			else if (skinconfig.iKnife == 3)
			{
				g_SkinCFG[WEAPON_KNIFE_CT].nItemDefinitionIndex = WEAPON_KNIFE_BUTTERFLY;
				g_SkinCFG[WEAPON_KNIFE_T].nItemDefinitionIndex = WEAPON_KNIFE_BUTTERFLY;
			}
			else if (skinconfig.iKnife == 4)
			{
				g_SkinCFG[WEAPON_KNIFE_CT].nItemDefinitionIndex = WEAPON_KNIFE_FLIP;
				g_SkinCFG[WEAPON_KNIFE_T].nItemDefinitionIndex = WEAPON_KNIFE_FLIP;
			}
			else if (skinconfig.iKnife == 5)
			{
				g_SkinCFG[WEAPON_KNIFE_CT].nItemDefinitionIndex = WEAPON_KNIFE_GUT;
				g_SkinCFG[WEAPON_KNIFE_T].nItemDefinitionIndex = WEAPON_KNIFE_GUT;
			}
			else if (skinconfig.iKnife == 6)
			{
				g_SkinCFG[WEAPON_KNIFE_CT].nItemDefinitionIndex = WEAPON_KNIFE_KARAMBIT;
				g_SkinCFG[WEAPON_KNIFE_T].nItemDefinitionIndex = WEAPON_KNIFE_KARAMBIT;
			}
			else if (skinconfig.iKnife == 7)
			{
				g_SkinCFG[WEAPON_KNIFE_CT].nItemDefinitionIndex = WEAPON_KNIFE_TACTICAL;
				g_SkinCFG[WEAPON_KNIFE_T].nItemDefinitionIndex = WEAPON_KNIFE_TACTICAL;
			}
			else if (skinconfig.iKnife == 8)
			{
				g_SkinCFG[WEAPON_KNIFE_CT].nItemDefinitionIndex = WEAPON_KNIFE_FALCHION;
				g_SkinCFG[WEAPON_KNIFE_T].nItemDefinitionIndex = WEAPON_KNIFE_FALCHION;
			}
			else if (skinconfig.iKnife == 9)
			{
				g_SkinCFG[WEAPON_KNIFE_CT].nItemDefinitionIndex = WEAPON_KNIFE_PUSH;
				g_SkinCFG[WEAPON_KNIFE_T].nItemDefinitionIndex = WEAPON_KNIFE_PUSH;
			}
			else if (skinconfig.iKnife == 10)
			{
				g_SkinCFG[WEAPON_KNIFE_CT].nItemDefinitionIndex = WEAPON_KNIFE_BOWIE;
				g_SkinCFG[WEAPON_KNIFE_T].nItemDefinitionIndex = WEAPON_KNIFE_BOWIE;
			}

			last_model = skinconfig.iKnife;

			if (last_wear != skinconfig.flKnifeWear)
			{
				g_SkinCFG[WEAPON_KNIFE_CT].flFallbackWear = skinconfig.flKnifeWear;
				g_SkinCFG[WEAPON_KNIFE_T].flFallbackWear = skinconfig.flKnifeWear;
				last_wear = skinconfig.flKnifeWear;
			}

			if (last_quality != skinconfig.iKnifeQuality && skinconfig.iKnifeQuality)
			{
				std::vector<int> qualities;
				qualities.push_back(0);
				qualities.push_back(1);
				qualities.push_back(2);
				qualities.push_back(3);
				qualities.push_back(5);
				qualities.push_back(6);
				qualities.push_back(7);
				qualities.push_back(8);
				qualities.push_back(9);
				qualities.push_back(10);
				qualities.push_back(12);

				g_SkinCFG[WEAPON_KNIFE_CT].nEntityQuality = qualities[skinconfig.iKnifeQuality];
				g_SkinCFG[WEAPON_KNIFE_T].nEntityQuality = qualities[skinconfig.iKnifeQuality];

				last_quality = skinconfig.iKnifeQuality;
			}
		}
	}
}

void SkinChanger::set_skins()
{
	// StatTrak� AWP | Dragon Lore
	g_SkinCFG[WEAPON_AWP].nFallbackPaintKit = 344;
	g_SkinCFG[WEAPON_AWP].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_AWP].nFallbackStatTrak = 1337;

	// AK-47 | Aquamarine Revenge
	g_SkinCFG[WEAPON_AK47].nFallbackPaintKit = 474;
	g_SkinCFG[WEAPON_AK47].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_AK47].nFallbackStatTrak = 1337;

	// M4A4 | Howl
	g_SkinCFG[WEAPON_M4A1].nFallbackPaintKit = 309;
	g_SkinCFG[WEAPON_M4A1].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_M4A1].nFallbackStatTrak = 1337;

	// Glock-18 | Fade
	g_SkinCFG[WEAPON_GLOCK].nFallbackPaintKit = 38;
	g_SkinCFG[WEAPON_GLOCK].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_GLOCK].nFallbackStatTrak = 1337;

	// Desert Eagle | Blaze
	g_SkinCFG[WEAPON_DEAGLE].nFallbackPaintKit = 37;
	g_SkinCFG[WEAPON_DEAGLE].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_DEAGLE].nFallbackStatTrak = 1337;

	// Tec-9 | Fuel Injector
	g_SkinCFG[WEAPON_TEC9].nFallbackPaintKit = 614;
	g_SkinCFG[WEAPON_TEC9].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_TEC9].nFallbackStatTrak = 1337;

	// P250 | Whiteout
	g_SkinCFG[WEAPON_P250].nFallbackPaintKit = 102;
	g_SkinCFG[WEAPON_P250].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_P250].nFallbackStatTrak = 1337;

	// USP-S | Cortex
	g_SkinCFG[WEAPON_USP_SILENCER].nFallbackPaintKit = 705;
	g_SkinCFG[WEAPON_USP_SILENCER].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_USP_SILENCER].nFallbackStatTrak = 1337;

	// Scar-20 | Cyrex
	g_SkinCFG[WEAPON_SCAR20].nFallbackPaintKit = 312;
	g_SkinCFG[WEAPON_SCAR20].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_SCAR20].nFallbackStatTrak = 1337;

	// G3SG1 | The Executioner
	g_SkinCFG[WEAPON_G3SG1].nFallbackPaintKit = 511;
	g_SkinCFG[WEAPON_G3SG1].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_G3SG1].nFallbackStatTrak = 1337;

	// MP7 | Nemesis
	g_SkinCFG[WEAPON_MP7].nFallbackPaintKit = 481;
	g_SkinCFG[WEAPON_MP7].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_MP7].nFallbackStatTrak = 1337;

	// MP9 | Airlock
	g_SkinCFG[WEAPON_MP9].nFallbackPaintKit = 609;
	g_SkinCFG[WEAPON_MP9].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_MP9].nFallbackStatTrak = 1337;

	// PP-Bizon | Judgement of Anubis
	g_SkinCFG[WEAPON_BIZON].nFallbackPaintKit = 542;
	g_SkinCFG[WEAPON_BIZON].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_BIZON].nFallbackStatTrak = 1337;

	// SSG08 | Dragonfire
	g_SkinCFG[WEAPON_SSG08].nFallbackPaintKit = 623;
	g_SkinCFG[WEAPON_SSG08].flFallbackWear = 0.00000001f;
	g_SkinCFG[WEAPON_SSG08].nFallbackStatTrak = 1337;
}

void SkinChanger::set_viewmodel()
{
	bool has_model = false;

	if (skinconfig.iKnife && skinconfig.bOverrideKnife)
		has_model = true;

	if (!has_model)
	{
		int nOriginalKnifeCT = m_pModelInfo->GetModelIndex("models/weapons/v_knife_default_ct.mdl");
		int nOriginalKnifeT = m_pModelInfo->GetModelIndex("models/weapons/v_knife_default_t.mdl");

		g_ViewModelCFG[nOriginalKnifeCT] = "models/weapons/v_knife_default_ct.mdl";
		g_ViewModelCFG[nOriginalKnifeT] = "models/weapons/v_knife_default_t.mdl";
	}
	else
	{
		int nOriginalKnifeCT = m_pModelInfo->GetModelIndex("models/weapons/v_knife_default_ct.mdl");
		int nOriginalKnifeT = m_pModelInfo->GetModelIndex("models/weapons/v_knife_default_t.mdl");

		std::vector<char*> models;
		models.push_back("models/weapons/v_knife_bayonet.mdl");
		models.push_back("models/weapons/v_knife_m9_bay.mdl");
		models.push_back("models/weapons/v_knife_butterfly.mdl");
		models.push_back("models/weapons/v_knife_flip.mdl");
		models.push_back("models/weapons/v_knife_gut.mdl");
		models.push_back("models/weapons/v_knife_karam.mdl");
		models.push_back("models/weapons/v_knife_tactical.mdl");
		models.push_back("models/weapons/v_knife_falchion_advanced.mdl");
		models.push_back("models/weapons/v_knife_push.mdl");
		models.push_back("models/weapons/v_knife_survival_bowie.mdl");

		g_ViewModelCFG[nOriginalKnifeCT] = models[skinconfig.iKnife - 1];
		g_ViewModelCFG[nOriginalKnifeT] = models[skinconfig.iKnife - 1];
	}
}

bool SkinChanger::apply_skins(CBaseAttributableItem* pWeapon, int nWeaponIndex)
{
	if (g_SkinCFG.find(nWeaponIndex) == g_SkinCFG.end())
		return false;

	*pWeapon->GetFallbackPaintKit() = g_SkinCFG[nWeaponIndex].nFallbackPaintKit;
	*pWeapon->GetEntityQuality() = g_SkinCFG[nWeaponIndex].nEntityQuality;
	*pWeapon->GetFallbackSeed() = g_SkinCFG[nWeaponIndex].nFallbackSeed;
	*pWeapon->GetFallbackStatTrak() = g_SkinCFG[nWeaponIndex].nFallbackStatTrak;
	*pWeapon->GetFallbackWear() = g_SkinCFG[nWeaponIndex].flFallbackWear;

	if (g_SkinCFG[nWeaponIndex].nItemDefinitionIndex)
		*pWeapon->GetItemDefinitionIndex() = g_SkinCFG[nWeaponIndex].nItemDefinitionIndex;

	if (g_SkinCFG[nWeaponIndex].szWeaponName) {
		sprintf_s(pWeapon->GetCustomName(), 32, "%s", g_SkinCFG[nWeaponIndex].szWeaponName);
	}

	*pWeapon->GetItemIDHigh() = -1;

	return true;
}

bool SkinChanger::apply_viewmodel(IClientEntity* pLocal, CBaseAttributableItem* pWeapon, int nWeaponIndex) 
{
	CBaseViewModel* pViewModel = (CBaseViewModel*)m_pEntityList->GetClientEntityFromHandle(*(HANDLE*)((DWORD)pLocal + 0x32DC));

	if (!pViewModel)
		return false;

	DWORD hViewModelWeapon = pViewModel->GetWeapon();
	CBaseAttributableItem* pViewModelWeapon = (CBaseAttributableItem*)m_pEntityList->GetClientEntityFromHandle((HANDLE)hViewModelWeapon);

	if (pViewModelWeapon != pWeapon)
		return false;

	int nViewModelIndex = pViewModel->GetModelIndex();

	if (g_ViewModelCFG.find(nViewModelIndex) == g_ViewModelCFG.end())
		return false;

	pViewModel->SetWeaponModel(g_ViewModelCFG[nViewModelIndex], pWeapon);

	return true;
}

bool SkinChanger::apply_killcon(IGameEvent* pEvent) 
{
	int nUserID = pEvent->GetInt("attacker");

	if (!nUserID)
		return false;

	if (m_pEngine->GetPlayerForUserID(nUserID) != m_pEngine->GetLocalPlayer())
		return false;

	const char* szWeapon = pEvent->GetString("weapon");

	for (auto ReplacementIcon : g_KillIconCfg) 
	{
		if (!strcmp(szWeapon, ReplacementIcon.first)) 
		{
			pEvent->SetString("weapon", ReplacementIcon.second);
			break;
		}
	}

	return true;
}
