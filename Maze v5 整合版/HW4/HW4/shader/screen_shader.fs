#version 330 core

#define SIGMA 10.0
#define BSIGMA 0.1
#define MSIZE 3

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

float normpdf(float x, float sigma) {
    return 0.39894 * exp(-0.5*x*x/(sigma*sigma)) / sigma;
}

float normpdf3(vec3 v, float sigma) {
    return 0.39894 * exp(-0.5*dot(v,v)/(sigma*sigma)) / sigma;
}

const float kernel[15] = float[](
    0.031225216,
    0.033322271,
    0.035206333,
    0.036826804,
    0.038138565,
    0.039104044,
    0.039695028,
    0.039894000,
    0.039695028,
    0.039104044,
    0.038138565,
    0.036826804,
    0.035206333,
    0.033322271,
    0.031225216
);

void main()
{
    vec3 c = texture(screenTexture, TexCoords.st).rgb;

    //declare stuff
    const int kSize = (MSIZE-1)/2;
    // float kernel[MSIZE];
    vec3 result = vec3(0.0);
    
    //create the 1-D kernel
    float Z = 0.0;
    // for (int j = 0; j <= kSize; ++j)
    // {
    //     kernel[kSize+j] = kernel[kSize-j] = normpdf(j, SIGMA);
    // }
    
    vec3 cc;
    float factor;
    float bZ = 1.0 / normpdf(0.0, BSIGMA);
    //read out the texels
    for (int i=-kSize; i <= kSize; ++i)
    {
        for (int j=-kSize; j <= kSize; ++j)
        {
            cc = texture(screenTexture, TexCoords.st + vec2(i, j)/textureSize(screenTexture, 0)).rgb;
            factor = normpdf3(cc-c, BSIGMA) * bZ * kernel[kSize+j] * kernel[kSize+i];
            Z += factor;
            result += factor*cc;
        }
    }
    
    FragColor = vec4(result/Z, 1.0);
}