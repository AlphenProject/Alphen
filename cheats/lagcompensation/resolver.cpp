#include "animation_system.h"
#include "..\ragebot\aim.h"
#include "../autowall/autowall.h"

void resolver::initialize(player_t* e, adjust_data* record, const float& goal_feet_yaw, const float& pitch)
{
    player = e;
    player_record = record;

    original_goal_feet_yaw = math::normalize_yaw(goal_feet_yaw);
    original_pitch = math::normalize_pitch(pitch);
}

void resolver::reset()
{
    player = nullptr;
    player_record = nullptr;

}
float_t get_backward_side(player_t* player)
{
    return math::calculate_angle(player->m_vecOrigin(), player->m_vecOrigin()).y;
}

static auto resolve_update_animations(player_t* e)
{
    e->update_clientside_animation();
};

Vector GetHitboxPos(player_t* player, matrix3x4_t* mat, int hitbox_id)
{
    if (!player)
        return Vector();

    auto hdr = m_modelinfo()->GetStudioModel(player->GetModel());

    if (!hdr)
        return Vector();

    auto hitbox_set = hdr->pHitboxSet(player->m_nHitboxSet());

    if (!hitbox_set)
        return Vector();

    auto hitbox = hitbox_set->pHitbox(hitbox_id);

    if (!hitbox)
        return Vector();

    Vector min, max;

    math::vector_transform(hitbox->bbmin, mat[hitbox->bone], min);
    math::vector_transform(hitbox->bbmax, mat[hitbox->bone], max);

    return (min + max) * 0.5f;
}

float_t MaxYawModificator(player_t* enemy)
{
    auto animstate = enemy->get_animation_state();

    if (!animstate)
        return 0.0f;

    auto speedfactor = math::clamp(animstate->m_flFeetSpeedForwardsOrSideWays, 0.0f, 1.0f);
    auto avg_speedfactor = (animstate->m_flStopToFullRunningFraction * -0.3f - 0.2f) * speedfactor + 1.0f;

    auto duck_amount = animstate->m_fDuckAmount;

    if (duck_amount)
    {
        auto max_velocity = math::clamp(animstate->m_flFeetSpeedUnknownForwardOrSideways, 0.0f, 1.0f);
        auto duck_speed = duck_amount * max_velocity;

        avg_speedfactor += duck_speed * (0.5f - avg_speedfactor);
    }

    return animstate->yaw_desync_adjustment() * avg_speedfactor;
}

float_t GetBackwardYaw(player_t* player) {

    return math::calculate_angle(player->m_vecOrigin(), player->m_vecOrigin()).y;
}

