#version 150

uniform mat4 modelViewProjection;
uniform float THICKNESS;
uniform float MITER_LIMIT;
uniform vec2 WIN_SCALE;

in vec3 vertexPosition;
in vec3 pointA;
in vec3 pointB;
in vec3 pointC;
in vec3 dataDefinedColor;

out VertexData {
    vec2 mTexCoord;
    vec3 mColor;
} VertexOut;

// near plane in clip space is z = -w; a point is behind it when z + w < 0
bool isBehindNearPlane( vec4 clipPos )
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

void main(void)
{
    vec4 clipA = modelViewProjection * vec4(pointA, 1.0);
    vec4 clipB = modelViewProjection * vec4(pointB, 1.0);
    vec4 clipC = modelViewProjection * vec4(pointC, 1.0);

    VertexOut.mTexCoord = vec2(0.0, 0.5);
    VertexOut.mColor = dataDefinedColor;

    if (isBehindNearPlane(clipA) || isBehindNearPlane(clipB) || isBehindNearPlane(clipC))
    {
        gl_Position = vec4(0.0, 0.0, 0.0, -1.0);
        return;
    }

    vec2 screenA = toScreenSpace(clipA);
    vec2 screenB = toScreenSpace(clipB);
    vec2 screenC = toScreenSpace(clipC);

    vec2 ab = screenB - screenA;
    vec2 cb = screenB - screenC;

    // handle duplicate points, otherwise joins start acting crazy
    if (pointA == pointB) ab = -cb;
    if (pointC == pointB) cb = -ab;

    vec2 abNorm = normalize(vec2(-ab.y, ab.x));
    vec2 cbNorm = -normalize(vec2(-cb.y, cb.x));

    vec2 tangent = normalize(normalize(-cb) + normalize(ab));
    vec2 miterDir = vec2(-tangent.y, tangent.x);
    float sigma = sign(dot(ab + cb, miterDir));

    vec2 p0 = 0.5 * THICKNESS * sigma * (sigma < 0.0 ? abNorm : cbNorm);
    vec2 p2 = 0.5 * THICKNESS * sigma * (sigma < 0.0 ? cbNorm : abNorm);

    vec3 dir0 = normalize(pointB - pointA);
    vec3 dir1 = normalize(pointC - pointB);

    if (pointA == pointB) dir0 = dir1;
    if (pointC == pointB) dir1 = dir0;

    vec2 p1;
    if (dot(dir0, dir1) < -MITER_LIMIT)
    {
        // too sharp, go bevel
        p1 = 0.5 * (p0 + p2);
    }
    else
    {
        p1 = 0.5 * miterDir * sigma * THICKNESS / dot(miterDir, abNorm);
    }

    vec2 screenPos = screenB + vertexPosition.x * p0 + vertexPosition.y * p1 + vertexPosition.z * p2;


    gl_Position = vec4(screenPos / ( WIN_SCALE * 0.5 ), clipB.z / clipB.w, 1.0);

#ifdef CLIPPING
    vec4 worldPosition = inverseViewProjectionMatrix * gl_Position;
    setClipDistance(vec3(worldPosition / worldPosition.w));
#endif
}
