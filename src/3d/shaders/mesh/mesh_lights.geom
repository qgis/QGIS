#version 330 core

layout( triangles ) in;
layout( triangle_strip, max_vertices = 3 ) out;


uniform bool flatTriangles;

in MeshVertex {
    vec3 position;
    vec3 wordPosition;
    vec3 wordNormal;
    float magnitude;
} gs_in[];

out treatedVertex {
    vec3 position;
    vec3 wordPosition;
    vec3 wordNormal;
} gs_out;

void main()
{
    int configuration = int(gl_in[0].gl_Position.z < 0) * int(4)
            + int(gl_in[1].gl_Position.z < 0) * int(2)
            + int(gl_in[2].gl_Position.z < 0);

    // If all vertices are behind us, cull the primitive
    if (configuration == 7)
         return;

     vec3 normal[3];

    if (flatTriangles)
    {
        vec3 v1=gs_in[1].wordPosition-gs_in[0].wordPosition;
        vec3 v2=gs_in[2].wordPosition-gs_in[0].wordPosition;
        vec3 normalTriangle=cross(v1,v2);
        normalTriangle=normalize(normalTriangle);

        normal[0]=normalTriangle;
        normal[1]=normalTriangle;
        normal[2]=normalTriangle;
    }
    else
    {
        normal[0]=gs_in[0].wordNormal;
        normal[1]=gs_in[1].wordNormal;
        normal[2]=gs_in[2].wordNormal;
    }


    gs_out.wordPosition=gs_in[0].wordPosition;
    gs_out.wordNormal=normal[0];
    gs_out.position = gs_in[0].position;
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();


    gs_out.wordPosition=gs_in[1].wordPosition;
    gs_out.wordNormal=normal[1];
    gs_out.position = gs_in[1].position;
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();


    gs_out.wordPosition=gs_in[2].wordPosition;
    gs_out.wordNormal=normal[2];
    gs_out.position = gs_in[2].position;
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    // Finish the primitive off
    EndPrimitive();
}
