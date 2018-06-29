#include "../includes.h"
#include "../UTILS/mopvar.h"
#include "../SDK/IEngine.h"
#include "../SDK/CUserCmd.h"
#include "../SDK/CBaseEntity.h"
#include "../SDK/CClientEntityList.h"
#include "../SDK/CBaseAnimState.h"
#include "../SDK/CGlobalVars.h"
#include "../SDK/CBaseWeapon.h"
#include "../FEATURES/Aimbot.h"
#include "../FEATURES/AntiAim.h"
#include "../SDK/CClientEntityList.h"
#include "../SDK/CTrace.h"
float randnum(float Min, float Max)
{
	return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}

float get_curtime(SDK::CUserCmd* ucmd) {
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return 0;

	int g_tick = 0;
	SDK::CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = (float)local_player->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * mopvar::Globals->interval_per_tick;
	return curtime;
}

float anim_velocity(SDK::CBaseEntity* LocalPlayer)
{
	if (LocalPlayer->GetAnimState() == nullptr)
		return false;

	int vel = LocalPlayer->GetAnimState()->speed_2d;
	return vel;
}

bool first_supress(const float yaw_to_break, SDK::CUserCmd* cmd)
{
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return false;

	static float next_lby_update_time = 0;
	float curtime = get_curtime(cmd);

	auto animstate = local_player->GetAnimState();
	if (!animstate)
		return false;

	if (!(local_player->GetFlags() & FL_ONGROUND))
		return false;

	if (SETTINGS::settings.delta_val < 120)
		return false;

	if (animstate->speed_2d > 0.1)
		next_lby_update_time = curtime + 0.22 - TICKS_TO_TIME(1);

	if (next_lby_update_time < curtime)
	{
		next_lby_update_time = curtime + 1.1;
		return true;
	}

	return false;
}

bool second_supress(const float yaw_to_break, SDK::CUserCmd* cmd)
{
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return false;

	static float next_lby_update_time = 0;
	float curtime = get_curtime(cmd);

	auto animstate = local_player->GetAnimState();
	if (!animstate)
		return false;

	if (!(local_player->GetFlags() & FL_ONGROUND))
		return false;

	if (SETTINGS::settings.delta_val < 120)
		return false;

	if (animstate->speed_2d > 0.1)
		next_lby_update_time = curtime + 0.22 + TICKS_TO_TIME(1);


	if (next_lby_update_time < curtime)
	{
		next_lby_update_time = curtime + 1.1;
		return true;
	}

	return false;
}

bool next_lby_update(const float yaw_to_break, SDK::CUserCmd* cmd)
{
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return false;

	static float next_lby_update_time = 0;
	float curtime = get_curtime(cmd);

	auto animstate = local_player->GetAnimState();
	if (!animstate)
		return false;

	if (!(local_player->GetFlags() & FL_ONGROUND))
		return false;

	if (animstate->speed_2d > 0.1)
		next_lby_update_time = curtime + 0.22f;

	if (next_lby_update_time < curtime)
	{
		next_lby_update_time = curtime + 1.1f;
		return true;
	}

	return false;
}
void inline SinCos(float radians, float* sine, float* cosine)
{
	*sine = sin(radians);
	*cosine = cos(radians);
}
void AngleVectors3(const Vector &angles, Vector& forward, Vector& right, Vector& up)
{
	float sr, sp, sy, cr, cp, cy;

	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	SinCos(DEG2RAD(angles[0]), &sp, &cp);
	SinCos(DEG2RAD(angles[2]), &sr, &cr);

	forward.x = (cp * cy);
	forward.y = (cp * sy);
	forward.z = (-sp);
	right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
	right.y = (-1 * sr * sp * sy + -1 * cr *  cy);
	right.z = (-1 * sr * cp);
	up.x = (cr * sp * cy + -sr * -sy);
	up.y = (cr * sp * sy + -sr * cy);
	up.z = (cr * cp);
}

