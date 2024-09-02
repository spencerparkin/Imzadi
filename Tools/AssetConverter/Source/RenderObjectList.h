#pragma once

#include <wx/listctrl.h>
#include <vector>
#include "Scene.h"

class RenderObjectList : public wxListCtrl
{
public:
	RenderObjectList(wxWindow* parent);
	virtual ~RenderObjectList();

	void AddRenderObject(Imzadi::RenderObject* renderObject);
	void Clear();
	void UpdateListView();

	enum
	{
		ID_ContextMenu_PlayAnimation,
		ID_ContextMenu_DrawPorts
	};

protected:
	virtual wxString OnGetItemText(long item, long column) const override;

	void OnItemSelected(wxListEvent& event);
	void OnItemRightClicked(wxListEvent& event);
	void OnPlayAnimation(wxCommandEvent& event);
	void OnDrawPorts(wxCommandEvent& event);
	void OnUpdateUI(wxUpdateUIEvent& event);

private:
	std::vector<Imzadi::Reference<Imzadi::RenderObject>> renderObjectArray;
	long contextMenuItem;
};