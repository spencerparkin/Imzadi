#pragma once

#include <wx/frame.h>

class Canvas;

class Frame : public wxFrame
{
public:
	Frame(const wxPoint& pos, const wxSize& size);
	virtual ~Frame();

	enum
	{
		ID_Exit = wxID_HIGHEST,
		ID_About,
		ID_AddBox,
		ID_AddCapsule,
		ID_AddPolygon,
		ID_AddSphere
	};

	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnAddShape(wxCommandEvent& event);

private:
	Canvas* canvas;
};