#version 150 core

#ifdef DATA_DEFINED
in DataColor {
    vec3 diffuse;
    vec3 warm;
    vec3 cool;
    vec3 specular;
} vs_in;
#else
uniform vec3 kd;            // Diffuse reflectivity
uniform vec3 ks;            // Specular reflectivity
uniform vec3 kblue;         // Cool color
uniform vec3 kyellow;       // Warm color
#endif


uniform float alpha;        // Fraction of diffuse added to kblue
uniform float beta;         // Fraction of diffuse added to kyellow
uniform float shininess;    // Specular shininess factor

uniform vec3 eyePosition;

in vec3 worldPosition;
in vec3 worldNormal;

out vec4 fragColor;

#pragma include light.inc.frag

vec3 goochModel( const in vec3 pos, const in vec3 n )
{
    // Based upon the original Gooch lighting model paper at:
    // http://www.cs.northwestern.edu/~ago820/SIG98/abstract.html

    // Calculate kcool and kwarm from equation (3)
    #ifdef DATA_DEFINED
      vec3 kcool = clamp(vs_in.cool + alpha * vs_in.diffuse, 0.0, 1.0);
      vec3 kwarm = clamp(vs_in.warm + beta * vs_in.diffuse, 0.0, 1.0);
    #else
      vec3 kcool = clamp(kblue + alpha * kd, 0.0, 1.0);
      vec3 kwarm = clamp(kyellow + beta * kd, 0.0, 1.0);
    #endif

    vec3 result = vec3(0.0);
    for (int i = 0; i < lightCount; ++i) {
        // Calculate the vector from the light to the fragment
        vec3 s = normalize( vec3( lights[i].position ) - pos );

        // Calculate the cos theta factor mapped onto the range [0,1]
        float sDotNFactor = ( 1.0 + dot( s, n ) ) / 2.0;

        // Calculate the tone by blending the kcool and kwarm contributions
        // as per equation (2)
        vec3 intensity = mix( kcool, kwarm, sDotNFactor );

        // Calculate the vector from the fragment to the eye position
        vec3 v = normalize( eyePosition - pos );

        // Reflect the light beam using the normal at this fragment
        vec3 r = reflect( -s, n );

        // Calculate the specular component
        float specular = 0.0;
        if ( dot( s, n ) > 0.0 )
            specular = pow( max( dot( r, v ), 0.0 ), shininess );

        // Sum the blended tone and specular highlight
        #ifdef DATA_DEFINED
          result += intensity + vs_in.specular * specular;
        #else
          result += intensity + ks * specular;
        #endif
    }

    return result;
}

void main()
{
    fragColor = vec4( goochModel( worldPosition, normalize( worldNormal ) ), 1.0 );
}
