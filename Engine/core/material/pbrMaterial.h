#pragma once
#include "material.h"

class Texture2D;
struct PBRMaterialSpecification
{
	glm::vec3	albedoColor			{ 0.2, 0.2, 0.2 };
	float		metallic			{ 0.0			};
	float		roughness			{ 0.5			};
	const char* albedoMapPath		{ nullptr		};
	const char* metallicMapPath		{ nullptr		};
	const char* roughnessMapPath	{ nullptr		};
	const char* normalMapPath		{ nullptr		};
	const char* heightMapPath		{ nullptr		};
};

class PBRMaterial :public Material
{
public:
	PBRMaterial(const PBRMaterialSpecification& spec);
	~PBRMaterial();
	virtual void setUniforms(Program* p) const override;
private:
	glm::vec3                  mAlbedoColor     { 0.2, 0.2, 0.2 };
	float                      mMetallic	    { 0.8		    };
	float                      mRoughness	    { 0.2		    };
	std::shared_ptr<Texture2D> mAlbedoMap;
	std::shared_ptr<Texture2D> mMetallicMap;
	std::shared_ptr<Texture2D> mRoughnessMap;
	std::shared_ptr<Texture2D> mNormalMap;
	std::shared_ptr<Texture2D> mHeightMap;
	bool					   mHasAlbedoTex    { false			};
	bool					   mHasMetallicTex  { false			};
	bool					   mHasRoughnessTex { false			};
	bool					   mHasNormalTex    { false			};
	bool					   mhasHeightTex	{ false			};
};