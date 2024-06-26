#include "SkyDomeRenderObject.h"
#include "Camera.h"
#include "Game.h"

using namespace Imzadi;

SkyDomeRenderObject::SkyDomeRenderObject()
{
}

/*virtual*/ SkyDomeRenderObject::~SkyDomeRenderObject()
{
}

/*virtual*/ void SkyDomeRenderObject::Render(Camera* camera, RenderPass renderPass)
{
	if (renderPass != RenderPass::MAIN_PASS)
		return;

	Shader* shader = this->skyDome->GetShader();
	if (!shader)
		return;

	CubeTexture* cubeTexture = this->skyDome->GetCubeTexture();
	if (!cubeTexture)
		return;

	Buffer* vertexBuffer = this->skyDome->GetVertexBuffer();
	Buffer* indexBuffer = this->skyDome->GetIndexBuffer();

	ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();

	deviceContext->IASetPrimitiveTopology(this->skyDome->GetPrimType());
	deviceContext->IASetInputLayout(shader->GetInputLayout());

	deviceContext->VSSetShader(shader->GetVertexShader(), NULL, 0);
	deviceContext->PSSetShader(shader->GetPixelShader(), NULL, 0);

	ID3D11Buffer* indexBufferIface = indexBuffer->GetBuffer();
	DXGI_FORMAT format = indexBuffer->GetFormat();
	deviceContext->IASetIndexBuffer(indexBufferIface, format, 0);

	UINT stride = vertexBuffer->GetStride();
	UINT offset = 0;
	ID3D11Buffer* vertexBufferIface = vertexBuffer->GetBuffer();
	deviceContext->IASetVertexBuffers(0, 1, &vertexBufferIface, &stride, &offset);

	ID3D11ShaderResourceView* shaderResourceView = cubeTexture->GetTextureView();
	ID3D11SamplerState* samplerState = Game::Get()->GetGeneralSamplerState();
	deviceContext->PSSetShaderResources(0, 1, &shaderResourceView);
	deviceContext->PSSetSamplers(0, 1, &samplerState);

	ID3D11Buffer* constantsBuffer = shader->GetConstantsBuffer();
	if (constantsBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		deviceContext->Map(constantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);

		UINT bufferSize = shader->GetConstantsBufferSize();
		::memset(mappedSubresource.pData, 0, bufferSize);

		const Shader::Constant* constant = nullptr;

		Matrix4x4 objectToCameraMat;
		camera->GetWorldToCameraTransform().GetToMatrix(objectToCameraMat);

		Matrix4x4 cameraToProjMat;
		camera->GetProjectionMatrix(cameraToProjMat);

		Matrix4x4 objectToProjMat = cameraToProjMat * objectToCameraMat;

		if (shader->GetConstantInfo("objectToProjection", constant))
			StoreShaderConstant(&mappedSubresource, constant, &objectToProjMat);

		deviceContext->Unmap(constantsBuffer, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &constantsBuffer);
		deviceContext->PSSetConstantBuffers(0, 1, &constantsBuffer);
	}

	UINT numElements = indexBuffer->GetNumElements();
	deviceContext->DrawIndexed(numElements, 0, 0);
}

/*virtual*/ void SkyDomeRenderObject::GetWorldBoundingSphere(Vector3& center, double& radius) const
{
	// TODO: Write this.
}