#include "..\includes.h"

#include "hooks.h"
#include "../UTILS/mopvar.h"
#include "../UTILS/offsets.h"

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_internal.h"
#include "../ImGui/dx9/imgui_dx9.h"
#include "../ImGui/dx9/imgui_impl_dx9.h"
#include "../NewMenu.h"
#include <intrin.h>
#include "../SDK/RenderView.h"
#include "../SDK/CInput.h"
#include "../SDK/IClient.h"
#include "../SDK/CPanel.h"
#include "../UTILS/render.h"
#include "../SDK/ConVar.h"
#include "../SDK/CGlowObjectManager.h"
#include "../SDK/IEngine.h"
#include "../SDK/CTrace.h"
#include "../SDK/CClientEntityList.h"
#include "../SDK/RecvData.h"
#include "../UTILS/NetvarHookManager.h"
#include "../SDK/ModelInfo.h"
#include "../SDK/ModelRender.h"
#include "../SDK/RenderView.h"
#include "../SDK/CTrace.h"
#include "../SDK/CViewSetup.h"
#include "../SDK/CGlobalVars.h"

#include "../FEATURES/Movement.h"
#include "../FEATURES/Visuals.h"
#include "../FEATURES/Chams.h"
#include "../FEATURES/AntiAim.h"
#include "../FEATURES/Aimbot.h"
#include "../FEATURES/Resolver.h"
#include "../FEATURES/Backtracking.h"
#include "../FEATURES/FakeWalk.h"
#include "../FEATURES/FakeLag.h"


#define STUDIO_NONE						0x00000000
#define STUDIO_RENDER					0x00000001
#define STUDIO_VIEWXFORMATTACHMENTS		0x00000002
#define STUDIO_DRAWTRANSLUCENTSUBMODELS 0x00000004
#define STUDIO_TWOPASS					0x00000008
#define STUDIO_STATIC_LIGHTING			0x00000010
#define STUDIO_WIREFRAME				0x00000020
#define STUDIO_ITEM_BLINK				0x00000040
#define STUDIO_NOSHADOWS				0x00000080
#define STUDIO_WIREFRAME_VCOLLIDE		0x00000100
#define STUDIO_NOLIGHTING_OR_CUBEMAP	0x00000200
#define STUDIO_SKIP_FLEXES				0x00000400
#define STUDIO_DONOTMODIFYSTENCILSTATE	0x00000800	// TERROR

// Not a studio flag, but used to flag model as a non-sorting brush model
#define STUDIO_TRANSPARENCY				0x80000000

// Not a studio flag, but used to flag model as using shadow depth material override
#define STUDIO_SHADOWDEPTHTEXTURE		0x40000000

// Not a studio flag, but used to flag model as doing custom rendering into shadow texture
#define STUDIO_SHADOWTEXTURE			0x20000000

#define STUDIO_SKIP_DECALS				0x10000000

//#include "../MENU/menu_framework.h"

#include <intrin.h>

//--- Other Globally Used Variables ---///
static bool tick = false;

//--- Declare Signatures and Patterns Here ---///
static auto CAM_THINK = UTILS::FindSignature("client.dll", "85 C0 75 30 38 86");
//SDK::CGlowObjectManager* pGlowObjectManager = (SDK::CGlowObjectManager*)(UTILS::FindSignature("client.dll", "0F 11 05 ? ? ? ? 83 C8 01") + 0x3);
static auto linegoesthrusmoke = UTILS::FindPattern("client.dll", (PBYTE)"\x55\x8B\xEC\x83\xEC\x08\x8B\x15\x00\x00\x00\x00\x0F\x57\xC0", "xxxxxxxx????xxx");

namespace HOOKS
{
	
	CreateMoveFn original_create_move;
	PaintTraverseFn original_paint_traverse;
	FrameStageNotifyFn original_frame_stage_notify;
	DrawModelExecuteFn original_draw_model_execute;
	SceneEndFn original_scene_end;
	TraceRayFn original_trace_ray;
	EndSceneResetFn oEndSceneReset;
	EndSceneFn oEndScene;
	SendDatagramFn original_send_datagram;
	OverrideViewFn original_override_view;
	//ToFirstPersonFn original_to_firstperson;
	SvCheatsGetBoolFn original_get_bool;

