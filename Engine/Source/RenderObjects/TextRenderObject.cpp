#include "TextRenderObject.h"

using namespace Imzadi;

TextRenderObject::TextRenderObject()
{
	this->flags = 0;
	this->vertexBuffer = nullptr;
}

/*virtual*/ TextRenderObject::~TextRenderObject()
{
}

/*virtual*/ void TextRenderObject::Prepare()
{
	if ((this->flags & Flag::REBUILD_VERTEX_BUFFER) == 0)
		return;

	// TODO: Build the vertex buffer here.
}

/*virtual*/ void TextRenderObject::Render(Camera* camera, RenderPass renderPass)
{
	// TODO: Write this.  Render our vertex buffer as configured.
}

/*virtual*/ void TextRenderObject::GetWorldBoundingSphere(Imzadi::Vector3& center, double& radius) const
{
	// TODO: Write this.
}

/*virtual*/ int TextRenderObject::SortKey() const
{
	return 1;
}

void TextRenderObject::SetFlags(uint32_t givenFlags)
{
	this->flags = givenFlags;
}

uint32_t TextRenderObject::GetFlags() const
{
	return this->flags;
}

void TextRenderObject::SetText(const std::string& givenText)
{
	if (this->text != givenText)
	{
		this->text = givenText;
		this->flags |= Flag::REBUILD_VERTEX_BUFFER;
	}
}

const std::string& TextRenderObject::GetText() const
{
	return this->text;
}

bool TextRenderObject::SetFront(const std::string& fontName)
{
	return true;
}