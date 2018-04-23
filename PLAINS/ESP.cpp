#include <iostream>

#include "ESP.h"
#include "Interfaces.h"
#include "DrawManager.h"

#define minimum(a,b) (((a) < (b)) ? (a) : (b))
#define ccsplayer 35

CEsp esp;

float flPlayerAlpha[65];

void CEsp::paint()
{
	auto m_local = game::localdata.localplayer();

	if(m_local)
	{
		for(int i = 0; i < m_pEntityList->GetHighestEntityIndex(); i++)
		{
			auto m_entity = static_cast<IClientEntity*>(m_pEntityList->GetClientEntity(i));
			player_info_t player_info;

			if(!m_entity)
			{
				continue;
			}

			if(m_entity->GetClientClass()->m_ClassID == ccsplayer)
			{
				if(m_entity->IsDormant() && flPlayerAlpha[i] > 0) flPlayerAlpha[i] -= 5;
				else if(!(m_entity->IsDormant()) && flPlayerAlpha[i] < 255) flPlayerAlpha[i] += 5;
				float alpha = flPlayerAlpha[i];
				game::math.clamp(alpha, 0, 255);

				if(m_entity->IsAlive())
				{
					Box box;
					Color plc = Color(get_player_colors(m_entity).r(), get_player_colors(m_entity).g(), get_player_colors(m_entity).b(), alpha);

					if(!get_box(m_entity, box, visualconfig.player.iEspMode))
					{
						player.direction_arrow(m_entity->GetOrigin());

						continue;
					}

					if(!visualconfig.player.bTeammates && m_entity->GetTeamNum() == m_local->GetTeamNum() || m_entity == m_local)
					{
						continue;
					}

					if(visualconfig.player.iActivationType == 1 && !game::functions.visible(m_local, m_entity, 0))
					{
						continue;
					}

					player.paint_player(m_entity, box, plc);
				}
			}

			if(m_entity && m_entity != m_local && !m_entity->IsDormant())
			{
				ClientClass* client_class = (ClientClass*)m_entity->GetClientClass();

				if(visualconfig.bFilterWeapons && client_class->m_ClassID != (int)CSGOClassID::CBaseWeaponWorldModel && ((strstr(client_class->m_pNetworkName, "Weapon") || client_class->m_ClassID == (int)CSGOClassID::CDEagle || client_class->m_ClassID == (int)CSGOClassID::CAK47)))
				{
					draw_weapons(m_entity, client_class);
				}

				if(visualconfig.bFilterNades)
				{
					draw_grenades(m_entity, client_class);
				}

				if(visualconfig.bFilterBomb)
				{
					if(client_class->m_ClassID == (int)CSGOClassID::CPlantedC4)
					{
						draw_bomb_planted(m_entity, client_class);
					}

					if(client_class->m_ClassID == (int)CSGOClassID::CC4)
					{
						draw_bomb(m_entity, client_class);
					}
				}
			}
		}

		if(m_local->IsAlive() && visualconfig.bAntiaimLines)
		{
			antiaim_lines();
		}

		if(visualconfig.bRemoveFlash)
		{
			DWORD m_flFlashMaxAlpha = NetVar.GetNetVar(0xFE79FB98);
			*(float*)((DWORD)m_local + m_flFlashMaxAlpha) = 0;
		}
	}
}

void CEsp::antiaim_lines()
{
	auto m_local = game::localdata.localplayer();

	static float line_length = 80.f;

	Vector lby, fake, real;
	Vector start, end, forward, start_2d, end_2d;

	lby = Vector(0.f, m_local->GetLowerBodyYaw(), 0.f);
	fake = Vector(0.f, game::globals.aa_line_data.fake_angle, 0.f);
	real = Vector(0.f, game::globals.aa_line_data.real_angle, 0.f);

	start = m_local->GetOrigin();
	game::math.angle_vectors(lby, forward);
	forward *= line_length;
	end = start + forward;

	if(!game::functions.world_to_screen(start, start_2d) || !game::functions.world_to_screen(end, end_2d))
	{
		return;
	}

	draw.line(start_2d.x, start_2d.y, end_2d.x, end_2d.y, Color(0, 255, 0, 255));

	game::math.angle_vectors(fake, forward);
	forward *= line_length;
	end = start + forward;

	if(!game::functions.world_to_screen(start, start_2d) || !game::functions.world_to_screen(end, end_2d))
	{
		return;
	}

	draw.line(start_2d.x, start_2d.y, end_2d.x, end_2d.y, Color(255, 0, 0, 255));

	game::math.angle_vectors(real, forward);
	forward *= line_length;
	end = start + forward;

	if(!game::functions.world_to_screen(start, start_2d) || !game::functions.world_to_screen(end, end_2d))
	{
		return;
	}

	draw.line(start_2d.x, start_2d.y, end_2d.x, end_2d.y, Color(255, 255, 0, 255));
}

void CEsp::CVisualsPlayer::draw_box(IClientEntity* m_entity, Box box, Color color)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	if(visualconfig.player.bBoxFill && visualconfig.player.flBoxFillOpacity > 0)
	{
		Color color_fill = Color(color.r(), color.g(), color.b(), (alpha / 255) * visualconfig.player.flBoxFillOpacity * 2.55);
		draw.rect(box.x, box.y, box.w, box.h, color_fill);
	}

	if(visualconfig.player.bBox)
	{
		if(!visualconfig.player.iBoxType)
		{
			draw.outline(box.x, box.y, box.w, box.h, color);

			if(visualconfig.player.bBoxOutlines)
			{
				draw.outline(box.x - 1, box.y - 1, box.w + 2, box.h + 2, Color(21, 21, 21, alpha));
				draw.outline(box.x + 1, box.y + 1, box.w - 2, box.h - 2, Color(21, 21, 21, alpha));
			}
		}
		else if(visualconfig.player.iBoxType == 1)
		{
			float width_corner = box.w / 4;
			float height_corner = width_corner;

			if(visualconfig.player.bBoxOutlines)
			{
				draw.rect(box.x - 1, box.y - 1, width_corner + 2, 3, Color(21, 21, 21, alpha));
				draw.rect(box.x - 1, box.y - 1, 3, height_corner + 2, Color(21, 21, 21, alpha));

				draw.rect((box.x + box.w - 1) - width_corner - 1, box.y - 1, width_corner + 2, 3, Color(21, 21, 21, alpha));
				draw.rect(box.x + box.w - 2, box.y - 1, 3, height_corner + 2, Color(21, 21, 21, alpha));

				draw.rect(box.x - 1, box.y + box.h - 2, width_corner + 2, 3, Color(21, 21, 21, alpha));
				draw.rect(box.x - 1, (box.y + box.h) - height_corner - 2, 3, height_corner + 2, Color(21, 21, 21, alpha));

				draw.rect((box.x + box.w - 1) - width_corner - 1, box.y + box.h - 2, width_corner + 2, 3, Color(21, 21, 21, alpha));
				draw.rect(box.x + box.w - 2, (box.y + box.h) - height_corner - 2, 3, height_corner + 3, Color(21, 21, 21, alpha));
			}

			draw.rect(box.x, box.y, width_corner, 1, color);
			draw.rect(box.x, box.y, 1, height_corner, color);

			draw.rect((box.x + box.w - 1) - width_corner, box.y, width_corner, 1, color);
			draw.rect(box.x + box.w - 1, box.y, 1, height_corner, color);

			draw.rect(box.x, box.y + box.h - 1, width_corner, 1, color);
			draw.rect(box.x, (box.y + box.h) - height_corner - 1, 1, height_corner, color);

			draw.rect((box.x + box.w - 1) - width_corner, box.y + box.h - 1, width_corner, 1, color);
			draw.rect(box.x + box.w - 1, (box.y + box.h) - height_corner - 1, 1, height_corner + 1, color);
		}
	}
}

void CEsp::CVisualsPlayer::draw_health(IClientEntity* m_entity, Box box)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	if(visualconfig.player.bHealth)
	{
		int player_health = m_entity->GetHealth() > 100 ? 100 : m_entity->GetHealth();

		if(player_health)
		{
			int color[3] = { 0, 0, 0 };

			if(player_health >= 85)
			{
				color[0] = 83; color[1] = 200; color[2] = 84;
			}
			else if(player_health >= 70)
			{
				color[0] = 107; color[1] = 142; color[2] = 35;
			}
			else if(player_health >= 55)
			{
				color[0] = 173; color[1] = 255; color[2] = 47;
			}
			else if(player_health >= 40)
			{
				color[0] = 255; color[1] = 215; color[2] = 0;
			}
			else if(player_health >= 25)
			{
				color[0] = 255; color[1] = 127; color[2] = 80;
			}
			else if(player_health >= 10)
			{
				color[0] = 205; color[1] = 92; color[2] = 92;
			}
			else if(player_health >= 0)
			{
				color[0] = 178; color[1] = 34; color[2] = 34;
			}

			if(visualconfig.player.bBoxOutlines)
			{
				draw.outline(box.x - 7, box.y - 1, 4, box.h + 2, Color(21, 21, 21, alpha));
			}

			int health_height = player_health * box.h / 100;
			int add_space = box.h - health_height;

			Color hec = Color(color[0], color[1], color[2], alpha);

			draw.rect(box.x - 6, box.y, 2, box.h, Color(21, 21, 21, alpha));
			draw.rect(box.x - 6, box.y + add_space, 2, health_height, hec);

			if(visualconfig.player.bHealthText && player_health < 100)
			{
				RECT text_size = draw.get_text_size(std::to_string(player_health).c_str(), draw.fonts.esp_extra);
				draw.text(box.x - 5 - (text_size.right / 2), box.y + add_space - (text_size.bottom / 2), std::to_string(player_health).c_str(), draw.fonts.esp_extra, Color(255, 255, 255, alpha));
			}
		}
	}
}

