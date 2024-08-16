#pragma once

#include <wx/frame.h>
#include <wx/timer.h>

class Canvas;
class RenderObjectList;
class RenderObjectProperties;
class AnimationSlider;

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
		ID_ShowLogWindow,
		ID_ShowSkeleton
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
	void OnShowSkeleton(wxCommandEvent& event);
	void OnUpdateUI(wxUpdateUIEvent& event);
	void OnCloseWindow(wxCloseEvent& event);

	struct FlagChoice
	{
		wxString name;
		uint32_t flag;
	};

	bool FlagsFromDialog(const wxString& prompt, const std::vector<FlagChoice>& flagChoiceArray, uint32_t& chosenFlags);

	Canvas* canvas;
	RenderObjectList* renderObjectList;
	RenderObjectProperties* renderObjectProperties;
	AnimationSlider* animationSlider;
	wxTimer timer;
	bool inTimer;
};