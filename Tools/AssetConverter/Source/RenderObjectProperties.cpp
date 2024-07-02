#include "RenderObjectProperties.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "RenderObjects/RenderMeshInstance.h"
#include "RenderObjects/TextRenderObject.h"
#include "RenderObjects/SkyDomeRenderObject.h"
#include "Assets/Skeleton.h"
#include "Assets/SkyDome.h"
#include "Assets/CubeTexture.h"

RenderObjectProperties::RenderObjectProperties(wxWindow* parent) : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE | wxTE_RICH | wxHSCROLL)
{
}

/*virtual*/ RenderObjectProperties::~RenderObjectProperties()
{
}

void RenderObjectProperties::PrintPropertiesOf(Imzadi::RenderObject* renderObject)
{
	this->Clear();

	auto renderMesh = dynamic_cast<Imzadi::RenderMeshInstance*>(renderObject);
	if (renderMesh)
	{
		this->AppendText("Render Mesh Properties\n");
		this->AppendText("======================\n");

		Imzadi::RenderMeshAsset* mesh = renderMesh->GetRenderMesh();
		
		wxString primType = "?";
		switch (mesh->GetPrimType())
		{
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
			primType = "TRIANGLE_LIST";
			break;
		}

		this->AppendText("Prim Type: " + primType + "\n");

		Imzadi::Buffer* indexBuffer = mesh->GetIndexBuffer();
		if (indexBuffer)
			this->AppendText(wxString::Format("Index buffer elements: %d\n", indexBuffer->GetNumElements()));
		
		Imzadi::Buffer* vertexBuffer = mesh->GetIndexBuffer();
		if (vertexBuffer)
			this->AppendText(wxString::Format("Vertex buffer elements: %d\n", vertexBuffer->GetNumElements()));

		//...

		this->AppendText("\n");
	}

	auto animatedMesh = dynamic_cast<Imzadi::AnimatedMeshInstance*>(renderObject);
	if (animatedMesh)
	{
		this->AppendText("Animated Mesh Properties\n");
		this->AppendText("========================\n");

		Imzadi::SkinnedRenderMesh* skinnedMesh = animatedMesh->GetSkinnedMesh();
		if (skinnedMesh)
		{
			std::unordered_set<std::string> animationNameSet;
			skinnedMesh->GetAnimationNames(animationNameSet);
			if (animationNameSet.size() == 0)
				this->AppendText("No animations associated with the skinned mesh.\n");
			else
			{
				this->AppendText(wxString::Format("%d animation(s) associated with the skinned mesh.\n", int(animationNameSet.size())));
				int i = 0;
				for(const std::string& animName : animationNameSet)
					this->AppendText(wxString::Format("%d: %s\n", ++i, animName.c_str()) + "\n");
			}

			Imzadi::Skeleton* skeleton = skinnedMesh->GetSkeleton();
			if (!skeleton)
				this->AppendText("No skeleton found!\n");
			else
			{
				std::vector<Imzadi::Bone*> boneArray;
				skeleton->GatherBones(boneArray);
				if (boneArray.size() == 0)
					this->AppendText("No bones found!\n");
				else
				{
					this->AppendText(wxString::Format("Found %d bone(s) in the skeleton.\n", int(boneArray.size())));
					for (int i = 0; i < boneArray.size(); i++)
						this->AppendText(wxString::Format("%d: %s\n", i + 1, boneArray[i]->GetName().c_str()));
				}
			}
		}
	}

	auto textRenderObject = dynamic_cast<Imzadi::TextRenderObject*>(renderObject);
	if (textRenderObject)
	{
		this->AppendText("Text Properties\n");
		this->AppendText("===============\n");

		Imzadi::Vector3 color = textRenderObject->GetColor();
		this->AppendText(wxString::Format("Color: %f, %f, %f\n", color.x, color.y, color.z));

		Imzadi::Font* font = textRenderObject->GetFont();
		this->AppendText(wxString::Format("Font: %s\n", font->GetName().c_str()));
	}

#if 0
	auto skyDomeRenderObject = dynamic_cast<Imzadi::SkyDomeRenderObject*>(renderObject);
	if (skyDomeRenderObject)
	{
		this->AppendText("Sky Dome Properties\n");
		this->AppendText("===================\n");

		Imzadi::SkyDome* skyDome = skyDomeRenderObject->GetSkyDome();
		if (skyDome)
		{
			Imzadi::CubeTexture* cubeTexture = skyDome->GetCubeTexture();
			if (cubeTexture)
			{
				D3D11_TEXTURE2D_DESC textureDesc{};
				cubeTexture->GetTexture()->GetDesc(&textureDesc);

				this->AppendText(wxString::Format("Cube texture is %d x %d for each side.", textureDesc.Width, textureDesc.Height));
			}
		}
	}
#endif
}