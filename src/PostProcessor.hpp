#pragma once

class PostProcessor {

public:

    // state
    unsigned int width;
	unsigned int height;

    // constructor/decostructor
    PostProcessor(unsigned int width, unsigned int height, bool useMSAA, unsigned int numSampleMSAA);
    ~PostProcessor();

    // Getter/Setter
    [[nodiscard]] bool GetUseMSAA() const;
    [[nodiscard]] unsigned int GetNumSampleMSAA() const;
    void SetUseMSAA(bool value);
    void SetNumSampleMSAA(unsigned int value);

    // prepares the postprocessor's framebuffer operations before rendering the game
    void BeginRender() const;

    // should be called after rendering the game, so it stores all the rendered data into a texture object
    static void EndRender();

    // renders the PostProcessor texture quad
    void Render(double deltaTime) const;

private:

    // options
    bool         useMSAA;
    bool         useHDR;
    unsigned int numSampleMSAA;

    // render state
    unsigned int FBO;
    unsigned int CB;
    unsigned int RBO;

    unsigned int FBOMSAA;     // Frame  Buffer Object for Multi-Sample AntiAliasing
    unsigned int CBMSAA;      // Color  Buffer        for Multi-Sample AntiAliasing
    unsigned int RBOMSAA;     // Render Buffer Object for Multi-Sample AntiAliasing

    unsigned int FBOHDR;     // Frame  Buffer Object for High Dynamic Range
    unsigned int CBHDR;      // Color  Buffer        for High Dynamic Range
    unsigned int RBOHDR;     // Render Buffer Object for High Dynamic Range

    void initRenderData();
};
