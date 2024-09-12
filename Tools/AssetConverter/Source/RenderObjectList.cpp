#include "RenderObjectList.h"
#include "RenderObjects/RenderMeshInstance.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "RenderObjects/TextRenderObject.h"
#include "RenderObjects/SkyDomeRenderObject.h"
#include "App.h"
#include "Frame.h"
#include "GamePreview.h"
#include "RenderObjectProperties.h"
#include <format>
#include <wx/menu.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>

RenderObjectList::RenderObjectList(wxWindow* parent) : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_VIRTUAL | wxLC_REPORT)
{
	this->contextMenuItem = -1;

	this->AppendColumn("Render Object", wxLIST_FORMAT_LEFT, 120);
	this->AppendColumn("Type", wxLIST_FORMAT_LEFT, 120);

	this->Bind(wxEVT_LIST_ITEM_SELECTED, &RenderObjectList::OnItemSelected, this);
	this->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &RenderObjectList::OnItemRightClicked, this);
	this->Bind(wxEVT_MENU, &RenderObjectList::OnPlayAnimation, this, ID_ContextMenu_PlayAnimation);
	this->Bind(wxEVT_MENU, &RenderObjectList::OnDrawPorts, this, ID_ContextMenu_DrawPorts);
	this->Bind(wxEVT_UPDATE_UI, &RenderObjectList::OnUpdateUI, this, ID_ContextMenu_DrawPorts);
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

void RenderObjectList::OnItemRightClicked(wxListEvent& event)
{
	this->contextMenuItem = event.GetIndex();

	wxMenu contextMenu;
	contextMenu.Append(new wxMenuItem(&contextMenu, ID_ContextMenu_PlayAnimation, "Play Animation", "Play an animation on this render object."));
	contextMenu.Append(new wxMenuItem(&contextMenu, ID_ContextMenu_DrawPorts, "Draw Ports", "Toggle the display of any ports authored on the render object.", wxITEM_CHECK));

	wxPoint pos = event.GetPoint();
	this->PopupMenu(&contextMenu, pos);
}

void RenderObjectList::OnPlayAnimation(wxCommandEvent& event)
{
	Imzadi::RenderObject* renderObject = this->renderObjectArray[this->contextMenuItem];
	auto animatedMesh = dynamic_cast<Imzadi::AnimatedMeshInstance*>(renderObject);
	if (animatedMesh)
	{
		wxTextEntryDialog animationDialog(wxGetApp().GetFrame(), "Play which animation?", "Choose Animation to Play");
		if (animationDialog.ShowModal() == wxID_OK)
		{
			std::string animName((const char*)animationDialog.GetValue().c_str());
			if (!animatedMesh->SetAnimation(animName))
				wxMessageBox(wxString::Format("No animation with name \"%s\" found.", animName.c_str()), "Error", wxICON_ERROR | wxOK, wxGetApp().GetFrame());
			else
			{
				((GamePreview*)Imzadi::Game::Get())->SetAnimatingMesh(animatedMesh);
			}
		}
	}
}

void RenderObjectList::OnDrawPorts(wxCommandEvent& event)
{
	Imzadi::RenderObject* renderObject = this->renderObjectArray[this->contextMenuItem];
	auto renderMesh = dynamic_cast<Imzadi::RenderMeshInstance*>(renderObject);
	if (renderMesh)
		renderMesh->SetDrawPorts(!renderMesh->GetDrawPorts());
}

void RenderObjectList::OnUpdateUI(wxUpdateUIEvent& event)
{
	switch (event.GetId())
	{
		case ID_ContextMenu_DrawPorts:
		{
			Imzadi::RenderObject* renderObject = this->renderObjectArray[this->contextMenuItem];
			auto renderMesh = dynamic_cast<Imzadi::RenderMeshInstance*>(renderObject);
			if (renderMesh)
				event.Check(renderMesh->GetDrawPorts());

			break;
		}
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
				else if (dynamic_cast<const Imzadi::TextRenderObject*>(renderObject))
					return "Text";
				else if (dynamic_cast<const Imzadi::SkyDomeRenderObject*>(renderObject))
					return "Sky Dome";
				break;
			}
		}
	}

	return "?";
}