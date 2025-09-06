#pragma once
#include <glad/glad.h>

enum class ShaderType
{
	Vertex = GL_VERTEX_SHADER,
	Fragment = GL_FRAGMENT_SHADER,
	Geometry = GL_GEOMETRY_SHADER,
	Compute = GL_COMPUTE_SHADER,
	TessControl = GL_TESS_CONTROL_SHADER,
	TessEvaluation = GL_TESS_EVALUATION_SHADER
};

enum class ProgramType
{
	Graphic,
	Compute
};