//-------------------------------------------------------------------------------------------------
// File : main.cpp
// Desc : Main Entry Point.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3d.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <string>
#include <SampleApp.h>
#include <SampleAllocator.h>
#include <SampleUtil.h>
#include <SampleMath.h>
#include <SampleGui.h>
#include <imgui/imgui.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    Vec3 Position;  //!< �ʒu���W�ł�.
    Vec4 Color;     //!< ���_�J���[�ł�.
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Transform structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Transform
{
    Mat4 World;     //!< ���[���h�s��ł�.
    Mat4 View;      //!< �r���[�s��ł�.
    Mat4 Proj;      //!< �ˉe�s��ł�.
};


//-------------------------------------------------------------------------------------------------
// Forward declarations.
//-------------------------------------------------------------------------------------------------
bool InitA3D();
void TermA3D();
void DrawA3D();
void Resize(uint32_t w, uint32_t h, void* ptr);

//-------------------------------------------------------------------------------------------------
// Global Varaibles.
//-------------------------------------------------------------------------------------------------
IApp*                       g_pApp                  = nullptr;  //!< �E�B���h�E�̐������s���w���p�[�N���X�ł�.
a3d::IDevice*               g_pDevice               = nullptr;  //!< �f�o�C�X�ł�.
a3d::ISwapChain*            g_pSwapChain            = nullptr;  //!< �X���b�v�`�F�C���ł�.
a3d::IQueue*                g_pGraphicsQueue        = nullptr;  //!< �R�}���h�L���[�ł�.
a3d::IFence*                g_pFence                = nullptr;  //!< �t�F���X�ł�.
a3d::IDescriptorSetLayout*  g_pDescriptorSetLayout  = nullptr;  //!< �f�B�X�N���v�^�Z�b�g���C�A�E�g�ł�.
a3d::IPipelineState*        g_pPipelineState        = nullptr;  //!< �p�C�v���C���X�e�[�g�ł�.
a3d::IBuffer*               g_pVertexBuffer         = nullptr;  //!< ���_�o�b�t�@�ł�.
a3d::IBuffer*               g_pIndexBuffer          = nullptr;  //!< �C���f�b�N�X�o�b�t�@�ł�.
a3d::ITexture*              g_pColorBuffer[2]       = {};       //!< �J���[�o�b�t�@�ł�.
a3d::ITextureView*          g_pColorView[2]         = {};       //!< �J���[�r���[�ł�.
a3d::ICommandList*          g_pCommandList[2]       = {};       //!< �R�}���h���X�g�ł�.
a3d::IFrameBuffer*          g_pFrameBuffer[2]       = {};       //!< �t���[���o�b�t�@�ł�.
a3d::IBuffer*               g_pConstantBuffer[2]    = {};       //!< �萔�o�b�t�@�ł�.
a3d::IBufferView*           g_pConstantView[2]      = {};       //!< �萔�o�b�t�@�r���[�ł�.
a3d::IDescriptorSet*        g_pDescriptorSet[2]     = {};       //!< �f�B�X�N���v�^�Z�b�g�ł�.
a3d::Viewport               g_Viewport              = {};       //!< �r���[�|�[�g�ł�.
a3d::Rect                   g_Scissor               = {};       //!< �V�U�[��`�ł�.
a3d::IQueryPool*            g_pQueryPool[2]         = {};       //!< �N�G���v�[���ł�.
a3d::IBuffer*               g_pQueryResultBuffer[2] = {};       //!< �N�G�����ʂ��i�[����o�b�t�@�ł�.
Transform                   g_Transform             = {};       //!< �ϊ��s��ł�.
void*                       g_pCbHead[2]            = {};       //!< �萔�o�b�t�@�̐擪�|�C���^�ł�.
float                       g_RotateAngle           = 0.0f;     //!< ��]�p�ł�.
bool                        g_Prepare               = false;    //!< ����������������true.
uint32_t                    g_PrevBufferIndex       = 0;        //!< �O�̃o�b�t�@�ԍ�.
SampleAllocator             g_Allocator;                        //!< �A���P�[�^.


