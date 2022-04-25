//-------------------------------------------------------------------------------------------------
// File : main.cpp
// Desc : Main Entry Point.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3d.h>
#include <cassert>
#include <cstring>
#include <string>
#include <SampleApp.h>
#include <SampleAllocator.h>
#include <SampleUtil.h>
#include <SampleMath.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    Vec3 Position;  //!< 位置座標です.
    Vec4 Color;     //!< 頂点カラーです.
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Transform structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Transform
{
    Mat4 World;     //!< ワールド行列です.
    Mat4 View;      //!< ビュー行列です.
    Mat4 Proj;      //!< 射影行列です.
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
IApp*                       g_pApp                  = nullptr;  //!< ウィンドウの生成を行うヘルパークラスです.
a3d::IDevice*               g_pDevice               = nullptr;  //!< デバイスです.
a3d::ISwapChain*            g_pSwapChain            = nullptr;  //!< スワップチェインです.
a3d::IQueue*                g_pGraphicsQueue        = nullptr;  //!< コマンドキューです.
a3d::IFence*                g_pFence                = nullptr;  //!< フェンスです.
a3d::IDescriptorSetLayout*  g_pDescriptorSetLayout  = nullptr;  //!< ディスクリプタセットレイアウトです.
a3d::IPipelineState*        g_pPipelineState        = nullptr;  //!< パイプラインステートです.
a3d::IBuffer*               g_pVertexBuffer         = nullptr;  //!< 頂点バッファです.
a3d::IBuffer*               g_pIndexBuffer          = nullptr;  //!< インデックスバッファです.
a3d::ITexture*              g_pDepthBuffer          = nullptr;  //!< 深度バッファです.
a3d::IDepthStencilView*     g_pDepthView            = nullptr;  //!< 深度ビューです.
a3d::ITexture*              g_pColorBuffer[2]       = {};       //!< カラーバッファです.
a3d::IRenderTargetView*     g_pColorView[2]         = {};       //!< カラービューです.
a3d::ICommandList*          g_pCommandList[2]       = {};       //!< コマンドリストです.
a3d::IBuffer*               g_pConstantBuffer[2]    = {};       //!< 定数バッファです.
a3d::IConstantBufferView*   g_pConstantView[4]      = {};       //!< 定数バッファビューです.
a3d::Viewport               g_Viewport              = {};       //!< ビューポートです.
a3d::Rect                   g_Scissor               = {};       //!< シザー矩形です.
Transform                   g_Transform             = {};       //!< 変換行列です.
float                       g_RotateAngle           = 0.0f;     //!< 回転角です.
void*                       g_pCbHead[2]            = {};       //!< 定数バッファの先頭ポインタです.
bool                        g_Prepare               = false;    //!< 準備ができたらtrue.
SampleAllocator             g_Allocator;                        //!< アロケータ.


//-------------------------------------------------------------------------------------------------
//      メインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
void Main()
{
    // ウィンドウ生成.
    if (!CreateApp(960, 540, &g_pApp))
    { return; }

    // リサイズ時のコールバック関数を設定.
    g_pApp->SetResizeCallback(Resize, nullptr);

    // A3D初期化.
    if (!InitA3D())
    {
        g_pApp->Release();
        return;
    }

    // メインループ.
    while( g_pApp->IsLoop() )
    {
        // 描画処理.
        DrawA3D();
    }

    // 後始末.
    TermA3D();
    g_pApp->Release();
}

//-------------------------------------------------------------------------------------------------
//      A3Dの初期化を行います.
//-------------------------------------------------------------------------------------------------
bool InitA3D()
{
    g_Prepare = false;

    // グラフィックスシステムの初期化.
    {
        a3d::SystemDesc desc = {};
        desc.pSystemAllocator = &g_Allocator;

        if (!a3d::InitSystem(&desc))
        { return false; }
    }

    // デバイスの生成.
    {
        a3d::DeviceDesc desc = {};

        desc.EnableDebug = true;

        // 最大ディスクリプタ数を設定.
        desc.MaxColorTargetCount            = 2;
        desc.MaxDepthTargetCount            = 2;
        desc.MaxShaderResourceCount         = 4;

        // 最大サブミット数を設定.
        desc.MaxGraphicsQueueSubmitCount    = 256;
        desc.MaxCopyQueueSubmitCount        = 256;
        desc.MaxComputeQueueSubmitCount     = 256;

        // デバイスを生成.
        if (!a3d::CreateDevice(&desc, &g_pDevice))
        { return false; }

        // コマンドキューを取得.
        g_pDevice->GetGraphicsQueue(&g_pGraphicsQueue);
    }

    auto info = g_pDevice->GetInfo();

    #if SAMPLE_IS_VULKAN && A3D_IS_WIN
        auto format = a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM;
    #else
        auto format = a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM;
    #endif

    // スワップチェインの生成.
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

        // スワップチェインからバッファを取得.
        g_pSwapChain->GetBuffer(0, &g_pColorBuffer[0]);
        g_pSwapChain->GetBuffer(1, &g_pColorBuffer[1]);

        a3d::TargetViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = format;
        viewDesc.MipSlice           = 0;
        viewDesc.MipLevels          = desc.MipLevels;
        viewDesc.FirstArraySlice    = 0;
        viewDesc.ArraySize          = 1;

        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateRenderTargetView(g_pColorBuffer[i], &viewDesc, &g_pColorView[i]))
            { return false; }
        }
    }

    // 深度バッファの生成.
    {
        a3d::TextureDesc desc = {};
        desc.Dimension          = a3d::RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = g_pApp->GetWidth();
        desc.Height             = g_pApp->GetHeight();
        desc.DepthOrArraySize   = 1;
        desc.Format             = a3d::RESOURCE_FORMAT_D32_FLOAT;
        desc.MipLevels          = 1;
        desc.SampleCount        = 1;
        desc.Layout             = a3d::RESOURCE_LAYOUT_OPTIMAL;
        desc.Usage              = a3d::RESOURCE_USAGE_DEPTH_STENCIL;
        desc.InitState          = a3d::RESOURCE_STATE_DEPTH_WRITE;
        desc.HeapType           = a3d::HEAP_TYPE_DEFAULT;

        if (!g_pDevice->CreateTexture(&desc, &g_pDepthBuffer))
        { return false; }

        a3d::TargetViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = desc.Format;
        viewDesc.MipSlice           = 0;
        viewDesc.MipLevels          = desc.MipLevels;
        viewDesc.FirstArraySlice    = 0;
        viewDesc.ArraySize          = desc.DepthOrArraySize;

        if (!g_pDevice->CreateDepthStencilView(g_pDepthBuffer, &viewDesc, &g_pDepthView))
        { return false; }
    }

    // コマンドリストを生成.
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

    // フェンスを生成.
    {
        if (!g_pDevice->CreateFence(&g_pFence))
        { return false; }
    }

    // 頂点バッファを生成.
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

    // インデックスバッファを生成.
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

    // 定数バッファを生成.
    {
        auto stride = a3d::RoundUp<uint32_t>( sizeof(Transform), info.ConstantBufferMemoryAlignment );

        a3d::BufferDesc desc = {};
        desc.Size       = stride * 2;
        desc.Stride     = stride;
        desc.InitState  = a3d::RESOURCE_STATE_GENERAL;
        desc.Usage      = a3d::RESOURCE_USAGE_CONSTANT_BUFFER;
        desc.HeapType   = a3d::HEAP_TYPE_UPLOAD;

        a3d::ConstantBufferViewDesc viewDesc = {};
        viewDesc.Offset = 0;
        viewDesc.Range  = stride;

        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateBuffer(&desc, &g_pConstantBuffer[i]))
            { return false; }

            viewDesc.Offset = 0;
            if (!g_pDevice->CreateConstantBufferView(g_pConstantBuffer[i], &viewDesc, &g_pConstantView[i * 2 + 0]))
            { return false; }

            viewDesc.Offset += stride;
            if (!g_pDevice->CreateConstantBufferView(g_pConstantBuffer[i], &viewDesc, &g_pConstantView[i * 2 + 1]))
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

    // シェーダバイナリを読み込みます.
    a3d::ShaderBinary vs = {};
    a3d::ShaderBinary ps = {};
    {
        FixedSizeString vsPath;
        FixedSizeString psPath;
        GetShaderPath("SimpleVS", vsPath);
        GetShaderPath("SimplePS", psPath);

        if (!LoadShaderBinary(vsPath.c_str(), vs))
        { return false; }

        if (!LoadShaderBinary(psPath.c_str(), ps))
        {
            DisposeShaderBinary(vs);
            return false;
        }
    }

    // ディスクリプタセットレイアウトを生成します.
    {
        a3d::DescriptorSetLayoutDesc desc = {};
        desc.EntryCount                = 1;
        desc.Entries[0].ShaderMask     = a3d::SHADER_MASK_VS;
        desc.Entries[0].ShaderRegister = 0;
        desc.Entries[0].Type           = a3d::DESCRIPTOR_TYPE_CBV;

        if (!g_pDevice->CreateDescriptorSetLayout(&desc, &g_pDescriptorSetLayout))
        { return false; }
    }

    // グラフィックスパイプラインステートを生成します.
    {
        // 入力要素です.
        a3d::InputElementDesc inputElements[] = {
            { "POSITION", 0, 0, a3d::RESOURCE_FORMAT_R32G32B32_FLOAT   , 0,  0, a3d::INPUT_CLASSIFICATION_PER_VERTEX },
            { "COLOR"   , 0, 1, a3d::RESOURCE_FORMAT_R32G32B32A32_FLOAT, 0, 12, a3d::INPUT_CLASSIFICATION_PER_VERTEX },
        };

        // 入力レイアウトです.
        a3d::InputLayoutDesc inputLayout = {};
        inputLayout.ElementCount = 2;
        inputLayout.pElements    = inputElements;

        // グラフィックスパイプラインステートを設定します.
        a3d::GraphicsPipelineStateDesc desc = a3d::GraphicsPipelineStateDesc::Default();

        // シェーダの設定.
        desc.VS = vs;
        desc.PS = ps;

        // 入力アウトの設定.
        desc.InputLayout = inputLayout;

        // ディスクリプタセットレイアウトの設定.
        desc.pLayout = g_pDescriptorSetLayout;

        // フォーマットの設定.
        desc.RenderTargetCount  = 1;
        desc.RenderTarget[0]    = format;
        desc.DepthTarget        = a3d::RESOURCE_FORMAT_D32_FLOAT;

        // キャッシュ済みパイプラインステートの設定.
        desc.pCachedPSO = nullptr;

        // グラフィックスパイプラインステートの生成.
        if (!g_pDevice->CreateGraphicsPipeline(&desc, &g_pPipelineState))
        {
            DisposeShaderBinary(vs);
            DisposeShaderBinary(ps);
            return false;
        }
    }
     // 不要になったので破棄します.
    DisposeShaderBinary(vs);
    DisposeShaderBinary(ps);

    // ビューポートの設定.
    g_Viewport.X        = 0.0f;
    g_Viewport.Y        = 0.0f;
    g_Viewport.Width    = static_cast<float>(g_pApp->GetWidth());
    g_Viewport.Height   = static_cast<float>(g_pApp->GetHeight());
    g_Viewport.MinDepth = 0.0f;
    g_Viewport.MaxDepth = 1.0f;

    // シザー矩形の設定.
    g_Scissor.Offset.X      = 0;
    g_Scissor.Offset.Y      = 0;
    g_Scissor.Extent.Width  = g_pApp->GetWidth();
    g_Scissor.Extent.Height = g_pApp->GetHeight();

    g_Prepare = true;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      A3Dの終了処理を行います.
