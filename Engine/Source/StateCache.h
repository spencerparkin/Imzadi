#pragma once

#include "Defines.h"
#include <d3d11.h>
#include <unordered_map>
#include <string>

template<>
struct std::hash<D3D11_RASTERIZER_DESC>
{
	std::size_t operator()(const D3D11_RASTERIZER_DESC& desc) const
	{
		return Imzadi::HashBuffer((const char*)&desc, sizeof(desc));
	}
};

template<>
struct std::hash<D3D11_DEPTH_STENCIL_DESC>
{
	std::size_t operator()(const D3D11_DEPTH_STENCIL_DESC& desc) const
	{
		return Imzadi::HashBuffer((const char*)&desc, sizeof(desc));
	}
};

template<>
struct std::hash<D3D11_BLEND_DESC>
{
	std::size_t operator()(const D3D11_BLEND_DESC& desc) const
	{
		return Imzadi::HashBuffer((const char*)&desc, sizeof(desc));
	}
};

inline bool operator==(const D3D11_RASTERIZER_DESC& descA, const D3D11_RASTERIZER_DESC& descB)
{
	return 0 == ::memcmp(&descA, &descB, sizeof(D3D11_RASTERIZER_DESC));
}

inline bool operator==(const D3D11_DEPTH_STENCIL_DESC& descA, const D3D11_DEPTH_STENCIL_DESC& descB)
{
	return 0 == ::memcmp(&descA, &descB, sizeof(D3D11_DEPTH_STENCIL_DESC));
}

inline bool operator==(const D3D11_BLEND_DESC& descA, const D3D11_BLEND_DESC& descB)
{
	return 0 == ::memcmp(&descA, &descB, sizeof(D3D11_BLEND_DESC));
}

namespace Imzadi
{
	class StateCacheInterface;

	template<typename State, typename StateDesc>
	class IMZADI_API StateCacheGuts
	{
	public:
		StateCacheGuts(StateCacheInterface* stateCacheIface)
		{
			this->currentStateHash = 0;
			this->stateCacheIface = stateCacheIface;
		}

		virtual ~StateCacheGuts()
		{
		}

		void SetState(const StateDesc* stateDesc)
		{
			std::size_t stateHash = std::hash<StateDesc>{}(*stateDesc);
			if (stateHash == this->currentStateHash)
				return;

			auto iter = this->stateMap.find(*stateDesc);
			State* state = nullptr;
			if (iter != this->stateMap.end())
				state = iter->second;
			else
			{
				state = (State*)this->stateCacheIface->CreateState(stateDesc);
				this->stateMap.insert(std::pair<StateDesc, State*>(*stateDesc, state));
			}

			this->stateCacheIface->ApplyState(state);
			this->currentStateHash = stateHash;
		}

		void ClearCache()
		{
			for (auto pair : this->stateMap)
				SafeRelease(pair.second);

			this->stateMap.clear();
			this->currentStateHash = 0;
		}

	private:
		typedef std::unordered_map<StateDesc, State*> StateMap;
		StateMap stateMap;
		std::size_t currentStateHash;
		StateCacheInterface* stateCacheIface;
	};

	class IMZADI_API StateCacheInterface
	{
	public:
		virtual void* CreateState(const void* desc) = 0;
		virtual void ApplyState(void* state) = 0;
		virtual void SetState(const void* desc) = 0;
		virtual void ClearCache() = 0;
	};

	template<typename State, typename StateDesc>
	class IMZADI_API StateCache : public StateCacheInterface
	{
	public:
		virtual void* CreateState(const void* desc) override
		{
			return nullptr;
		}

		virtual void ApplyState(void* state) override
		{
		}

		virtual void SetState(const void* desc) override
		{
		}

		virtual void ClearCache() override
		{
		}
	};

	template<>
	class IMZADI_API StateCache<ID3D11RasterizerState, D3D11_RASTERIZER_DESC> : public StateCacheInterface
	{
	public:
		StateCache() : guts(this)
		{
		}

		virtual void* CreateState(const void* desc) override;
		virtual void ApplyState(void* state) override;
		virtual void SetState(const void* desc) override;
		virtual void ClearCache() override;

	private:
		StateCacheGuts<ID3D11RasterizerState, D3D11_RASTERIZER_DESC> guts;
	};

	template<>
	class IMZADI_API StateCache<ID3D11DepthStencilState, D3D11_DEPTH_STENCIL_DESC> : public StateCacheInterface
	{
	public:
		StateCache() : guts(this)
		{
		}

		virtual void* CreateState(const void* desc) override;
		virtual void ApplyState(void* state) override;
		virtual void SetState(const void* desc) override;
		virtual void ClearCache() override;

	private:
		StateCacheGuts<ID3D11DepthStencilState, D3D11_DEPTH_STENCIL_DESC> guts;
	};

	template<>
	class IMZADI_API StateCache<ID3D11BlendState, D3D11_BLEND_DESC> : public StateCacheInterface
	{
	public:
		StateCache() : guts(this)
		{
		}

		virtual void* CreateState(const void* desc) override;
		virtual void ApplyState(void* state) override;
		virtual void SetState(const void* desc) override;
		virtual void ClearCache() override;

	private:
		StateCacheGuts<ID3D11BlendState, D3D11_BLEND_DESC> guts;
	};
}