cbuffer FrameConsts : register(b0)
{
  row_major matrix Transform;
  float4 ColorIn;
};

struct VSInput
{
  float2 Pos   : POSITION;
  float2 Tex   : TEXCOORD;
};

struct PSInput
{
  float4 Pos   : SV_POSITION;
  float2 Tex   : TEXCOORD;
  float4 Color;
};

//~ VERTEX SHADER

PSInput VS_Main(VSInput Vertex)
{
  PSInput VSOutput = (PSInput)0;
  row_major matrix Transform = mul(Proj, World);
  float4 NewVertPos = mul(Transform, float4(Vertex.Pos, 1.0f, 1.0f));
  //VSOutput.Color = float4(1.0f, 1.3f, 0.3f, 1.0f);
  VSOutput.Color = ColorIn;
  VSOutput.Pos = NewPos;
  VSOutput.Tex = Vertex.Tex;
  
  return VSOutput;
}

//~ PIXEL SHADER

float4 PS_Main(PSInput Frag) : SV_TARGET
{
  //return Tex.Sample(Sampler, Frag.Tex);
  return Frag.Color;
}