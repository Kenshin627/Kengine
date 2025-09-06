#pragma once
#include <memory>
#include "graphic/program/program.h"

class Program;
class Material
{
public:
	Material();
	void bind() const;
	void unBind() const;
	virtual ~Material() = default;
	Material(const Material& obj) = delete;
	Material(Material&& obj) = delete;
	Material& operator=(const Material& obj) = delete;
	virtual void setUniforms(Program* p) const = 0;
	virtual void beginDraw();
	virtual void endDraw();
	virtual void initProgram();
	Program* getProgram() const { return mProgram.get(); }
protected:
	Scope<Program> mProgram;
};