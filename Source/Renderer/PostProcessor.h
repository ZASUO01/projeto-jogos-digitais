#pragma once
#include "Shader.h"
#include "VertexArray.h"

class PostProcessor
{
public:
    PostProcessor();
    ~PostProcessor();

    bool Initialize(int width, int height);
    void Shutdown();

    void BeginRender();
    void EndRender();
    void ApplyEffects();

    void SetGlowIntensity(float intensity) { mGlowIntensity = intensity; }
    void SetChromaticAberration(float amount) { mChromaticAberration = amount; }

private:
    unsigned int mFramebuffer;
    unsigned int mTextureColorBuffer;
    unsigned int mRBO;

    Shader* mPostProcessShader;
    VertexArray* mScreenQuad;

    float mGlowIntensity;
    float mChromaticAberration;
    float mTime;
};