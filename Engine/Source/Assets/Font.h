#pragma once

#include "AssetCache.h"
#include "Assets/Texture.h"
#include "Assets/Shader.h"
#include "Math/Vector2.h"

namespace Imzadi
{
	/**
	 * An instance of this class contains all the information we need to
	 * render a certain font.
	 */
	class IMZADI_API Font : public Asset
	{
	public:
		Font();
		virtual ~Font();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;

		struct CharacterInfo
		{
			Vector2 minUV;
			Vector2 maxUV;
			Vector2 penOffset;
			double advance;
			double width;
			double height;
		};

		bool GetCharInfo(char ch, CharacterInfo& info) const;
		Shader* GetShader() { return this->textShader.Get(); }
#if 0
		ID3D11Buffer* GetIndexBuffer() { return this->indexBuffer; }
#endif
		Texture* GetTextureAtlas() { return this->textureAtlas.Get(); }
		const std::string& GetName() const { return this->name; }

	private:

		typedef std::vector<CharacterInfo> CharacterInfoArray;

		CharacterInfoArray charInfoArray;
		Reference<Texture> textureAtlas;
		Reference<Shader> textShader;
#if 0
		ID3D11Buffer* indexBuffer;
#endif
		std::string name;
	};
}