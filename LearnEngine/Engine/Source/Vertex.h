#pragma once

#include "Vector.h"

struct TVertex
{
    TVertex() {}
    TVertex(
        const TVector3f& p,
        const TVector3f& n,
        const TVector3f& t,
        const TVector2f& uv) :
        Position(p),
        Normal(n),
        TangentU(t),
        TexC(uv) {}
    TVertex(
        float px, float py, float pz,
        float nx, float ny, float nz,
        float tx, float ty, float tz,
        float u, float v) :
        Position(px, py, pz),
        Normal(nx, ny, nz),
        TangentU(tx, ty, tz),
        TexC(u, v) {}

    TVector3f Position;
    TVector3f Normal;
    TVector3f TangentU;
    TVector2f TexC;
};