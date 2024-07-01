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

	uint32_t numMips = 1;

	if ((flags & Flag::MIP_MAPS) != 0)
	{
		if (!IMZADI_IS_POW_TWO(image.GetWidth()) || !IMZADI_IS_POW_TWO(image.GetHeight()))
		{
			IMZADI_LOG_ERROR("If doing mip-maps, the texture dimensions must be a power of two, not %d x %d.", image.GetWidth(), image.GetHeight());
			return false;
		}

		if (image.GetWidth() != image.GetHeight())
		{
			IMZADI_LOG_ERROR("If doing mip-maps, we require, for simplicity, that the texture dimensions be square, not %d x %d.", image.GetWidth(), image.GetHeight());
			return false;
		}

		numMips = 0;
		uint32_t size = image.GetWidth();
		while (size > 0)
		{
			numMips++;
			size >>= 1;
		}
	}

	textureDoc.AddMember("num_mips", rapidjson::Value().SetUint(numMips), textureDoc.GetAllocator());

	if ((flags & Flag::FOR_CUBE_MAP) != 0)
		textureDoc.AddMember("for_staging", rapidjson::Value().SetBool(true), textureDoc.GetAllocator());

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

	uint32_t textureDataBufferSize = 0;
	uint32_t width = image.GetWidth();
	uint32_t height = image.GetHeight();
	for (uint32_t i = 0; i < numMips; i++)
	{
		textureDataBufferSize += width * height * texelSizeBytes;
		width >>= 1;
		height >>= 1;
	}

	std::unique_ptr<unsigned char> textureDataBuffer(new unsigned char[textureDataBufferSize]);

	unsigned char* mipTexture = textureDataBuffer.get();
	width = image.GetWidth();
	height = image.GetHeight();
	for (uint32_t i = 0; i < numMips; i++)
	{
		IMZADI_ASSERT(width == image.GetWidth() && height == image.GetHeight());

		for (uint32_t row = 0; row < height; row++)
		{
			for (uint32_t col = 0; col < width; col++)
			{
				int wantedRow = row;
				if ((flags & Flag::FLIP_VERTICAL) != 0)
					wantedRow = height - 1 - row;

				int wantedCol = col;
				if ((flags & Flag::FLIP_HORIZONTAL) != 0)
					wantedCol = width - 1 - col;

				unsigned char* texel = &mipTexture[(wantedRow * width + wantedCol) * texelSizeBytes];

				if ((flags & Flag::COLOR) != 0)
				{
					const unsigned char* colorPixel = &image.GetData()[(row * width + col) * 3];

					texel[0] = colorPixel[0];
					texel[1] = colorPixel[1];
					texel[2] = colorPixel[2];
				}

				if ((flags & Flag::ALPHA) != 0)
				{
					const unsigned char* alphaPixel = &image.GetAlpha()[row * width + col];

					if ((flags & Flag::COLOR) != 0)
						texel[3] = *alphaPixel;
					else
						texel[0] = *alphaPixel;
				}
			}
		}

		mipTexture += width * height * texelSizeBytes;
		width >>= 1;
		height >>= 1;

		if (i + 1 < numMips)
		{
			image.Rescale(width, height, wxIMAGE_QUALITY_HIGH);
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