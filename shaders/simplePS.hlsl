//-------------------------------------------------------------------------------------------------
// File : simplePS.hlsl
// Desc : Simple Pixel Shader.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

#include "spirvHelper.hlsli"

///////////////////////////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    LOCATION(0) float4 Position : SV_POSITION;
    LOCATION(1) float4 Color    : COLOR;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PSOutput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct PSOutput
{
    LOCATION(0) float4 Color : SV_TARGET0;
};

//-------------------------------------------------------------------------------------------------
//      ピクセルシェーダメインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput)0;

    output.Color = input.Color;

    return output;
}