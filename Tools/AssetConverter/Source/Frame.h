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
		ID_ConvertAsset,
		ID_PreviewAsset,
		ID_ClearScene,
		ID_About,
		ID_Exit,
		ID_Timer
	};

	Canvas* GetCanvas() { return this->canvas; }

protected:
	void OnConvertAsset(wxCommandEvent& event);
	void OnPreviewAsset(wxCommandEvent& event);
	void OnClearScene(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnTimer(wxTimerEvent& event);

	Canvas* canvas;
	wxTimer timer;
	bool inTimer;
};