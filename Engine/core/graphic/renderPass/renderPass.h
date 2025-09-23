#pragma once
#include<glad/glad.h>
#include <glm.hpp>
#include <vector>
#include <memory>
#include "typedef.h"

class Program;
class FrameBuffer;
class Geometry;
class Scene;

enum class RenderTarget
{
	SCREEN,
	FRAMEBUFFER
};

struct RenderState
{
	glm::vec4	 viewport		 { 0, 0, 800, 600						     };
	RenderTarget target			 { RenderTarget::SCREEN					     };
	bool		 depthTest       { true                                      };
	GLenum		 depthFunc		 { GL_LESS									 };
	GLenum	     depthMask		 { GL_TRUE									 };
	bool		 cullFace        { true                                     };
	GLenum		 cullFaceMode    { GL_BACK                                   };	
	GLenum		 frontFace       { GL_CCW                                    };
	bool		 blend           { false                                     };
	bool		 stencilTest     { false                                     };
	bool		 scissorTest     { false                                     };
	GLenum		 blendSrcFactor  { GL_SRC_ALPHA                              };
	GLenum		 blendDstFactor  { GL_ONE_MINUS_SRC_ALPHA                    };
	GLenum		 polygonMode     { GL_FILL                                   };
	int		     clearBits       { GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT };
	float		 clearDepth      { 1.0f                                      };
	float		 clearStencil    { 0.0f                                      };
	glm::vec4	 clearColor      { 0.1f, 0.1f, 0.1f, 1.0f                    };	
	glm::vec4	 scissorBox		 { 0, 0, 800, 600                            };
};

/// <summary>
/// RenderPass
/// </summary>
class RenderPass
{
public:
	RenderPass(const RenderState& state);
	virtual ~RenderPass() = default;
	RenderPass(const RenderPass& pass) = delete;
	RenderPass(RenderPass&& pass) = delete;
	RenderPass& operator=(const RenderPass& pass) = delete;
	RenderPass& operator=(const RenderPass&& pass) = delete;
	virtual void beginPass();
	virtual void runPass(Scene* scene) = 0;
	virtual void endPass();
	virtual void resize(uint width, uint height);
	//update per pass per frame
	void updateRenderState() const;
	void setRenderState(const RenderState& state);
	bool checkPassReady();
	void setLastPassFBOs(const std::initializer_list<FrameBuffer*>& fbo);
	std::shared_ptr<FrameBuffer> getCurrentFrameBuffer() const;
	virtual void renderUI() {};
protected:
	std::shared_ptr<Program>					 mProgram;
	std::shared_ptr<FrameBuffer>				 mFrameBuffer;
	std::vector<FrameBuffer*>			         mlastPassFrameBuffer;
	RenderState									 mRenderState;
	glm::vec2									 mSize;
};