RWTexture2D<float4> OutputTexture : register(u0);
RWBuffer<uint>TimeCounter : register(u1);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GRid : SV_GroupID, uint3 GTid : SV_GroupThreadID)
{
    OutputTexture[DTid.xy] = float4(1.0f, 0.0f, 0.0f, 1.0f);

    if (all(DTid.xy == uint2(0, 0)))
    {
        TimeCounter[0] = 1;
    }

    uint2 dispatchSize = uint2((1920 + 7) / 8, (1080 + 7) / 8);
    uint2 lastThread = dispatchSize * uint2(8, 8) - uint2(1, 1);

    if (all(DTid.xy == lastThread))
    {
        TimeCounter[1] = 2;
    }
}