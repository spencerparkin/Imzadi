#pragma once

#include <wx/frame.h>
#include <wx/timer.h>

class Canvas;
class RenderObjectList;
class RenderObjectProperties;

class Frame : public wxFrame
{
public:
	Frame(const wxPoint& pos, const wxSize& size);
	virtual ~Frame();

	enum
	{
		ID_ConvertAsset = wxID_HIGHEST,
		ID_PreviewAsset,
		ID_ClearScene,
		ID_About,
		ID_Exit,
		ID_Timer,
		ID_ShowLogWindow
	};

	Canvas* GetCanvas() { return this->canvas; }
	RenderObjectProperties* GetRenderObjectPropertiesControl() { return this->renderObjectProperties; }

protected:
	void OnConvertAsset(wxCommandEvent& event);
	void OnPreviewAsset(wxCommandEvent& event);
	void OnClearScene(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnShowLogWindow(wxCommandEvent& event);
	void OnUpdateUI(wxUpdateUIEvent& event);
	void OnCloseWindow(wxCloseEvent& event);

	Canvas* canvas;
	RenderObjectList* renderObjectList;
	RenderObjectProperties* renderObjectProperties;
	wxTimer timer;
	bool inTimer;
};