//-------------------------------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
//-------------------------------------------------------------------------------------------------
void Main()
{
    // �E�B���h�E����.
    if (!CreateApp(960, 540, &g_pApp))
    { return; }

    // ���T�C�Y���̃R�[���o�b�N�֐���ݒ�.
    g_pApp->SetResizeCallback(Resize, nullptr);

    // A3D������.
    if (!InitA3D())
    {
        g_pApp->Release();
        return;
    }

    // ���C�����[�v.
    while( g_pApp->IsLoop() )
    {
        // �`�揈��.
        DrawA3D();
    }

    // ��n��.
    TermA3D();
    g_pApp->Release();
}

//-------------------------------------------------------------------------------------------------
//      A3D�̏��������s���܂�.
//-------------------------------------------------------------------------------------------------
bool InitA3D()
{
    g_Prepare = false;

    // �O���t�B�b�N�X�V�X�e���̏�����.
    {
        a3d::SystemDesc desc = {};
        desc.pSystemAllocator = &g_Allocator;

        if (!a3d::InitSystem(&desc))
        { return false; }
    }

    // �f�o�C�X�̐���.
    {
        a3d::DeviceDesc desc = {};

        desc.EnableDebug = true;

        // �ő�f�B�X�N���v�^����ݒ�.
        desc.MaxColorTargetCount            = 256;
        desc.MaxDepthTargetCount            = 256;
        desc.MaxShaderResourceCount         = 256;
        desc.MaxSamplerCount                = 256;

        // �ő�T�u�~�b�g����ݒ�.
        desc.MaxGraphicsQueueSubmitCount    = 256;
        desc.MaxCopyQueueSubmitCount        = 256;
        desc.MaxComputeQueueSubmitCount     = 256;

        // �f�o�C�X�𐶐�.
        if (!a3d::CreateDevice(&desc, &g_pDevice))
        { return false; }

        // �R�}���h�L���[���擾.
        g_pDevice->GetGraphicsQueue(&g_pGraphicsQueue);
    }

    auto info = g_pDevice->GetInfo();

    #if SAMPLE_IS_VULKAN && TARGET_PC
        auto format = a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM;
    #else
        auto format = a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM;
    #endif

    // �X���b�v�`�F�C���̐���.
    {
        a3d::SwapChainDesc desc = {};
        desc.Extent.Width   = g_pApp->GetWidth();
        desc.Extent.Height  = g_pApp->GetHeight();
        desc.Format         = format;
        desc.MipLevels      = 1;
        desc.SampleCount    = 1;
        desc.BufferCount    = 2;
        desc.SyncInterval   = 1;
        desc.InstanceHandle = g_pApp->GetInstanceHandle();
        desc.WindowHandle   = g_pApp->GetWindowHandle();

        if (!g_pDevice->CreateSwapChain(&desc, &g_pSwapChain))
        { return false; }

        // �X���b�v�`�F�C������o�b�t�@���擾.
        g_pSwapChain->GetBuffer(0, &g_pColorBuffer[0]);
        g_pSwapChain->GetBuffer(1, &g_pColorBuffer[1]);

        a3d::TextureViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = format;
        viewDesc.TextureAspect      = a3d::TEXTURE_ASPECT_COLOR;
        viewDesc.MipSlice           = 0;
        viewDesc.MipLevels          = desc.MipLevels;
        viewDesc.FirstArraySlice    = 0;
        viewDesc.ArraySize          = 1;
        viewDesc.ComponentMapping.R = a3d::TEXTURE_SWIZZLE_R;
        viewDesc.ComponentMapping.G = a3d::TEXTURE_SWIZZLE_G;
        viewDesc.ComponentMapping.B = a3d::TEXTURE_SWIZZLE_B;
        viewDesc.ComponentMapping.A = a3d::TEXTURE_SWIZZLE_A;

        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateTextureView(g_pColorBuffer[i], &viewDesc, &g_pColorView[i]))
            { return false; }
        }
    }

    // �t���[���o�b�t�@�̐���
    {
        // �t���[���o�b�t�@�̐ݒ�.
        a3d::FrameBufferDesc desc = {};
        desc.ColorCount         = 1;
        desc.pColorTargets[0]   = g_pColorView[0];
        desc.pDepthTarget       = nullptr;

        // 1���ڂ̃t���[���o�b�t�@�𐶐�.
        if (!g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[0]))
        { return false; }

        // 2���ڂ̃t���[���o�b�t�@�𐶐�.
        desc.pColorTargets[0] = g_pColorView[1];
        if (!g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[1]))
        { return false; }
    }

    // �R�}���h���X�g�𐶐�.
    {
        a3d::CommandListDesc desc = {};
        desc.Type       = a3d::COMMANDLIST_TYPE_DIRECT;
        desc.BufferSize = 4096;

        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateCommandList(&desc, &g_pCommandList[i]))
            { return false; }
        }
    }

    // �t�F���X�𐶐�.
    {
        if (!g_pDevice->CreateFence(&g_pFence))
        { return false; }
    }

    // ���_�o�b�t�@�𐶐�.
    {
        Vertex vertices[] = {
            { Vec3( 1.0f, -1.0f, 0.0f), Vec4(1.0f, 0.0f, 0.0f, 1.0f) },
            { Vec3(-1.0f, -1.0f, 0.0f), Vec4(0.0f, 1.0f, 0.0f, 1.0f) },
            { Vec3(-1.0f,  1.0f, 0.0f), Vec4(0.0f, 0.0f, 1.0f, 1.0f) },
            { Vec3( 1.0f,  1.0f, 0.0f), Vec4(1.0f, 0.0f, 1.0f, 1.0f) },
        };

        a3d::BufferDesc desc = {};
        desc.Size       = sizeof(vertices);
        desc.Stride     = sizeof(Vertex);
        desc.InitState  = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage      = a3d::RESOURCE_USAGE_VERTEX_BUFFER;
        desc.HeapType   = a3d::HEAP_TYPE_UPLOAD;

        if ( !g_pDevice->CreateBuffer(&desc, &g_pVertexBuffer) )
        { return false; }

        auto ptr = g_pVertexBuffer->Map();
        if ( ptr == nullptr )
        { return false; }

        memcpy( ptr, vertices, sizeof(vertices) );
        
        g_pVertexBuffer->Unmap();
    }

    // �C���f�b�N�X�o�b�t�@�𐶐�.
    {
        uint32_t indices[] = {
            0, 1, 2,
            2, 0, 3,
        };

        a3d::BufferDesc desc = {};
        desc.Size       = sizeof(indices);
        desc.Stride     = sizeof(uint32_t);
        desc.InitState  = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage      = a3d::RESOURCE_USAGE_INDEX_BUFFER;
        desc.HeapType   = a3d::HEAP_TYPE_UPLOAD;

        if ( !g_pDevice->CreateBuffer(&desc, &g_pIndexBuffer) )
        { return false; }

        auto ptr = g_pIndexBuffer->Map();
        if ( ptr == nullptr )
        { return false; }

        memcpy( ptr, indices, sizeof(indices) );

        g_pIndexBuffer->Unmap();
    }

    // �萔�o�b�t�@�𐶐�.
    {
        auto stride = a3d::RoundUp<uint32_t>( sizeof(Transform), info.ConstantBufferMemoryAlignment );

        a3d::BufferDesc desc = {};
        desc.Size       = stride;
        desc.Stride     = stride;
        desc.InitState  = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage      = a3d::RESOURCE_USAGE_CONSTANT_BUFFER;
        desc.HeapType   = a3d::HEAP_TYPE_UPLOAD;

        a3d::BufferViewDesc viewDesc = {};
        viewDesc.Offset = 0;
        viewDesc.Range  = stride;

        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateBuffer(&desc, &g_pConstantBuffer[i]))
            { return false; }

            if (!g_pDevice->CreateBufferView(g_pConstantBuffer[i], &viewDesc, &g_pConstantView[i]))
            { return false; }

            g_pCbHead[i] = g_pConstantBuffer[i]->Map();
        }

        Vec3 pos = Vec3(0.0f, 0.0f, -5.0f);
        Vec3 at  = Vec3(0.0f, 0.0f, 0.0f);
        Vec3 up  = Vec3(0.0f, 1.0f, 0.0f);

        auto aspect = static_cast<float>(g_pApp->GetWidth()) / static_cast<float>(g_pApp->GetHeight());

        g_Transform.World = Mat4::Identity();
        g_Transform.View  = Mat4::LookAt(pos, at, up);
        g_Transform.Proj  = Mat4::PersFov(ToRadian(45.0f), aspect, 0.1f, 1000.0f);
    }

    // �V�F�[�_�o�C�i����ǂݍ��݂܂�.
    a3d::ShaderBinary vs = {};
    a3d::ShaderBinary ps = {};
    {
        auto vsFilePath = GetShaderPathForSampleProgram("simpleVS");
        auto psFilePath = GetShaderPathForSampleProgram("simplePS");

        if (!LoadShaderBinary(vsFilePath.c_str(), vs))
        { return false; }

        if (!LoadShaderBinary(psFilePath.c_str(), ps))
        {
            DisposeShaderBinary(vs);
            return false;
        }
    }

    // �f�B�X�N���v�^�Z�b�g���C�A�E�g�𐶐����܂�.
    {
        a3d::DescriptorSetLayoutDesc desc = {};
        desc.MaxSetCount               = 2;
        desc.EntryCount                = 1;
        desc.Entries[0].ShaderMask     = a3d::SHADER_MASK_VERTEX;
        desc.Entries[0].ShaderRegister = 0;
        desc.Entries[0].Type           = a3d::DESCRIPTOR_TYPE_CBV;

        if (!g_pDevice->CreateDescriptorSetLayout(&desc, &g_pDescriptorSetLayout))
        { return false; }

        for(auto i=0; i<2; ++i)
        {
            if (!g_pDescriptorSetLayout->CreateDescriptorSet(&g_pDescriptorSet[i]))
            { return false; }

            g_pDescriptorSet[i]->SetView(0, g_pConstantView[i]);
            g_pDescriptorSet[i]->Update();
        }
    }

    // �O���t�B�b�N�X�p�C�v���C���X�e�[�g�𐶐����܂�.
    {
        // ���͗v�f�ł�.
        a3d::InputElementDesc inputElements[] = {
            { a3d::SEMANTICS_POSITION, a3d::RESOURCE_FORMAT_R32G32B32_FLOAT   , 0,  0, a3d::INPUT_CLASSIFICATION_PER_VERTEX },
            { a3d::SEMANTICS_COLOR0  , a3d::RESOURCE_FORMAT_R32G32B32A32_FLOAT, 0, 12, a3d::INPUT_CLASSIFICATION_PER_VERTEX },
        };

        // ���̓��C�A�E�g�ł�.
        a3d::InputLayoutDesc inputLayout = {};
        inputLayout.ElementCount = 2;
        inputLayout.pElements    = inputElements;

        // �X�e���V���e�X�g�ݒ�ł�.
        a3d::StencilTestDesc stencilTest = {};
        stencilTest.StencilFailOp      = a3d::STENCIL_OP_KEEP;
        stencilTest.StencilDepthFailOp = a3d::STENCIL_OP_KEEP;
        stencilTest.StencilFailOp      = a3d::STENCIL_OP_KEEP;
        stencilTest.StencilCompareOp   = a3d::COMPARE_OP_NEVER;

        // �O���t�B�b�N�X�p�C�v���C���X�e�[�g��ݒ肵�܂�.
        a3d::GraphicsPipelineStateDesc desc = {};

        // �V�F�[�_�̐ݒ�.
        desc.VS = vs;
        desc.PS = ps;

        // �u�����h�X�e�[�g�̐ݒ�.
        desc.BlendState.IndependentBlendEnable          = false;
        desc.BlendState.LogicOpEnable                   = false;
        desc.BlendState.LogicOp                         = a3d::LOGIC_OP_NOOP;
        for(auto i=0; i<8; ++i)
        {
            desc.BlendState.ColorTarget[i].BlendEnable      = false;
            desc.BlendState.ColorTarget[i].SrcBlend         = a3d::BLEND_FACTOR_ONE;
            desc.BlendState.ColorTarget[i].DstBlend         = a3d::BLEND_FACTOR_ZERO;
            desc.BlendState.ColorTarget[i].BlendOp          = a3d::BLEND_OP_ADD;
            desc.BlendState.ColorTarget[i].SrcBlendAlpha    = a3d::BLEND_FACTOR_ONE;
            desc.BlendState.ColorTarget[i].DstBlendAlpha    = a3d::BLEND_FACTOR_ZERO;
            desc.BlendState.ColorTarget[i].BlendOpAlpha     = a3d::BLEND_OP_ADD;
            desc.BlendState.ColorTarget[i].EnableWriteR     = true;
            desc.BlendState.ColorTarget[i].EnableWriteG     = true;
            desc.BlendState.ColorTarget[i].EnableWriteB     = true;
            desc.BlendState.ColorTarget[i].EnableWriteA     = true;
        }

        // ���X�^���C�U�\�X�e�[�g�̐ݒ�.
        desc.RasterizerState.PolygonMode                = a3d::POLYGON_MODE_SOLID;
        desc.RasterizerState.CullMode                   = a3d::CULL_MODE_NONE;
        desc.RasterizerState.FrontCounterClockWise      = false;
        desc.RasterizerState.DepthBias                  = 0;
        desc.RasterizerState.DepthBiasClamp             = 0.0f;
        desc.RasterizerState.SlopeScaledDepthBias       = 0;
        desc.RasterizerState.DepthClipEnable            = false;
        desc.RasterizerState.EnableConservativeRaster   = false;
        
        // �}���`�T���v���X�e�[�g�̐ݒ�.
        desc.MultiSampleState.EnableAlphaToCoverage = false;
        desc.MultiSampleState.EnableMultiSample     = false;
        desc.MultiSampleState.SampleCount           = 1;

        // �[�x�X�e�[�g�̐ݒ�.
        desc.DepthState.DepthTestEnable      = true;
        desc.DepthState.DepthWriteEnable     = true;
        desc.DepthState.DepthCompareOp       = a3d::COMPARE_OP_LEQUAL;

        // �X�e���V���X�e�[�g�̐ݒ�.
        desc.StencilState.StencilTestEnable    = false;
        desc.StencilState.StencllReadMask      = 0;
        desc.StencilState.StencilWriteMask     = 0;
        desc.StencilState.FrontFace            = stencilTest;
        desc.StencilState.BackFace             = stencilTest;

        // �e�b�Z���[�V�����X�e�[�g�̐ݒ�.
        desc.TessellationState.PatchControlCount = 0;

        // ���̓A�E�g�̐ݒ�.
        desc.InputLayout = inputLayout;

        // �f�B�X�N���v�^�Z�b�g���C�A�E�g�̐ݒ�.
        desc.pLayout = g_pDescriptorSetLayout;

        // �v���~�e�B�u�g�|���W�[�̐ݒ�.
        desc.PrimitiveTopology = a3d::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        // �t�H�[�}�b�g�̐ݒ�.
        desc.ColorCount                 = 1;
        desc.ColorTarget[0].Format      = format;
        desc.ColorTarget[0].SampleCount = 1;

        // �L���b�V���ς݃p�C�v���C���X�e�[�g�̐ݒ�.
        desc.pCachedPSO = nullptr;

        // �O���t�B�b�N�X�p�C�v���C���X�e�[�g�̐���.
        if (!g_pDevice->CreateGraphicsPipeline(&desc, &g_pPipelineState))
        {
            DisposeShaderBinary(vs);
            DisposeShaderBinary(ps);
            return false;
        }
    }
    
    // �s�v�ɂȂ����̂Ŕj�����܂�.
    DisposeShaderBinary(vs);
    DisposeShaderBinary(ps);


    // �r���[�|�[�g�̐ݒ�.
    g_Viewport.X        = 0.0f;
    g_Viewport.Y        = 0.0f;
    g_Viewport.Width    = static_cast<float>(g_pApp->GetWidth());
    g_Viewport.Height   = static_cast<float>(g_pApp->GetHeight());
    g_Viewport.MinDepth = 0.0f;
    g_Viewport.MaxDepth = 1.0f;

    // �V�U�[��`�̐ݒ�.
    g_Scissor.Offset.X      = 0;
    g_Scissor.Offset.Y      = 0;
    g_Scissor.Extent.Width  = g_pApp->GetWidth();
    g_Scissor.Extent.Height = g_pApp->GetHeight();

    // �N�G���v�[���̐���.
    {
        a3d::QueryPoolDesc desc = {};
        desc.Count = 1;
        desc.Type  = a3d::QUERY_TYPE_OCCLUSION;

        for(auto i=0; i<2; ++i)
        {
            if(!g_pDevice->CreateQueryPool(&desc, &g_pQueryPool[i]))
            { return false; }
        }
    }

    // �N�G�����ʊi�[�o�b�t�@�̐���.
    {
        a3d::BufferDesc desc = {};
        desc.Size       = sizeof(uint64_t);
        desc.Stride     = sizeof(uint64_t);
        desc.InitState  = a3d::RESOURCE_STATE_COPY_DST;
        desc.Usage      = a3d::RESOURCE_USAGE_QUERY_BUFFER;
        desc.HeapType   = a3d::HEAP_TYPE_READBACK;

        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateBuffer(&desc, &g_pQueryResultBuffer[i]))
            { return false; }
        }
    }

    // GUI�}�l�[�W���̏�����.
    if (!GuiMgr::GetInstance().Init(g_pDevice, g_pFrameBuffer[0], g_pApp))
    { return false; }

    g_Prepare = true;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      A3D�̏I���������s���܂�.
