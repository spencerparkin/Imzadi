#pragma once

#include <wx/window.h>

class Canvas : public wxWindow
{
public:
	Canvas(wxWindow* parent);
	virtual ~Canvas();

	void OnSize(wxSizeEvent& event);
};