float CAntiAim::GetBestHeadAngle(SDK::CBaseEntity* player, SDK::CUserCmd* cmd)
{

	static float last_real;

	float Back, Right, Left;

	Vector src3D, dst3D, forward, right, up, src, dst;
	SDK::trace_t tr;
	SDK::Ray_t backray, rightray, leftray;
	SDK::CTraceFilter filter;

	Vector angles;
	mopvar::Engine->GetViewAngles(angles);
	angles.x = 0.f;

	AngleVectors3(angles, forward, right, up);

	filter.pSkip1 = player;
	src3D = player->GetVecOrigin() + player->GetViewOffset();
	dst3D = src3D + (forward * 384.f);


	rightray.Init(src3D + right * 35.f, dst3D + right * 35.f);
	mopvar::Trace->TraceRay(rightray, MASK_SHOT, &filter, &tr);
	Right = (tr.end - tr.start).Length();

	leftray.Init(src3D - right * 35.f, dst3D - right * 35.f);
	mopvar::Trace->TraceRay(leftray, MASK_SHOT, &filter, &tr);
	Left = (tr.end - tr.start).Length();





	if (Left > Right)
	{
		SETTINGS::settings.flip_bool = true;
		/*if (GLOBAL::should_send_packet)
			return + randnum(-177, 177);
		else
		{
			if (first_supress(cmd->viewangles.y + 110, cmd))
				return last_real + 110;
			else if (next_lby_update(cmd->viewangles.y + SETTINGS::settings.delta_val, cmd))
			{//else if
				return last_real + SETTINGS::settings.delta_val;

			}
			else if (second_supress(cmd->viewangles.y - 110, cmd))
				return last_real - 110;
			else
			{
				return - 90;
				last_real = cmd->viewangles.y;

			}
		}*/
	}
	else if (Right > Left)
	{
		SETTINGS::settings.flip_bool = false;
		/*if (GLOBAL::should_send_packet)
			return + randnum(-177, 177);
		else
		{
			if (first_supress(cmd->viewangles.y - 110, cmd))
				return last_real - 110;
			else if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd))
			{

				return last_real - SETTINGS::settings.delta_val;
			}
			else if (second_supress(cmd->viewangles.y + 110, cmd))
				return last_real + 110;
			else
			{
				return + 90;
				last_real = cmd->viewangles.y;

			}
		}*/
	}
	else
	{
		if (GLOBAL::should_send_packet)
			return + randnum(-177, 177);
		else
		{
			if (first_supress(cmd->viewangles.y - 110, cmd))
				return last_real - 110;
			else if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd))
			{

				return last_real - SETTINGS::settings.delta_val;
			}
			else if (second_supress(cmd->viewangles.y + 110, cmd))
				cmd->viewangles.y = last_real + 110;
			else
			{
				return + 180;
				last_real = cmd->viewangles.y;

			}
		}
	}


	return 0;
}
void CAntiAim::do_antiaim(SDK::CUserCmd* cmd)
{
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	if (local_player->GetHealth() <= 0)
		return;

	auto weapon = reinterpret_cast<SDK::CBaseWeapon*>(mopvar::ClientEntityList->GetClientEntity(local_player->GetActiveWeaponIndex()));

	if (!weapon)
		return;

	if (cmd->buttons & IN_USE)
		return;

	if (cmd->buttons & IN_ATTACK && aimbot->can_shoot())
		return;

	if (weapon->get_full_info()->WeaponType == 9)
		return;

	if (!SETTINGS::settings.antiaim_bool)
		return;

	if (SETTINGS::settings.walldetection){
		GetBestHeadAngle(local_player, cmd);
	}
static	bool flip;
	flip = !flip;
	if (!SETTINGS::settings.lag_bool)
		GLOBAL::should_send_packet = flip;
	//float delta = abs(MATH::NormalizeYaw(GLOBAL::real_angles.y - local_player->GetLowerBodyYaw()));
	//std::cout << std::to_string(delta) << std::endl;
	if (SETTINGS::settings.antiaim_type == 1) {
		cmd->viewangles.x = 89.000000;
		backwards(cmd);
	}
	else if (SETTINGS::settings.antiaim_type == 2)
	{
		cmd->viewangles.x = 89.000000;
		sideways(cmd);
	}
	else if (SETTINGS::settings.antiaim_type == 3)
	{
		cmd->viewangles.x = 89.000000;
		bigdaddyAA(cmd);
	}
	else if (SETTINGS::settings.antiaim_type == 4)
	{
		cmd->viewangles.x = 89.000000;
		lowerbody(cmd);
	}
	else if (SETTINGS::settings.antiaim_type == 5)
	{
		cmd->viewangles.x = 89.000000;
		lowerbody_pysen(cmd);
	}
	else if (SETTINGS::settings.antiaim_type == 6)
	{
		cmd->viewangles.x = 89.000000;
		b1gboysAA(cmd);
	}
	if (!GLOBAL::should_send_packet)
		GLOBAL::real_angles = cmd->viewangles;

	}

void CAntiAim::fix_movement(SDK::CUserCmd* cmd)
{
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	Vector real_viewangles;
	mopvar::Engine->GetViewAngles(real_viewangles);

	Vector vecMove(cmd->forwardmove, cmd->sidemove, cmd->upmove);
	float speed = sqrt(vecMove.x * vecMove.x + vecMove.y * vecMove.y);

	Vector angMove;
	MATH::VectorAngles(vecMove, angMove);

	float yaw = DEG2RAD(cmd->viewangles.y - real_viewangles.y + angMove.y);

	cmd->forwardmove = cos(yaw) * speed;
	cmd->sidemove = sin(yaw) * speed;

	cmd->viewangles = MATH::NormalizeAngle(cmd->viewangles);
}

void CAntiAim::backwards(SDK::CUserCmd* cmd)
{
	if (GLOBAL::should_send_packet)
		cmd->viewangles.y += randnum(-180, 180);
	else
		cmd->viewangles.y += 180.000000;
}

