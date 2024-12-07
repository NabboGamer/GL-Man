#pragma once

#include "Shader.hpp"
#include "custom_types.hpp"

class PostProcessor {

public:

    // state
    unsigned int width;
	unsigned int height;
    Shader*      hdrShader;
    Shader*      blurShader;
    Shader*      bloomShader;

    // constructor/decostructor
    PostProcessor(unsigned int width, unsigned int height, bool useMSAA, unsigned int numSampleMSAA, 
                  Shader* hdrShader, bool useHDR, float exposure, float gamma, Shader* blurShader, Shader* bloomShader);
    ~PostProcessor();

    // Getter/Setter
    [[nodiscard]] bool GetUseMSAA() const;
    [[nodiscard]] unsigned int GetNumSampleMSAA() const;
    [[nodiscard]] float GetExposure() const;
    [[nodiscard]] float GetGamma() const;
    [[nodiscard]] bool IsInitialized() const;
    void SetUseMSAA(bool value);
    void SetNumSampleMSAA(unsigned int value);
    void SetExposure(float value);
    void SetGamma(float value);
    void SetInitialized(bool value, CustomStructs::Config config);

    // prepares the postprocessor's framebuffer operations before rendering the game
    void BeginRender() const;

    // should be called after rendering the game, so it stores all the rendered data into a texture object
    void EndRender();

    // renders the PostProcessor texture quad
    void Render(double deltaTime) const;

private:

    bool initialized;
    // options
    bool          useMSAA;
    bool          useHDR;
    unsigned int  numSampleMSAA;
    float         exposure;
    float         gamma;

    // render state
    unsigned int FBO;
    unsigned int CB;
    unsigned int RBO;

    unsigned int FBOMSAA;     // Frame  Buffer Object for Multi-Sample AntiAliasing
    unsigned int CBMSAA[2];      // Color  Buffer        for Multi-Sample AntiAliasing
    unsigned int RBOMSAA;     // Render Buffer Object for Multi-Sample AntiAliasing

    unsigned int FBOHDR;     // Frame  Buffer Object for High Dynamic Range
    unsigned int CBHDR[2];      // Color  Buffer        for High Dynamic Range
    unsigned int RBOHDR;     // Render Buffer Object for High Dynamic Range

    unsigned int FBOBLUR[2];
    unsigned int CBBLUR[2];

    void initRenderData();
    static void renderQuad();
};
