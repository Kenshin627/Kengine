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
class Renderer;

enum class RenderTarget
{
	SCREEN,
	FRAMEBUFFER
};

struct RenderState
{
	glm::vec4	 viewport		 { 0, 0, 800, 600						     };
	bool		 depthTest       { true                                      };
	RenderTarget target			 { RenderTarget::FRAMEBUFFER				 };
	GLenum		 depthFunc		 { GL_LESS									 };
	GLenum	     depthMask		 { GL_TRUE									 };
	bool		 cullFace        { true                                      };
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

enum class RenderPassKey
{
	SSAO,
	SSAOBLUR,
	CSM,
	BLOOM,
	BLOOMBLUR,
	GEOMETRY,
	DEFFEREDSHADING,
	FORWARDSHADING,
	TONEMAPPING
};

/// <summary>
/// RenderPass
/// </summary>
class RenderPass
{
public:
	RenderPass(Renderer* s, const RenderState& state);
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
	//void setLastPassFBOs(const std::initializer_list<FrameBuffer*>& fbo);
	FrameBuffer* getCurrentFrameBuffer() const;
	void setPrev(RenderPass* pass);
	void setNext(RenderPass* pass);
	RenderPass* prev() const;
	RenderPass* next() const;
	void active();
	void deActive();
	bool isActive() const;
protected:
	std::shared_ptr<Program>					 mProgram;
	std::shared_ptr<FrameBuffer>				 mFrameBuffer;
	//std::vector<FrameBuffer*>			         mlastPassFrameBuffer;
	RenderState									 mRenderState;
	glm::vec2									 mSize;
	RenderPass*									 mPrevPass{ nullptr };
	RenderPass*									 mNextPass{ nullptr };
	Renderer*									 mOwner   { nullptr };
	bool										 mIsActive{ true    };
};