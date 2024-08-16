#pragma once

#include <wx/slider.h>

class AnimationSlider : public wxSlider
{
public:
	AnimationSlider(wxWindow* parent);
	virtual ~AnimationSlider();

	void OnSliderPositionChanged(wxCommandEvent& event);
};