//-------------------------------------------------------------------------------------------------
void TermA3D()
{
    g_Prepare = false;

    // アイドル状態になるまで待つ.
    g_pGraphicsQueue->WaitIdle();
    g_pDevice->WaitIdle();

    // ダブルバッファリソースの破棄.
    for(auto i=0; i<2; ++i)
    {
        // カラービューの破棄.
        a3d::SafeRelease(g_pColorView[i]);

        // カラーバッファの破棄.
        a3d::SafeRelease(g_pColorBuffer[i]);

        // コマンドリストの破棄.
        a3d::SafeRelease(g_pCommandList[i]);

        // 破棄前にメモリマッピングを解除.
        if (g_pConstantBuffer[i] != nullptr)
        { g_pConstantBuffer[i]->Unmap(); }

        // 定数バッファの破棄.
        a3d::SafeRelease(g_pConstantBuffer[i]);
    }

    for(auto i=0; i<4; ++i)
    {
        // 定数バッファビューの破棄.
        a3d::SafeRelease(g_pConstantView[i]);
    }

    // パイプラインステートの破棄.
    a3d::SafeRelease(g_pPipelineState);

    // 頂点バッファの破棄.
    a3d::SafeRelease(g_pVertexBuffer);

    // インデックスバッファの破棄.
    a3d::SafeRelease(g_pIndexBuffer);

    // ディスクリプタセットレイアウトの破棄.
    a3d::SafeRelease(g_pDescriptorSetLayout);

    // フェンスの破棄.
    a3d::SafeRelease(g_pFence);

    // 深度ステンシルターゲットビューの破棄.
    a3d::SafeRelease(g_pDepthView);

    // 深度バッファの破棄.
    a3d::SafeRelease(g_pDepthBuffer);

    // スワップチェインの破棄.
    a3d::SafeRelease(g_pSwapChain);

    // グラフィックスキューの破棄.
    a3d::SafeRelease(g_pGraphicsQueue);

    // デバイスの破棄.
    a3d::SafeRelease(g_pDevice);

    // グラフィックスシステムの終了処理.
    a3d::TermSystem();
}

