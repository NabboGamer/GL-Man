#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "PostProcessor.hpp"
#include "LoggerManager.hpp"

PostProcessor::PostProcessor(const unsigned int width, const unsigned int height, const bool useMSAA, const unsigned int numSampleMSAA)
             : width(width), height(height), useMSAA(useMSAA), numSampleMSAA(numSampleMSAA) {
    this->initRenderData();
}

PostProcessor::~PostProcessor() {
	glDeleteFramebuffers(1, &this->FBOMSAA);
	glDeleteTextures(1, &this->CBMSAA);
	glDeleteRenderbuffers(1, &this->RBOMSAA);
	glDeleteFramebuffers(1, &this->FBO);
	glDeleteTextures(1, &this->CB);
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

void PostProcessor::BeginRender() const {
	if (this->useMSAA) {
		glBindFramebuffer(GL_FRAMEBUFFER, this->FBOMSAA);
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear the selected framebuffer
}

void PostProcessor::EndRender() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Turn to the default framebuffer
}

void PostProcessor::Render(double deltaTime) const {
	if (this->useMSAA) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, this->FBOMSAA);												// Read from MSAA framebuffer
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);															// Write to default framebuffer
		glBlitFramebuffer(0, 0, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height),		// Source coordinates
						  0, 0, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height),		// Destination coordinates
						  GL_COLOR_BUFFER_BIT, GL_NEAREST);														// Copy the color buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);																// Unbind framebuffers
	}
	else {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, this->FBO);  
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height),
						  0, 0, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height),
						  GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}


void PostProcessor::initRenderData() {
	// --- Normal Framebuffer setup ---
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


	// --- MSAA Framebuffer Setup ---
	glGenFramebuffers(1, &this->FBOMSAA);
	glBindFramebuffer(GL_FRAMEBUFFER, this->FBOMSAA);

	// Create a multisample color attachment texture
	glGenTextures(1, &this->CBMSAA);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, this->CBMSAA);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLsizei>(this->numSampleMSAA), GL_RGBA16F, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height), GL_TRUE);

	// Create a multisample renderbuffer object for depth and stencil attachments
	glGenRenderbuffers(1, &this->RBOMSAA);
	glBindRenderbuffer(GL_RENDERBUFFER, this->RBOMSAA);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, static_cast<GLsizei>(this->numSampleMSAA), GL_DEPTH24_STENCIL8, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height));

	// Attach buffers
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, this->CBMSAA, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->RBOMSAA);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LoggerManager::LogError("MSAA Framebuffer not complete!");
	}

	// Unbind everything at the end of the configuration process
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	///HDR
	/*// Create the framebuffer
	glGenFramebuffers(1, &this->FBOHDR);

	// Create a floating point color attachment texture
	glGenTextures(1, &this->CBHDR);
	glBindTexture(GL_TEXTURE_2D, this->CBHDR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height), 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Create a floating point renderbuffer object for depth attachments
	glGenRenderbuffers(1, &this->RBOHDR);
	glBindRenderbuffer(GL_RENDERBUFFER, this->RBOHDR);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height));

	// Attach buffers
	glBindFramebuffer(GL_FRAMEBUFFER, this->FBOHDR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->CBHDR, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->RBOHDR);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LoggerManager::LogError("Framebuffer not complete!" << std::endl);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);*/
}
