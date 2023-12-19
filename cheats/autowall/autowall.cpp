// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

#include "autowall.h"
using Fna = bool(__fastcall)(IClientEntity*);
bool autowall::is_breakable_entity(IClientEntity* e)
{
	if (!e && !e->EntIndex())
		return false;

	static auto is_breakable = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 56 8B F1 85 F6 74 68"));

	int chTakeDamage = *(uintptr_t*)(e + 0x280);
	int chTakeDamageBackup = chTakeDamage;
	auto client_class = e->GetClientClass();

	if ((client_class->m_pNetworkName[1] == 'B' &&
		client_class->m_pNetworkName[9] == 'e' &&
		client_class->m_pNetworkName[10] == 'S' &&
		client_class->m_pNetworkName[16] == 'e') ||
		(client_class->m_pNetworkName[1] != 'B' ||
			client_class->m_pNetworkName[5] != 'D'))
		chTakeDamage = DAMAGE_YES;

	using Fn = bool(__thiscall*)(IClientEntity*);
	auto bBreakable = ((Fn)is_breakable)(e);
	chTakeDamage = chTakeDamageBackup;

	return bBreakable;
}

void autowall::scale_damage(player_t* e, CGameTrace& enterTrace, weapon_info_t* weaponData, float& currentDamage)
{
	const auto is_armored = [&]() -> bool
		{
			if (e->m_ArmorValue() > 0.f)
			{
				switch (enterTrace.hitgroup)
				{
				case HITGROUP_GENERIC:
				case HITGROUP_CHEST:
				case HITGROUP_STOMACH:
				case HITGROUP_LEFTARM:
				case HITGROUP_RIGHTARM:
					return true;
				case HITGROUP_HEAD:
					return e->m_bHasHelmet();
				default:
					break;
				}
			}

			return false;
		};

	switch (enterTrace.hitgroup)
	{
	case HITGROUP_HEAD:
		if (e->m_bHasHeavyArmor())
			currentDamage = currentDamage * .5f;
		else
			currentDamage *= 4.f;
		break;
	case HITGROUP_STOMACH:
		currentDamage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		currentDamage *= .75f;
		break;
	default:
		break;
	}

	if (is_armored())
	{
		auto modifier = 1.f, armor_bonus_ratio = .5f, armor_ratio = weaponData->flArmorRatio * .5f;

		if (e->m_bHasHeavyArmor())
		{
			armor_bonus_ratio = 0.5f;
			armor_ratio = weaponData->flArmorRatio * 0.5f;
			modifier = 1.f;
		}

		auto new_damage = currentDamage * armor_ratio;

		if (e->m_bHasHeavyArmor())
			new_damage = new_damage * 0.85000002f;

		if ((currentDamage - currentDamage * armor_ratio) * (modifier * armor_bonus_ratio) > e->m_ArmorValue())
			new_damage = currentDamage - e->m_ArmorValue() / armor_bonus_ratio;

		currentDamage = new_damage;
	}

	return;
}
void TraceLine(Vector& absStart, const Vector& absEnd, unsigned int mask, player_t* ignore, CGameTrace* ptr)
{
	Ray_t ray;
	ray.Init(absStart, absEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;

	m_trace()->TraceRay(ray, mask, &filter, ptr);
}
bool autowall::trace_to_exit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, const Vector& direction)
{
	static ConVar* sv_clip_penetration_traces_to_players = m_cvar()->FindVar(crypt_str("sv_clip_penetration_traces_to_players"));

	Vector          new_end, out;
	float           dist = 0.0f;
	int                iterations = 23;
	int                first_contents = 0;
	int             contents;
	Ray_t r{};

	while (1)
	{
		iterations--;

		if (iterations <= 0 || dist > 90.f)
			break;

		dist += 4.0f;
		out = startPosition + (direction * dist);

		contents = m_trace()->GetPointContents(out, 0x4600400B, nullptr);

		if (first_contents == -1)
			first_contents = contents;

		if (contents & 0x600400B && (!(contents & CONTENTS_HITBOX) || first_contents == contents))
			continue;

		new_end = out - (direction * 4.f);

		TraceLine(out, new_end, 0x4600400B, nullptr, &exitTrace);

		if (exitTrace.startsolid && (exitTrace.surface.flags & SURF_HITBOX) != 0)
		{
			TraceLine(out, startPosition, MASK_SHOT_HULL, (player_t*)exitTrace.hit_entity, &exitTrace);

			if (exitTrace.DidHit() && !exitTrace.startsolid)
			{
				out = exitTrace.endpos;
				return true;
			}
			continue;
		}

		if (!exitTrace.DidHit() || exitTrace.startsolid)
		{
			if (enterTrace.hit_entity != m_entitylist()->GetClientEntity(0))
			{
				if (exitTrace.hit_entity && is_breakable_entity(exitTrace.hit_entity))
				{
					exitTrace.surface.surfaceProps = enterTrace.surface.surfaceProps;
					exitTrace.endpos = startPosition + direction;
					return true;
				}
			}

			continue;
		}

		if ((exitTrace.surface.flags & 0x80u) != 0)
		{
			if (enterTrace.hit_entity && is_breakable_entity(enterTrace.hit_entity) && exitTrace.hit_entity && is_breakable_entity(exitTrace.hit_entity))
			{
				out = exitTrace.endpos;
				return true;
			}

			if (!(enterTrace.surface.flags & 0x80u))
				continue;
		}

		if (exitTrace.plane.normal.Dot(direction) <= 1.f)
		{
			out -= direction * (exitTrace.fraction * 4.0f);
			return true;
		}
	}

	return false;
}

