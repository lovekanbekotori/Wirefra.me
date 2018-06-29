#pragma once
namespace SDK
{
	class CUserCmd;
	class CBaseEntity;
}

class CAntiAim
{
public:
	void do_antiaim(SDK::CUserCmd* cmd);
	void fix_movement(SDK::CUserCmd* cmd);
private:
	void backwards(SDK::CUserCmd* cmd);
	void legit(SDK::CUserCmd* cmd);
	void sideways(SDK::CUserCmd* cmd);
	float GetBestHeadAngle(SDK::CBaseEntity* player, SDK::CUserCmd* cmd);
	void lowerbody(SDK::CUserCmd* cmd);
	void lowerbody_pysen(SDK::CUserCmd* cmd);
	void b1gboysAA(SDK::CUserCmd* cmd);

//	void backjitter(SDK::CUserCmd* cmd);
	void bigdaddyAA(SDK::CUserCmd* cmd);

};

extern CAntiAim* antiaim;