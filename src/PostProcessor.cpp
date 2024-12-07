#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "PostProcessor.hpp"
#include "LoggerManager.hpp"

PostProcessor::PostProcessor(const unsigned int width, const unsigned int height, const bool useMSAA, 
							 const unsigned int numSampleMSAA, Shader* hdrShader, const bool useHDR,
							 const float exposure, const float gamma, Shader* blurShader, Shader* bloomShader)
             : width(width), height(height), useMSAA(useMSAA), numSampleMSAA(numSampleMSAA),
               hdrShader(hdrShader), useHDR(useHDR), exposure(exposure), gamma(gamma), initialized(false), blurShader(blurShader),
			   bloomShader(bloomShader) {
	//this->initRenderData();
}

PostProcessor::~PostProcessor() {
	glDeleteFramebuffers(1, &this->FBO);
	glDeleteTextures(1, &this->CB);
	glDeleteRenderbuffers(1, &this->RBO);
	glDeleteFramebuffers(1, &this->FBOMSAA);
	glDeleteTextures(2, this->CBMSAA);
	glDeleteRenderbuffers(1, &this->RBOMSAA);
	glDeleteFramebuffers(1, &this->FBOHDR);
	glDeleteTextures(2, this->CBHDR);
	glDeleteRenderbuffers(1, &this->RBOHDR);
}

bool PostProcessor::GetUseMSAA() const{
	return this->useMSAA;
}

void PostProcessor::SetUseMSAA(const bool value) {
	this->useMSAA = value;
}

unsigned int PostProcessor::GetNumSampleMSAA() const {
	return this->numSampleMSAA;
}

void PostProcessor::SetNumSampleMSAA(const unsigned int value) {
	this->numSampleMSAA = value;
}

float PostProcessor::GetExposure() const {
	return this->exposure;
}

void PostProcessor::SetExposure(const float value) {
	this->exposure = value;
}

float PostProcessor::GetGamma() const {
	return this->gamma;
}

void PostProcessor::SetGamma(const float value) {
	this->gamma = value;
}

bool PostProcessor::IsInitialized() const {
	return this->initialized;
}


void PostProcessor::SetInitialized(const bool value, const CustomStructs::Config config) {
	this->initialized = value;
	if (this->initialized) {
		this->useMSAA       = config.useMSAA;
		this->numSampleMSAA = config.numSampleMSAA;
		this->useHDR        = config.useHDR;
		this->exposure      = config.exposure;
		/*LoggerManager::LogInfo("---------------------------------");
		LoggerManager::LogInfo("useMSAA={}", this->useMSAA);
		LoggerManager::LogInfo("numSampleMSAA={}", this->numSampleMSAA);
		LoggerManager::LogInfo("useHDR={}", this->useHDR);
		LoggerManager::LogInfo("exposure={}", this->exposure);
		LoggerManager::LogInfo("---------------------------------");*/
		this->initRenderData();
	}
}

void PostProcessor::BeginRender() const {
	if (this->useMSAA) {
		glBindFramebuffer(GL_FRAMEBUFFER, this->FBOMSAA);
	}
	else if (this->useHDR) {
		glBindFramebuffer(GL_FRAMEBUFFER, this->FBOHDR);
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, this->FBOHDR);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear the selected framebuffer
}

void PostProcessor::EndRender() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Turn to the default framebuffer
}

