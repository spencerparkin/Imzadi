#include "FontMaker.h"
#include "Log.h"
#include "JsonUtils.h"
#include "App.h"
#include "TextureMaker.h"
#include <wx/image.h>
#include <wx/filename.h>

FontMaker::FontMaker()
{
	this->library = nullptr;
}

/*virtual*/ FontMaker::~FontMaker()
{
	if (this->library)
		FT_Done_Library(this->library);
}

bool FontMaker::MakeFont(const wxString& fontFile)
{
	wxFileName nativeFontFile(fontFile);
	wxString fontFolder = nativeFontFile.GetPath();
	wxString fontName = nativeFontFile.GetName();

	wxFileName atlasFileName;
	atlasFileName.SetPath(fontFolder);
	atlasFileName.SetName(fontName);
	atlasFileName.SetExt("png");

	wxFileName fontFileName;
	fontFileName.SetPath(fontFolder);
	fontFileName.SetName(fontName);
	fontFileName.SetExt("font");

	FT_Error error = 0;

	if (!this->library)
	{
		error = FT_Init_FreeType(&this->library);
		if (error)
		{
			IMZADI_LOG_ERROR("Got error %d trying to initialize free-type library.", error);
			return false;
		}
	}

	FT_Face face = nullptr;

	bool success = false;
	while (true)
	{
		rapidjson::Document fontDoc;
		fontDoc.SetObject();

		rapidjson::Value characterArrayValue;
		characterArrayValue.SetArray();

		error = FT_New_Face(this->library, (const char*)fontFile.c_str(), 0, &face);
		if (error)
		{
			IMZADI_LOG_ERROR("Got error %d trying to create new font-face from file %s.", error, (const char* )fontFile.c_str());
			break;
		}

		IMZADI_LOG_INFO("Num glyphs found: %d", face->num_glyphs);
		IMZADI_LOG_INFO("Num faces found: %d", face->num_faces);
		IMZADI_LOG_INFO("Num charmaps found: %d", face->num_charmaps);
		IMZADI_LOG_INFO("Num fixed sizes found: %d", face->num_fixed_sizes);
	
		error = FT_Set_Pixel_Sizes(face, 32, 32);
		if (error)
		{
			IMZADI_LOG_ERROR("Failed to set pixel sizes with error: %d", error);
			break;
		}

		FT_UInt charWidth = 64;
		FT_UInt charHeight = 64;
		FT_UInt atlasWidth = 1024;
		FT_UInt atlasHeight = 1024;
		FT_UInt atlasNumRows = atlasHeight / charHeight;
		FT_UInt atlasNumCols = atlasWidth / charWidth;

		if (atlasWidth * atlasHeight < 256 * charWidth * charHeight)
		{
			IMZADI_LOG_ERROR("Can't fit 256 characters of size %d x %d in a %d x %d image.", charWidth, charHeight, atlasWidth, atlasHeight);
			break;
		}

		wxImage fontAtlasImage(atlasWidth, atlasHeight);
		fontAtlasImage.SetAlpha(nullptr);

		unsigned char* atlasImageColorBuffer = fontAtlasImage.GetData();
		unsigned char* atlasImageAlphaBuffer = fontAtlasImage.GetAlpha();

		FT_UInt bytesPerAtlasColorPixel = 3;
		FT_UInt bytesPerAtlasAlphaPixel = 1;

		::memset(atlasImageColorBuffer, 0x00, atlasWidth * atlasHeight * bytesPerAtlasColorPixel);
		::memset(atlasImageAlphaBuffer, 0x00, atlasWidth * atlasHeight * bytesPerAtlasAlphaPixel);

		// Loop through all 256 ASCII characters codes.
		FT_UInt i;
		for (i = 0; i < 256; i++)
		{
			rapidjson::Value characterValue;
			characterValue.SetObject();

			FT_UInt glyphIndex = FT_Get_Char_Index(face, i);

			error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
			if (error)
			{
				IMZADI_LOG_ERROR("Failed to load glyph at index %d for char %c with error: %d", glyphIndex, char(i), error);
				break;
			}

			if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP)
			{
				error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
				if (error)
				{
					IMZADI_LOG_ERROR("Failed to render glyph at index %d for char %c with error: %d", glyphIndex, char(i), error);
					break;
				}
			}

			FT_GlyphSlot slot = face->glyph;

			if (!slot->bitmap.buffer)
			{
				IMZADI_LOG_WARNING("Character %d didn't have a bitmap associated with it.", i);
				characterValue.AddMember("no_glyph", rapidjson::Value().SetBool(true), fontDoc.GetAllocator());
			}
			else
			{
				if (slot->bitmap.width > charWidth || slot->bitmap.rows > charHeight)
				{
					IMZADI_LOG_ERROR("Rendered glyph is size %d x %d, which won't fit in %d x %d.", slot->bitmap.width, slot->bitmap.rows, charWidth, charHeight);
					break;
				}

				FT_UInt atlasRow = i / atlasNumCols;
				FT_UInt atlasCol = i % atlasNumCols;

				FT_UInt atlasY = atlasRow * charHeight;
				FT_UInt atlasX = atlasCol * charWidth;

				FT_UInt bytesPerGlyphPixel = 0;
				if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
					bytesPerGlyphPixel = 1;
				else
				{
					IMZADI_LOG_ERROR("Pixel mode %d not yet accounted for.", slot->bitmap.pixel_mode);
					break;
				}

				float minU = float(atlasX) / float(atlasWidth);
				float minV = float(atlasY) / float(atlasHeight);
				float maxU = float(atlasX + slot->bitmap.width) / float(atlasWidth);
				float maxV = float(atlasY + slot->bitmap.rows) / float(atlasHeight);

				characterValue.AddMember("min_u", rapidjson::Value().SetFloat(minU), fontDoc.GetAllocator());
				characterValue.AddMember("min_v", rapidjson::Value().SetFloat(minV), fontDoc.GetAllocator());
				characterValue.AddMember("max_u", rapidjson::Value().SetFloat(maxU), fontDoc.GetAllocator());
				characterValue.AddMember("max_v", rapidjson::Value().SetFloat(maxV), fontDoc.GetAllocator());

				// TODO: We need more information than this about the character, such as how
				//       we advance from one character to the next and what the base-line is, etc.

				for (int row = 0; row < slot->bitmap.rows; row++)
				{
					for (int col = 0; col < slot->bitmap.width; col++)
					{
						unsigned char* glyphPixel = &slot->bitmap.buffer[row * slot->bitmap.pitch + col * bytesPerGlyphPixel];

						unsigned char* atlasColorPixel = &atlasImageColorBuffer[(atlasY + row) * atlasWidth * bytesPerAtlasColorPixel + (atlasX + col) * bytesPerAtlasColorPixel];
						unsigned char* atlasAlphaPixel = &atlasImageAlphaBuffer[(atlasY + row) * atlasWidth * bytesPerAtlasAlphaPixel + (atlasX + col) * bytesPerAtlasAlphaPixel];

						switch (slot->bitmap.pixel_mode)
						{
							case FT_PIXEL_MODE_GRAY:
							{
								float alpha = float(*glyphPixel) / float(slot->bitmap.num_grays);
								if (alpha > 1.0f)
									alpha = 1.0f;
								else if (alpha < 0.0f)
									alpha = 0.0f;

								unsigned char greyValue = unsigned char(alpha * 255.0f);

								atlasColorPixel[0] = 0x00;
								atlasColorPixel[1] = 0x00;
								atlasColorPixel[2] = 0x00;
								atlasAlphaPixel[0] = greyValue;

								break;
							}
						}
					}
				}
			}

			characterArrayValue.PushBack(characterValue, fontDoc.GetAllocator());
		}

		if (i < 256)
			break;

		if (!fontAtlasImage.SaveFile(atlasFileName.GetFullPath(), wxBITMAP_TYPE_PNG))
		{
			IMZADI_LOG_ERROR("Failed to save atlas image file: %s", (const char*)atlasFileName.GetFullPath().c_str());
			break;
		}

		fontDoc.AddMember("character_array", characterArrayValue, fontDoc.GetAllocator());

		TextureMaker textureMaker;
		if (!textureMaker.MakeTexture(atlasFileName.GetFullPath(), TextureMaker::Flag::ALPHA | TextureMaker::Flag::COMPRESS))
			break;

		fontDoc.AddMember("texture", rapidjson::Value().SetString(wxGetApp().MakeAssetFileReference(textureMaker.GetTextureFilePath()), fontDoc.GetAllocator()), fontDoc.GetAllocator());

		if (!JsonUtils::WriteJsonFile(fontDoc, fontFileName.GetFullPath()))
			break;

		success = true;
		break;
	}

	if (face)
		FT_Done_Face(face);

	return success;
}