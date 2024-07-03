#pragma once

#include "AssetCache.h"
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace Imzadi
{
	/**
	 * This is a class that can be used to manage static and dynamic buffers
	 * that need to be available to the GPU for rendering, and which may or
	 * may not need to be available to the CPU for mutation.  Typical use-cases
	 * include, but are not limited to, index buffers, vertex buffers, texture
	 * buffers and shader constants buffers.
	 */
	class IMZADI_API Buffer : public Asset
	{
	public:
		Buffer();
		virtual ~Buffer();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;

		enum Type
		{
			/**
			 * This is the initial type of the buffer, which is not usable and will generate errors.
			 */
			UNKNOWN,

			/**
			 * The buffer is made read-only and resident in high-availability GPU memory.  There is no way
			 * to change it once loaded from the CPU into GPU memory.
			 */
			STATIC,

			/**
			 * The buffer is relatively small and made read/write in GPU memory with low-availability.
			 * A typical use-case is that of constants buffers updated every draw-call.
			 */
			DYNAMIC_SMALL,

			/**
			 * The buffer is relatively large and made read-only and resident in high-availability GPU memory,
			 * but can be updated with a GPU operation.  A typical use-case is that of real-time mesh deformations
			 * updating every frame.
			 */
			DYNAMIC_LARGE,

			/**
			 * This buffer type is not yet supported, but the idea here is to provide a buffer in
			 * which the GPU can write and then the CPU can read back.
			 */
			READBACK
		};

		/**
		 * Initialize this buffer to be of the given type.  Failure occurs if
		 * the buffer has already been created.  Failure can also occur if the
		 * buffer is static, but no initial contents are given.  If dynamic,
		 * initial contents are optional, but left undefined if not given and
		 * until updated.
		 * 
		 * @param[in] desiredBufferType See the @ref Type num for details.
		 * @param[in] initialBufferData This is the data to use to initially populate the buffer.  This can be null.
		 * @param[in] initialBufferSize This is how big the given buffer is and how big this buffer will be.  It must be non-zero.
		 * @return True is returned on success; false, otherwise.
		 */
		bool Create(Type desiredBufferType, const uint8_t* initialBufferData, uint32_t initialBufferSize);

		/**
		 * Update the contents of this buffer with that of the given buffer.
		 * This will, of course, fail if the buffer is static.  If it is dynamic-small,
		 * then the update is immediate after this call, but the GPU does incur a marsheling
		 * hit when it needs to access it.  If it is dynamic-large, then an update is scheduled
		 * by this call to occur just before rendering.  Of course, failure can also occur here
		 * in the case that a buffer overrun is detected.
		 * 
		 * @param[in] inputBufferData This is a pointer to a buffer whose contents is to replace all or part of this object's buffer.
		 * @param[in] inputBufferSize This should be the size of the given buffer.
		 * @param[in] offset This is a byte-offset from the start of this object's buffer to where the given buffer will be layed down.
		 * @return True is returned on success; false, otherwise.
		 */
		bool Write(const uint8_t* inputBufferData, uint32_t inputBufferSize, uint32_t offset);

		/**
		 * This is not yet supported and will always return false for now.  The idea
		 * here, though, is to provide support for read-back capability from operations
		 * performed on the GPU that write into the buffer.
		 */
		bool Read(uint8_t* outputBufferData, uint32_t& outputBufferSize);

		/**
		 * Return the size of this buffer in bytes.
		 */
		uint32_t GetBufferSize() const { return this->bufferSize; }

		/**
		 * Get a pointer to the buffer resource that is being used by this object.
		 */
		ID3D12Resource* GetBufferResource();

	protected:

		virtual bool GetResourceDesc(D3D12_RESOURCE_DESC& resourceDesc);

		ComPtr<ID3D12Resource> uploadBufferResource;
		ComPtr<ID3D12Resource> bufferResource;
		Type bufferType;
		uint32_t bufferSize;
		uint8_t* mappedBufferData;
	};

	/**
	 * These can be index or vertex buffers.
	 */
	class IMZADI_API ElementBuffer : public Buffer
	{
	public:
		ElementBuffer();
		virtual ~ElementBuffer();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;

		UINT GetStride() { return this->strideBytes; }
		UINT GetNumElements() { return this->numElements; }
		DXGI_FORMAT GetFormat() { return this->componentFormat; }

	private:
		
		virtual bool GetResourceDesc(D3D12_RESOURCE_DESC& resourceDesc) override;

		UINT numElements;					///< Here, an "element" refers to a vertex or an index.
		UINT strideBytes;					///< This is the byte-distance in the buffer from the start of one element to the next.
		DXGI_FORMAT componentFormat;		///< This is the format of each component of an element.
	};
}