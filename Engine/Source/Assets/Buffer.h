#pragma once

#include "AssetCache.h"
#include <d3d11.h>

namespace Imzadi
{
	class BareBuffer;

	/**
	 * Instances of this class can represent an index or vertex buffer.
	 */
	class IMZADI_API Buffer : public Asset
	{
	public:
		Buffer();
		virtual ~Buffer();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;

		ID3D11Buffer* GetBuffer() { return this->buffer; }
		UINT GetStride() { return this->strideBytes; }
		UINT GetNumElements() { return this->numElements; }
		DXGI_FORMAT GetFormat() { return this->componentFormat; }

		bool GetBareBuffer(Reference<BareBuffer>& givenBareBuffer);

	private:
		ID3D11Buffer* buffer;				///< A pointer to the DX11 buffer interface.
		UINT numElements;					///< Here, an "element" refers to a vertex or an index.
		UINT strideBytes;					///< This is the byte-distance in the buffer from the start of one element to another.
		DXGI_FORMAT componentFormat;		///< This is the format of each component of an element.
		Reference<BareBuffer> bareBuffer;	///< This is a bare copy of the buffer, which is typically null/not-used.
	};

	/**
	 * This is a buffer not hidden behind the ID3D11Buffer interface.  Of course,
	 * this means it's not GPU-accessable.
	 */
	class IMZADI_API BareBuffer : public ReferenceCounted
	{
	public:
		BareBuffer();
		virtual ~BareBuffer();

		void SetSize(UINT size);
		UINT GetSize() const;

		BYTE* GetBuffer() { return this->buffer; }
		const BYTE* GetBuffer() const { return this->buffer; }

		BareBuffer* Clone() const;

	private:
		BYTE* buffer;
		UINT bufferSize;
	};
}