void CAntiAim::legit(SDK::CUserCmd* cmd)
{
	if (GLOBAL::should_send_packet)
		cmd->viewangles.y += 0;
	else
		cmd->viewangles.y += 90;
}

void CAntiAim::sideways(SDK::CUserCmd* cmd)
{
	if (SETTINGS::settings.flip_bool)
	{
		if (GLOBAL::should_send_packet)
			cmd->viewangles.y += 90;
		else
			cmd->viewangles.y -= 90;
	}
	else
	{
		if (GLOBAL::should_send_packet)
			cmd->viewangles.y -= 90;
		else
			cmd->viewangles.y += 90;
	}
}

void CAntiAim::bigdaddyAA(SDK::CUserCmd* cmd)
{

	if (SETTINGS::settings.flip_bool)
	{
		if (GLOBAL::should_send_packet)
			cmd->viewangles.y += 89.865 + 442800;
		else
			cmd->viewangles.y -= 253800 + 180 + ((rand() % 15) - (14.5246 * 0.5f));

	}
	else
	{
		if (GLOBAL::should_send_packet)
			cmd->viewangles.y -= 89.865 + 442800;
		else
			cmd->viewangles.y += 253800 + 180 + ((rand() % 15) - (14.5246 * 0.5f));
	}

	
}

void CAntiAim::b1gboysAA(SDK::CUserCmd* cmd)
{
	static float last_real;

	if (SETTINGS::settings.flip_bool)
	{
		if (GLOBAL::should_send_packet)
			cmd->viewangles.y += randnum(-120, 120);
		else
		{
			//if (first_supress(cmd->viewangles.y + 110, cmd))
			//cmd->viewangles.y = last_real + 110;
			if (next_lby_update(cmd->viewangles.y + SETTINGS::settings.delta_val, cmd)) //else if
				cmd->viewangles.y = last_real + SETTINGS::settings.delta_val;
			//else if (second_supress(cmd->viewangles.y - 110, cmd))
			//	cmd->viewangles.y = last_real - 110;
			else
			{
				cmd->viewangles.y -= 65443065 + ((rand() % 15) - (14.5246 * 0.5f));;
				last_real = cmd->viewangles.y;
			}
		}
	}
	else
	{
		if (GLOBAL::should_send_packet)
			cmd->viewangles.y += randnum(-120, 120);
		else
		{
			if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd))
				cmd->viewangles.y = last_real - SETTINGS::settings.delta_val;
			else
			{
				cmd->viewangles.y += 65443065 + ((rand() % 15) - (14.5246 * 0.5f));;
				last_real = cmd->viewangles.y;
			}
		}
	}
}

void CAntiAim::lowerbody(SDK::CUserCmd* cmd)
{
	static float last_real;

	if (SETTINGS::settings.flip_bool)
	{
		if (GLOBAL::should_send_packet)
			cmd->viewangles.y += randnum(-120, 120);
		else
		{
			//if (first_supress(cmd->viewangles.y + 110, cmd))
				//cmd->viewangles.y = last_real + 110;
			if (next_lby_update(cmd->viewangles.y + SETTINGS::settings.delta_val, cmd)) //else if
				cmd->viewangles.y = last_real + SETTINGS::settings.delta_val;
			//else if (second_supress(cmd->viewangles.y - 110, cmd))
			//	cmd->viewangles.y = last_real - 110;
			else
			{
				cmd->viewangles.y -= 90;
				last_real = cmd->viewangles.y;
			}
		}
	}
	else
	{
		if (GLOBAL::should_send_packet)
			cmd->viewangles.y += randnum(-120, 120);
		else
		{
			if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd))
				cmd->viewangles.y = last_real - SETTINGS::settings.delta_val;
			else
			{
				cmd->viewangles.y += 90;
				last_real = cmd->viewangles.y;
			}
		}
	}
}
void CAntiAim::lowerbody_pysen(SDK::CUserCmd* cmd)
{
	auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());
	if (next_lby_update(cmd->viewangles.y - SETTINGS::settings.delta_val, cmd)) {
		cmd->viewangles.y = local_player->GetLowerBodyYaw() + 1809;
		cmd->viewangles.z = 1050.f;
	}


	if (SETTINGS::settings.flip_bool)
	{
		if (GLOBAL::should_send_packet) {
			cmd->viewangles.y = local_player->GetLowerBodyYaw() + 1814;
			cmd->viewangles.z = 1050.f;
		}
		else
			cmd->viewangles.y -= 36078 ;
	}
	else
	{
		if (GLOBAL::should_send_packet)
		{
			cmd->viewangles.y = local_player->GetLowerBodyYaw() + 1814;
			cmd->viewangles.z = 1050.f;
		}
		else {
			cmd->viewangles.y += 36078;
		}

	}
}



CAntiAim* antiaim = new CAntiAim();