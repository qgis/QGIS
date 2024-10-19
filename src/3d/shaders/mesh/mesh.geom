#version 330 core

layout( triangles ) in;
layout( triangle_strip, max_vertices = 3 ) out;

uniform mat4 viewportMatrix;
uniform bool flatTriangles;

in MeshVertex {
    vec3 worldPosition;
    vec3 worldNormal;
    float magnitude;
} gs_in[];

out treatedVertex {
    vec3 worldPosition;
    vec3 worldNormal;
    float magnitude;
    noperspective vec4 edgeA;
    noperspective vec4 edgeB;
    flat int configuration;
} gs_out;

#ifdef CLIPPING
    #pragma include ../clipplane.shaderinc
#endif

const int infoA[]  = int[]( 0, 0, 0, 0, 1, 1, 2 );
const int infoB[]  = int[]( 1, 1, 2, 0, 2, 1, 2 );
const int infoAd[] = int[]( 2, 2, 1, 1, 0, 0, 0 );
const int infoBd[] = int[]( 2, 2, 1, 2, 0, 2, 1 );

vec2 transformToViewport( const in vec4 p )
{
    return vec2( viewportMatrix * ( p / p.w ) );
}

void main()
{
    // modified copy of light.inc.frag from Qt source : examples/qt3d/wireframe/robustwireframe.geom
    gs_out.configuration = int(gl_in[0].gl_Position.z < 0) * int(4)
           + int(gl_in[1].gl_Position.z < 0) * int(2)
           + int(gl_in[2].gl_Position.z < 0);

    // If all vertices are behind us, cull the primitive
    if (gs_out.configuration == 7)
        return;

    // Transform each vertex into viewport space
    vec2 p[3];
    p[0] = transformToViewport( gl_in[0].gl_Position );
    p[1] = transformToViewport( gl_in[1].gl_Position );
    p[2] = transformToViewport( gl_in[2].gl_Position );

    vec3 normal[3];
    if (flatTriangles)
    {
        vec3 v1=gs_in[1].worldPosition-gs_in[0].worldPosition;
        vec3 v2=gs_in[2].worldPosition-gs_in[0].worldPosition;
        vec3 normalTriangle=cross(v1,v2);
        normalTriangle=normalize(normalTriangle);

        normal[0]=normalTriangle;
        normal[1]=normalTriangle;
        normal[2]=normalTriangle;
    }
    else
    {
        normal[0]=gs_in[0].worldNormal;
        normal[1]=gs_in[1].worldNormal;
        normal[2]=gs_in[2].worldNormal;
    }

    if (gs_out.configuration == 0)
    {
        // Common configuration where all vertices are within the viewport
        gs_out.edgeA = vec4(0.0);
        gs_out.edgeB = vec4(0.0);

        // Calculate lengths of 3 edges of triangle
        float a = length( p[1] - p[2] );
        float b = length( p[2] - p[0] );
        float c = length( p[1] - p[0] );

        // Calculate internal angles using the cosine rule
        float alpha = acos( ( b * b + c * c - a * a ) / ( 2.0 * b * c ) );
        float beta = acos( ( a * a + c * c - b * b ) / ( 2.0 * a * c ) );

        // Calculate the perpendicular distance of each vertex from the opposing edge
        float ha = abs( c * sin( beta ) );
        float hb = abs( c * sin( alpha ) );
        float hc = abs( b * sin( alpha ) );

        // Now add this perpendicular distance as a per-vertex property in addition to
        // the position and normal calculated in the vertex shader.

        // Vertex 0 (a)
        gs_out.edgeA = vec4( ha, 0.0, 0.0, 0.0 );
        gs_out.worldNormal = normal[0];
        gs_out.worldPosition = gs_in[0].worldPosition;
        gs_out.magnitude=gs_in[0].magnitude;
        gl_Position = gl_in[0].gl_Position;
        #ifdef CLIPPING
            setClipDistance(gs_in[0].worldPosition);
        #endif
        EmitVertex();

        // Vertex 1 (b)
        gs_out.edgeA = vec4( 0.0, hb, 0.0, 0.0 );
        gs_out.worldNormal = normal[1];
        gs_out.worldPosition = gs_in[1].worldPosition;
        gs_out.magnitude=gs_in[1].magnitude;
        gl_Position = gl_in[1].gl_Position;
        #ifdef CLIPPING
            setClipDistance(gs_in[1].worldPosition);
        #endif
        EmitVertex();

        // Vertex 2 (c)
        gs_out.edgeA = vec4( 0.0, 0.0, hc, 0.0 );
        gs_out.worldNormal = normal[2];
        gs_out.worldPosition = gs_in[2].worldPosition;
        gs_out.magnitude=gs_in[2].magnitude;
        gl_Position = gl_in[2].gl_Position;
        #ifdef CLIPPING
            setClipDistance(gs_in[2].worldPosition);
        #endif
        EmitVertex();

        // Finish the primitive off
        EndPrimitive();
    }
    else
    {
        // Viewport projection breaks down for one or two vertices.
        // Caclulate what we can here and defer rest to fragment shader.
        // Since this is coherent for the entire primitive the conditional
        // in the fragment shader is still cheap as all concurrent
        // fragment shader invocations will take the same code path.

        // Copy across the viewport-space points for the (up to) two vertices
        // in the viewport
        gs_out.edgeA.xy = p[infoA[gs_out.configuration]];
        gs_out.edgeB.xy = p[infoB[gs_out.configuration]];

        // Copy across the viewport-space edge vectors for the (up to) two vertices
        // in the viewport
        gs_out.edgeA.zw = normalize( gs_out.edgeA.xy - p[ infoAd[gs_out.configuration] ] );
        gs_out.edgeB.zw = normalize( gs_out.edgeB.xy - p[ infoBd[gs_out.configuration] ] );

        // Pass through the other vertex attributes
        gs_out.worldPosition = gs_in[0].worldPosition;
        gs_out.worldNormal = normal[0];
        gs_out.magnitude=gs_in[0].magnitude;
        gl_Position = gl_in[0].gl_Position;
        #ifdef CLIPPING
            setClipDistance(gs_in[0].worldPosition);
        #endif
        EmitVertex();

        gs_out.worldPosition = gs_in[1].worldPosition;
        gs_out.worldNormal = normal[1];
        gs_out.magnitude=gs_in[1].magnitude;
        gl_Position = gl_in[1].gl_Position;
        #ifdef CLIPPING
            setClipDistance(gs_in[1].worldPosition);
        #endif
        EmitVertex();

        gs_out.worldPosition = gs_in[2].worldPosition;
        gs_out.worldNormal = normal[2];
        gs_out.magnitude=gs_in[1].magnitude;
        gl_Position = gl_in[2].gl_Position;
        #ifdef CLIPPING
            setClipDistance(gs_in[2].worldPosition);
        #endif
        EmitVertex();

        // Finish the primitive off
        EndPrimitive();
     }
}