//-------------------------------------------------------------------------------------------------
void TermA3D()
{
    g_Prepare = false;

    GuiMgr::GetInstance().Term();

    // �_�u���o�b�t�@���\�[�X�̔j��.
    for(auto i=0; i<2; ++i)
    {
        // �N�G���o�b�t�@��j�����܂�.
        a3d::SafeRelease(g_pQueryResultBuffer[i]);

        // �N�G���v�[����j�����܂�.
        a3d::SafeRelease(g_pQueryPool[i]);

        // �t���[���o�b�t�@�̔j��.
        a3d::SafeRelease(g_pFrameBuffer[i]);

        // �J���[�r���[�̔j��.
        a3d::SafeRelease(g_pColorView[i]);

        // �J���[�o�b�t�@�̔j��.
        a3d::SafeRelease(g_pColorBuffer[i]);
        
        // �R�}���h���X�g�̔j��.
        a3d::SafeRelease(g_pCommandList[i]);

        // �萔�o�b�t�@�r���[�̔j��.
        a3d::SafeRelease(g_pConstantView[i]);

        // �j���O�Ƀ������}�b�s���O������.
        if (g_pConstantBuffer[i] != nullptr)
        { g_pConstantBuffer[i]->Unmap(); }

        // �萔�o�b�t�@�̔j��.
        a3d::SafeRelease(g_pConstantBuffer[i]);

        // �f�B�X�N���v�^�Z�b�g�̔j��.
        a3d::SafeRelease(g_pDescriptorSet[i]);
    }

    // �p�C�v���C���X�e�[�g�̔j��.
    a3d::SafeRelease(g_pPipelineState);

    // ���_�o�b�t�@�̔j��.
    a3d::SafeRelease(g_pVertexBuffer);

    // �C���f�b�N�X�o�b�t�@�̔j��.
    a3d::SafeRelease(g_pIndexBuffer);

    // �f�B�X�N���v�^�Z�b�g���C�A�E�g�̔j��.
    a3d::SafeRelease(g_pDescriptorSetLayout);

    // �t�F���X�̔j��.
    a3d::SafeRelease(g_pFence);

    // �X���b�v�`�F�C���̔j��.
    a3d::SafeRelease(g_pSwapChain);

    // �O���t�B�b�N�X�L���[�̔j��.
    a3d::SafeRelease(g_pGraphicsQueue);

    // �f�o�C�X�̔j��.
    a3d::SafeRelease(g_pDevice);

    // �O���t�B�b�N�X�V�X�e���̏I������.
    a3d::TermSystem();
}

