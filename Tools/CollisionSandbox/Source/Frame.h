#pragma once

#include <wx/frame.h>
#include <wx/timer.h>

class Canvas;

class Frame : public wxFrame
{
public:
	Frame(const wxPoint& pos, const wxSize& size);
	virtual ~Frame();

	enum
	{
		ID_Exit = wxID_HIGHEST,
		ID_ClearWorld,
		ID_About,
		ID_LoadShapes,
		ID_DumpWorld,
		ID_RestoreWorld,
		ID_AddBox,
		ID_AddCapsule,
		ID_AddPolygon,
		ID_AddSphere,
		ID_LoadPolygons,
		ID_MergePolygons,
		ID_DrawBoxTree,
		ID_DrawShapes,
		ID_DrawShapeBoxes,
		ID_Timer,
	};

	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnLoadShapes(wxCommandEvent& event);
	void OnAddShape(wxCommandEvent& event);
	void OnClearWorld(wxCommandEvent& event);
	void OnDebugDrawToggle(wxCommandEvent& event);
	void OnDumpOrRestoreWorld(wxCommandEvent& event);
	void OnLoadPolygons(wxCommandEvent& event);
	void OnMergePolygons(wxCommandEvent& event);
	void OnUpdateUI(wxUpdateUIEvent& event);
	void OnTimer(wxTimerEvent& event);

	Canvas* GetCanvas() { return this->canvas; }

private:
	Canvas* canvas;
	wxTimer timer;
	bool inTimer;
};