	VMT::VMTHookManager iclient_hook_manager;
	VMT::VMTHookManager panel_hook_manager;
	VMT::VMTHookManager model_render_hook_manager;
	VMT::VMTHookManager render_view_hook_manager;
	VMT::VMTHookManager trace_hook_manager;
	VMT::VMTHookManager net_channel_hook_manager;
	VMT::VMTHookManager override_view_hook_manager;
	VMT::VMTHookManager input_table_manager;
	VMT::VMTHookManager get_bool_manager;
	VMT::VMTHookManager direct;
	void marquee(std::string& panicova_zlomena_noha)
	{
		std::string temp_string = panicova_zlomena_noha;
		panicova_zlomena_noha.erase(0, 1);
		panicova_zlomena_noha += temp_string[0];
	}

	bool __stdcall HookedCreateMove(float sample_input_frametime, SDK::CUserCmd* cmd)
	{



		if (!cmd || cmd->command_number == 0)
			return false;

		uintptr_t* FPointer; __asm { MOV FPointer, EBP }
		byte* SendPacket = (byte*)(*FPointer - 0x1C);

		if (!SendPacket)
			return false;

		GLOBAL::should_send_packet = *SendPacket;
		// S T A R T

		if (mopvar::Engine->IsConnected() && mopvar::Engine->IsInGame())
		{
			if (SETTINGS::settings.aim_type == 0)
				slidebitch->do_fakewalk(cmd);

			if (SETTINGS::settings.bhop_bool)
				movement->bunnyhop(cmd);

			if (SETTINGS::settings.strafe_bool)
				movement->autostrafer(cmd);

			for (int i = 1; i <= 65; i++)
			{
				auto entity = mopvar::ClientEntityList->GetClientEntity(i);

				if (!entity)
					continue;

				auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

				if (!local_player)
					return;

				bool is_local_player = entity == local_player;
				bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;

				if (is_local_player)
					continue;

				if (is_teammate)
					continue;

				if (entity->GetHealth() <= 0)
					continue;

				if (entity->GetIsDormant())
					continue;

				if (SETTINGS::settings.stop_bool)
					movement->quick_stop(entity, cmd);
			}



			if (SETTINGS::settings.clantag)
			{
				//Wirefra.me clantag(nonlaggy xD)
				int iLastTime;

				if (int(mopvar::Globals->curtime) != iLastTime)
				{

					static std::string cur_clantag = "    Wirefra.me    ";
					static int old_time;

					static int i = 0;

					if (i > 32)
					{
						marquee(cur_clantag);
						//	SETTINGS::settings.namemenu = cur_clantag.c_str();
						setclantag(cur_clantag.c_str());
						i = 0;
					}
					else
					{
						i++;
					}
				}

				iLastTime = int(mopvar::Globals->curtime);
			}

			if (SETTINGS::settings.aim_type == 0)
			{
				aimbot->shoot_enemy(cmd);
				aimbot->fix_recoil(cmd);
				backtracking->backtrack_player(cmd);
				fakelag->do_fakelag();
			}

			if (SETTINGS::settings.aim_type == 1 && SETTINGS::settings.back_bool)
				backtracking->run_legit(cmd);

			if (SETTINGS::settings.antiaim_bool || SETTINGS::settings.antiaim_type > 0 || SETTINGS::settings.aim_type == 0)
			{
				antiaim->do_antiaim(cmd);
				antiaim->fix_movement(cmd);
			}

			//if (!GLOBAL::should_send_packet)
				//GLOBAL::real_angles = cmd->viewangles;

			if (SETTINGS::settings.ragdoll)
			{
				auto mat4 = mopvar::cvar->FindVar("cl_ragdoll_gravity");
				mat4->SetValue(0);
			}
			else {
				auto mat4 = mopvar::cvar->FindVar("cl_ragdoll_gravity");
				mat4->SetValue(0);
			}
		}


		// E N D
		*SendPacket = GLOBAL::should_send_packet;

		//UTILS::ClampLemon(cmd->viewangles);

		return false;
	}
	void __stdcall HookedPaintTraverse(int VGUIPanel, bool ForceRepaint, bool AllowForce)
	{
		std::string panel_name = mopvar::Panel->GetName(VGUIPanel);

		if (panel_name == "HudZoom")
			return;

		if (panel_name == "MatSystemTopPanel")
		{
			if (FONTS::ShouldReloadFonts())
				FONTS::InitFonts();

			if (mopvar::Engine->IsConnected() && mopvar::Engine->IsInGame())
			{
				if (SETTINGS::settings.esp_bool)
				{
					visuals->Draw();
					visuals->ClientDraw();
				}
			}

		//	MENU::PPGUI_PP_GUI::Begin();
			//ENU::Do();
			//MENU::PPGUI_PP_GUI::End();

			UTILS::INPUT::input_handler.Update();

			RENDER::DrawSomething();
		}

		original_paint_traverse(mopvar::Panel, VGUIPanel, ForceRepaint, AllowForce);
	}