void PostProcessor::Render(double deltaTime) const {
	if (this->useMSAA && this->useHDR) {
		//LoggerManager::LogInfo("Use MSAA, HDR, Gamma Correction");
		// 1. Transfer MSAA to HDR texture
		for (int i = 0; i < 2; ++i) { // Supponiamo di avere due color attachments
			// Seleziona l'attachment i-esimo come sorgente e destinazione
			glBindFramebuffer(GL_READ_FRAMEBUFFER, this->FBOMSAA);
			glReadBuffer(GL_COLOR_ATTACHMENT0 + i); // Color attachment i

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->FBOHDR);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + i); // Color attachment i

			// Esegui il blit
			glBlitFramebuffer(
				0, 0, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height),
				0, 0, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height),
				GL_COLOR_BUFFER_BIT, GL_NEAREST
			);
		}

		// 2. HDR Post-Process
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		hdrShader->Use();
		hdrShader->SetInteger("hdrBuffer", 0);
		hdrShader->SetFloat("exposure", this->exposure);
		hdrShader->SetFloat("gamma", this->gamma);
		hdrShader->SetBool("useHDR", true);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, this->CBHDR[0]);

		//renderQuad();
	}
	else if (this->useHDR) {
		//LoggerManager::LogInfo("Use HDR, Gamma Correction");
		// Directly render HDR (no MSAA)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		hdrShader->Use();
		hdrShader->SetInteger("hdrBuffer", 0);
		hdrShader->SetFloat("exposure", this->exposure);
		hdrShader->SetFloat("gamma", this->gamma);
		hdrShader->SetBool("useHDR", true);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, this->CBHDR[0]);

		//renderQuad();
	}
	else if (this->useMSAA) {
		//LoggerManager::LogInfo("Use MSAA, Gamma Correction");
		// MSAA without HDR
		for (int i = 0; i < 2; ++i) { // Supponiamo di avere due color attachments
			// Seleziona l'attachment i-esimo come sorgente e destinazione
			glBindFramebuffer(GL_READ_FRAMEBUFFER, this->FBOMSAA);
			glReadBuffer(GL_COLOR_ATTACHMENT0 + i); // Color attachment i

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->FBOHDR);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + i); // Color attachment i

			// Esegui il blit
			glBlitFramebuffer(
				0, 0, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height),
				0, 0, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height),
				GL_COLOR_BUFFER_BIT, GL_NEAREST
			);
		}
		// Gamma Correction Post-Process
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		hdrShader->Use();
		hdrShader->SetInteger("hdrBuffer", 0);
		hdrShader->SetFloat("exposure", this->exposure);
		hdrShader->SetFloat("gamma", this->gamma);
		hdrShader->SetBool("useHDR", false);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, this->CBHDR[0]);

		//renderQuad();
	}
	else {
		//LoggerManager::LogInfo("Use Gamma Correction");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		hdrShader->Use();
		hdrShader->SetInteger("hdrBuffer", 0);
		hdrShader->SetFloat("exposure", this->exposure);
		hdrShader->SetFloat("gamma", this->gamma);
		hdrShader->SetBool("useHDR", false);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, this->CBHDR[0]);

		//renderQuad();
	}

	bool horizontal = true, first_iteration = true;
	unsigned int amount = 10;
	
	for (unsigned int i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBOBLUR[horizontal]);
		blurShader->Use().SetInteger("horizontal", horizontal);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? CBHDR[1] : CBBLUR[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
		renderQuad();
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	bloomShader->Use();
	bloomShader->SetFloat("exposure", this->exposure);
	bloomShader->SetFloat("gamma", this->gamma);
	bloomShader->SetBool("useHDR", this->useHDR);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, CBHDR[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, CBBLUR[!horizontal]);
    //bloomShader->SetFloat("exposure", exposure);
	renderQuad();


	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}


void PostProcessor::initRenderData() {
	/// --- Normal Framebuffer setup ---
	glGenFramebuffers(1, &this->FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);

	// Color attachment
	glGenTextures(1, &this->CB);
	glBindTexture(GL_TEXTURE_2D, this->CB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->CB, 0);

	// Depth and stencil attachment
	glGenRenderbuffers(1, &this->RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, this->RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height));
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->RBO);

	// Check framebuffer completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LoggerManager::LogError("Normal Framebuffer not complete!");
	}

	// Unbind
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	/// --- MSAA Framebuffer Setup ---
	glGenFramebuffers(1, &this->FBOMSAA);
	glBindFramebuffer(GL_FRAMEBUFFER, this->FBOMSAA);

	// Create a multisample color attachment texture
	glGenTextures(2, this->CBMSAA);
	for (int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, this->CBMSAA[i]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLsizei>(this->numSampleMSAA), GL_RGBA16F, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height), GL_TRUE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, this->CBMSAA[i], 0);
	}

	// Create a multisample renderbuffer object for depth and stencil attachments
	glGenRenderbuffers(1, &this->RBOMSAA);
	glBindRenderbuffer(GL_RENDERBUFFER, this->RBOMSAA);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, static_cast<GLsizei>(this->numSampleMSAA), GL_DEPTH24_STENCIL8, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height));

	// Attach buffers
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->RBOMSAA);
	unsigned int attachMSAA[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachMSAA);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LoggerManager::LogError("MSAA Framebuffer not complete!");
	}

	// Unbind everything at the end of the configuration process
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	/// --- HDR Framebuffer Setup ---
	glGenFramebuffers(1, &this->FBOHDR);
	glBindFramebuffer(GL_FRAMEBUFFER, this->FBOHDR);

	// Create a floating-point color attachment texture
	glGenTextures(2, this->CBHDR);
	for (int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, this->CBHDR[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, this->CBHDR[i], 0);
	}

	// Create a renderbuffer for depth (and stencil if needed)
	glGenRenderbuffers(1, &this->RBOHDR);
	glBindRenderbuffer(GL_RENDERBUFFER, this->RBOHDR);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height));

	// Attach color and depth buffers
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->RBOHDR);
	unsigned int attachHDR[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachHDR);
	// Check framebuffer completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LoggerManager::LogError("HDR Framebuffer not complete!");
	}

	// Unbind everything at the end of the configuration process
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/// --- BLUR Framebuffer Setup ---
	glGenFramebuffers(2, this->FBOBLUR);
	glGenTextures(2, this->CBBLUR);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, this->FBOBLUR[i]);
		glBindTexture(GL_TEXTURE_2D, this->CBBLUR[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->CBBLUR[i], 0);
		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LoggerManager::LogError("BLUR Framebuffer {} not complete!", i);
	}

	// Unbind everything at the end of the configuration process
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::renderQuad() {
	static unsigned int quadVAO = 0;
	static unsigned int quadVBO;

	if (quadVAO == 0) {
		constexpr float quadVertices[] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}