//-------------------------------------------------------------------------------------------------
//      A3Dによる描画処理を行います.
//-------------------------------------------------------------------------------------------------
void DrawA3D()
{
    if (!g_Prepare)
    { return; }

    // バッファ番号を取得します.
    auto idx = g_pSwapChain->GetCurrentBufferIndex();

    // 定数バッファを更新.
    {
        g_RotateAngle += 0.025f;
        g_Transform.World = Mat4::Translation(0.0f, 0.0, 0.25f) * Mat4::RotateY(g_RotateAngle);

        auto stride = g_pConstantBuffer[idx]->GetDesc().Stride;

        auto ptr = static_cast<uint8_t*>(g_pCbHead[idx]);
        memcpy(ptr, &g_Transform, sizeof(g_Transform));

        g_Transform.World = Mat4::Scale(1.0f, 0.5f, 1.0f) * Mat4::RotateX(g_RotateAngle * 2.0f);
        ptr += stride;
        memcpy(ptr, &g_Transform, sizeof(g_Transform));
    }

    // コマンドの記録を開始します.
    auto pCmd = g_pCommandList[idx];
    pCmd->Begin();

    // 書き込み用のバリアを設定します.
    pCmd->TextureBarrier(
        g_pColorBuffer[idx],
        a3d::RESOURCE_STATE_PRESENT,
        a3d::RESOURCE_STATE_COLOR_WRITE);

    // フレームバッファをクリアします.
    a3d::ClearColorValue clearColor = {};
    clearColor.R            = 0.25f;
    clearColor.G            = 0.25f;
    clearColor.B            = 0.25f;
    clearColor.A            = 1.0f;
    clearColor.SlotIndex    = 0;

    a3d::ClearDepthStencilValue clearDepth = {};
    clearDepth.Depth                = 1.0f;
    clearDepth.Stencil              = 0;
    clearDepth.EnableClearDepth     = true;
    clearDepth.EnableClearStencil   = false;

    a3d::IRenderTargetView* pRTVs[] = {
        g_pColorView[idx]
    };

    // フレームバッファを設定します.
    pCmd->BeginFrameBuffer(1, pRTVs, g_pDepthView, 1, &clearColor, &clearDepth);
    {
        // パイプラインステートを設定します.
        pCmd->SetPipelineState(g_pPipelineState);

        // ビューポートとシザー矩形を設定します.
        // NOTE : ビューポートとシザー矩形の設定は，必ずSetPipelineState() の後である必要があります.
        pCmd->SetViewports(1, &g_Viewport);
        pCmd->SetScissors (1, &g_Scissor);

        // 頂点バッファを設定します.
        pCmd->SetVertexBuffers(0, 1, &g_pVertexBuffer, nullptr);
        pCmd->SetIndexBuffer(g_pIndexBuffer, 0);

        // 手前の三角形.
        pCmd->SetDescriptorSetLayout(g_pDescriptorSetLayout);
        pCmd->SetView(0, g_pConstantView[idx * 2 + 0]);
        pCmd->DrawIndexedInstanced(6, 1, 0, 0, 0);

        // 奥側の三角形.
        pCmd->SetDescriptorSetLayout(g_pDescriptorSetLayout);
        pCmd->SetView(0, g_pConstantView[idx * 2 + 1]);
        pCmd->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }
    
    // 表示用のバリアを設定する前に，フレームバッファの設定を解除する必要があります.
    pCmd->EndFrameBuffer();

    // 表示用にバリアを設定します.
    pCmd->TextureBarrier(
        g_pColorBuffer[idx],
        a3d::RESOURCE_STATE_COLOR_WRITE,
        a3d::RESOURCE_STATE_PRESENT);

    // コマンドリストへの記録を終了します.
    pCmd->End();

    // コマンドキューに登録します.
    g_pGraphicsQueue->Submit(pCmd);

    // コマンドを実行します.
    g_pGraphicsQueue->Execute(g_pFence);

    // コマンドを実行完了を待機します.
    if (!g_pFence->IsSignaled())
    { g_pFence->Wait(UINT32_MAX); }

    // 画面に表示します.
    g_pGraphicsQueue->Present( g_pSwapChain );
}