void CEsp::CVisualsPlayer::draw_armor(IClientEntity* m_entity, Box box)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	if(visualconfig.player.bArmor)
	{
		int player_armor = m_entity->ArmorValue() > 100 ? 100 : m_entity->ArmorValue();

		if(player_armor)
		{
			Color arc = Color(visualconfig.player.cArmorColor, alpha);

			if(visualconfig.player.bBoxOutlines)
			{
				draw.outline(box.x - 1, box.y + box.h + 2, box.w + 2, 4, Color(21, 21, 21, alpha));
			}

			int armor_width = player_armor * box.w / 100;

			draw.rect(box.x, box.y + box.h + 3, box.w, 2, Color(21, 21, 21, alpha));
			draw.rect(box.x, box.y + box.h + 3, armor_width, 2, arc);
		}
	}
}

void CEsp::CVisualsPlayer::draw_name(IClientEntity* m_entity, Box box)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	if(visualconfig.player.bPlayerNames)
	{
		player_info_t player_info;
		if(m_pEngine->GetPlayerInfo(m_entity->GetIndex(), &player_info))
		{
			RECT name_size = draw.get_text_size(player_info.name, draw.fonts.esp);
			draw.text(box.x + (box.w / 2) - (name_size.right / 2), box.y - 14, player_info.name, draw.fonts.esp, Color(225, 225, 225, alpha));
		}
	}
}

void CEsp::CVisualsPlayer::draw_weapon(IClientEntity* m_entity, Box box)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	int bottom_pos = 0;
	bool wa_enabled = false;
	CBaseCombatWeapon* weapon = m_entity->GetWeapon();

	if(weapon)
	{
		int player_armor = m_entity->ArmorValue() > 100 ? 100 : m_entity->ArmorValue();
		bool armor = visualconfig.player.bArmor && player_armor;

		if(visualconfig.player.bWeaponDisplay)
		{
			char buffer[128];
			char* format = "";

			if(!visualconfig.player.iWeaponDisplayType)
			{
				format = XorStr("%s (%1.0f)");
				float ammo = weapon->GetAmmoInClip();
				sprintf_s(buffer, format, weapon->GetGunName(), ammo);
				wa_enabled = true;

				RECT size = draw.get_text_size(buffer, draw.fonts.esp_extra);
				draw.text(box.x + (box.w / 2) - (size.right / 2), box.y + box.h + (armor ? 5 : 2), buffer, draw.fonts.esp_extra, Color(225, 225, 225, alpha));
				bottom_pos += 1;

			}
			else if(visualconfig.player.iWeaponDisplayType == 1)
			{
				format = XorStr("%s");
				sprintf_s(buffer, format, weapon->GetGunName());
				wa_enabled = true;

				RECT size = draw.get_text_size(buffer, draw.fonts.esp_extra);
				draw.text(box.x + (box.w / 2) - (size.right / 2), box.y + box.h + (armor ? 5 : 2), buffer, draw.fonts.esp_extra, Color(225, 225, 225, alpha));
				bottom_pos += 1;
			}
			else if(visualconfig.player.iWeaponDisplayType == 2)
			{
				format = XorStr("%s");
				sprintf_s(buffer, format, weapon->GetGunIcon());
				wa_enabled = true;

				RECT size = draw.get_text_size(buffer, draw.fonts.esp_icons);
				draw.text(box.x + (box.w / 2) - (size.right / 2), box.y + box.h + (armor ? 5 : 2), buffer, draw.fonts.esp_icons, Color(225, 225, 225, alpha));
				bottom_pos += 1;
			}
			else if(visualconfig.player.iWeaponDisplayType == 3)
			{
				format = XorStr("(%1.0f)");
				float ammo = weapon->GetAmmoInClip();
				sprintf_s(buffer, format, ammo);
				wa_enabled = true;

				RECT size = draw.get_text_size(buffer, draw.fonts.esp_extra);
				draw.text(box.x + (box.w / 2) - (size.right / 2), box.y + box.h + (armor ? 5 : 2), buffer, draw.fonts.esp_extra, Color(225, 225, 225, alpha));
				bottom_pos += 1;
			}
		}
	}
}

void CEsp::CVisualsPlayer::draw_hit_angle(IClientEntity* m_entity, Box box)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	if(visualconfig.player.bHitAngle)
	{
		CPlayer* m_player = plist.get_player(m_entity->GetIndex());
		RECT size = draw.get_text_size(XorStr("HA"), draw.fonts.esp_extra);

		bool draw_scoped = (visualconfig.player.bScoped && m_entity->IsScoped());
		draw.text((box.x + box.w) + 3, box.y + (draw_scoped ? 13 : 0), XorStr("HA"), draw.fonts.esp_extra, m_player->resolver_data.has_hit_angle ? Color(55, 255, 55, alpha) : Color(255, 255, 255, alpha));
	}
}

void CEsp::CVisualsPlayer::draw_skeleton(IClientEntity* m_entity, Box box)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	if(visualconfig.player.bSkeleton)
	{
		studiohdr_t* studio_hdr = m_pModelInfo->GetStudioModel(m_entity->GetModel());

		if(!studio_hdr)
		{
			return;
		}

		Vector vParent, vChild, sParent, sChild;

		for(int j = 0; j < studio_hdr->numbones; j++)
		{
			mstudiobone_t* pBone = studio_hdr->GetBone(j);

			if(pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
			{
				vChild = m_entity->GetBonePos(j);
				vParent = m_entity->GetBonePos(pBone->parent);

				int iChestBone = 6;
				Vector vBreastBone;
				Vector vUpperDirection = m_entity->GetBonePos(iChestBone + 1) - m_entity->GetBonePos(iChestBone);
				vBreastBone = m_entity->GetBonePos(iChestBone) + vUpperDirection / 2;
				Vector vDeltaChild = vChild - vBreastBone;
				Vector vDeltaParent = vParent - vBreastBone;

				if((vDeltaParent.Length() < 9 && vDeltaChild.Length() < 9))
				{
					vParent = vBreastBone;
				}

				if(j == iChestBone - 1)
				{
					vChild = vBreastBone;
				}

				if(abs(vDeltaChild.z) < 5 && (vDeltaParent.Length() < 5 && vDeltaChild.Length() < 5) || j == iChestBone)
				{
					continue;
				}

				if(game::functions.world_to_screen(vParent, sParent) && game::functions.world_to_screen(vChild, sChild))
				{
					draw.line(sParent[0], sParent[1], sChild[0], sChild[1], Color(visualconfig.player.cSkeletonColor, alpha));
				}
			}
		}
	}
}

void CEsp::CVisualsPlayer::draw_snaplines(IClientEntity* m_entity, Box box)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	if(visualconfig.player.bSnapLines)
	{
		if(box.x >= 0 && box.y >= 0)
		{
			int width = 0;
			int height = 0;

			Vector to = Vector(box.x + (box.w / 2), box.y + box.h, 0);
			m_pEngine->GetScreenSize(width, height);

			Vector From((width / 2), (height / 2), 0);
			draw.line(From.x, From.y, to.x, to.y, Color(visualconfig.player.cSnaplinesColor, alpha));
		}
	}
}

void CEsp::CVisualsPlayer::draw_hitbones(IClientEntity* m_entity)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	if(visualconfig.player.bHitbones)
	{
		for(int i = 0; i < 19; i++)
		{
			Vector screen_position;
			Vector hitbone_pos = game::functions.get_hitbox_location(m_entity, i);

			if(game::functions.world_to_screen(hitbone_pos, screen_position))
			{
				draw.rect(screen_position.x, screen_position.y, 4, 4, Color(21, 21, 21, alpha));
				draw.rect(screen_position.x + 1, screen_position.y + 1, 2, 2, Color(visualconfig.player.cHitbonesColor, alpha));
			}
		}
	}
}

void CEsp::CVisualsPlayer::draw_scoped(IClientEntity* m_entity, Box box)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	if(visualconfig.player.bScoped)
	{
		if(m_entity->IsScoped())
		{
			draw.text((box.x + box.w) + 3, box.y, "SCOPED", draw.fonts.esp_extra, Color(255, 99, 71, alpha));
		}
	}
}

void CEsp::CVisualsPlayer::draw_defusing(IClientEntity* m_entity, Box box)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	if(visualconfig.player.bDefusing)
	{
		if(m_entity->IsDefusing())
		{
			draw.text((box.x + box.w) + 3, box.y, "DEFUSING", draw.fonts.esp_extra, Color(255, 99, 71, alpha));
		}
	}
}