//-------------------------------------------------------------------------------------------------
//      A3D�ɂ��`�揈�����s���܂�.
//-------------------------------------------------------------------------------------------------
void DrawA3D()
{
    if (!g_Prepare)
    { return; }

    // �o�b�t�@�ԍ����擾���܂�.
    auto idx = g_pSwapChain->GetCurrentBufferIndex();

    // �萔�o�b�t�@���X�V.
    {
        g_RotateAngle += 0.01f;
        g_Transform.World = Mat4::RotateY(g_RotateAngle);

        memcpy(g_pCbHead[idx], &g_Transform, sizeof(g_Transform));
    }

    // �R�}���h�̋L�^���J�n���܂�.
    auto pCmd = g_pCommandList[idx];
    pCmd->Begin();

    // �N�G�������Z�b�g���܂�.
    pCmd->ResetQuery(g_pQueryPool[idx]);

    // �������ݗp�̃o���A��ݒ肵�܂�.
    pCmd->TextureBarrier(
        g_pColorBuffer[idx],
        a3d::RESOURCE_STATE_PRESENT,
        a3d::RESOURCE_STATE_COLOR_WRITE);

    // �t���[���o�b�t�@��ݒ肵�܂�.
    pCmd->BeginFrameBuffer(g_pFrameBuffer[idx]);

    // �t���[���o�b�t�@���N���A���܂�.
    a3d::ClearColorValue clearColor = {};
    clearColor.Float[0] = 0.25f;
    clearColor.Float[1] = 0.25f;
    clearColor.Float[2] = 0.25f;
    clearColor.Float[3] = 1.0f;
    pCmd->ClearFrameBuffer(1, &clearColor, nullptr);

    {
        // �N�G���J�n.
        pCmd->BeginQuery(g_pQueryPool[idx], 0);

        // �p�C�v���C���X�e�[�g��ݒ肵�܂�.
        pCmd->SetPipelineState(g_pPipelineState);

        // �f�B�X�N���v�^�Z�b�g��ݒ肵�܂�.
        pCmd->SetDescriptorSet(g_pDescriptorSet[idx]);

        // �r���[�|�[�g�ƃV�U�[��`��ݒ肵�܂�.
        // NOTE : �r���[�|�[�g�ƃV�U�[��`�̐ݒ�́C�K��SetPipelineState() �̌�ł���K�v������܂�.
        pCmd->SetViewports(1, &g_Viewport);
        pCmd->SetScissors (1, &g_Scissor);

        // ���_�o�b�t�@��ݒ肵�܂�.
        pCmd->SetVertexBuffers(0, 1, &g_pVertexBuffer, nullptr);
        pCmd->SetIndexBuffer(g_pIndexBuffer, 0);

        // �O�p�`��`�悵�܂�.
        pCmd->DrawIndexedInstanced(6, 1, 0, 0, 0);

        // �N�G���I��.
        pCmd->EndQuery(g_pQueryPool[idx], 0);
    }

    // Debug GUI�̕`��.
    {
        // �`��J�n.
        GuiMgr::GetInstance().SwapBuffers();

        {
            uint64_t sampleCount = 0;
            auto ptr = g_pQueryResultBuffer[g_PrevBufferIndex]->Map();
            memcpy(&sampleCount, ptr, sizeof(uint64_t));
            g_pQueryResultBuffer[g_PrevBufferIndex]->Unmap();

            ImGuiWindowFlags flag = 0;
            flag |= ImGuiWindowFlags_NoResize;
            flag |= ImGuiWindowFlags_NoMove;
            flag |= ImGuiWindowFlags_NoTitleBar;

            ImGui::Begin("Info", nullptr, flag);
            ImGui::SetWindowPos(ImVec2(10, 10));
            ImGui::SetWindowSize(ImVec2(160, 50));
            ImGui::Text("FPS : %.3f", ImGui::GetIO().Framerate);
            ImGui::Text("Query Sample : %lu", sampleCount);
            ImGui::End();
        }

        // �`��I��.
        GuiMgr::GetInstance().Issue(pCmd);
    }
    
    // �\���p�̃o���A��ݒ肷��O�ɁC�t���[���o�b�t�@�̐ݒ����������K�v������܂�.
    pCmd->EndFrameBuffer();

    // �N�G�����ʂ��擾���܂�.(ResolveQuery()�̓t���[���o�b�t�@���o�C���h����Ă��Ȃ���ԂŌĂԕK�v������܂�).
    pCmd->ResolveQuery(g_pQueryPool[idx], 0, 1, g_pQueryResultBuffer[idx], 0);

    // �\���p�Ƀo���A��ݒ肵�܂�.
    pCmd->TextureBarrier(
        g_pColorBuffer[idx],
        a3d::RESOURCE_STATE_COLOR_WRITE,
        a3d::RESOURCE_STATE_PRESENT);

    // �R�}���h���X�g�ւ̋L�^���I�����܂�.
    pCmd->End();

    // �R�}���h�L���[�ɓo�^���܂�.
    g_pGraphicsQueue->Submit(pCmd);

    // �R�}���h�����s���܂�.
    g_pGraphicsQueue->Execute(g_pFence);

    // �R�}���h�����s������ҋ@���܂�.
    if (!g_pFence->IsSignaled())
    { g_pFence->Wait(UINT32_MAX); }

    g_PrevBufferIndex = idx;

    // ��ʂɕ\�����܂�.
    g_pGraphicsQueue->Present( g_pSwapChain );

}