bool autowall::handle_bullet_penetration(weapon_info_t* weaponData, CGameTrace& enterTrace, Vector& eyePosition, const Vector& direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, bool draw_impact)
{
	CGameTrace trace_exit;
	surfacedata_t* enter_surface_data = m_physsurface()->GetSurfaceData(enterTrace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;

	float enter_surf_penetration_modifier = enter_surface_data->game.flPenetrationModifier;
	float final_damage_modifier = 0.18f;
	float compined_penetration_modifier = 0.f;
	bool solid_surf = ((enterTrace.contents >> 3) & CONTENTS_SOLID);
	bool light_surf = ((enterTrace.surface.flags >> 7) & SURF_LIGHT);

	if
		(
			possibleHitsRemaining <= 0
			|| (!possibleHitsRemaining && !light_surf && !solid_surf && enter_material != CHAR_TEX_GLASS && enter_material != CHAR_TEX_GRATE)
			|| weaponData->flPenetration <= 0.f
			|| !trace_to_exit(enterTrace, trace_exit, enterTrace.endpos, direction)
			&& !(m_trace()->GetPointContents(enterTrace.endpos, MASK_SHOT_HULL | CONTENTS_HITBOX, NULL) & (MASK_SHOT_HULL | CONTENTS_HITBOX))
			)
	{
		return false;
	}

	surfacedata_t* exit_surface_data = m_physsurface()->GetSurfaceData(trace_exit.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;
	float exit_surf_penetration_modifier = exit_surface_data->game.flPenetrationModifier;

	if (enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE) {
		compined_penetration_modifier = 3.f;
		final_damage_modifier = 0.08f;
	}
	else if (light_surf || solid_surf)
	{
		compined_penetration_modifier = 1.f;
		final_damage_modifier = 0.18f;
	}
	else {
		compined_penetration_modifier = (enter_surf_penetration_modifier + exit_surf_penetration_modifier) * 0.5f;
		final_damage_modifier = 0.18f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD)
			compined_penetration_modifier = 3.f;
		else if (exit_material == CHAR_TEX_PLASTIC)
			compined_penetration_modifier = 2.0f;
	}

	float thickness = (trace_exit.endpos - enterTrace.endpos).LengthSqr();
	float modifier = max(0.f, 1.f / compined_penetration_modifier);
	float lost_damage = fmax(((modifier * thickness) / 24.f) + ((currentDamage * final_damage_modifier) + (fmax(3.75f / weaponData->flPenetration, 0.f) * 3.f * modifier)), 0.f);

	if (lost_damage > currentDamage)
		return false;

	if (lost_damage > 0.f)
		currentDamage -= lost_damage;

	if (currentDamage < 1.f)
		return false;

	eyePosition = trace_exit.endpos;
	possibleHitsRemaining--;

	return true;
}
bool autowall::fire_bullet(weapon_t* pWeapon, Vector& direction, bool& visible, float& currentDamage, int& hitbox, IClientEntity* e, float length, const Vector& pos)
{
	if (!pWeapon)
		return false;

	auto weaponData = pWeapon->get_csweapon_info();

	if (!weaponData)
		return false;

	CGameTrace enterTrace;
	CTraceFilter filter;

	filter.pSkip = g_ctx.local();
	currentDamage = weaponData->iDamage;

	auto eyePosition = pos;
	auto currentDistance = 0.0f;
	auto maxRange = weaponData->flRange;
	auto penetrationDistance = 3000.0f;
	auto penetrationPower = weaponData->flPenetration;
	auto possibleHitsRemaining = 4;

	while (currentDamage >= 1.0f)
	{
		maxRange -= currentDistance;
		auto end = eyePosition + direction * maxRange;

		CTraceFilter filter;
		filter.pSkip = g_ctx.local();

		util::trace_line(eyePosition, end, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace);
		util::clip_trace_to_players(e, eyePosition, end + direction * 40.0f, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace);

		auto enterSurfaceData = m_physsurface()->GetSurfaceData(enterTrace.surface.surfaceProps);
		auto enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;
		auto enterMaterial = enterSurfaceData->game.material;

		if (enterTrace.fraction == 1.0f)  //-V550
			break;

		currentDistance += enterTrace.fraction * maxRange;
		currentDamage *= pow(weaponData->flRangeModifier, currentDistance / 500.0f);

		if (currentDistance > penetrationDistance && weaponData->flPenetration || enterSurfPenetrationModifier < 0.1f)  //-V1051
			break;

		auto canDoDamage = enterTrace.hitgroup != HITGROUP_GEAR && enterTrace.hitgroup != HITGROUP_GENERIC;
		auto isPlayer = ((player_t*)enterTrace.hit_entity)->is_player();
		auto isEnemy = ((player_t*)enterTrace.hit_entity)->m_iTeamNum() != g_ctx.local()->m_iTeamNum();

		if (canDoDamage && isPlayer && isEnemy)
		{
			scale_damage((player_t*)enterTrace.hit_entity, enterTrace, weaponData, currentDamage);
			hitbox = enterTrace.hitbox;
			return true;
		}

		if (!possibleHitsRemaining)
			break;

		static auto damageReductionBullets = m_cvar()->FindVar(crypt_str("ff_damage_reduction_bullets"));
		static auto damageBulletPenetration = m_cvar()->FindVar(crypt_str("ff_damage_bullet_penetration"));

		if (!handle_bullet_penetration(weaponData, enterTrace, eyePosition, direction, possibleHitsRemaining, currentDamage, penetrationPower, damageReductionBullets->GetFloat(), damageBulletPenetration->GetFloat(), !e))
			break;

		visible = false;
	}

	return false;
}

autowall::returninfo_t autowall::wall_penetration(const Vector& eye_pos, Vector& point, IClientEntity* e)
{
	g_ctx.globals.autowalling = true;
	auto tmp = point - eye_pos;

	auto angles = ZERO;
	math::vector_angles(tmp, angles);

	auto direction = ZERO;
	math::angle_vectors(angles, direction);

	direction.NormalizeInPlace();

	auto visible = true;
	auto damage = -1.0f;
	auto hitbox = -1;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (fire_bullet(weapon, direction, visible, damage, hitbox, e, 0.0f, eye_pos))
	{
		g_ctx.globals.autowalling = false;
		return returninfo_t(visible, (int)damage, hitbox); //-V2003
	}
	else
	{
		g_ctx.globals.autowalling = false;
		return returninfo_t(false, -1, -1);
	}
}
