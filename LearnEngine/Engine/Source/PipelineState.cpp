#include "PipelineState.h"
#include "InputLayout.h"
#include "Shader.h"
#include "D3DRHI.h"
#include "D3DDevice.h"

TGraphicsPSODescriptor::TGraphicsPSODescriptor()
{
	{
		SampleDesc.Count = 1;
		SampleDesc.Quality = 0;
	}

	{
		RasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		RasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		RasterizerDesc.FrontCounterClockwise = FALSE;
		RasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		RasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		RasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		RasterizerDesc.DepthClipEnable = TRUE;
		RasterizerDesc.MultisampleEnable = FALSE;
		RasterizerDesc.AntialiasedLineEnable = FALSE;
		RasterizerDesc.ForcedSampleCount = 0;
		RasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	}

	{
		BlendDesc.AlphaToCoverageEnable = FALSE;
		BlendDesc.IndependentBlendEnable = FALSE;
		const D3D12_RENDER_TARGET_BLEND_DESC DefaultRenderTargetBlendDesc =
		{
			FALSE,FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL,
		};
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		{
			BlendDesc.RenderTarget[i] = DefaultRenderTargetBlendDesc;
		}
	}

	{
		DepthStencilDesc.DepthEnable = TRUE;
		DepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		DepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		DepthStencilDesc.StencilEnable = FALSE;
		DepthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		DepthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		const D3D12_DEPTH_STENCILOP_DESC DefaultStencilOp =
		{ 
			D3D12_STENCIL_OP_KEEP, 
			D3D12_STENCIL_OP_KEEP, 
			D3D12_STENCIL_OP_KEEP, 
			D3D12_COMPARISON_FUNC_ALWAYS 
		};
		DepthStencilDesc.FrontFace = DefaultStencilOp;
		DepthStencilDesc.BackFace = DefaultStencilOp;
	}
}


TGraphicsPSOManager::TGraphicsPSOManager(TD3DRHI* InD3DRHI, TInputLayoutManager* InInputLayoutManager)
	:D3DRHI(InD3DRHI), InputLayoutManager(InInputLayoutManager)
{

}

ID3D12PipelineState* TGraphicsPSOManager::FindAndCreate(const TGraphicsPSODescriptor& Descriptor)
{
	if (PSOMap.find(Descriptor) == PSOMap.end())
	{
		this->TryCreate(Descriptor);
	}
	auto Iter = PSOMap.find(Descriptor);
	return Iter->second.Get();
}

void TGraphicsPSOManager::TryCreate(const TGraphicsPSODescriptor& Descriptor)
{
	if (PSOMap.find(Descriptor) == PSOMap.end())
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc;
		ZeroMemory(&PsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
		InputLayoutManager->GetInputLayout(Descriptor.InputLayoutName, InputLayout);
		PsoDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };

		// Shader
		TShader* Shader = Descriptor.Shader;
		PsoDesc.pRootSignature = Shader->RootSignature.Get();
		PsoDesc.VS = D3D12_SHADER_BYTECODE({ Shader->ShaderPass.at("VS")->GetBufferPointer(), Shader->ShaderPass.at("VS")->GetBufferSize() });
		PsoDesc.PS = D3D12_SHADER_BYTECODE({ Shader->ShaderPass.at("PS")->GetBufferPointer(), Shader->ShaderPass.at("PS")->GetBufferSize() });

		PsoDesc.RasterizerState = Descriptor.RasterizerDesc;
		PsoDesc.BlendState = Descriptor.BlendDesc;
		PsoDesc.DepthStencilState = Descriptor.DepthStencilDesc;
		PsoDesc.PrimitiveTopologyType = Descriptor.PrimitiveTopologyType;
		PsoDesc.NumRenderTargets = Descriptor.NumRenderTargets;

		PsoDesc.SampleMask = UINT_MAX;
		PsoDesc.SampleDesc = Descriptor.SampleDesc;

		for (int i = 0; i < 8; i++)
		{
			PsoDesc.RTVFormats[i] = Descriptor.RTVFormats[i];
		}
		PsoDesc.DSVFormat = Descriptor.DSVFormat;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> PSO;
		auto D3DDevice = D3DRHI->GetDevice()->GetD3DDevice();
		ThrowIfFailed(D3DDevice->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PSO)));
		PSOMap.insert({ Descriptor, PSO });
	}
}