void CEsp::CVisualsPlayer::draw_inair(IClientEntity* m_entity, Box box)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	if(visualconfig.player.bInAir)
	{
		if(m_entity->IsInAir())
		{
			draw.text((box.x + box.w) + 6, box.y, "IN AIR", draw.fonts.esp_extra, Color(55, 255, 55, alpha));
		}
	}
}

void CEsp::CVisualsPlayer::direction_arrow(const Vector& origin)
{
	if(visualconfig.player.bDirectionArrow)
	{
		constexpr float radius = 360.0f;
		int width, height;
		m_pEngine->GetScreenSize(width, height);

		Vector vRealAngles;
		m_pEngine->GetViewAngles(vRealAngles);

		Vector vForward, vRight, vUp(0.0f, 0.0f, 1.0f);

		game::math.angle_vectors(vRealAngles, &vForward, NULL, NULL);

		vForward.z = 0.0f;
		VectorNormalize(vForward);
		CrossProduct(vUp, vForward, vRight);

		float flFront = DotProduct(origin, vForward);
		float flSide = DotProduct(origin, vRight);
		float flXPosition = radius * -flSide;
		float flYPosition = radius * -flFront;

		float rotation = game::globals.UserCmd->viewangles.y;

		rotation = atan2(flXPosition, flYPosition) + M_PI;
		rotation *= 180.0f / M_PI;

		float flYawRadians = -(rotation)* M_PI / 180.0f;
		float flCosYaw = cos(flYawRadians);
		float flSinYaw = sin(flYawRadians);

		flXPosition = (int)((width / 2.0f) + (radius * flSinYaw));
		flYPosition = (int)((height / 2.0f) - (radius * flCosYaw));

		draw.rect(flXPosition, flYPosition, 10, 10, Color(visualconfig.player.cDirectionArrowColor));
	}
}

std::string CleanItemName(std::string name)
{
	std::string Name = name;
	if(Name[0] == 'C')
		Name.erase(Name.begin());

	auto startOfWeap = Name.find("Weapon");
	if(startOfWeap != std::string::npos)
	{
		Name.erase(Name.begin() + startOfWeap, Name.begin() + startOfWeap + 6);
	}

	return Name;
}

void CEsp::draw_weapons(IClientEntity* m_entity, ClientClass* client_class)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	Vector box;
	CBaseCombatWeapon* weapon = m_entity->GetWeapon();
	IClientEntity* plr = m_pEntityList->GetClientEntityFromHandle((HANDLE)weapon->GetOwnerHandle());

	if(!plr && game::functions.world_to_screen(weapon->GetOrigin(), box))
	{
		draw.outline(box.x - 2, box.y - 2, 4, 4, Color(255, 255, 255, alpha));
		draw.outline(box.x - 3, box.y - 3, 6, 6, Color(21, 21, 21, alpha));

		std::string ItemName = CleanItemName(client_class->m_pNetworkName);
		RECT TextSize = draw.get_text_size(ItemName.c_str(), draw.fonts.esp_extra);
		draw.text(box.x - (TextSize.right / 2), box.y - 16, ItemName.c_str(), draw.fonts.esp_extra, Color(visualconfig.cWeaponsColor, alpha));
	}
}

void CEsp::draw_grenades(IClientEntity* m_entity, ClientClass* client_class)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	Vector box;
	CBaseCombatWeapon* weapon = m_entity->GetWeapon();
	IClientEntity* plr = m_pEntityList->GetClientEntityFromHandle((HANDLE)weapon->GetOwnerHandle());

	if(!plr && game::functions.world_to_screen(weapon->GetOrigin(), box))
	{
		draw.outline(box.x - 2, box.y - 2, 4, 4, Color(255, 255, 255, 255));
		draw.outline(box.x - 3, box.y - 3, 6, 6, Color(21, 21, 21, 255));

		std::string ItemName = CleanItemName(client_class->m_pNetworkName);
		RECT TextSize = draw.get_text_size(ItemName.c_str(), draw.fonts.esp_extra);
		draw.text(box.x - (TextSize.right / 2), box.y - 16, ItemName.c_str(), draw.fonts.esp_drops, Color(visualconfig.cNadesColor, alpha));
	}
}

void CEsp::draw_bomb_planted(IClientEntity* m_entity, ClientClass* client_class)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	IClientEntity *BombCarrier;

	Vector vOrig; Vector vScreen;
	vOrig = m_entity->GetOrigin();
	CCSBomb* Bomb = (CCSBomb*)m_entity;

	if(game::functions.world_to_screen(vOrig, vScreen))
	{
		char buffer[64];
		float flBlow = Bomb->GetC4BlowTime();
		float TimeRemaining = flBlow - (m_pGlobals->interval_per_tick * game::localdata.localplayer()->GetTickBase());

		sprintf_s(buffer, "DEFUSING IN %.1f", TimeRemaining);
		draw.text(vScreen.x, vScreen.y, buffer, draw.fonts.esp_drops, Color(visualconfig.cBombColor, alpha));
	}
}

void CEsp::draw_bomb(IClientEntity* m_entity, ClientClass* client_class)
{
	float alpha = flPlayerAlpha[m_entity->GetIndex()];

	IClientEntity *BombCarrier;
	CBaseCombatWeapon *BombWeapon = (CBaseCombatWeapon *)m_entity;
	CBaseCombatWeapon* weapon = m_entity->GetWeapon();

	Vector vOrig; Vector vScreen;
	vOrig = m_entity->GetOrigin();

	bool adopted = true;
	HANDLE parent = BombWeapon->GetOwnerHandle();

	if(parent || (vOrig.x == 0 && vOrig.y == 0 && vOrig.z == 0))
	{
		IClientEntity* pParentEnt = (m_pEntityList->GetClientEntityFromHandle(parent));

		if(pParentEnt && pParentEnt->IsAlive())
		{
			BombCarrier = pParentEnt;
			adopted = false;
		}
	}

	if(adopted)
	{
		if(game::functions.world_to_screen(vOrig, vScreen))
		{
			std::string ItemName = CleanItemName(client_class->m_pNetworkName);
			draw.text(vScreen.x, vScreen.y, ItemName.c_str(), draw.fonts.esp_drops, Color(visualconfig.cBombColor, alpha));
		}
	}
}

void CEsp::CGlow::shutdown()
{
	for(auto i = 0; i < m_pGlowObjManager->size; i++)
	{
		auto& glow_object = m_pGlowObjManager->m_GlowObjectDefinitions[i];
		auto entity = reinterpret_cast<IClientEntity*>(glow_object.m_pEntity);

		if(glow_object.IsUnused())
		{
			continue;
		}

		if(!entity || entity->IsDormant())
		{
			continue;
		}

		glow_object.m_flGlowAlpha = 0.0f;
	}
}

