#version 150

uniform float	THICKNESS;		// the thickness of the line in pixels
uniform float	MITER_LIMIT;	// 1.0: always miter, -1.0: never miter, 0.75: default
uniform vec2	WIN_SCALE;		// the size of the viewport in pixels

layout( lines_adjacency ) in;
layout( triangle_strip, max_vertices = 7 ) out;


//in VertexData{
//	vec3 mColor;
//} VertexIn[4];

out VertexData{
    vec2 mTexCoord;
//	vec3 mColor;
} VertexOut;

vec2 toScreenSpace( vec4 vertex )
{
    return vec2( vertex.xy / vertex.w ) * WIN_SCALE;
}

vec4 clip_near_plane(vec4 pt1, vec4 pt2)
{
  // Figure out intersection point of line pt1-pt2 and near plane in homogeneous coordinates.
  // Near plane is z=-1 in NDC, that means in homogeneous coordinates that's z/w=-1
  // Going from line equation P = P1 + u * (P2 - P1) we need to figure out "u"
  // In the above equation P, P1, P2 are vectors, so individual coordinate values are
  // x = x1 + u * (x2 - x1) and so on for y,z,w as well. Now combining near plane equation z/w=-1
  // with line equation gives us the following equation for "u" (it's easy to do the math on paper)

  float u = (-pt1.z - pt1.w) / ((pt2.z-pt1.z) + (pt2.w - pt1.w));
  return pt1 + (pt2-pt1)*u;
}

void main( void )
{
    vec4 px0 = gl_in[0].gl_Position;
    vec4 px1 = gl_in[1].gl_Position;
    vec4 px2 = gl_in[2].gl_Position;
    vec4 px3 = gl_in[3].gl_Position;

    // This implements rejection of lines from Cohen-Sutherland line clipping algorithm.
    // Thanks to that we filter out majority of lines that may otherwise cause issues.
    // Lines that can't be trivially rejected, should be further clipped
    int px1c = int(px1.w+px1.x<0) << 0 | int(px1.w-px1.x<0) << 1 | int(px1.w+px1.y<0) << 2 | int(px1.w-px1.y<0) << 3 | int(px1.w+px1.z<0) << 4 | int(px1.w-px1.z<0) << 5;
    int px2c = int(px2.w+px2.x<0) << 0 | int(px2.w-px2.x<0) << 1 | int(px2.w+px2.y<0) << 2 | int(px2.w-px2.y<0) << 3 | int(px2.w+px2.z<0) << 4 | int(px2.w-px2.z<0) << 5;
    if ((px1c & px2c) != 0)
      return;  // trivial reject

    // Perform line clipping with near plane if needed. We search for intersection between the line and the near plane.
    // In case the near plane intersects line between segment's endpoints, we need to adjust the line
    // otherwise we would use completely non-sense points when points get 'behind' the camera.
    // It seems we don't need to clip against other five planes - only the near plane is critical because
    // that turns the coordinates after perspective division in toScreenSpace() into a mess because of w < 1
    if ((px1c & 16) != 0)
    {
      // first point is in front of the near plane - need to clip it
      px1 = clip_near_plane(px1, px2);
      px0 = px1;
    }
    if ((px2c & 16) != 0)
    {
      // second point is in front of the near plane - need to clip it
      px2 = clip_near_plane(px1, px2);
      px3 = px2;
    }

    // get the four vertices passed to the shader:
    vec2 p0 = toScreenSpace( px0 );	// start of previous segment
    vec2 p1 = toScreenSpace( px1 );	// end of previous segment, start of current segment
    vec2 p2 = toScreenSpace( px2 );	// end of current segment, start of next segment
    vec2 p3 = toScreenSpace( px3 );	// end of next segment

    // these are already 'final' depths in range [0,1] so we don't need to further transform them
    float p1z = px1.z / px1.w;
    float p2z = px2.z / px2.w;

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
