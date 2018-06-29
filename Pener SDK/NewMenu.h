#pragma once

extern void DrawMenu();

class dot {
public:
	dot(Vector2D p, Vector2D v) {
		m_vel = v;
		m_pos = p;
	}

	void update();
	void draw();

	Vector2D m_pos, m_vel;
};

extern void dot_draw();
extern void dot_destroy();

namespace NewMenu
{
	extern bool ShouldDraw;
	class CBaseRenderable;
	class CWindow;
	class CTab;
	void InitializeMenu();
	void Render();
	void DestroyObjects();
};