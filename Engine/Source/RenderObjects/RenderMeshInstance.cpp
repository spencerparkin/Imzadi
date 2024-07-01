#include "RenderMeshInstance.h"
#include "Camera.h"
#include "Scene.h"
#include "Assets/RenderMesh.h"
#include "Assets/Buffer.h"
#include "Assets/Texture.h"
#include "Assets/Shader.h"
#include "Game.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector4.h"

using namespace Imzadi;

RenderMeshInstance::RenderMeshInstance()
{
	this->objectToWorld.SetIdentity();
	this->surfaceProperties.shininessExponent = 50.0;
}

/*virtual*/ RenderMeshInstance::~RenderMeshInstance()
{
}

void RenderMeshInstance::Render(Camera* camera, RenderPass renderPass)
{
#if 0
	ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();

	Shader* shader = nullptr;

	switch (renderPass)
	{
	case RenderPass::MAIN_PASS:
		shader = this->mesh->GetShader();
		break;
	case RenderPass::SHADOW_PASS:
		shader = this->mesh->GetShadowShader();
		break;
	}

	if (!shader)
		return;

	D3D11_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Game::Get()->GetBlendStateCache()->SetState(&blendDesc);

	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = TRUE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	Game::Get()->GetRasterStateCache()->SetState(&rasterizerDesc);

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0;
	depthStencilDesc.StencilWriteMask = 0;
	Game::Get()->GetDepthStencilStateCache()->SetState(&depthStencilDesc);

	Buffer* vertexBuffer = this->mesh->GetVertexBuffer();
	Buffer* indexBuffer = this->mesh->GetIndexBuffer();
	Texture* texture = this->mesh->GetTexture();

	deviceContext->IASetPrimitiveTopology(this->mesh->GetPrimType());
	deviceContext->IASetInputLayout(shader->GetInputLayout());

	deviceContext->VSSetShader(shader->GetVertexShader(), NULL, 0);
	deviceContext->PSSetShader(shader->GetPixelShader(), NULL, 0);

	UINT stride = vertexBuffer->GetStride();
	UINT offset = 0;
	ID3D11Buffer* vertexBufferIface = vertexBuffer->GetBuffer();
	deviceContext->IASetVertexBuffers(0, 1, &vertexBufferIface, &stride, &offset);

	ID3D11Buffer* constantsBuffer = shader->GetConstantsBuffer();
	if (constantsBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		deviceContext->Map(constantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);

		UINT bufferSize = shader->GetConstantsBufferSize();
		::memset(mappedSubresource.pData, 0, bufferSize);

		const Shader::Constant* constant = nullptr;

		Matrix4x4 worldToCameraMat;
		camera->GetWorldToCameraTransform().GetToMatrix(worldToCameraMat);

		Matrix4x4 objectToWorldMat;
		this->objectToWorld.GetToMatrix(objectToWorldMat);

		Matrix4x4 cameraToProjMat;
		camera->GetProjectionMatrix(cameraToProjMat);

		Matrix4x4 objectToProjMat = cameraToProjMat * worldToCameraMat * objectToWorldMat;

		if (shader->GetConstantInfo("objectToProjection", constant))
			StoreShaderConstant(&mappedSubresource, constant, &objectToProjMat);

		if (shader->GetConstantInfo("objectToWorld", constant))
			StoreShaderConstant(&mappedSubresource, constant, &objectToWorldMat);

		if (shader->GetConstantInfo("lightDirection", constant))
			StoreShaderConstant(&mappedSubresource, constant, &Game::Get()->GetLightParams().lightDirection);

		if (shader->GetConstantInfo("directionalLightIntensity", constant))
			StoreShaderConstant(&mappedSubresource, constant, &Game::Get()->GetLightParams().directionalLightIntensity);

		if (shader->GetConstantInfo("ambientLightIntensity", constant))
			StoreShaderConstant(&mappedSubresource, constant, &Game::Get()->GetLightParams().ambientLightIntensity);

		if (shader->GetConstantInfo("shininessExponent", constant))
			StoreShaderConstant(&mappedSubresource, constant, &this->surfaceProperties.shininessExponent);

		if (shader->GetConstantInfo("lightColor", constant))
			StoreShaderConstant(&mappedSubresource, constant, &Game::Get()->GetLightParams().lightColor);

		if (shader->GetConstantInfo("cameraEyePoint", constant))
			StoreShaderConstant(&mappedSubresource, constant, &camera->GetCameraToWorldTransform().translation);

		if (renderPass == RenderPass::MAIN_PASS)
		{
			Camera* lightSourceCamera = Game::Get()->GetLightSourceCamera();
			if (lightSourceCamera)
			{
				const Transform& lightCameraToWorld = lightSourceCamera->GetCameraToWorldTransform();
				const Camera::OrthographicParams& orthoParams = lightSourceCamera->GetOrthographicParameters();

				Vector3 lightCameraEyePoint = lightCameraToWorld.translation;
				Vector3 lightCameraXAxis, lightCameraYAxis, lightCameraZAxis;
				double lightCameraWidth = 0.0, lightCameraHeight = 0.0;
				double lightCameraNear = 0.0, lightCameraFar = 0.0;

				lightCameraToWorld.matrix.GetColumnVectors(lightCameraXAxis, lightCameraYAxis, lightCameraZAxis);
				lightCameraWidth = orthoParams.width;
				lightCameraHeight = orthoParams.height;
				lightCameraNear = orthoParams.nearClip;
				lightCameraFar = orthoParams.farClip;

				if (shader->GetConstantInfo("lightCameraEyePoint", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraEyePoint);

				if (shader->GetConstantInfo("lightCameraXAxis", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraXAxis);

				if (shader->GetConstantInfo("lightCameraYAxis", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraYAxis);

				if (shader->GetConstantInfo("lightCameraWidth", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraWidth);

				if (shader->GetConstantInfo("lightCameraHeight", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraHeight);

				if (shader->GetConstantInfo("lightCameraNear", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraNear);

				if (shader->GetConstantInfo("lightCameraFar", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraFar);
			}
		}

		deviceContext->Unmap(constantsBuffer, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &constantsBuffer);
		deviceContext->PSSetConstantBuffers(0, 1, &constantsBuffer);
	}

	if (renderPass == RenderPass::MAIN_PASS)
	{
		std::vector<ID3D11ShaderResourceView*> shaderResourceViewArray;
		std::vector<ID3D11SamplerState*> samplerStateArray;

		if (texture)
		{
			shaderResourceViewArray.push_back(texture->GetTextureView());
			samplerStateArray.push_back(Game::Get()->GetGeneralSamplerState());
		}

		ID3D11ShaderResourceView* shadowBufferResourceView = Game::Get()->GetShadowBufferResourceViewForShader();
		ID3D11SamplerState* shadowBufferSamplerState = Game::Get()->GetGeneralSamplerState();

		if (shadowBufferResourceView && shadowBufferSamplerState)
		{
			shaderResourceViewArray.push_back(shadowBufferResourceView);
			samplerStateArray.push_back(shadowBufferSamplerState);
		}

		deviceContext->PSSetShaderResources(0, shaderResourceViewArray.size(), shaderResourceViewArray.data());
		deviceContext->PSSetSamplers(0, samplerStateArray.size(), samplerStateArray.data());
	}
	else if (renderPass == RenderPass::SHADOW_PASS)
	{
		deviceContext->PSSetShaderResources(0, 0, NULL);
		deviceContext->PSSetSamplers(0, 0, NULL);
	}

	if (!indexBuffer)
		deviceContext->Draw(this->mesh->GetVertexBuffer()->GetNumElements(), 0);
	else
	{
		ID3D11Buffer* indexBufferIface = indexBuffer->GetBuffer();
		DXGI_FORMAT format = indexBuffer->GetFormat();
		deviceContext->IASetIndexBuffer(indexBufferIface, format, 0);

		UINT numElements = indexBuffer->GetNumElements();
		deviceContext->DrawIndexed(numElements, 0, 0);
	}
#endif
}

/*virtual*/ void RenderMeshInstance::GetWorldBoundingSphere(Vector3& center, double& radius) const
{
	this->objectSpaceBoundingBox.GetSphere(center, radius);
	center = this->objectToWorld.TransformPoint(center);
}