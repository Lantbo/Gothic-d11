//--------------------------------------------------------------------------------------
// Simple vertex shader
//--------------------------------------------------------------------------------------

cbuffer Matrices_PerFrame : register( b0 )
{
	matrix M_View;
	matrix M_Proj;
	matrix M_ViewProj;	
};

cbuffer Matrices_PerInstances : register( b1 )
{
	matrix M_World;
	float4 M_Color;
	float M_Fatness;
	float M_Scaling;
	float2 M_Pad1;
};


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
	float2 vTex1		: TEXCOORD0;
	float2 vTex2		: TEXCOORD1;
	float4 vDiffuse		: DIFFUSE;
};

struct VS_OUTPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vNormalWS		: TEXCOORD3;
	float3 vWorldPosition	: TEXCOORD4;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	float3 positionWorld = mul(float4((Input.vPosition + M_Fatness * Input.vNormal) * M_Scaling, 1), M_World).xyz;
	
	//Output.vPosition = float4(Input.vPosition, 1);
	Output.vTexcoord2 = Input.vTex2;
	Output.vTexcoord = Input.vTex1;
	Output.vDiffuse  = M_Color;
	Output.vNormalWS = mul(Input.vNormal, (float3x3)M_World);
	Output.vWorldPosition = positionWorld;

	return Output;
}

