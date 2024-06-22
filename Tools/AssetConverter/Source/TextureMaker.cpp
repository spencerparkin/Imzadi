#include "TextureMaker.h"
#include "App.h"
#include "JsonUtils.h"
#include "Log.h"
#include <wx/image.h>
#include <compressapi.h>

TextureMaker::TextureMaker()
{
}

/*virtual*/ TextureMaker::~TextureMaker()
{
}

bool TextureMaker::MakeTexture(const wxString& imageFilePath, uint32_t flags)
{
	wxFileName fileName(imageFilePath);

	if (!fileName.Normalize())
	{
		IMZADI_LOG_ERROR("Failed to normalized image file path: %s", (const char*)imageFilePath.c_str());
		return false;
	}

	wxString imageFolder = fileName.GetPath();
	wxString imageName = fileName.GetName();

	this->textureFileName.SetPath(imageFolder);
	this->textureFileName.SetName(imageName);
	this->textureFileName.SetExt("texture");

	// If we already made the texture, then there is no need to make it again.
	// A texture can be referenced multiple times across assets, of course.
	std::string key((const char*)textureFileName.GetFullPath().MakeLower().c_str());
	if ((flags & Flag::ALWAYS_MAKE) == 0 && this->madeTextureSet.find(key) != this->madeTextureSet.end())
	{
		IMZADI_LOG_INFO("Already made texture: %s", (const char*)textureFileName.GetFullPath().c_str());
		return true;
	}

	wxFileName textureDataFileName;
	textureDataFileName.SetPath(imageFolder);
	textureDataFileName.SetName(imageName);
	textureDataFileName.SetExt("texture_data");

	wxImage image;
	if (!image.LoadFile(imageFilePath))
	{
		IMZADI_LOG_ERROR("Failed to load image file: %s", (const char*)imageFilePath.c_str());
		return false;
	}

	rapidjson::Document textureDoc;
	textureDoc.SetObject();
	textureDoc.AddMember("width", rapidjson::Value().SetUint(image.GetWidth()), textureDoc.GetAllocator());
	textureDoc.AddMember("height", rapidjson::Value().SetUint(image.GetHeight()), textureDoc.GetAllocator());
	textureDoc.AddMember("data", rapidjson::Value().SetString(wxGetApp().MakeAssetFileReference(textureDataFileName.GetFullPath()), textureDoc.GetAllocator()), textureDoc.GetAllocator());

	uint32_t texelSizeBytes = 0;
	if ((flags & (Flag::COLOR | Flag::ALPHA)) == (Flag::COLOR | Flag::ALPHA))
	{
		textureDoc.AddMember("format", rapidjson::Value().SetString("RGBA", textureDoc.GetAllocator()), textureDoc.GetAllocator());
		texelSizeBytes = 4;
	}
	else if ((flags & Flag::COLOR) == Flag::COLOR)
	{
		textureDoc.AddMember("format", rapidjson::Value().SetString("RGB", textureDoc.GetAllocator()), textureDoc.GetAllocator());
		texelSizeBytes = 3;
	}
	else if ((flags & Flag::ALPHA) == Flag::ALPHA)
	{
		textureDoc.AddMember("format", rapidjson::Value().SetString("A", textureDoc.GetAllocator()), textureDoc.GetAllocator());
		texelSizeBytes = 1;
	}
	else
	{
		IMZADI_LOG_ERROR("Unable to determine texture format.");
		return false;
	}

	if ((flags & Flag::COMPRESS) == Flag::COMPRESS)
		textureDoc.AddMember("compressed", rapidjson::Value().SetBool(true), textureDoc.GetAllocator());

	if ((flags & Flag::MAKE_ALPHA) && !image.HasAlpha())
	{
		// According to the wxWidgets docs, this should allocate an alpha buffer for us.
		image.SetAlpha(nullptr);
		::memset(image.GetAlpha(), 0x00, image.GetWidth() * image.GetHeight());
	}

	if ((flags & Flag::ALPHA) != 0 && !image.HasAlpha())
	{
		IMZADI_LOG_ERROR("Alpha was desired, but no alpha found in loaded image.");
		return false;
	}

	uint32_t textureDataBufferSize = image.GetWidth() * image.GetHeight() * texelSizeBytes;
	std::unique_ptr<unsigned char> textureDataBuffer(new unsigned char[textureDataBufferSize]);

	for (int row = 0; row < image.GetHeight(); row++)
	{
		for (int col = 0; col < image.GetWidth(); col++)
		{
			unsigned char* texel = nullptr;
			if ((flags & Flag::FLIP_VERTICAL) == 0)
				texel = &textureDataBuffer.get()[(row * image.GetWidth() + col) * texelSizeBytes];
			else
				texel = &textureDataBuffer.get()[((image.GetHeight() - 1 - row) * image.GetWidth() + col) * texelSizeBytes];

			if ((flags & Flag::COLOR) != 0)
			{
				const unsigned char* colorPixel = &image.GetData()[(row * image.GetWidth() + col) * 3];

				texel[0] = colorPixel[0];
				texel[1] = colorPixel[1];
				texel[2] = colorPixel[2];
			}

			if ((flags & Flag::ALPHA) != 0)
			{
				const unsigned char* alphaPixel = &image.GetAlpha()[row * image.GetWidth() + col];

				if ((flags & Flag::COLOR) != 0)
					texel[3] = *alphaPixel;
				else
					texel[0] = *alphaPixel;
			}
		}
	}

	if ((flags & Flag::COMPRESS) == 0)
	{
		if (!this->DumpTextureData(textureDataFileName.GetFullPath(), textureDataBuffer.get(), textureDataBufferSize))
			return false;
	}
	else
	{
		COMPRESSOR_HANDLE compressor = NULL;
		if (!CreateCompressor(COMPRESS_ALGORITHM_XPRESS, NULL, &compressor))
		{
			IMZADI_LOG_ERROR("Failed to create compressor.");
			return false;
		}

		ULONG_PTR compressedBufferSize = textureDataBufferSize;
		std::unique_ptr<unsigned char> compressedBuffer(new unsigned char[compressedBufferSize]);
		
		// TODO: I'm seeing some compressed textures larger than the original PNG file.  :/
		if (!Compress(compressor, textureDataBuffer.get(), textureDataBufferSize, compressedBuffer.get(), compressedBufferSize, &compressedBufferSize))
		{
			IMZADI_LOG_ERROR("Failed to compressed texture buffer.  Error code: %d", GetLastError());
			return false;
		}

		CloseCompressor(compressor);

		if (!this->DumpTextureData(textureDataFileName.GetFullPath(), compressedBuffer.get(), compressedBufferSize))
			return false;
	}

	if (!JsonUtils::WriteJsonFile(textureDoc, this->textureFileName.GetFullPath()))
		return false;

	this->madeTextureSet.insert(key);
	return true;
}

bool TextureMaker::DumpTextureData(const wxString& textureDataFilePath, const unsigned char* textureDataBuffer, uint32_t textureDataBufferSize)
{
	std::filesystem::path path((const char*)textureDataFilePath.c_str());
	if (std::filesystem::exists(path))
		std::filesystem::remove(path);

	std::fstream fileStream;
	fileStream.open((const char*)textureDataFilePath.c_str(), std::ios::out | std::ios::binary);
	if (!fileStream.is_open())
	{
		IMZADI_LOG_ERROR("Failed to open (for binary writing) the file: %s", (const char*)textureDataFilePath.c_str());
		return false;
	}

	fileStream.write((const char*)textureDataBuffer, textureDataBufferSize);
	fileStream.close();
	return true;
}