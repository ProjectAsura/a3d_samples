//-------------------------------------------------------------------------------------------------
// File : imguiVS.hlsl
// Desc : Vertex Shader For ImGui
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

#include "spirvHelper.hlsli"

///////////////////////////////////////////////////////////////////////////////////////////////////
// VSInput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct VSInput
{
    LOCATION(0) float2 Position : POSITION;     //!< 位置座標.
    LOCATION(5) float2 TexCoord : TEXCOORD0;    //!< テクスチャ座標.
    LOCATION(1) float4 Color    : COLOR0;       //!< カラー. 
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    LOCATION(0) float4 Position : SV_POSITION;  //!< 位置座標.
    LOCATION(1) float4 Color    : COLOR;        //!< カラー.
    LOCATION(2) float2 TexCoord : TEXCOORD;     //!< テクスチャ座標.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Transform constant buffer
///////////////////////////////////////////////////////////////////////////////////////////////////
RESOURCE(cbuffer Transform, b0, 0)
{
    float4x4 Proj;      //!< 射影行列.
};

//-------------------------------------------------------------------------------------------------
//      頂点シェーダメインエントリーポイント.
//-------------------------------------------------------------------------------------------------
VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    
    output.Position = mul( Proj, float4(input.Position, 0.0f, 1.0f) );
    output.Color    = input.Color;
    output.TexCoord = input.TexCoord;

    FLIP_Y(output.Position);
    return output;
}