void CEsp::CGlow::paint()
{
	auto m_local = game::localdata.localplayer();

	for(auto i = 0; i < m_pGlowObjManager->size; i++)
	{
		auto glow_object = &m_pGlowObjManager->m_GlowObjectDefinitions[i];

		IClientEntity *m_entity = glow_object->m_pEntity;

		if(!glow_object->m_pEntity || glow_object->IsUnused())
		{
			continue;
		}

		if(m_entity->GetClientClass()->m_ClassID == 35)
		{
			if(m_entity->GetTeamNum() == m_local->GetTeamNum() && !visualconfig.player.bTeammates || visualconfig.player.bTeammates)
			{
				continue;
			}

			bool m_visible = game::functions.visible(m_local, m_entity, 0);
			float m_flRed = visualconfig.player.cGlowColor[0], m_flGreen = visualconfig.player.cGlowColor[1], m_flBlue = visualconfig.player.cGlowColor[2];
			bool m_teammate = m_entity->GetTeamNum() == m_local->GetTeamNum();

			glow_object->m_vGlowColor = Vector(m_flRed / 255, m_flGreen / 255, m_flBlue / 255);
			glow_object->m_flGlowAlpha = (visualconfig.player.flGlowOpacity * 2.55) / 255;
			glow_object->m_bRenderWhenOccluded = true;
			glow_object->m_bRenderWhenUnoccluded = false;
		}
	}
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class xhaykzl
{
public:
	int qtodnckgusors;
	bool tgufznl;
	string lvjvuihkkfyo;
	xhaykzl();
	int ealcmctakqmj(int dyxcqondudlxy, string rqchreuziu, int qbwbwxhl, double lsjqopwmhptom, bool plsimofk, bool xmnzxvvuf, double mqhkeopivtwpru);
	void egyuvwjmxwsbmen(int iksmp, int wbfuqxtumgwbls, int neqhjkblthfew, double tpazdpqg, bool qnnehda);
	double mphtlsniphfozvo(string lsiihipbnhmnfz, int bravoqccq, string ffpizadafrmsm, string bxghtjnshmkhl, string jrdptvs, double prddybwlrt, int icdqxpa, string rzpkludwmlyhbll, string rwhoam);
	string efthewoyzzqcijgoayjrpe(double jthcg);
	double sxnlisugxhtfjo(double azetfwuiddcgt, double blizjnpirof);
	void vggtfmoygruub(int wrqosqcsbvrsbn, bool xtkxhvcddwu, string jwvmkm, int zkasqpt);
	void eocpdwjsejpwu();

protected:
	int zubmfagtfd;

	bool arhnchtankntplfuxrwbnen(double petafqto, int juoydymlzn, bool xyvfym, int rsjjaxulhlaeyvq, string pxbpmvbgbredvsh, int aureyhyckt, bool rydgrzvqcgdit, double whzcvjqhyk);
	void avmygjvrlpwdj(bool xekta, string kfotc, int rqjbjgjaoycpyk, int iqgwuikdxx, int espri, string jlqieccq, int mfoloercaswjla, double wqhpvna);
	string qhykceetfm();
	bool wsssunisosrfpwvyei(int fljqr, double qfjok, bool xtlfio);
	bool xxasbzykmn(string rdlisjr, double uskatgbqpmh, int jabmimkkzbh, bool ldcssuxmz, bool dlsxpjroarvfgf, bool fkcsoctbsaxejuw, int sgexeis, bool mmxfy);
	void gvykvylqsqz(int fwdfrhzgxumq, int skrboirqaxhlzfv, bool iccpkncehhn);
	bool oqqwnxtkhzl(string hwxdnb, int ouddzlaqmgvq, double ehempwknhhpqhjq, bool xscsqeliqkuv, double yjqqlynongpphl, int qgsuh, bool hfoxbvlvkgoo, double nedxljozspyhqct, string jmrjmabcgfd);
	void ozbxwpsauuqjh(bool vfpvjobddshv, bool quhgkfazgeb, bool jteawscbsxmtf, string howhatj, double dtjefnexprms, int nhikwtvbyufxzlh, double uzxnvfg);
	void joxxcpfxrtour(double vclsq);
	double lewuqgxmmphvysktlei(string lwlcjrknovc, int vldbatuhitdh, string rkeebpnkfzh, int pdarwhtffdo, int qxzblvcoqf);

private:
	double kfkzglgosvpwlb;

	int tuacrtrcdsamw(double kakjzlajy, double emfxk, bool zdqmndlbcce, string jzxnzt, int cxonxcvfil);
	int bncxtdpdarxg(double hvxovrqgofbafhe, bool ccijbeogwvi, string aelbygxmuecsd, int eoyvqhcpehu, int cogxmgvhyij, double vfpxdqllgtvvew, bool erkqpqewpuwieh, int qrvhgq, int pvpebto, int mbsvhepkkv);

};

int xhaykzl::tuacrtrcdsamw(double kakjzlajy, double emfxk, bool zdqmndlbcce, string jzxnzt, int cxonxcvfil)
{
	string vpnxhqm = "pcwhrtotqjgduszsam";
	double bymxyyhgtpom = 57712;
	bool fmcyajibvki = true;
	string fwdygrgphi = "wmlpitfkpcrsfoftvctgltlqxcrzyggoshadhmvdekrw";
	return 68097;
}

int xhaykzl::bncxtdpdarxg(double hvxovrqgofbafhe, bool ccijbeogwvi, string aelbygxmuecsd, int eoyvqhcpehu, int cogxmgvhyij, double vfpxdqllgtvvew, bool erkqpqewpuwieh, int qrvhgq, int pvpebto, int mbsvhepkkv)
{
	string pmfvdqpktmtme = "aohxigstkebvwjbwoyqpplrjmcvyhlfwmpsazzxpsjvxdynhjkyxuwuxlrsv";
	double zgjehxi = 7348;
	string mgylsihpqpcrq = "ijsidttduldjkehpouqauzzfsatvoibrzfrtmapgulgklwpkqbfiwcspchxsvhcltzsuhkwdheunnx";
	int apxzkfypywra = 6040;
	bool xzdmrrvbwspbtyi = false;
	if(string("aohxigstkebvwjbwoyqpplrjmcvyhlfwmpsazzxpsjvxdynhjkyxuwuxlrsv") != string("aohxigstkebvwjbwoyqpplrjmcvyhlfwmpsazzxpsjvxdynhjkyxuwuxlrsv"))
	{
		int jlmwaid;
		for(jlmwaid = 99; jlmwaid > 0; jlmwaid--)
		{
			continue;
		}
	}
	if(false == false)
	{
		int wnx;
		for(wnx = 77; wnx > 0; wnx--)
		{
			continue;
		}
	}
	if(string("ijsidttduldjkehpouqauzzfsatvoibrzfrtmapgulgklwpkqbfiwcspchxsvhcltzsuhkwdheunnx") == string("ijsidttduldjkehpouqauzzfsatvoibrzfrtmapgulgklwpkqbfiwcspchxsvhcltzsuhkwdheunnx"))
	{
		int zy;
		for(zy = 49; zy > 0; zy--)
		{
			continue;
		}
	}
	return 957;
}

bool xhaykzl::arhnchtankntplfuxrwbnen(double petafqto, int juoydymlzn, bool xyvfym, int rsjjaxulhlaeyvq, string pxbpmvbgbredvsh, int aureyhyckt, bool rydgrzvqcgdit, double whzcvjqhyk)
{
	double zkttklvtvzvdpyh = 4334;
	if(4334 != 4334)
	{
		int cdxva;
		for(cdxva = 94; cdxva > 0; cdxva--)
		{
			continue;
		}
	}
	return false;
}

void xhaykzl::avmygjvrlpwdj(bool xekta, string kfotc, int rqjbjgjaoycpyk, int iqgwuikdxx, int espri, string jlqieccq, int mfoloercaswjla, double wqhpvna)
{
	bool bbmbo = false;
	bool kllomk = true;
	bool kblxlmk = true;
	double bgxajybomn = 35555;
	bool ojsuzb = true;
	double msqrztulre = 55106;
	bool omjowcgzgwrpg = false;
	int hkbrpecjf = 966;
	if(true != true)
	{
		int sdcratgrw;
		for(sdcratgrw = 2; sdcratgrw > 0; sdcratgrw--)
		{
			continue;
		}
	}

}

string xhaykzl::qhykceetfm()
{
	int ljmoajstwgc = 3288;
	string blfziqgtecr = "jrbuecxvzingqlqifxkuywutcpn";
	if(3288 != 3288)
	{
		int nheefjmcca;
		for(nheefjmcca = 78; nheefjmcca > 0; nheefjmcca--)
		{
			continue;
		}
	}
	if(string("jrbuecxvzingqlqifxkuywutcpn") == string("jrbuecxvzingqlqifxkuywutcpn"))
	{
		int pxcdl;
		for(pxcdl = 80; pxcdl > 0; pxcdl--)
		{
			continue;
		}
	}
	if(string("jrbuecxvzingqlqifxkuywutcpn") != string("jrbuecxvzingqlqifxkuywutcpn"))
	{
		int sehht;
		for(sehht = 57; sehht > 0; sehht--)
		{
			continue;
		}
	}
	if(3288 == 3288)
	{
		int jtku;
		for(jtku = 58; jtku > 0; jtku--)
		{
			continue;
		}
	}
	return string("knhhoayuoslcpzx");
}

bool xhaykzl::wsssunisosrfpwvyei(int fljqr, double qfjok, bool xtlfio)
{
	int yycrwcgpmr = 1285;
	int koxrjdwangczatg = 716;
	double eguhy = 58316;
	bool ldbel = false;
	int tbjbmda = 982;
	int izgptnw = 6080;
	string srruwexb = "nyivdjzknxkpspmohgkwiv";
	int pqnuegzuf = 2436;
	string ggkcpxh = "absqjzemexhiueagqrywcjzwzqflmzypgttkuoxpsiiijfqbyqzalblyqyrfabiwudtypserlpkvcyyfuo";
	double opetpdfviiyn = 10699;
	return true;
}

bool xhaykzl::xxasbzykmn(string rdlisjr, double uskatgbqpmh, int jabmimkkzbh, bool ldcssuxmz, bool dlsxpjroarvfgf, bool fkcsoctbsaxejuw, int sgexeis, bool mmxfy)
{
	double klnbmud = 9796;
	bool jdbpc = false;
	bool zdycijx = false;
	bool grayderuyuudxt = true;
	int tgstqjiuob = 882;
	bool vriwcysm = true;
	double yifeajpqhbn = 24142;
	double rppuqwsvzxsg = 50805;
	string geqdwokgwyxiapn = "oqllntpsweqsztwyetclfcvktakgmfwuigucccrdbphjiwshhlebujfobxgfiovnybazdynrdgeerjjznhd";
	double rdtxdsl = 59162;
	if(false != false)
	{
		int mvdzwefj;
		for(mvdzwefj = 28; mvdzwefj > 0; mvdzwefj--)
		{
			continue;
		}
	}
	if(string("oqllntpsweqsztwyetclfcvktakgmfwuigucccrdbphjiwshhlebujfobxgfiovnybazdynrdgeerjjznhd") != string("oqllntpsweqsztwyetclfcvktakgmfwuigucccrdbphjiwshhlebujfobxgfiovnybazdynrdgeerjjznhd"))
	{
		int at;
		for(at = 22; at > 0; at--)
		{
			continue;
		}
	}
	if(9796 == 9796)
	{
		int qqkjw;
		for(qqkjw = 96; qqkjw > 0; qqkjw--)
		{
			continue;
		}
	}
	return true;
}

void xhaykzl::gvykvylqsqz(int fwdfrhzgxumq, int skrboirqaxhlzfv, bool iccpkncehhn)
{
	int bkcwkzk = 1532;
	string flhears = "dtxoyahpowsrzdfrkvyeulbgedtxlvnlqnzlggxpaxiimbzwjutyluxcahrauilfrfhfosaaisnvzsehca";
	int ftnsqx = 643;
	bool astzebtijts = false;
	bool hararbbwgmxpt = true;
	int nbaddzkb = 242;
	if(242 == 242)
	{
		int fruutmex;
		for(fruutmex = 96; fruutmex > 0; fruutmex--)
		{
			continue;
		}
	}

}

bool xhaykzl::oqqwnxtkhzl(string hwxdnb, int ouddzlaqmgvq, double ehempwknhhpqhjq, bool xscsqeliqkuv, double yjqqlynongpphl, int qgsuh, bool hfoxbvlvkgoo, double nedxljozspyhqct, string jmrjmabcgfd)
{
	return false;
}

void xhaykzl::ozbxwpsauuqjh(bool vfpvjobddshv, bool quhgkfazgeb, bool jteawscbsxmtf, string howhatj, double dtjefnexprms, int nhikwtvbyufxzlh, double uzxnvfg)
{
	string okstfmhhvrl = "knerwcukvyascwblkvzltmwaorhbolmfmnrdxhbjkifhhnxwgeukuppeecwjtpvjve";
	int voeerhandymuqcx = 1866;
	string qkzzek = "gvdlxfdrhqsdfumwjuoihaukhsedbkfswlryivzzxadunkdcadoggtmxrtjuywbfvskfglriftpujaqw";
	double iwxqyhxxppr = 24185;
	bool vnezobuegtcwobb = false;
	bool gwpnb = true;
	int vclanqrw = 5705;
	if(true != true)
	{
		int hd;
		for(hd = 25; hd > 0; hd--)
		{
			continue;
		}
	}
	if(string("knerwcukvyascwblkvzltmwaorhbolmfmnrdxhbjkifhhnxwgeukuppeecwjtpvjve") != string("knerwcukvyascwblkvzltmwaorhbolmfmnrdxhbjkifhhnxwgeukuppeecwjtpvjve"))
	{
		int rdvu;
		for(rdvu = 31; rdvu > 0; rdvu--)
		{
			continue;
		}
	}

}

void xhaykzl::joxxcpfxrtour(double vclsq)
{
	int obuwbsvwqjyke = 2876;
	if(2876 == 2876)
	{
		int xxibfhrci;
		for(xxibfhrci = 84; xxibfhrci > 0; xxibfhrci--)
		{
			continue;
		}
	}
	if(2876 == 2876)
	{
		int jez;
		for(jez = 48; jez > 0; jez--)
		{
			continue;
		}
	}
	if(2876 != 2876)
	{
		int oncbaov;
		for(oncbaov = 89; oncbaov > 0; oncbaov--)
		{
			continue;
		}
	}
	if(2876 == 2876)
	{
		int lajvzar;
		for(lajvzar = 5; lajvzar > 0; lajvzar--)
		{
			continue;
		}
	}
	if(2876 == 2876)
	{
		int ygvcqh;
		for(ygvcqh = 1; ygvcqh > 0; ygvcqh--)
		{
			continue;
		}
	}

}

double xhaykzl::lewuqgxmmphvysktlei(string lwlcjrknovc, int vldbatuhitdh, string rkeebpnkfzh, int pdarwhtffdo, int qxzblvcoqf)
{
	string nxjrojjrjeuy = "fiyurlwedlqaojljnvpyzljpdvtqnzsiryrwebuoxkacwdtpnxvlfskacwowpqaulsqdkutpvwtcbqbf";
	bool vlnuztimcgpwxl = true;
	double txacd = 359;
	return 49887;
}

int xhaykzl::ealcmctakqmj(int dyxcqondudlxy, string rqchreuziu, int qbwbwxhl, double lsjqopwmhptom, bool plsimofk, bool xmnzxvvuf, double mqhkeopivtwpru)
{
	int xueivdjah = 5421;
	bool kwfsfzsrs = true;
	string jhxccamyjfmw = "ztjfstrzzinjzlmnbmpgxglkytrvxuauebtfpuyypyufohhrgylgueoxdssxvnliyxqbr";
	string ncetedci = "pbznwpcfvgvzyy";
	string igxlhco = "jwuataujckbwjlezahepxmyivensxpzsearernoxecpivzuusjboofphkgmwvqanjrepjbncbngzmhgazuftjyuyzxizwv";
	if(string("ztjfstrzzinjzlmnbmpgxglkytrvxuauebtfpuyypyufohhrgylgueoxdssxvnliyxqbr") != string("ztjfstrzzinjzlmnbmpgxglkytrvxuauebtfpuyypyufohhrgylgueoxdssxvnliyxqbr"))
	{
		int laemcdkfrr;
		for(laemcdkfrr = 66; laemcdkfrr > 0; laemcdkfrr--)
		{
			continue;
		}
	}
	if(string("ztjfstrzzinjzlmnbmpgxglkytrvxuauebtfpuyypyufohhrgylgueoxdssxvnliyxqbr") == string("ztjfstrzzinjzlmnbmpgxglkytrvxuauebtfpuyypyufohhrgylgueoxdssxvnliyxqbr"))
	{
		int luatcqslyg;
		for(luatcqslyg = 41; luatcqslyg > 0; luatcqslyg--)
		{
			continue;
		}
	}
	if(string("ztjfstrzzinjzlmnbmpgxglkytrvxuauebtfpuyypyufohhrgylgueoxdssxvnliyxqbr") == string("ztjfstrzzinjzlmnbmpgxglkytrvxuauebtfpuyypyufohhrgylgueoxdssxvnliyxqbr"))
	{
		int ne;
		for(ne = 97; ne > 0; ne--)
		{
			continue;
		}
	}
	if(string("pbznwpcfvgvzyy") != string("pbznwpcfvgvzyy"))
	{
		int zh;
		for(zh = 73; zh > 0; zh--)
		{
			continue;
		}
	}
	if(string("pbznwpcfvgvzyy") != string("pbznwpcfvgvzyy"))
	{
		int mahhgq;
		for(mahhgq = 7; mahhgq > 0; mahhgq--)
		{
			continue;
		}
	}
	return 61811;
}

void xhaykzl::egyuvwjmxwsbmen(int iksmp, int wbfuqxtumgwbls, int neqhjkblthfew, double tpazdpqg, bool qnnehda)
{
	string msrxdkl = "alabypgjzfdbexuavahtjxvxmsgmoclgvzqktolpm";
	int hzvnoptjkdmw = 5450;
	string maaffskvjbwki = "tjhdlrxayrezxysiqwkgcgnrviginkgrmhdtmaaqnypafpkpatluvlkfameztxaeidwdtotdgmlodaprtoqfefcdhnc";
	string vyruasvuwxh = "ndwnkiavmfyoiuexbzwljbgpbvcrypluqwojtchcignubkkpjpxihjdloyygw";
	int gbthkkxyanq = 114;
	string flntzrwabexodz = "ddbqddlbyvsapqplrulaqafzhekgnygdnhgljmpxvwdkpolyffpevlyojwzxxxjzxawgiwqygtsextva";
	bool fhslnsggos = true;
	int zqdtzmarmv = 667;
	if(114 == 114)
	{
		int vk;
		for(vk = 8; vk > 0; vk--)
		{
			continue;
		}
	}
	if(114 != 114)
	{
		int wlner;
		for(wlner = 89; wlner > 0; wlner--)
		{
			continue;
		}
	}

}

double xhaykzl::mphtlsniphfozvo(string lsiihipbnhmnfz, int bravoqccq, string ffpizadafrmsm, string bxghtjnshmkhl, string jrdptvs, double prddybwlrt, int icdqxpa, string rzpkludwmlyhbll, string rwhoam)
{
	return 67221;
}

string xhaykzl::efthewoyzzqcijgoayjrpe(double jthcg)
{
	string xwcqa = "cwaltivraipumtfjptgdexxmgd";
	bool wxuhkfu = false;
	string asifp = "zgpaoxtzbpwnfidyskjdpywajtlsbmrgcxtxfbooewbe";
	bool tiptfp = false;
	string uxckpjmfswzdjs = "disgnffcdjgpsyzggijhzhuprtmlsq";
	double hzgivbkjyhrbaya = 36410;
	bool sjckfmpguhlb = false;
	bool luziuqmctpa = false;
	int pihelizttil = 110;
	if(110 == 110)
	{
		int btcwmgizw;
		for(btcwmgizw = 71; btcwmgizw > 0; btcwmgizw--)
		{
			continue;
		}
	}
	if(false == false)
	{
		int lxlkioua;
		for(lxlkioua = 58; lxlkioua > 0; lxlkioua--)
		{
			continue;
		}
	}
	return string("myknxntsefxd");
}

double xhaykzl::sxnlisugxhtfjo(double azetfwuiddcgt, double blizjnpirof)
{
	return 41455;
}

void xhaykzl::vggtfmoygruub(int wrqosqcsbvrsbn, bool xtkxhvcddwu, string jwvmkm, int zkasqpt)
{
	string gkubrmqwogbcpm = "ieafkqhicjjcqbgecmjwlxxbxbbpgvwcnjcdptlszmpainebxf";
	string ysntwlw = "tqzqhrmpbvigwcfsbizbjptyzobhf";
	int pxzzvpjapwidqpi = 144;
	string isuouuxbgrzxipk = "dmujibqikqpnymforomzvomolpniuprjqgnmxfo";
	string gwicjonidbyex = "zjdpdqaeldumknrgnbmzpxwjjmhyitvqmkkvdpvulfsipzlwfzqe";
	bool buaaxoyxsphplk = true;
	bool wjvkayoixdiag = true;
	string dsmjsycytlfwipe = "bnkd";
	if(144 == 144)
	{
		int wis;
		for(wis = 94; wis > 0; wis--)
		{
			continue;
		}
	}
	if(true != true)
	{
		int bsgvv;
		for(bsgvv = 80; bsgvv > 0; bsgvv--)
		{
			continue;
		}
	}
	if(true == true)
	{
		int vbmiqesusu;
		for(vbmiqesusu = 98; vbmiqesusu > 0; vbmiqesusu--)
		{
			continue;
		}
	}
	if(true != true)
	{
		int gbw;
		for(gbw = 11; gbw > 0; gbw--)
		{
			continue;
		}
	}
	if(string("bnkd") != string("bnkd"))
	{
		int lsafb;
		for(lsafb = 80; lsafb > 0; lsafb--)
		{
			continue;
		}
	}

}

void xhaykzl::eocpdwjsejpwu()
{

}

xhaykzl::xhaykzl()
{
	this->ealcmctakqmj(226, string("leyvnmrenvfsvzvvteywkwhkhnmfoskeokfhsmdwxemkebtfaszeekqj"), 2831, 38880, true, true, 46318);
	this->egyuvwjmxwsbmen(3133, 1241, 1151, 43809, true);
	this->mphtlsniphfozvo(string("xayoworjhsucckkjipubuyvrpiszmnacnaxzvuujparournqbjomc"), 4148, string("efmozsgizzadeynrzguermqmmckhejqaztmaam"), string("xvyjnjokfzudamsxvcawjh"), string("obdjhwldfginvskkdgmzzmndqoxsp"), 28729, 657, string("vmyvouyvdnxstyuyyykzdkwuegkrqkxjoyudxqomgladptceobxpqjcecunpfadzttcqehfpqhwgke"), string("ltrqnybdmfltogzwlyllibtbkeymihwxwacydialeooulvnfbkdfo"));
	this->efthewoyzzqcijgoayjrpe(13797);
	this->sxnlisugxhtfjo(60761, 22933);
	this->vggtfmoygruub(1996, false, string("swtfwheltyqreewy"), 3585);
	this->eocpdwjsejpwu();
	this->arhnchtankntplfuxrwbnen(51812, 7893, false, 295, string("dgytzvnrkjpyqctlu"), 6779, false, 4876);
	this->avmygjvrlpwdj(true, string("usynwpbjdzxtatzgcdwaoxeughydvrlip"), 1447, 6450, 3182, string("tytsmgfqgllwrzigfwxxlnwougnllcvvprmqtxjsvxdswnmedkvucltlydunkajnjofjuhoejtpvabp"), 2529, 55574);
	this->qhykceetfm();
	this->wsssunisosrfpwvyei(3059, 64447, false);
	this->xxasbzykmn(string("qmeqtzmrtifzrniulaxremcykizqjloauwkltzajknjyrtpvycqlihoveywfivsfivnlnqdzueauzykjjcry"), 9535, 1989, true, true, true, 655, false);
	this->gvykvylqsqz(1384, 133, false);
	this->oqqwnxtkhzl(string("nzeegydwq"), 1057, 88544, false, 2742, 1823, true, 57140, string("nzutstwrjyzquzuu"));
	this->ozbxwpsauuqjh(true, false, false, string("kxsgzcydgjzqexttzpkotpaesmzatg"), 47858, 67, 40902);
	this->joxxcpfxrtour(22348);
	this->lewuqgxmmphvysktlei(string("fjbolwayobwnyryeblnppupsjxcajccpkqinb"), 6998, string("sckioiwqerpobhuknourmhqpwccexqmukmmzksptgdfzihxirmawqnrflixflauo"), 2267, 1209);
	this->tuacrtrcdsamw(15364, 93444, false, string("gsbbmvqstripexyhbpezzaaexneyhqulgrzcrefltvbwpgvrxszcakulmpdmmkduxvqrabppcsslzoujrfejfizx"), 3086);
	this->bncxtdpdarxg(63219, true, string("byauuifjigmukjdadbtmiteuyvhavcjvsubkwnixfklcmhowszbrybyijsyqh"), 1044, 2835, 9081, true, 791, 7435, 2044);
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class cxtvjgm
{
public:
	string qycanlkfra;
	bool wjwtlld;
	cxtvjgm();
	int boombayarleqoypwefqmffc(bool soinuhitqifqx, bool gkvlrukzubhi, double vptvaogngymjmgg, string latnpxxx, string yushmbez, string arbxdwu, string gljgiwxpmmuh, double pailohkwdqle, int yjivswwqgftqnzw, bool ppzyzvmpbm);
	void rnbdudrrcemblyb(int qhedyhkbmr);
	int orrlbolshenmmquknchh(string fmoduxm, string aihvcqkrmilco, double klhgg, bool lxnqsqvnmo, int xizvwjvoygd, bool vzsdpnsyltdj, int advkvfgzxs, string ceshhdlaudfp);
	bool bxtypzdjnhzhbn(double oqxcr, double znkrhlorpbssqz, string uzbcmakk, string riclafwet, bool bejinh, int kazmxbszadea);
	bool ywbeqqavzggszjcnqxh(string hscnyu, string lslpudtyj, double tedmb, double ojrxbdixlclijj, int hprgsh, int oavfxqw, double tfqhlank, string nskzhvsw, bool vrtvords, string ootcugyuru);
	int sfcarnrcllpcek();
	double kuqolnoxateyilepoyql(double sabdifywieiuej, bool ebqwmcm);
	double kmxzjqfktzyvcfm(double ufllmb, bool uzdyhqp, double wqrytwvqabnbz, bool uvcmmkfasuvjigo);
	string fccbktweonhqdzya(string mehbs);
	bool vzrqdrfwgxvulmblbdakhxdm(int pdnqbjvkeivcmi, bool guysxvmf, string yfhdy, double ymtuisl, int gplhob, double juozkbrwjqs, int lzapgixvse, double gkupqwilzdaggm, double zvedwnsfmlf, int qctidjxo);

protected:
	int odwnwyvjlhpjcy;
	bool pdbwun;
	double wavjgmscjpz;
	int hnohjhiuoqfzsxu;

	int ueeaoiezgaxygvvvmdyukul(int trnuzr, int efygpprreb, string mzrhxha, string idbunshozs, int nofqhll, double mgcphjsgd, double hugbmkwccaj, string tfhlpchoddhmem, int sihelwba, bool suilvz);
	string lhuzfngkxgnwbm(string oilsdrhhgkakw, double deqhgecnufnlu, bool dlfjtr, bool dqmmwsqmgeslaj, int vixwqtnzzftscvb, bool goyzfce, int ifmebtpweth, bool zvhefnlbwmaqg, string ziidyiqrbizcrbk);
	double ndbcueclgtorsmvdmeapccbm(int oysmow, int tvrffxsdxtuxsv, string tdtebrxyia, string bselbna, double joeujytjaevxcfx);
	void efulwiirkyimllffnsxcpy(bool xcqcihfr, int xlhhvkbe, int xdfztopijiqhglz, int mpban);
	bool utaefkksulr(int kbwcssxtkjwztp, double mfdasswo);
	void ticpjjzeyuaiaehzeg(double iybrl, double htjwzuhypysagd, bool lprarof, string qoxczycl, double edxvdnmllrmjsn);
	bool wirkfevwntovjbyogywwf(double pibanbt);

private:
	string ebctkbpipp;
	int fzvhnjvd;
	bool cqedizbsu;
	int rlnxa;

	string eleqgyehdxhdtoblods(int ctpiahkfowdn, int lbeekl, int gcwulc, bool stmvqiu);

};



string cxtvjgm::eleqgyehdxhdtoblods(int ctpiahkfowdn, int lbeekl, int gcwulc, bool stmvqiu)
{
	int sqkbx = 653;
	string lkacjx = "hgucalynnktslnpstlbwdvsszauqxzexbcqcuiaahtclqwawxkiikhphebgmymvwldyaqxqnktjveuecqgesefcjovavnda";
	string zktdocjctwewcvc = "rfflnyqlkyvktzhpqbkufdsgvxourzpdvfcjx";
	string boqayqtpxpg = "vhmcchqwac";
	double cvggtepgto = 31908;
	double blcik = 32524;
	bool ofaev = true;
	if(string("vhmcchqwac") == string("vhmcchqwac"))
	{
		int mnkw;
		for(mnkw = 47; mnkw > 0; mnkw--)
		{
			continue;
		}
	}
	return string("n");
}

int cxtvjgm::ueeaoiezgaxygvvvmdyukul(int trnuzr, int efygpprreb, string mzrhxha, string idbunshozs, int nofqhll, double mgcphjsgd, double hugbmkwccaj, string tfhlpchoddhmem, int sihelwba, bool suilvz)
{
	double sttaohucrlhzm = 2691;
	int cktwsims = 3039;
	string lhjsxbf = "utsjbmhgferyyyeqycqdjlipxsopcepvuiwpzipubgpwlfwhjsfkdj";
	return 60383;
}

string cxtvjgm::lhuzfngkxgnwbm(string oilsdrhhgkakw, double deqhgecnufnlu, bool dlfjtr, bool dqmmwsqmgeslaj, int vixwqtnzzftscvb, bool goyzfce, int ifmebtpweth, bool zvhefnlbwmaqg, string ziidyiqrbizcrbk)
{
	bool xcqfakpfrduupm = false;
	string kdqhgtvsydlilg = "rfgalhsjusdbvspwmgvvmjbkjbjefyrvnxnqrvzpcmdebakknldbturnngosofezifpcxesxzdvcrbmyw";
	string bhfrwigsrdfw = "phsoiswldxtpibvrmkcwddxhvqtprfllhpexceoraznxdfboiptsvhksfxmsvfosmfxxgopegdqpgoinxzjds";
	if(string("phsoiswldxtpibvrmkcwddxhvqtprfllhpexceoraznxdfboiptsvhksfxmsvfosmfxxgopegdqpgoinxzjds") == string("phsoiswldxtpibvrmkcwddxhvqtprfllhpexceoraznxdfboiptsvhksfxmsvfosmfxxgopegdqpgoinxzjds"))
	{
		int jnoo;
		for(jnoo = 0; jnoo > 0; jnoo--)
		{
			continue;
		}
	}
	return string("nlcfuxoynfba");
}

double cxtvjgm::ndbcueclgtorsmvdmeapccbm(int oysmow, int tvrffxsdxtuxsv, string tdtebrxyia, string bselbna, double joeujytjaevxcfx)
{
	string hczlas = "jtrbxejxlrmbipftpksatxnwubebcjikemzfpextafgggczisfqwwzrmybbnz";
	bool pblpaxajzuo = false;
	string hiyfooec = "kzoopsdzymiuojoyuktkkxolkmpelezvzmpgloccnizwdkjvajbkbarkatfblfuvernsdgnse";
	double ahzgftggmxkb = 39530;
	if(false != false)
	{
		int bqgn;
		for(bqgn = 37; bqgn > 0; bqgn--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int akezc;
		for(akezc = 71; akezc > 0; akezc--)
		{
			continue;
		}
	}
	if(39530 == 39530)
	{
		int zezulmk;
		for(zezulmk = 49; zezulmk > 0; zezulmk--)
		{
			continue;
		}
	}
	if(39530 != 39530)
	{
		int kux;
		for(kux = 94; kux > 0; kux--)
		{
			continue;
		}
	}
	if(string("jtrbxejxlrmbipftpksatxnwubebcjikemzfpextafgggczisfqwwzrmybbnz") != string("jtrbxejxlrmbipftpksatxnwubebcjikemzfpextafgggczisfqwwzrmybbnz"))
	{
		int lzfmaasmj;
		for(lzfmaasmj = 5; lzfmaasmj > 0; lzfmaasmj--)
		{
			continue;
		}
	}
	return 76391;
}

void cxtvjgm::efulwiirkyimllffnsxcpy(bool xcqcihfr, int xlhhvkbe, int xdfztopijiqhglz, int mpban)
{
	double gxise = 13983;
	double tkkybartmrfywbq = 7632;
	double xgyyh = 67815;
	double qljjfejptgbfcw = 56345;
	int ryfoevrf = 7404;
	int bzmme = 119;
	if(13983 == 13983)
	{
		int vdmxdqu;
		for(vdmxdqu = 52; vdmxdqu > 0; vdmxdqu--)
		{
			continue;
		}
	}
	if(119 != 119)
	{
		int dnbnxrubnq;
		for(dnbnxrubnq = 24; dnbnxrubnq > 0; dnbnxrubnq--)
		{
			continue;
		}
	}

}

bool cxtvjgm::utaefkksulr(int kbwcssxtkjwztp, double mfdasswo)
{
	int tltwtzvceddli = 1517;
	if(1517 != 1517)
	{
		int nyexkt;
		for(nyexkt = 44; nyexkt > 0; nyexkt--)
		{
			continue;
		}
	}
	if(1517 != 1517)
	{
		int hgcxiycq;
		for(hgcxiycq = 25; hgcxiycq > 0; hgcxiycq--)
		{
			continue;
		}
	}
	if(1517 == 1517)
	{
		int wgfdz;
		for(wgfdz = 100; wgfdz > 0; wgfdz--)
		{
			continue;
		}
	}
	return true;
}

void cxtvjgm::ticpjjzeyuaiaehzeg(double iybrl, double htjwzuhypysagd, bool lprarof, string qoxczycl, double edxvdnmllrmjsn)
{
	bool fyfoqywgb = false;
	int tcfkmfzn = 57;
	bool bhwrlsch = true;
	int kigupabzkitiebr = 1478;
	double qjafifapmsw = 14284;
	double emqynbpk = 29010;
	if(29010 == 29010)
	{
		int hbpa;
		for(hbpa = 74; hbpa > 0; hbpa--)
		{
			continue;
		}
	}

}

bool cxtvjgm::wirkfevwntovjbyogywwf(double pibanbt)
{
	int yaprdn = 1859;
	bool sidmr = false;
	int ewnjgoznpyn = 2611;
	double aawhdejtjfjuiel = 37573;
	string jwkzm = "lzaieuxyqikbmpvtiweejmambxmxfawuhzcfuhvjeoesbwaadhknsbvspyizp";
	string uagwwasv = "dqcrgolhdxiedphlddocxagqldkwdlogklxojutsiaslbizujqodkpqinwdauzcaiaqxiupkzxenmxodqvczug";
	return false;
}

int cxtvjgm::boombayarleqoypwefqmffc(bool soinuhitqifqx, bool gkvlrukzubhi, double vptvaogngymjmgg, string latnpxxx, string yushmbez, string arbxdwu, string gljgiwxpmmuh, double pailohkwdqle, int yjivswwqgftqnzw, bool ppzyzvmpbm)
{
	return 91300;
}

void cxtvjgm::rnbdudrrcemblyb(int qhedyhkbmr)
{
	bool fpdspvmqnkz = true;
	bool rzpsdklyrqn = false;
	int ggdjotjxfgxvh = 2403;
	if(false == false)
	{
		int dacn;
		for(dacn = 14; dacn > 0; dacn--)
		{
			continue;
		}
	}

}

int cxtvjgm::orrlbolshenmmquknchh(string fmoduxm, string aihvcqkrmilco, double klhgg, bool lxnqsqvnmo, int xizvwjvoygd, bool vzsdpnsyltdj, int advkvfgzxs, string ceshhdlaudfp)
{
	double qazqw = 60074;
	int vsxvnj = 5858;
	int qzguvdofw = 6516;
	int najbcj = 4061;
	bool emlnbsnbnlurs = false;
	bool xldwnbfujtspoyg = false;
	return 27775;
}

bool cxtvjgm::bxtypzdjnhzhbn(double oqxcr, double znkrhlorpbssqz, string uzbcmakk, string riclafwet, bool bejinh, int kazmxbszadea)
{
	string ynctv = "laewcryiycvvwbvhurlvezhzmluggbjqoxthldwbszptvvufbhlcubkcfjpttcwaztlm";
	bool rlkccixxfmqdjf = true;
	bool imxobwmxfsorbzn = true;
	bool lxkpluvoxjqy = true;
	int jrfglreu = 6412;
	double bqmds = 15763;
	double hklkdnq = 2399;
	string mzuovgiepb = "laxtbozjp";
	string wqcnohsgn = "rp";
	return true;
}

bool cxtvjgm::ywbeqqavzggszjcnqxh(string hscnyu, string lslpudtyj, double tedmb, double ojrxbdixlclijj, int hprgsh, int oavfxqw, double tfqhlank, string nskzhvsw, bool vrtvords, string ootcugyuru)
{
	double gkbnylgstahrh = 22445;
	bool wwjtldmkfnraod = false;
	string ksrmof = "namjzqoenjxbbnirhsrtfupaej";
	int fxxsfght = 3761;
	string nqcbmyyjsswv = "ml";
	double xlleviw = 3640;
	if(string("ml") != string("ml"))
	{
		int lzltv;
		for(lzltv = 90; lzltv > 0; lzltv--)
		{
			continue;
		}
	}
	if(3761 == 3761)
	{
		int jbatfce;
		for(jbatfce = 23; jbatfce > 0; jbatfce--)
		{
			continue;
		}
	}
	return false;
}

int cxtvjgm::sfcarnrcllpcek()
{
	bool dvozhoae = true;
	int ibdazylmsvek = 3377;
	int udamgz = 3846;
	double hlayiph = 16650;
	string zyayedrij = "qvcnvwkryiumrtaxxdcvgjp";
	double wxrrn = 26608;
	double sesjeguonmrlp = 42164;
	string ghrdyw = "qzloxnkmpfbasivkyiuzsulhsebkpdwcwhwkglqvhmnomqqnynuaa";
	bool upcvwaoqbgmonoa = false;
	bool ywuafjqpwsnuxr = false;
	if(false != false)
	{
		int nl;
		for(nl = 87; nl > 0; nl--)
		{
			continue;
		}
	}
	if(42164 == 42164)
	{
		int eqvkxkheu;
		for(eqvkxkheu = 99; eqvkxkheu > 0; eqvkxkheu--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int ukbl;
		for(ukbl = 100; ukbl > 0; ukbl--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int xiencnop;
		for(xiencnop = 11; xiencnop > 0; xiencnop--)
		{
			continue;
		}
	}
	if(3377 != 3377)
	{
		int iham;
		for(iham = 90; iham > 0; iham--)
		{
			continue;
		}
	}
	return 64831;
}

double cxtvjgm::kuqolnoxateyilepoyql(double sabdifywieiuej, bool ebqwmcm)
{
	bool dtizhjbh = true;
	bool rmfxmzhuqhfqby = true;
	bool smtnkjqdehhxhy = false;
	int ifnqrjlqmb = 3693;
	bool xtaphtaffve = true;
	bool crnywluxwufpjp = true;
	bool ntvhuiupi = false;
	string kzqjncjy = "mucjpjnjojsxwuqz";
	if(true != true)
	{
		int svftnasaim;
		for(svftnasaim = 51; svftnasaim > 0; svftnasaim--)
		{
			continue;
		}
	}
	return 75061;
}

double cxtvjgm::kmxzjqfktzyvcfm(double ufllmb, bool uzdyhqp, double wqrytwvqabnbz, bool uvcmmkfasuvjigo)
{
	bool zxbuxjfzc = false;
	string rxrwmfrue = "hwyaqfhlfmlauokpkujxgo";
	bool xtbga = true;
	int uoycbtiegirw = 4011;
	int olicvmeouvpgqwi = 5181;
	string zmcgpa = "vpsyjiqeurnatoj";
	int rqgdnvdu = 710;
	if(4011 == 4011)
	{
		int em;
		for(em = 24; em > 0; em--)
		{
			continue;
		}
	}
	if(4011 == 4011)
	{
		int plvt;
		for(plvt = 22; plvt > 0; plvt--)
		{
			continue;
		}
	}
	return 39313;
}

string cxtvjgm::fccbktweonhqdzya(string mehbs)
{
	string bfrku = "qwbsphubaurkpcwpajwtcsfodgbmeeufzgkucwilrozyy";
	int wrsvzlaxlztow = 261;
	string hfaphsujwordnkq = "brfexxtngabqkkeaswkjqzneysumislwryvlfibiksnwsoceguzalrabz";
	double qeogfitg = 39407;
	double mnfsdxjonan = 4884;
	bool vngidos = false;
	bool censt = false;
	double xgdcr = 30102;
	int moetzmspyxmmm = 4531;
	if(string("brfexxtngabqkkeaswkjqzneysumislwryvlfibiksnwsoceguzalrabz") == string("brfexxtngabqkkeaswkjqzneysumislwryvlfibiksnwsoceguzalrabz"))
	{
		int dbz;
		for(dbz = 29; dbz > 0; dbz--)
		{
			continue;
		}
	}
	if(false == false)
	{
		int txsdr;
		for(txsdr = 76; txsdr > 0; txsdr--)
		{
			continue;
		}
	}
	if(261 == 261)
	{
		int daymtq;
		for(daymtq = 0; daymtq > 0; daymtq--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int rsq;
		for(rsq = 16; rsq > 0; rsq--)
		{
			continue;
		}
	}
	if(string("qwbsphubaurkpcwpajwtcsfodgbmeeufzgkucwilrozyy") == string("qwbsphubaurkpcwpajwtcsfodgbmeeufzgkucwilrozyy"))
	{
		int wnbs;
		for(wnbs = 68; wnbs > 0; wnbs--)
		{
			continue;
		}
	}
	return string("tbfuxvteulyilez");
}

bool cxtvjgm::vzrqdrfwgxvulmblbdakhxdm(int pdnqbjvkeivcmi, bool guysxvmf, string yfhdy, double ymtuisl, int gplhob, double juozkbrwjqs, int lzapgixvse, double gkupqwilzdaggm, double zvedwnsfmlf, int qctidjxo)
{
	int pusovqdefiiuj = 198;
	if(198 != 198)
	{
		int ii;
		for(ii = 77; ii > 0; ii--)
		{
			continue;
		}
	}
	if(198 == 198)
	{
		int pngwiyyvw;
		for(pngwiyyvw = 56; pngwiyyvw > 0; pngwiyyvw--)
		{
			continue;
		}
	}
	return false;
}

cxtvjgm::cxtvjgm()
{
	this->boombayarleqoypwefqmffc(true, true, 29561, string("eobmqgwijwoujyjimbmzhpivynrtpvgslav"), string("znrtpalznhaabjwqwmohzxulbdiraljfgjmfqmkndbnwalmldcwjdxuhdupwlymubwvbvlinmxjnxkqfvrafyxdajpoaizjjund"), string("rezvrdzupyqizglafuwwuzfzhevokluuqteqgfemgtscjuclagreefnsxdcqhosjt"), string("rqwvquhobscvqlylwabrnlysyduovtcvmpjuvdock"), 31185, 4888, true);
	this->rnbdudrrcemblyb(760);
	this->orrlbolshenmmquknchh(string("mmluzyqoucwvdmvbebutgkni"), string("yrfzlucibdhrxgrkokjxjyhuenjoiwwigyzabxjrazixvquepogdjfajcszxwmotiggdlhddmrtevm"), 21546, false, 3330, true, 915, string("sxqifnnskphweljnrfcsofcuyumiaynbkzkyndgrpmaskpc"));
	this->bxtypzdjnhzhbn(8245, 29193, string("zvjxwnmrqvulspbmzqhqsyrdqntwdvmxzecxnpakpqghmocltovsfjcwhetfgrotw"), string("hqevvocorcootuxurvntdlikjkeyfnyxbnpes"), true, 1447);
	this->ywbeqqavzggszjcnqxh(string("dtgazerlhkerodjvmetpqrask"), string("bortdtgxivktjublqlwinsoxsrlfejwfarh"), 68100, 14753, 4138, 591, 3864, string("zvaztsdhfsttgcjgwnclxazdfnhn"), false, string("mwycpdyumtttwzmshnzulgaicrarvhlquyduksmgwxppbbiqpeajiziqrcsavbo"));
	this->sfcarnrcllpcek();
	this->kuqolnoxateyilepoyql(43982, true);
	this->kmxzjqfktzyvcfm(22926, true, 11038, false);
	this->fccbktweonhqdzya(string("sjezbnautucrjdxugnvyhsbyddwydsphsdpqfbkvekfpogyhrloziwarhyofrxnjiypmpkptabjctfptz"));
	this->vzrqdrfwgxvulmblbdakhxdm(1408, false, string("njqopjngcxxajoevosjudrdrdzzztvokekqbxrfdecjkwijuurvdzchhtb"), 31441, 5774, 19265, 8022, 25396, 30529, 178);
	this->ueeaoiezgaxygvvvmdyukul(450, 7459, string("mibuqmbybqhohgzkzmkmddmlchjutbazraccntmtz"), string("bpxfmoxkekastectwzpybtkumrgmrpqegdypthnzslnyywfbe"), 1185, 5319, 12752, string("siraaxwyicfphyfmgqnw"), 859, false);
	this->lhuzfngkxgnwbm(string("htdriwsefwkbrrhdxppefjgechhocwhlfczgbcpstoyznlpcxofjnagietagccd"), 41300, false, true, 1160, false, 1510, true, string("joiaymkkjbcihbgofbbkrbxgtgheyzuwtxycllzemvxcyddfxk"));
	this->ndbcueclgtorsmvdmeapccbm(7346, 459, string("mlxajjseqydpstmxnmjhujwkhvvsdyosxyurkvyiyjgyiopejesxgooezcptaejbuuyqobjoohwmzebtmvodnzqfouawes"), string("cpjpjcdhefrwmifdzjcqbkirwnqmvhaavmtkhuouwxpbnvrsreplkirzqyjtxcpvrfriylixjxevcgiqalogzospu"), 43121);
	this->efulwiirkyimllffnsxcpy(false, 1115, 200, 2658);
	this->utaefkksulr(581, 26033);
	this->ticpjjzeyuaiaehzeg(15081, 4802, false, string("bvtyptpnabtjnqncxmertoflweijhuxkqqlgjdrtatzdmpuidppjvl"), 40927);
	this->wirkfevwntovjbyogywwf(9234);
	this->eleqgyehdxhdtoblods(453, 1162, 9097, true);
}