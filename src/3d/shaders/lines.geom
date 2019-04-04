#version 150

uniform float	THICKNESS;		// the thickness of the line in pixels
uniform float	MITER_LIMIT;	// 1.0: always miter, -1.0: never miter, 0.75: default
uniform vec2	WIN_SCALE;		// the size of the viewport in pixels

uniform mat4 modelViewProjection;

uniform vec3 camNearPlanePoint;
uniform vec3 camNearPlaneNormal;

layout( lines_adjacency ) in;
layout( triangle_strip, max_vertices = 7 ) out;


in VertexData{
  vec3 worldPosition;
//	vec3 mColor;
} VertexIn[4];

out VertexData{
    vec2 mTexCoord;
//	vec3 mColor;
} VertexOut;

vec2 toScreenSpace( vec4 vertex )
{
    return vec2( vertex.xy / vertex.w ) * WIN_SCALE;
}

vec4 clip_line_point(vec3 pt0, vec3 pt1, vec4 projected)
{
    // we have line segment given by pt0 and pt1 (in world coordinates) and 'projected' point
    // (in clip coordinates) that is one of the endpoints. If the projected point's w >= 1
    // then everything is fine because the point is in front of the camera's near plane and
    // it is projected correctly. If not, the projected point is wrong and needs to be adjusted.
    // we place it at the intersection of the line and near plane to fix its position.

    if (projected.w < 1)
    {
        vec3 lineDir = pt1 - pt0;
        float d = dot(camNearPlaneNormal, camNearPlanePoint - pt0) / dot(lineDir, camNearPlaneNormal);
        if (d > 0 && d < 1)
        {
            // figure out the intersection point of line and near plane
            vec3 wpIntersect = pt0 + lineDir * d;
            vec4 wpIntersectProj = modelViewProjection * vec4( wpIntersect, 1.0 );
            return wpIntersectProj;
        }
    }
    return projected;
}

void main( void )
{
    // these are original positions in world coordinates
    vec3 wp0 = VertexIn[0].worldPosition;
    vec3 wp1 = VertexIn[1].worldPosition;
    vec3 wp2 = VertexIn[2].worldPosition;
    vec3 wp3 = VertexIn[3].worldPosition;

    // Perform line clipping first. we search for intersection between the line and the near plane.
    // In case the near plane intersects line between segment's endpoints, we need to adjust the line
    // otherwise we would use completely non-sense points when points get 'behind' the camera.
    // We do this also for the 'previous' and 'next' segments to get the miters right.
    vec4 projp0 = clip_line_point(wp0, wp1, gl_in[0].gl_Position);
    vec4 projp1 = clip_line_point(wp1, wp2, gl_in[1].gl_Position);
    vec4 projp2 = clip_line_point(wp1, wp2, gl_in[2].gl_Position);
    vec4 projp3 = clip_line_point(wp2, wp3, gl_in[3].gl_Position);

    // get the four vertices passed to the shader:
    vec2 p0 = toScreenSpace( projp0 );	// start of previous segment
    vec2 p1 = toScreenSpace( projp1 );	// end of previous segment, start of current segment
    vec2 p2 = toScreenSpace( projp2 );	// end of current segment, start of next segment
    vec2 p3 = toScreenSpace( projp3 );	// end of next segment

    // these are already 'final' depths in range [0,1] so we don't need to further transform them
    float p1z = projp1.z / projp1.w;
    float p2z = projp2.z / projp2.w;

    // determine the direction of each of the 3 segments (previous, current, next)
    vec2 v0 = normalize( p1 - p0 );
    vec2 v1 = normalize( p2 - p1 );
    vec2 v2 = normalize( p3 - p2 );

    // Martin's addition to fix flicker on starting ending point of lines
    if (p1 == p0) v0 = v1;
    if (p3 == p2) v2 = v1;

    // determine the normal of each of the 3 segments (previous, current, next)
    vec2 n0 = vec2( -v0.y, v0.x );
    vec2 n1 = vec2( -v1.y, v1.x );
    vec2 n2 = vec2( -v2.y, v2.x );

    // determine miter lines by averaging the normals of the 2 segments
    vec2 miter_a = normalize( n0 + n1 );	// miter at start of current segment
    vec2 miter_b = normalize( n1 + n2 );	// miter at end of current segment

    // determine the length of the miter by projecting it onto normal and then inverse it
    float length_a = THICKNESS / dot( miter_a, n1 );
    float length_b = THICKNESS / dot( miter_b, n1 );

    // prevent excessively long miters at sharp corners
    if( dot( v0, v1 ) < -MITER_LIMIT ) {
        miter_a = n1;
        length_a = THICKNESS;

        // close the gap
        if( dot( v0, n1 ) > 0 ) {
            VertexOut.mTexCoord = vec2( 0, 0 );
            //VertexOut.mColor = VertexIn[1].mColor;
            gl_Position = vec4( ( p1 + THICKNESS * n1 ) / WIN_SCALE, p1z, 1.0 );
            EmitVertex();

            VertexOut.mTexCoord = vec2( 0, 0 );
            //VertexOut.mColor = VertexIn[1].mColor;
            gl_Position = vec4( ( p1 + THICKNESS * n0 ) / WIN_SCALE, p1z, 1.0 );
            EmitVertex();

            VertexOut.mTexCoord = vec2( 0, 0.5 );
            //VertexOut.mColor = VertexIn[1].mColor;
            gl_Position = vec4( p1 / WIN_SCALE, p1z, 1.0 );
            EmitVertex();

            EndPrimitive();
        }
        else {
            VertexOut.mTexCoord = vec2( 0, 1 );
            //VertexOut.mColor = VertexIn[1].mColor;
            gl_Position = vec4( ( p1 - THICKNESS * n0 ) / WIN_SCALE, p1z, 1.0 );
            EmitVertex();

            VertexOut.mTexCoord = vec2( 0, 1 );
            //VertexOut.mColor = VertexIn[1].mColor;
            gl_Position = vec4( ( p1 - THICKNESS * n1 ) / WIN_SCALE, p1z, 1.0 );
            EmitVertex();

            VertexOut.mTexCoord = vec2( 0, 0.5 );
            //VertexOut.mColor = VertexIn[1].mColor;
            gl_Position = vec4( p1 / WIN_SCALE, p1z, 1.0 );
            EmitVertex();

            EndPrimitive();
        }
    }

    if( dot( v1, v2 ) < -MITER_LIMIT ) {
        miter_b = n1;
        length_b = THICKNESS;
    }

    // generate the triangle strip
    VertexOut.mTexCoord = vec2( 0, 0 );
    //VertexOut.mColor = VertexIn[1].mColor;
    gl_Position = vec4( ( p1 + length_a * miter_a ) / WIN_SCALE, p1z, 1.0 );
    EmitVertex();

    VertexOut.mTexCoord = vec2( 0, 1 );
    //VertexOut.mColor = VertexIn[1].mColor;
    gl_Position = vec4( ( p1 - length_a * miter_a ) / WIN_SCALE, p1z, 1.0 );
    EmitVertex();

    VertexOut.mTexCoord = vec2( 0, 0 );
    //VertexOut.mColor = VertexIn[2].mColor;
    gl_Position = vec4( ( p2 + length_b * miter_b ) / WIN_SCALE, p2z, 1.0 );
    EmitVertex();

    VertexOut.mTexCoord = vec2( 0, 1 );
    //VertexOut.mColor = VertexIn[2].mColor;
    gl_Position = vec4( ( p2 - length_b * miter_b ) / WIN_SCALE, p2z, 1.0 );
    EmitVertex();

    EndPrimitive();
}