	static bool menu_open = false;
	static bool d3d_init = false;
	bool PressedKeys[256] = {};
	void OpenMenu()
	{
		static bool is_down = false;
		static bool is_clicked = false;

		if (GetAsyncKeyState(VK_INSERT))
		{
			is_clicked = false;
			is_down = true;
		}
		else if (!GetAsyncKeyState(VK_INSERT) && is_down)
		{
			is_clicked = true;
			is_down = false;
		}
		else
		{
			is_clicked = false;
			is_down = false;
		}

		if (is_clicked)
		{
			menu_open = !menu_open;
			std::string msg = "cl_mouseenable " + std::to_string(!menu_open);
			mopvar::Engine->ClientCmd(msg.c_str());
		}
	}
	namespace INIT
	{
		HMODULE Dll;
		HWND Window;
		WNDPROC OldWindow;
	}
	LRESULT ImGui_ImplDX9_WndProcHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		ImGuiIO& io = ImGui::GetIO();
		switch (msg) {
		case WM_LBUTTONDOWN:
			io.MouseDown[0] = true;
			return true;
		case WM_LBUTTONUP:
			io.MouseDown[0] = false;
			return true;
		case WM_RBUTTONDOWN:
			io.MouseDown[1] = true;
			return true;
		case WM_RBUTTONUP:
			io.MouseDown[1] = false;
			return true;
		case WM_MBUTTONDOWN:
			io.MouseDown[2] = true;
			return true;
		case WM_MBUTTONUP:
			io.MouseDown[2] = false;
			return true;
		case WM_XBUTTONDOWN:
			if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) == MK_XBUTTON1)
				io.MouseDown[3] = true;
			else if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) == MK_XBUTTON2)
				io.MouseDown[4] = true;
			return true;
		case WM_XBUTTONUP:
			if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) == MK_XBUTTON1)
				io.MouseDown[3] = false;
			else if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) == MK_XBUTTON2)
				io.MouseDown[4] = false;
			return true;
		case WM_MOUSEWHEEL:
			io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
			return true;
		case WM_MOUSEMOVE:
			io.MousePos.x = (signed short)(lParam);
			io.MousePos.y = (signed short)(lParam >> 16);
			return true;
		case WM_KEYDOWN:
			if (wParam < 256)
				io.KeysDown[wParam] = 1;
			return true;
		case WM_KEYUP:
			if (wParam < 256)
				io.KeysDown[wParam] = 0;
			return true;
		case WM_CHAR:
			// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
			if (wParam > 0 && wParam < 0x10000)
				io.AddInputCharacter((unsigned short)wParam);
			return true;
		}
		return 0;
	}
	LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg) {
		case WM_LBUTTONDOWN:
			PressedKeys[VK_LBUTTON] = true;
			break;
		case WM_LBUTTONUP:
			PressedKeys[VK_LBUTTON] = false;
			break;
		case WM_RBUTTONDOWN:
			PressedKeys[VK_RBUTTON] = true;
			break;
		case WM_RBUTTONUP:
			PressedKeys[VK_RBUTTON] = false;
			break;
		case WM_MBUTTONDOWN:
			PressedKeys[VK_MBUTTON] = true;
			break;
		case WM_MBUTTONUP:
			PressedKeys[VK_MBUTTON] = false;
			break;
		case WM_XBUTTONDOWN:
		{
			UINT button = GET_XBUTTON_WPARAM(wParam);
			if (button == XBUTTON1)
			{
				PressedKeys[VK_XBUTTON1] = true;
			}
			else if (button == XBUTTON2)
			{
				PressedKeys[VK_XBUTTON2] = true;
			}
			break;
		}
		case WM_XBUTTONUP:
		{
			UINT button = GET_XBUTTON_WPARAM(wParam);
			if (button == XBUTTON1)
			{
				PressedKeys[VK_XBUTTON1] = false;
			}
			else if (button == XBUTTON2)
			{
				PressedKeys[VK_XBUTTON2] = false;
			}
			break;
		}
		case WM_KEYDOWN:
			PressedKeys[wParam] = true;
			break;
		case WM_KEYUP:
			PressedKeys[wParam] = false;
			break;
		default: break;
		}

		OpenMenu();

		if (d3d_init && menu_open && ImGui_ImplDX9_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;

		return CallWindowProc(INIT::OldWindow, hWnd, uMsg, wParam, lParam);
	}

	void GUI_Init(IDirect3DDevice9* pDevice)
	{
		ImGui_ImplDX9_Init(INIT::Window, pDevice);


		ImGuiStyle * style = &ImGui::GetStyle();

		style->WindowPadding = ImVec2(15, 15);
		style->WindowRounding = 5.0f;
		style->FramePadding = ImVec2(5, 5);
		style->FrameRounding = 4.0f;
		style->ItemSpacing = ImVec2(12, 8);
		style->ItemInnerSpacing = ImVec2(8, 6);
		style->IndentSpacing = 25.0f;
		style->ScrollbarSize = 15.0f;
		style->ScrollbarRounding = 9.0f;
		style->GrabMinSize = 5.0f;
		style->GrabRounding = 3.0f;

		style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_Border] = ImVec4(0.07f, 0.07f, 0.09f, 0.88f);
		style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
		style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
		style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
		style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
		style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
		style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
		style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
		style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
		style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

		d3d_init = true;
	}
	float clip(float n, float lower, float upper)
	{
		n = (n > lower) * n + !(n > lower) * lower;
		return (n < upper) * n + !(n < upper) * upper;
	}
	long __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice)
	{
		D3DCOLOR rectColor = D3DCOLOR_XRGB(255, 0, 0);
		D3DRECT BarRect = { 1, 1, 1, 1 };

		pDevice->Clear(1, &BarRect, D3DCLEAR_TARGET | D3DCLEAR_TARGET, rectColor, 0, 0);

		if (!d3d_init)
			GUI_Init(pDevice);

		ImGui::GetIO().MouseDrawCursor = menu_open;

		ImGui_ImplDX9_NewFrame();

		POINT mp;

		GetCursorPos(&mp);

		ImGuiIO& io = ImGui::GetIO();

		io.MousePos.x = mp.x;
		io.MousePos.y = mp.y;
		static float flAlpha;
		if (menu_open)
		{
			
			ImGuiStyle * style = &ImGui::GetStyle();
			
			ImGuiIO& io = ImGui::GetIO();
			static constexpr auto frequency = 1 / 0.32f;
			flAlpha = clip(flAlpha + frequency * io.DeltaTime, 0.f, 1.f);
			style->Alpha = flAlpha;
			ImGui::SetNextWindowSize(ImVec2(680, 550));
			ImGui::Begin("", &menu_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
			{
				DrawMenu();
			}
			ImGui::End();
		}
		else
		{
			//ImGuiStyle * style = &ImGui::GetStyle();
			flAlpha = 0;
		}
		ImGui::Render();

		return oEndScene(pDevice);
	}

	long __stdcall Hooked_EndScene_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		if (!d3d_init)
			return oEndSceneReset(pDevice, pPresentationParameters);

		ImGui_ImplDX9_InvalidateDeviceObjects();

		auto hr = oEndSceneReset(pDevice, pPresentationParameters);

		ImGui_ImplDX9_CreateDeviceObjects();

		return hr;
	}
	void __fastcall HookedFrameStageNotify(void* ecx, void* edx, int stage)
	{

		switch (stage)
		{
		case FRAME_NET_UPDATE_POSTDATAUPDATE_START:

			//--- Angle Resolving and Such ---//
			if (mopvar::Engine->IsConnected() && mopvar::Engine->IsInGame())
			{
				for (int i = 1; i < 65; i++)
				{
					auto entity = mopvar::ClientEntityList->GetClientEntity(i);
					auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

					if (!entity)
						continue;

					if (!local_player)
						continue;

					bool is_local_player = entity == local_player;
					bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;

					if (is_local_player)
						continue;

					if (is_teammate)
						continue;

					if (entity->GetHealth() <= 0)
						continue;

					//backtracking->DisableInterpolation(entity);

					if (SETTINGS::settings.aim_type == 0)
						resolver->resolve(entity);
				}
			}
			break;

		case FRAME_NET_UPDATE_POSTDATAUPDATE_END:

			break;

		case FRAME_RENDER_START:

			if (mopvar::Engine->IsConnected() && mopvar::Engine->IsInGame())
			{
				auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

				if (!local_player)
					return;

				//--- Thirdperson Deadflag Stuff ---//
				if (in_tp)
					*(Vector*)((DWORD)local_player + 0x31C8) = GLOBAL::real_angles;
			}
			break;

			//static Vector oldViewPunch;
			//static Vector oldAimPunch;
			//auto pLocal = reinterpret_cast<SDK::CBaseEntity*>(mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer()));

		//	Vector* view_punch = pLocal->GetViewPunchPtr();
		//	Vector* aim_punch = pLocal->GetPunchAnglePtr();
		case FRAME_NET_UPDATE_START:
			if (mopvar::Engine->IsConnected() && mopvar::Engine->IsInGame())
			{
			/*	if (view_punch && aim_punch && SETTINGS::settings.novisrecoil) {
					oldViewPunch = *view_punch;
					oldAimPunch = *aim_punch;

					view_punch->Init();
					aim_punch->Init();
				}*/


				if (SETTINGS::settings.snipercross_bool) {
				visuals->DrawSniperCrosshair();
				mopvar::Engine->ClientCmd_Unrestricted("crosshair 1");

				}else
				mopvar::Engine->ClientCmd_Unrestricted("crosshair 0");

					


				if (SETTINGS::settings.beam_bool)
					visuals->DrawBulletBeams();
			}

			break;
		case FRAME_NET_UPDATE_END:

			if (mopvar::Engine->IsConnected() && mopvar::Engine->IsInGame())
			{
				for (int i = 1; i < 65; i++)
				{
					auto entity = mopvar::ClientEntityList->GetClientEntity(i);
					auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

					if (!entity)
						continue;

					if (!local_player)
						continue;

					bool is_local_player = entity == local_player;
					bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;

					if (is_local_player)
						continue;

					if (is_teammate)
						continue;

					if (entity->GetHealth() <= 0)
						continue;

					if (SETTINGS::settings.aim_type == 0)
						backtracking->DisableInterpolation(entity);
				}
			}

			break;
		}

		original_frame_stage_notify(ecx, stage);
	}
	void __fastcall HookedDrawModelExecute(void* ecx, void* edx, SDK::IMatRenderContext* context, const SDK::DrawModelState_t& state, const SDK::ModelRenderInfo_t& render_info, matrix3x4_t* matrix)
	{
		if (mopvar::Engine->IsConnected() && mopvar::Engine->IsInGame())
		{
			for (int i = 1; i < 65; i++)
			{
				auto entity = mopvar::ClientEntityList->GetClientEntity(i);
				auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

				if (!entity)
					continue;

				if (!local_player)
					continue;

				if (entity && entity->GetIsScoped() && in_tp && entity == local_player)
				{
					mopvar::RenderView->SetBlend(0.4);
				}
			}
		}

		std::string strModelName = mopvar::ModelInfo->GetModelName(render_info.pModel);


		

		if (SETTINGS::settings.wirehands)
		{
			if (strModelName.find("arms") != std::string::npos && SETTINGS::settings.wirehands)
			{
				SDK::IMaterial* WireHands = mopvar::MaterialSystem->FindMaterial(strModelName.c_str(), TEXTURE_GROUP_MODEL);
				WireHands->SetMaterialVarFlag(SDK::MATERIAL_VAR_WIREFRAME, true);
				mopvar::ModelRender->ForcedMaterialOverride(WireHands);
			}
		}
		else {
			SDK::IMaterial* Hands = mopvar::MaterialSystem->FindMaterial(strModelName.c_str(), TEXTURE_GROUP_MODEL);
			Hands->SetMaterialVarFlag(SDK::MATERIAL_VAR_WIREFRAME, false);
		}

		original_draw_model_execute(ecx, context, state, render_info, matrix);
	}
	void __fastcall HookedSceneEnd(void* ecx, void* edx)
	{
		original_scene_end(ecx);

		static SDK::IMaterial* ignorez = chams->CreateMaterial(true, true, false);
		static SDK::IMaterial* notignorez = chams->CreateMaterial(false, true, false);

		CColor color;
		color = SETTINGS::settings.glow_col;


		
		if (mopvar::Engine->IsConnected() && mopvar::Engine->IsInGame())
		{
			if (SETTINGS::settings.drawfakechams)
			{
				auto pLocal = reinterpret_cast<SDK::CBaseEntity*>(mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer()));
				if (pLocal)
				{

					static SDK::IMaterial* mat = chams->CreateMaterial(false, true, true);
					if (mat)
					{
						typedef Vector Vector3D;
						Vector3D OrigAng;
						OrigAng = pLocal->GetEyeAngles();
						pLocal->SetAngle2(Vector(0, GLOBAL::fake_angles.y, 0)); // paste he  re ur AA.y value or pLocal->GetLby() (for example)
						bool LbyColor = false; // u can make LBY INDICATOR. When LbyColor is true. Color will be Green , if false it will be White
											   /*CColor NormalColor = CColor( 1, 1, 1 );
											   CColor lbyUpdateColor = CColor(0, 1, 0);*/
											   //	SetColorModulation(LbyColor ? lbyUpdateColor : NormalColor);
						mat->ColorModulate(255, 255, 255);
						mopvar::ModelRender->ForcedMaterialOverride(mat);
						pLocal->DrawModel(STUDIO_RENDER, 150);
						mopvar::ModelRender->ForcedMaterialOverride(nullptr);
						pLocal->SetAngle2(OrigAng);
					}
				}
			}

			for (auto i = 0; i < mopvar::GlowObjManager->GetSize(); i++)
			{
				auto &glowObject = mopvar::GlowObjManager->m_GlowObjectDefinitions[i];
				auto entity = reinterpret_cast<SDK::CBaseEntity*>(glowObject.m_pEntity);
				auto m_pLocalPlayer = reinterpret_cast<SDK::CBaseEntity*>(mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer()));

				if (!entity)
					continue;


				if (glowObject.IsUnused())
					continue;

				bool is_local_player = entity == m_pLocalPlayer;
				bool is_teammate = m_pLocalPlayer->GetTeam() == entity->GetTeam() && !is_local_player;



				if (!SETTINGS::settings.glow_bool)
					continue;

				if (is_teammate)
					continue;


				auto class_id = entity->GetClientClass()->m_ClassID;


				switch (class_id)
				{
				default:
					glowObject.m_flAlpha = 0.f;
					break;
				case 35:
					if (SETTINGS::settings.glow_style == 1)
					{
						glowObject.m_nGlowStyle = 0;
					}
					else if (SETTINGS::settings.glow_style == 2)
					{
						glowObject.m_nGlowStyle = 1;

					}
					glowObject.m_flAlpha = SETTINGS::settings.alphaglow;
					break;
				}




				glowObject.m_flRed = SETTINGS::settings.slider1;
				glowObject.m_flGreen = SETTINGS::settings.slider2;
				glowObject.m_flBlue = SETTINGS::settings.slider3;
				glowObject.m_bRenderWhenOccluded = true;
				glowObject.m_bRenderWhenUnoccluded = false;
			}

			for (int i = 1; i < 65; i++)
			{
				auto entity = mopvar::ClientEntityList->GetClientEntity(i);
				auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

				if (!entity)
					continue;

				if (!local_player)
					continue;

				bool is_local_player = entity == local_player;
				bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;

				if (is_local_player)
					continue;

				if (is_teammate)
					continue;

				//--- Colored Models ---//

				if (entity && SETTINGS::settings.chams_type == 2)
				{
					ignorez->ColorModulate(SETTINGS::settings.imodel_col); //255, 40, 200
					mopvar::ModelRender->ForcedMaterialOverride(ignorez);
					entity->DrawModel(0x1, 255);
					notignorez->ColorModulate(SETTINGS::settings.vmodel_col); //0, 125, 255
					mopvar::ModelRender->ForcedMaterialOverride(notignorez);
					entity->DrawModel(0x1, 255);
					mopvar::ModelRender->ForcedMaterialOverride(nullptr);
				}
				else if (entity && SETTINGS::settings.chams_type == 1)
				{
					notignorez->ColorModulate(SETTINGS::settings.vmodel_col); //255, 40, 200
					mopvar::ModelRender->ForcedMaterialOverride(notignorez);
					entity->DrawModel(0x1, 255);
					mopvar::ModelRender->ForcedMaterialOverride(nullptr);
				}
			}


			//--- Wireframe Smoke ---//
			std::vector<const char*> vistasmoke_wireframe =
			{
				"particle/vistasmokev1/vistasmokev1_smokegrenade",
			};

			std::vector<const char*> vistasmoke_nodraw =
			{
				"particle/vistasmokev1/vistasmokev1_fire",
				"particle/vistasmokev1/vistasmokev1_emods",
				"particle/vistasmokev1/vistasmokev1_emods_impactdust",
			};

			for (auto mat_s : vistasmoke_wireframe)
			{
				SDK::IMaterial* mat = mopvar::MaterialSystem->FindMaterial(mat_s, TEXTURE_GROUP_OTHER);
				mat->SetMaterialVarFlag(SDK::MATERIAL_VAR_WIREFRAME, true); //wireframe
			}

			for (auto mat_n : vistasmoke_nodraw)
			{
				SDK::IMaterial* mat = mopvar::MaterialSystem->FindMaterial(mat_n, TEXTURE_GROUP_OTHER);
				mat->SetMaterialVarFlag(SDK::MATERIAL_VAR_NO_DRAW, true);
			}

			static auto smokecout = *(DWORD*)(linegoesthrusmoke + 0x8);
			*(int*)(smokecout) = 0;

			//--- Entity Glow ---//
			//if (!pGlowObjectManager)
				//return;

			/*for (int i = 0; i < pGlowObjectManager->GetSize(); ++i)
			{
				auto& glowObject = pGlowObjectManager->m_GlowObjectDefinitions[i];
				auto glowEntity = reinterpret_cast<SDK::CBaseEntity*>(glowObject.get_entity());

				if (glowObject.IsUnused())
					continue;

				if (!glowEntity)
					continue;

				auto class_id = glowEntity->GetClientClass()->m_ClassID;

				switch (class_id)
				{
				default:
					break;
				case 35:
					glowObject.red = 255.f / 255.0f;
					glowObject.green = 255.f / 255.0f;
					glowObject.blue = 255.f / 255.0f;
					break;
				}
				glowObject.alpha = 0.7f;
				glowObject.renderWhenOccluded = true;
				glowObject.renderWhenUnoccluded = false;
			}*/
		}
	}
	void __fastcall HookedOverrideView(void* ecx, void* edx, SDK::CViewSetup* pSetup)
	{
		auto local_player = mopvar::ClientEntityList->GetClientEntity(mopvar::Engine->GetLocalPlayer());

		if (!local_player)
			return;

		if (GetAsyncKeyState(VK_MBUTTON) & 1)
			in_tp = !in_tp;

		//--- Actual Thirdperson Stuff ---//
		if (mopvar::Engine->IsConnected() && mopvar::Engine->IsInGame())
		{
			//auto svcheats = mopvar::cvar->FindVar("sv_cheats");
			//auto svcheatsspoof = new SDK::SpoofedConvar(svcheats);
			//svcheatsspoof->SetInt(1);

			auto GetCorrectDistance = [&local_player](float ideal_distance) -> float
			{
				Vector inverse_angles;
				mopvar::Engine->GetViewAngles(inverse_angles);

				inverse_angles.x *= -1.f, inverse_angles.y += 180.f;

				Vector direction;
				MATH::AngleVectors(inverse_angles, &direction);

				SDK::CTraceWorldOnly filter;
				SDK::trace_t trace;
				SDK::Ray_t ray;

				ray.Init(local_player->GetVecOrigin() + local_player->GetViewOffset(), (local_player->GetVecOrigin() + local_player->GetViewOffset()) + (direction * (ideal_distance + 5.f)));
				mopvar::Trace->TraceRay(ray, MASK_ALL, &filter, &trace);

				return ideal_distance * trace.flFraction;
			};

			if (SETTINGS::settings.tp_bool && in_tp)
			{
				if (local_player->GetHealth() <= 0)
					local_player->SetObserverMode(5);

				if (!mopvar::Input->m_fCameraInThirdPerson)
				{
					mopvar::Input->m_fCameraInThirdPerson = true;
					mopvar::Input->m_vecCameraOffset = Vector(GLOBAL::real_angles.x, GLOBAL::real_angles.y, GetCorrectDistance(100));
					Vector camForward;

					MATH::AngleVectors(Vector(mopvar::Input->m_vecCameraOffset.x, mopvar::Input->m_vecCameraOffset.y, 0), &camForward);
				}
			}
			else
			{
				mopvar::Input->m_fCameraInThirdPerson = false;
				mopvar::Input->m_vecCameraOffset = Vector(GLOBAL::real_angles.x, GLOBAL::real_angles.y, 0);
			}

			if (!local_player->GetIsScoped())
			{
				pSetup->fov = SETTINGS::settings.fov;
			}
		}
		original_override_view(ecx, pSetup);
	}
	void __fastcall HookedTraceRay(void *thisptr, void*, const SDK::Ray_t &ray, unsigned int fMask, SDK::ITraceFilter *pTraceFilter, SDK::trace_t *pTrace)
	{
		original_trace_ray(thisptr, ray, fMask, pTraceFilter, pTrace);
		pTrace->surface.flags |= SURF_SKY;
	}
	void __fastcall HookedSendDatagram(void* ecx, void* data)
	{
		original_send_datagram(ecx, data);
	}
	bool __fastcall HookedGetBool(void* pConVar, void* edx)
	{
		if ((uintptr_t)_ReturnAddress() == CAM_THINK)
			return true;

		return original_get_bool(pConVar);
	}
	void InitHooks()
	{
		mopvar::Engine->ClientCmd_Unrestricted("clear");
		Sleep(100);

		mopvar::Engine->ClientCmd_Unrestricted("toggleconsole");
		mopvar::cvar->ConsoleColorPrintf(CColor(255, 255, 255, 255), ("Wirefra."));
		mopvar::cvar->ConsoleColorPrintf(CColor(131, 175, 248, 255), ("me "));
		mopvar::cvar->ConsoleColorPrintf(CColor(255, 255, 255, 255), ("successfully injected.\n\n"));



		while (!(INIT::Window = FindWindowA("Valve001", nullptr)))
			Sleep(100);
		if (INIT::Window)
			INIT::OldWindow = (WNDPROC)SetWindowLongPtr(INIT::Window, GWL_WNDPROC, (LONG_PTR)Hooked_WndProc);

		DWORD DeviceStructureAddress = **(DWORD**)(UTILS::FindSignature("shaderapidx9.dll", "A1 ?? ?? ?? ?? 50 8B 08 FF 51 0C") + 1);
		if (DeviceStructureAddress) {
			direct.Init((DWORD**)DeviceStructureAddress);
			oEndSceneReset = reinterpret_cast<EndSceneResetFn>(direct.HookFunction<EndSceneResetFn>(16, Hooked_EndScene_Reset));
			oEndScene = reinterpret_cast<EndSceneFn>(direct.HookFunction<EndSceneFn>(42, Hooked_EndScene));
		}

		iclient_hook_manager.Init(mopvar::Client);
		original_frame_stage_notify = reinterpret_cast<FrameStageNotifyFn>(
			iclient_hook_manager.HookFunction<FrameStageNotifyFn>(36, HookedFrameStageNotify));
		//iclient_hook_manager.HookTable(true);

		panel_hook_manager.Init(mopvar::Panel);
		original_paint_traverse = reinterpret_cast<PaintTraverseFn>(
			panel_hook_manager.HookFunction<PaintTraverseFn>(41, HookedPaintTraverse));
		//panel_hook_manager.HookTable(true);

		model_render_hook_manager.Init(mopvar::ModelRender);
		original_draw_model_execute = reinterpret_cast<DrawModelExecuteFn>(model_render_hook_manager.HookFunction<DrawModelExecuteFn>(21, HookedDrawModelExecute));
		//model_render_hook_manager.HookTable(true);

		render_view_hook_manager.Init(mopvar::RenderView);
		original_scene_end = reinterpret_cast<SceneEndFn>(render_view_hook_manager.HookFunction<SceneEndFn>(9, HookedSceneEnd));
		//render_view_hook_manager.HookTable(true);

		trace_hook_manager.Init(mopvar::Trace);
		original_trace_ray = reinterpret_cast<TraceRayFn>(trace_hook_manager.HookFunction<TraceRayFn>(5, HookedTraceRay));
		//trace_hook_manager.HookTable(true);

		override_view_hook_manager.Init(mopvar::ClientMode);
		original_override_view = reinterpret_cast<OverrideViewFn>(override_view_hook_manager.HookFunction<OverrideViewFn>(18, HookedOverrideView));
		original_create_move = reinterpret_cast<CreateMoveFn>(override_view_hook_manager.HookFunction<CreateMoveFn>(24, HookedCreateMove));
		//override_view_hook_manager.HookTable(true);

		/*input_table_manager = VMT::CVMTHookManager(mopvar::Input);
		original_to_firstperson = reinterpret_cast<ToFirstPersonFn>(input_table_manager.HookFunction(36, HookedToFirstPerson));
		input_table_manager.HookTable(true);*/

		auto sv_cheats = mopvar::cvar->FindVar("sv_cheats");
		get_bool_manager = VMT::VMTHookManager(reinterpret_cast<DWORD**>(sv_cheats));
		original_get_bool = reinterpret_cast<SvCheatsGetBoolFn>(get_bool_manager.HookFunction<SvCheatsGetBoolFn>(13, HookedGetBool));

	

		}

	void EyeAnglesPitchHook(const SDK::CRecvProxyData *pData, void *pStruct, void *pOut)
	{
		*reinterpret_cast<float*>(pOut) = pData->m_Value.m_Float;

		auto entity = reinterpret_cast<SDK::CBaseEntity*>(pStruct);
		if (!entity)
			return;

	}
	void EyeAnglesYawHook(const SDK::CRecvProxyData *pData, void *pStruct, void *pOut)
	{
		*reinterpret_cast<float*>(pOut) = pData->m_Value.m_Float;

		auto entity = reinterpret_cast<SDK::CBaseEntity*>(pStruct);
		if (!entity)
			return;

		//resolver->record(entity, pData->m_Value.m_Float);
	}

	void InitNetvarHooks()
	{
		UTILS::netvar_hook_manager.Hook("DT_CSPlayer", "m_angEyeAngles[0]", EyeAnglesPitchHook);
		UTILS::netvar_hook_manager.Hook("DT_CSPlayer", "m_angEyeAngles[1]", EyeAnglesYawHook);
	}
}