void resolver::resolve_yaw()
{
    player_info_s player_info;
    auto choked = abs(TIME_TO_TICKS(player->m_flSimulationTime() - player->m_flOldSimulationTime()) - 1);

    if (!m_engine()->GetPlayerInfo(player->EntIndex(), &player_info))
        return;

    if (!g_ctx.local()->is_alive() || player->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
        return;

    if (player->get_move_type() == MOVETYPE_LADDER || player->get_move_type() == MOVETYPE_NOCLIP)
        return;

    int playerint = player->EntIndex();

    auto animstate = player->get_animation_state();

    float new_body_yaw_pose = 0.0f;
    auto m_flCurrentFeetYaw = player->get_animation_state()->m_flCurrentFeetYaw;
    auto m_flGoalFeetYaw = player->get_animation_state()->m_flGoalFeetYaw;
    auto m_flEyeYaw = player->get_animation_state()->m_flEyeYaw;
    float flMaxYawModifier = MaxYawModificator(player);
    float flMinYawModifier = player->get_animation_state()->pad10[512];
    auto anglesy = math::normalize_yaw(player->m_angEyeAngles().y - original_goal_feet_yaw);

    auto valid_lby = true;

    auto speed = player->m_vecVelocity().Length2D();

    float m_lby = player->m_flLowerBodyYawTarget() * 0.574f;

    auto player_stand = player->m_vecVelocity().Length2D();
    player_stand = 0.f;

    float m_flLastClientSideAnimationUpdateTimeDelta = 0.0f;
    auto trace = false;

    auto v54 = animstate->m_fDuckAmount;
    auto v55 = ((((*(float*)((uintptr_t)animstate + 0x11C)) * -0.30000001) - 0.19999999) * animstate->m_flFeetSpeedForwardsOrSideWays) + 1.0f;

    bool bWasMovingLastUpdate = false;
    bool bJustStartedMovingLastUpdate = false;

    auto unknown_velocity = *(float*)(uintptr_t(animstate) + 0x2A4);

    auto first_matrix = player_record->matrixes_data.first;
    auto second_matrix = player_record->matrixes_data.second;
    auto central_matrix = player_record->matrixes_data.zero;
    auto leftPose = GetHitboxPos(player, first_matrix, HITBOX_HEAD);
    auto rightPose = GetHitboxPos(player, second_matrix, HITBOX_HEAD);
    auto centralPose = GetHitboxPos(player, central_matrix, HITBOX_HEAD);

    static const float resolve_list[3] = { 0.f, player->m_angEyeAngles().y + player->get_max_desync_delta(), player->m_angEyeAngles().y - player->get_max_desync_delta() };

    int missed_player = player->EntIndex();

    auto delta = math::angle_distance(animstate->m_flGoalFeetYaw, animstate->m_flEyeYaw);

    ///////////////////// [ ANIMLAYERS ] /////////////////////
    int i = player->EntIndex();
    AnimationLayer layers[13];
    AnimationLayer movelayers[3][13];
    memcpy(layers, player->get_animlayers(), player->animlayer_count() * sizeof(AnimationLayer));

    if (g_ctx.globals.missed_shots[i] > 2)
    {
        animstate->m_flGoalFeetYaw = math::normalize_yaw(player->m_angEyeAngles().y + resolve_list[g_ctx.globals.missed_shots[i] % 3]);
    }

    if (player->m_fFlags() & FL_ONGROUND)
    {
        if (speed <= 1.f || player_record->layers[3].m_flWeight == layers[3].m_flWeight)
        {
            int m_side = 2 * (m_lby > 0.f) ? -1 : 1;
            animstate->m_flGoalFeetYaw = (player->m_angEyeAngles().y + 58.f) * m_side;
            if (g_ctx.globals.missed_shots[i] > 2)
            {
                animstate->m_flGoalFeetYaw = math::normalize_yaw(player->m_angEyeAngles().y + resolve_list[g_ctx.globals.missed_shots[i] % 3]);
            }
        }
        else if (speed > 1.f && (!(player_record->layers[12].m_flWeight) && (player_record->layers[6].m_flWeight == layers[6].m_flWeight)))
        {
            //lagcompensation::get().set(player, player_record);
            float delta_rotate_first = abs(layers[6].m_flPlaybackRate - movelayers[0][6].m_flPlaybackRate);
            float delta_rotate_second = abs(layers[6].m_flPlaybackRate - movelayers[2][6].m_flPlaybackRate);
            float delta_rotate_third = abs(layers[6].m_flPlaybackRate - movelayers[1][6].m_flPlaybackRate);

            if (delta_rotate_first < delta_rotate_second || delta_rotate_second <= delta_rotate_third || delta_rotate_second)
            {
                if (delta_rotate_first >= delta_rotate_third && delta_rotate_second > delta_rotate_third && !(delta_rotate_third))
                {
                    animstate->m_flGoalFeetYaw = math::normalize_yaw(player->m_angEyeAngles().y + player->get_max_desync_delta());
                    switch (g_ctx.globals.missed_shots[i] % 2)
                    {
                    case 0:    animstate->m_flGoalFeetYaw = math::normalize_yaw(player->m_angEyeAngles().y - player->get_max_desync_delta());
                        break;
                    case 1:
                        animstate->m_flGoalFeetYaw = math::normalize_yaw(player->m_angEyeAngles().y + resolve_list[g_ctx.globals.missed_shots[i] % 3]);
                        break;
                    default:
                        break;
                    }
                }
            }
            else
            {
                animstate->m_flGoalFeetYaw = math::normalize_yaw(player->m_angEyeAngles().y - player->get_max_desync_delta());
                switch (g_ctx.globals.missed_shots[i] % 2)
                {
                case 0:    animstate->m_flGoalFeetYaw = math::normalize_yaw(player->m_angEyeAngles().y + player->get_max_desync_delta());
                    break;
                case 1:
                    animstate->m_flGoalFeetYaw = math::normalize_yaw(player->m_angEyeAngles().y + resolve_list[g_ctx.globals.missed_shots[i] % 3]);
                    break;
                default:
                    break;
                }
            }
        }
    }   ///////////////////// [ ANIMLAYERS ] /////////////////////
}

float resolver::resolve_pitch()
{
    return original_pitch;
}
