#version 150

uniform mat4 modelViewProjection;
uniform float THICKNESS;
uniform vec2 WIN_SCALE;


in vec2 vertexPosition;
in vec3 pointPrev;
in vec3 pointA;
in vec3 pointB;
in vec3 pointNext;
in vec3 dataDefinedColor;

out VertexData {
    vec2 mTexCoord;
    vec3 mColor;
} VertexOut;

// near plane in clip space is z = -w; a point is behind it when z + w < 0
bool isBehindNearPlane(vec4 clipPos)
{
    return clipPos.z + clipPos.w < 0.0;
}

#ifdef CLIPPING
    uniform mat4 inverseViewProjectionMatrix;
    #pragma include clipplane.shaderinc
#endif

vec2 toScreenSpace(vec4 vertex)
{
    return vec2(vertex.xy / vertex.w) * WIN_SCALE * 0.5;
}

vec4 nearPlaneCrossing(vec4 a, vec4 b)
{
    float t = (-a.z - a.w) / ((b.z - a.z) + (b.w - a.w));
    return mix(a, b, t);
}

bool clipNearPlane(inout vec4 clipA, inout vec4 clipB)
{
    bool aBehind = isBehindNearPlane(clipA);
    bool bBehind = isBehindNearPlane(clipB);

    if (aBehind && bBehind)
        return false;

    if (aBehind || bBehind)
    {
        vec4 intersection = nearPlaneCrossing(clipA, clipB);

        if (aBehind)
            clipA = intersection;
        else
            clipB = intersection;
    }

    return true;
}

void main(void)
{
    vec4 clipA = modelViewProjection * vec4(pointA, 1.0);
    vec4 clipB = modelViewProjection * vec4(pointB, 1.0);

    VertexOut.mTexCoord = vec2(vertexPosition.x, vertexPosition.y + 0.5);
    VertexOut.mColor = dataDefinedColor;

    if (!clipNearPlane(clipA, clipB))
    {
        gl_Position = vec4(0.0, 0.0, 0.0, -1.0);
        return;
    }

    vec4 clipPrev = modelViewProjection * vec4(pointPrev, 1.0);
    vec4 clipNext = modelViewProjection * vec4(pointNext, 1.0);
    vec2 screenPrev = toScreenSpace(clipPrev);
    vec2 screenA = toScreenSpace(clipA);
    vec2 screenB = toScreenSpace(clipB);
    vec2 screenNext = toScreenSpace(clipNext);

    if (isBehindNearPlane(clipPrev))
        screenPrev = toScreenSpace(nearPlaneCrossing(clipPrev, clipA));

    if (isBehindNearPlane(clipNext))
        screenNext = toScreenSpace(nearPlaneCrossing(clipNext, clipB));

    vec2 p0 = screenPrev;
    vec2 p1 = screenA;
    vec2 p2 = screenB;
    vec2 pos = vertexPosition;
    if (vertexPosition.x == 1.0)
    {
        p0 = screenNext;
        p1 = screenB;
        p2 = screenA;
        pos = vec2(1.0 - vertexPosition.x, -vertexPosition.y);
    }

    vec2 tangent = normalize(normalize(p2 - p1) + normalize(p1 - p0));
    vec2 miterNormal = vec2(-tangent.y, tangent.x);

    vec2 p01 = p1 - p0;
    vec2 p21 = p1 - p2;
    vec2 p01Norm = normalize(vec2(-p01.y, p01.x));

    float sigma = sign(dot(p01 + p21, miterNormal));

    vec2 xBasis = p2 - p1;
    vec2 yBasis = normalize(vec2(-xBasis.y, xBasis.x));
    vec2 flatPos = p1 + xBasis * pos.x + yBasis * THICKNESS * pos.y;

    vec2 screenPos;
    if (sign(pos.y) == -sigma)
    {
        float denom = dot(miterNormal, p01Norm);
        float safeDenom = ( denom < 0.0 ? -1.0 : 1.0 ) * max(abs(denom), 0.05);
        vec2 pulledPos = p1 + 0.5 * miterNormal * -sigma * THICKNESS / safeDenom;
        float stability = smoothstep(0.05, 0.25, abs(denom));
        screenPos = mix(p1, pulledPos, stability);
    }
    else
    {
        screenPos = flatPos;
    }

    float z = mix(clipA.z / clipA.w, clipB.z / clipB.w, vertexPosition.x);

    gl_Position = vec4(screenPos / (WIN_SCALE * 0.5), z, 1.0);

#ifdef CLIPPING
    vec4 worldPosition = inverseViewProjectionMatrix * gl_Position;
    setClipDistance(vec3(worldPosition / worldPosition.w));
#endif
}
