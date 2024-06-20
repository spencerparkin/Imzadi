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

protected:
	virtual wxString OnGetItemText(long item, long column) const override;

	void OnItemSelected(wxListEvent& event);

private:
	std::vector<Imzadi::Reference<Imzadi::RenderObject>> renderObjectArray;
};