//-------------------------------------------------------------------------------------------------
//      リサイズ処理です.
//-------------------------------------------------------------------------------------------------
void Resize( uint32_t w, uint32_t h, void* pUser )
{
    A3D_UNUSED( pUser );

    // 準備が完了してない状態だったら，処理できないので即終了.
    if (!g_Prepare || g_pSwapChain == nullptr)
    { return; }

    // サンプル数以下の場合はクラッシュする原因となるので処理させない.
    {
        auto desc = g_pSwapChain->GetDesc();
        if ( w < desc.SampleCount || h < desc.SampleCount )
        { return; }
    }

    g_Prepare = false;

    // アイドル状態になるまで待つ.
    g_pGraphicsQueue->WaitIdle();
    g_pDevice->WaitIdle();

    for(auto i=0; i<2; ++i)
    {
        // カラービューの破棄.
        a3d::SafeRelease(g_pColorView[i]);

        // カラーバッファの破棄.
        a3d::SafeRelease(g_pColorBuffer[i]);
    }

    // 深度ステンシルターゲットビューの破棄.
    a3d::SafeRelease(g_pDepthView);

    // 深度バッファの破棄.
    a3d::SafeRelease(g_pDepthBuffer);

    // スワップチェインのリサイズ処理です.
    g_pSwapChain->ResizeBuffers( w, h );

    // テクスチャビューを生成.
    {
        auto desc = g_pSwapChain->GetDesc();

        // スワップチェインからバッファを取得.
        g_pSwapChain->GetBuffer(0, &g_pColorBuffer[0]);
        g_pSwapChain->GetBuffer(1, &g_pColorBuffer[1]);

        a3d::TargetViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = desc.Format;
        viewDesc.MipSlice           = 0;
        viewDesc.MipLevels          = desc.MipLevels;
        viewDesc.FirstArraySlice    = 0;
        viewDesc.ArraySize          = 1;

        for(auto i=0; i<2; ++i)
        {
            auto ret = g_pDevice->CreateRenderTargetView(g_pColorBuffer[i], &viewDesc, &g_pColorView[i]);
            assert(ret == true);
            A3D_UNUSED(ret);
        }
    }

    // 深度バッファの生成.
    {
        a3d::TextureDesc desc = {};
        desc.Dimension          = a3d::RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = g_pApp->GetWidth();
        desc.Height             = g_pApp->GetHeight();
        desc.DepthOrArraySize   = 1;
        desc.Format             = a3d::RESOURCE_FORMAT_D32_FLOAT;
        desc.MipLevels          = 1;
        desc.SampleCount        = 1;
        desc.Layout             = a3d::RESOURCE_LAYOUT_OPTIMAL;
        desc.Usage              = a3d::RESOURCE_USAGE_DEPTH_STENCIL;
        desc.InitState          = a3d::RESOURCE_STATE_DEPTH_WRITE;
        desc.HeapType           = a3d::HEAP_TYPE_DEFAULT;

        auto ret = g_pDevice->CreateTexture(&desc, &g_pDepthBuffer);
        assert(ret == true);
        A3D_UNUSED(ret);

        a3d::TargetViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = desc.Format;
        viewDesc.MipSlice           = 0;
        viewDesc.MipLevels          = desc.MipLevels;
        viewDesc.FirstArraySlice    = 0;
        viewDesc.ArraySize          = desc.DepthOrArraySize;

        ret = g_pDevice->CreateDepthStencilView(g_pDepthBuffer, &viewDesc, &g_pDepthView);
        assert(ret == true);
    }

    // ビューポートの設定.
    g_Viewport.X        = 0;
    g_Viewport.Y        = 0;
    g_Viewport.Width    = float(w);
    g_Viewport.Height   = float(h);

    // シザー矩形の設定.
    g_Scissor.Offset.X      = 0;
    g_Scissor.Offset.Y      = 0;
    g_Scissor.Extent.Width  = w;
    g_Scissor.Extent.Height = h;

    g_Prepare = true;
}