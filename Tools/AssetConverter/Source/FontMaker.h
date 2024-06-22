#pragma once

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_MODULE_H
#include <wx/string.h>

class FontMaker
{
public:
	FontMaker();
	virtual ~FontMaker();

	bool MakeFont(const wxString& fontFile);

private:
	FT_Library library;
};