#include <dinput.h>
#include "imgui\imgui.h"
#include "imgui\dx9\imgui_dx9.h"
#include <tchar.h>
#include <iostream>
#include "includes.h"
#include "UTILS/render.h"
#include "UTILS/mopvar.h"
#include "SDK/GameEvents.h"
#include "SDK/IEngine.h"
#include "SDK/CInputSystem.h"
#include "SDK/CClientEntityList.h"
#include "SDK/CBaseEntity.h"
#include "UTILS/mopvar.h"
#include "SDK/IEngine.h"
#include "SDK/CUserCmd.h"
#include "SDK/ConVar.h"
#include "SDK/CGlobalVars.h"
#include "SDK/IViewRenderBeams.h"
#include "FEATURES/Backtracking.h"
#include "SDK/CBaseEntity.h"
#include "SDK/CClientEntityList.h"
#include "SDK/CBaseWeapon.h"
#include "SDK/CTrace.h"	
#include "FEATURES/Visuals.h"
#include "UTILS/render.h"
#include "XorStr.hpp"
#include "FEATURES/Aimbot.h"
#include "SDK/IVDebugOverlay.h"
#include "UTILS/mopvar.h"
#include "MENU/ParticleSystem.h"


int config_sel;
std::vector<dot*> dots = {};

void dot::update() {

	auto opacity = 255.0f;

	m_pos += m_vel * (opacity);
}

void dot::draw() {
	int opacity = 55.0f * (255.0f);

	RENDER::DrawFilledRect(m_pos.x - 2, m_pos.y - 2, 2, 2, CColor(255, 255, 255, opacity));

	auto t = std::find(dots.begin(), dots.end(), this);
	if (t == dots.end()) {
		return;
	}

	for (auto i = t; i != dots.end(); i++) {
		if ((*i) == this) continue;

		auto dist = (m_pos - (*i)->m_pos).length();

		if (dist < 128) {
			int alpha = opacity * (dist / 128);
			RENDER::DrawFilledRect(m_pos.x - 1, m_pos.y - 2, (*i)->m_pos.x - 2, (*i)->m_pos.y - 1, CColor(255, 255, 255, alpha));
		}
	}
}

void dot_draw() {
	struct screen_size {
		int x, y;
	}; screen_size sc;

	sc.x = 1920;
	sc.y = 1080;
	//int s = rand() % 24;
	int s = 3;
	if (s == 0) {
		dots.push_back(new dot(Vector2D(rand() % (int)sc.x, -16), Vector2D((rand() % 7) - 3, rand() % 3 + 1)));
	}
	else if (s == 1) {
		dots.push_back(new dot(Vector2D(rand() % (int)sc.x, (int)sc.y + 16), Vector2D((rand() % 7) - 3, -1 * (rand() % 3 + 1))));
	}
	else if (s == 2) {
		dots.push_back(new dot(Vector2D(-16, rand() % (int)sc.y), Vector2D(rand() % 3 + 1, (rand() % 7) - 3)));
	}
	else if (s == 3) {
		dots.push_back(new dot(Vector2D((int)sc.x + 16, rand() % (int)sc.y), Vector2D(-1 * (rand() % 3 + 1), (rand() % 7) - 3)));
	}

	auto alph = 135.0f * (255.0f);
	auto a_int = (int)(alph);

	RENDER::DrawFilledRect(0, 0, sc.x, sc.y, CColor(0, 0, 0, a_int));

	for (auto i = dots.begin(); i < dots.end();) {
		if ((*i)->m_pos.y < -20 || (*i)->m_pos.y > sc.y + 20 || (*i)->m_pos.x < -20 || (*i)->m_pos.x > sc.x + 20) {
			delete (*i);
			i = dots.erase(i);
		}
		else {
			(*i)->update();
			i++;
		}
	}

	for (auto i = dots.begin(); i < dots.end(); i++) {
		(*i)->draw();
	}
}

void dot_destroy() {
	for (auto i = dots.begin(); i < dots.end(); i++) {
		delete (*i);
	}

	dots.clear();
}

void DrawRectRainbow(int x, int y, int width, int height, float flSpeed, float&
	flRainbow) {
	ImDrawList *windowDrawList = ImGui::GetWindowDrawList();
	CColor colColor(255, 255, 255, 255);

	flRainbow += flSpeed;
	if (flRainbow > 1.f) {
		flRainbow = 0.f;
	}
	for (int i = 0; i < width; i++) {
		float hue = (1.f / (float)width) *i;
		hue -= flRainbow;
		if (hue < 0.f) {
			hue += 1.f;
		}
		CColor colRainbow = colColor.HSBtoRGB(hue, 1.f, 1.f);
		if (SETTINGS::settings.rainbow_menu) {
			windowDrawList->AddRectFilled(ImVec2(x + i, y), ImVec2(width, height),
				colRainbow.GetU32());
		}
		else {
			windowDrawList->AddRectFilled(ImVec2(x + i, y), ImVec2(width, height),
				SETTINGS::settings.menu_col.GetU32());

		}
	} 
	
}

