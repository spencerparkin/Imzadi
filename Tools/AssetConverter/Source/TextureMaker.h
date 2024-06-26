#pragma once

#include <wx/string.h>
#include <wx/filename.h>
#include <unordered_set>

class TextureMaker
{
public:
	TextureMaker();
	virtual ~TextureMaker();

	enum Flag
	{
		COLOR			= 0x00000001,
		ALPHA			= 0x00000002,
		COMPRESS		= 0x00000004,
		MAKE_ALPHA		= 0x00000008,
		FLIP_VERTICAL	= 0x00000010,
		ALWAYS_MAKE		= 0x00000020,
		FOR_CUBE_MAP	= 0x00000040
	};

	bool MakeTexture(const wxString& imageFilePath, uint32_t flags);
	
	wxString GetTextureFilePath() const { return this->textureFileName.GetFullPath(); }

private:
	wxFileName textureFileName;

	std::unordered_set<std::string> madeTextureSet;

	bool DumpTextureData(const wxString& textureDataFilePath, const unsigned char* textureDataBuffer, uint32_t textureDataBufferSize);
};