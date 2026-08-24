#version 150

uniform mat4 modelViewProjection;
uniform float THICKNESS;
uniform vec2 WIN_SCALE;


in vec3 vertexPosition;
in vec3 pointA;
in vec3 pointB;
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

bool clipNearPlane(inout vec4 clipA, inout vec4 clipB)
{
    bool aBehind = isBehindNearPlane(clipA);
    bool bBehind = isBehindNearPlane(clipB);

    if (aBehind && bBehind)
        return false;

    if (aBehind || bBehind)
    {
        float t = (-clipA.z - clipA.w) / ((clipB.z - clipA.z) + (clipB.w - clipA.w));
        vec4 intersection = mix(clipA, clipB, t);

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

    vec2 screenA = toScreenSpace(clipA);
    vec2 screenB = toScreenSpace(clipB);

    vec2 dir = normalize(screenB - screenA);
    vec2 normal = vec2(-dir.y, dir.x);

    vec2 screenPos = mix(screenA, screenB, vertexPosition.x) + normal * THICKNESS * vertexPosition.y;

    float z = mix(clipA.z / clipA.w, clipB.z / clipB.w, vertexPosition.x);

    gl_Position = vec4(screenPos / (WIN_SCALE * 0.5), z, 1.0);

#ifdef CLIPPING
    vec4 worldPosition = inverseViewProjectionMatrix * gl_Position;
    setClipDistance(vec3(worldPosition / worldPosition.w));
#endif
}
