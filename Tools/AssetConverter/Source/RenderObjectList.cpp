#include "RenderObjectList.h"
#include "RenderObjects/RenderMeshInstance.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "App.h"
#include "Frame.h"
#include "RenderObjectProperties.h"
#include <format>

RenderObjectList::RenderObjectList(wxWindow* parent) : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_VIRTUAL | wxLC_REPORT)
{
	this->AppendColumn("Render Object", wxLIST_FORMAT_LEFT, 120);
	this->AppendColumn("Type", wxLIST_FORMAT_LEFT, 120);

	this->Bind(wxEVT_LIST_ITEM_SELECTED, &RenderObjectList::OnItemSelected, this);
}

/*virtual*/ RenderObjectList::~RenderObjectList()
{
}

void RenderObjectList::OnItemSelected(wxListEvent& event)
{
	long item = event.GetIndex();

	if (0 <= item && item < this->renderObjectArray.size())
	{
		Imzadi::RenderObject* renderObject = this->renderObjectArray[item];
		RenderObjectProperties* propertiesControl = wxGetApp().GetFrame()->GetRenderObjectPropertiesControl();
		propertiesControl->PrintPropertiesOf(renderObject);
	}
}

void RenderObjectList::AddRenderObject(Imzadi::RenderObject* renderObject)
{
	this->renderObjectArray.push_back(renderObject);
}

void RenderObjectList::Clear()
{
	this->renderObjectArray.clear();
}

void RenderObjectList::UpdateListView()
{
	this->SetItemCount(this->renderObjectArray.size());
	this->Refresh();
}

/*virtual*/ wxString RenderObjectList::OnGetItemText(long item, long column) const
{
	if (0 <= item && item < this->renderObjectArray.size())
	{
		const Imzadi::RenderObject* renderObject = this->renderObjectArray[item].Get();

		switch (column)
		{
			case 0:	// Render Object
			{
				return std::format("{}", uintptr_t(item));
			}
			case 1:	// Type
			{
				if (dynamic_cast<const Imzadi::AnimatedMeshInstance*>(renderObject))
					return "Dynamic Mesh";
				else if (dynamic_cast<const Imzadi::RenderMeshInstance*>(renderObject))
					return "Static Mesh";
				break;
			}
		}
	}

	return "?";
}