//-------------------------------------------------------------------------------------------------
//      ���T�C�Y�����ł�.
//-------------------------------------------------------------------------------------------------
void Resize( uint32_t w, uint32_t h, void* pUser )
{
    A3D_UNUSED( pUser );

    // �������������ĂȂ���Ԃ�������C�����ł��Ȃ��̂ő��I��.
    if (!g_Prepare || g_pSwapChain == nullptr)
    { return; }

    // �T���v�����ȉ��̏ꍇ�̓N���b�V�����錴���ƂȂ�̂ŏ��������Ȃ�.
    {
        auto desc = g_pSwapChain->GetDesc();
        if ( w < desc.SampleCount || h < desc.SampleCount )
        { return; }
    }

    g_Prepare = false;

    // �A�C�h����ԂɂȂ�܂ő҂�.
    g_pGraphicsQueue->WaitIdle();
    g_pDevice->WaitIdle();

    for(auto i=0; i<2; ++i)
    {
        // �t���[���o�b�t�@�̔j��.
        a3d::SafeRelease(g_pFrameBuffer[i]);

        // �J���[�r���[�̔j��.
        a3d::SafeRelease(g_pColorView[i]);

        // �J���[�o�b�t�@�̔j��.
        a3d::SafeRelease(g_pColorBuffer[i]);
    }

    // �X���b�v�`�F�C���̃��T�C�Y�����ł�.
    g_pSwapChain->ResizeBuffers( w, h );

    // �e�N�X�`���r���[�𐶐�.
    {
        auto desc = g_pSwapChain->GetDesc();

        // �X���b�v�`�F�C������o�b�t�@���擾.
        g_pSwapChain->GetBuffer(0, &g_pColorBuffer[0]);
        g_pSwapChain->GetBuffer(1, &g_pColorBuffer[1]);

        a3d::TextureViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = desc.Format;
        viewDesc.TextureAspect      = a3d::TEXTURE_ASPECT_COLOR;
        viewDesc.MipSlice           = 0;
        viewDesc.MipLevels          = desc.MipLevels;
        viewDesc.FirstArraySlice    = 0;
        viewDesc.ArraySize          = 1;
        viewDesc.ComponentMapping.R = a3d::TEXTURE_SWIZZLE_R;
        viewDesc.ComponentMapping.G = a3d::TEXTURE_SWIZZLE_G;
        viewDesc.ComponentMapping.B = a3d::TEXTURE_SWIZZLE_B;
        viewDesc.ComponentMapping.A = a3d::TEXTURE_SWIZZLE_A;

        for(auto i=0; i<2; ++i)
        {
            auto ret = g_pDevice->CreateTextureView(g_pColorBuffer[i], &viewDesc, &g_pColorView[i]);
            assert(ret == true);
        }
    }

    // �t���[���o�b�t�@�̐���
    {
        // �t���[���o�b�t�@�̐ݒ�.
        a3d::FrameBufferDesc desc = {};
        desc.ColorCount         = 1;
        desc.pColorTargets[0]   = g_pColorView[0];
        desc.pDepthTarget       = nullptr;

        // 1���ڂ̃t���[���o�b�t�@�𐶐�.
        auto ret = g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[0]);
        assert(ret == true);
        A3D_UNUSED(ret);

        // 2���ڂ̃t���[���o�b�t�@�𐶐�.
        desc.pColorTargets[0] = g_pColorView[1];
        ret = g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[1]);
        assert(ret == true);
    }

    // �r���[�|�[�g�̐ݒ�.
    g_Viewport.X        = 0;
    g_Viewport.Y        = 0;
    g_Viewport.Width    = float(w);
    g_Viewport.Height   = float(h);

    // �V�U�[��`�̐ݒ�.
    g_Scissor.Offset.X      = 0;
    g_Scissor.Offset.Y      = 0;
    g_Scissor.Extent.Width  = w;
    g_Scissor.Extent.Height = h;

    g_Prepare = true;
}