void DrawMenu()
{
	//dot_draw();

	const char* KeyStrings[] = {
		"",
		"Mouse 1",
		"Mouse 2",
		"Cancel",
		"Middle Mouse",
		"Mouse 4",
		"Mouse 5",
		"",
		"Backspace",
		"Tab",
		"",
		"",
		"Clear",
		"Enter",
		"",
		"",
		"Shift",
		"Control",
		"Alt",
		"Pause",
		"Caps",
		"",
		"",
		"",
		"",
		"",
		"",
		"Escape",
		"",
		"",
		"",
		"",
		"Space",
		"Page Up",
		"Page Down",
		"End",
		"Home",
		"Left",
		"Up",
		"Right",
		"Down",
		"",
		"",
		"",
		"Print",
		"Insert",
		"Delete",
		"",
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"A",
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
		"L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",
		"",
		"",
		"",
		"",
		"",
		"Numpad 0",
		"Numpad 1",
		"Numpad 2",
		"Numpad 3",
		"Numpad 4",
		"Numpad 5",
		"Numpad 6",
		"Numpad 7",
		"Numpad 8",
		"Numpad 9",
		"Multiply",
		"Add",
		"",
		"Subtract",
		"Decimal",
		"Divide",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",
		"F10",
		"F11",
		"F12",

	};
	const char* acc_mode[] = { "head", "body", "all" };
	const char* sound_mode[] = { "none", "Bameware", "Skeet", "Bubble" };
	const char* chams_mode[] = { "none", "visible", "invisible" };
	const char* pitchaa[] = { "none", "down", "jitter", "fake up", "zero" };
	const char* aaset[] = { "stand", "move", "air" };
	const char* aa_mode[] =
	{ "none", "Backwards", "Sideways", "bigdaddyAA", "Lowerbody", "pysenAA", "xplayAA"
	};
	const char* break_lby_mode[] =
	{ "none",
		"on bind",
		"freestanding"
	};
	const char* configs[] = { "Default", "Legit", "Autos", "Scouts", "Pistols" };
	const char* box_style[] = { "none", "full", "debug" };
	static float flRainbow;

	int curWidth = 1;
	ImVec2 curPos = ImGui::GetCursorPos();
	ImVec2 curWindowPos = ImGui::GetWindowPos();
	curPos.x += curWindowPos.x;
	curPos.y += curWindowPos.y;
	int size;
	int y;
	mopvar::Engine->GetScreenSize(y, size);
	DrawRectRainbow(curPos.x - 10, curPos.y - 7, ImGui::GetWindowSize().x + size, curPos.y + -4, SETTINGS::settings.flSpeed, flRainbow);
	static int tab_count = 0;

		const char* tabs[] = { "     Aimbot", "     Visuals", "     Misc","     Anti Aim","     Config","     Color" };

		ImGui::PushItemWidth(140);
		ImGui::ListBox("##tablist", &tab_count, tabs, ARRAYSIZE(tabs), 30);
		ImGui::PopItemWidth();

	ImGui::SameLine();
	ImGui::BeginChild("##tabs2", ImVec2(400, 0), true, 0);
	{
		

		switch (tab_count)
		{
		case 0: {
			const char* acc_type[] = {	"Head", "Body", "All" };
			ImGui::Checkbox("Activate", &SETTINGS::settings.aim_bool);
			ImGui::Combo("aimbot accuracy type", &SETTINGS::settings.acc_type, acc_type, ARRAYSIZE(acc_type));
			ImGui::Checkbox("Serversided Silent Aim", &SETTINGS::settings.reverse_bool);
			ImGui::Checkbox("Fake-lag fix", &SETTINGS::settings.fakefix_bool);
			ImGui::Checkbox("Quick Stop", &SETTINGS::settings.stop_bool);
			ImGui::SliderFloat("Hitchance", &SETTINGS::settings.chance_val, 0.f, 100.f, ("%1.f"));
			ImGui::SliderFloat("Minimum damage", &SETTINGS::settings.damage_val, 0.f, 100.f, ("%1.f"));
			
		}
		break;
		case 1: {
			ImGui::Checkbox("Enable", &SETTINGS::settings.esp_bool);
			ImGui::Combo("Draw box", &SETTINGS::settings.box_type, box_style, ARRAYSIZE(box_style));
			ImGui::Checkbox("Draw name", &SETTINGS::settings.name_bool);
			ImGui::Checkbox("Draw weapon", &SETTINGS::settings.weap_bool);
			ImGui::Checkbox("Draw info", &SETTINGS::settings.info_bool);
			ImGui::Checkbox("Draw health", &SETTINGS::settings.health_bool);
			ImGui::Combo("Chams", &SETTINGS::settings.chams_type, chams_mode, ARRAYSIZE(chams_mode));
			ImGui::Checkbox("Spread Crosshair", &SETTINGS::settings.recoil_circle);
			ImGui::Checkbox("Bullet tracers", &SETTINGS::settings.beam_bool);
			ImGui::SliderFloat("Field Of View", &SETTINGS::settings.fov, 0.f, 130.f, ("%1.f"));
			ImGui::Checkbox("Sniper Crosshair", &SETTINGS::settings.snipercross_bool);
			ImGui::Checkbox("FOV Arrows", &SETTINGS::settings.fov_bool);
			ImGui::Checkbox("Thirdperson(mouse3)", &SETTINGS::settings.tp_bool);
			ImGui::Checkbox("Night Mode", &SETTINGS::settings.night_bool);
			ImGui::Checkbox("Fake Chams", &SETTINGS::settings.drawfakechams);

		}
		break;
		case 2: {
			ImGui::Checkbox("Auto bunnyhop", &SETTINGS::settings.bhop_bool);
			ImGui::Checkbox("Auto strafer", &SETTINGS::settings.strafe_bool);
			ImGui::Checkbox("Fakelag", &SETTINGS::settings.lag_bool);
			ImGui::SliderFloat("Standing", &SETTINGS::settings.stand_lag, 0.f, 13.f);
			ImGui::SliderFloat("Moving", &SETTINGS::settings.move_lag, 0.f, 13.f);
			ImGui::SliderFloat("In Air", &SETTINGS::settings.jump_lag, 0.f, 13.f);
			ImGui::Checkbox("Gravity Ragdoll", &SETTINGS::settings.ragdoll);
			ImGui::Checkbox("Clantag", &SETTINGS::settings.clantag);
			ImGui::Combo("Hitsound", &SETTINGS::settings.hitsound, sound_mode, ARRAYSIZE(sound_mode));

		}
		break;
		case 3: {
			ImGui::Checkbox("Activate", &SETTINGS::settings.antiaim_bool);
			ImGui::Checkbox("Wall Detection", &SETTINGS::settings.walldetection);
			ImGui::NewLine();
			ImGui::Combo("Anti-aim type", &SETTINGS::settings.antiaim_type, aa_mode, ARRAYSIZE(aa_mode));
			ImGui::SliderFloat("LBY Delta", &SETTINGS::settings.delta_val, 0.f, 180.f, ("%.1f"));
		}
		break;
		case 4: {
			std::string config;
			ImGui::Combo("Config", &config_sel, configs, ARRAYSIZE(configs));


			switch (config_sel)
			{
			case 0: config = "default"; break;
			case 1: config = "legit"; break;
			case 2: config = "auto_hvh"; break;
			case 3: config = "scout_hvh"; break;
			case 4: config = "pistol_hvh"; break;
			}

			if (ImGui::Button(" Load "))
				SETTINGS::settings.Load(config);

			if (ImGui::Button(" Save "))
				SETTINGS::settings.Save(config);


			}
		break; 
		case 5: {
			ImGuiStyle * style = &ImGui::GetStyle();
			
			//Color Menu

			ImGui::Checkbox("Rainbow Menu", &SETTINGS::settings.rainbow_menu);
			if(SETTINGS::settings.rainbow_menu)
				ImGui::SliderFloat("Rainbow Speed", &SETTINGS::settings.flSpeed, 0.f, 0.010f);
			else 
			ImGui::MyColorPicker3("Menu Color", (float*)&SETTINGS::settings.menu_col);

			ImGui::Separator;

			ImGui::MyColorEdit3("Chams", (float*)&SETTINGS::settings.vmodel_col, 1 << 7);

			ImGui::MyColorEdit3("Spread Crosshair", (float*)&SETTINGS::settings.spread_col, 1 << 7);

			ImGui::MyColorEdit3("Health Bar", (float*)&SETTINGS::settings.health_col, 1 << 7);

			ImGui::MyColorEdit3("Box Color", (float*)&SETTINGS::settings.box_col, 1 << 7);

			}
		break;

		}
	}
	ImGui::EndChild();
}