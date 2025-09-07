#pragma once
#include<glad/glad.h>
#include <glm.hpp>
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
	bool		 cullFace        { false                                     };
	bool		 depthTest       { true                                      };
	bool		 blend           { false                                     };
	bool		 stencilTest     { false                                     };
	bool		 scissorTest     { false                                     };
	GLenum		 cullFaceMode    { GL_BACK                                   };
	GLenum		 depthFunc       { GL_LESS                                   };
	GLenum		 blendSrcFactor  { GL_SRC_ALPHA                              };
	GLenum		 blendDstFactor  { GL_ONE_MINUS_SRC_ALPHA                    };
	GLenum		 frontFace       { GL_CCW                                    };
	GLenum		 polygonMode     { GL_FILL                                   };
	int		     clearBits       { GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT };
	float		 clearDepth      { 1.0f                                      };
	float		 clearStencil    { 0.0f                                      };
	glm::vec4	 clearColor      { 0.2f, 0.2f, 0.2f, 1.0f                    };
	glm::vec4	 viewport        { 0, 0, 800, 600                            };
	RenderTarget target			 { RenderTarget::SCREEN						 };
	uint		 width			 { 800										 };
	uint		 height			 { 600										 };
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
	void beginPass();
	virtual void runPass(Scene* scene) = 0;
	void endPass();
	void resize(uint width, uint height);
	//update per pass per frame
	void updateRenderState() const;
	void setRenderState(const RenderState& state);
	bool checkPassReady();
	void setLastPassFBO(std::shared_ptr<FrameBuffer> fbo);
	std::shared_ptr<FrameBuffer> getCurrentFrameBuffer() const;
protected:
	std::shared_ptr<Program>		mProgram;
	std::shared_ptr<FrameBuffer>    mFrameBuffer;
	std::shared_ptr<FrameBuffer>    mlastPassFrameBuffer;
	std::shared_ptr<Geometry>       mGeometry;
	RenderState						mRenderState;
};