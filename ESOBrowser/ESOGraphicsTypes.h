#ifndef ESO_GRAPHICS_TYPES_H
#define ESO_GRAPHICS_TYPES_H

#include <stdint.h>

#pragma pack(push)
#pragma pack(4)

struct ESOLikeVertex {
	float Position[3];
	uint8_t DiffuseColor0[4];
	int16_t TextureCoord0[2];
	int16_t TextureCoord1[2];
};

struct ESOLikeVertexAside {
	int16_t Normal[2];
	int16_t Tangent[2];
	int16_t Binormal[2];
};

struct ESOMeshExtendedData {
	float BBoxMin[3];
	float BBoxMax[3];
	float Radius;
};

struct ESOMaterial {
	const char* Diffuse;
	const char* normal;
	const char* detail;
	const char* diffuse2;
	int ZosSphericalLighting;
	float ZosFresnel2;
	float ZosGlow2;
	float ZosGlossiness2;
	float AnimateU_Layer1;
	float AnimateV_Layer1;
	float AnimateU_Layer2;
	float AnimateV_Layer2;
	int alpha;
	int twoSided;
	const char* Specular;
	int materialType;
	int NoHalo;
	int ZosExportVersion;
	float ZosDayGlow;
	float ZosDuskGlow;
	float ZosNightGlow;
	int ZosPlayerLight;
	float SoftAlphaDepth;
	const char* tintmaskPath;
	const char* diffusePath;
	const char* normalPath;
	const char* detailPath;
	const char* diffuse2Path;
	const char* specularPath;
	const char* tintmask;
	float f4DiffuseTintPrimary[4];
	float f4SpecularTintPrimary[4];
	float f4NormalTintPrimary[4];
	float f4DiffuseTintSecondary[4];
	float f4SpecularTintSecondary[4];
	float f4NormalTintSecondary[4];
	float f4DiffuseTintAccent[4];
	float f4SpecularTintAccent[4];
	float f4NormalTintAccent[4];
	int TintPrimaryID;
	int TintSecondaryID;
	int TintAccentID;
	int DrawOrder;

	int NoShadows;
	const char* typeName;
	int ShaderType;
	int Wire;
	int TwoSided;
	int FaceMap;
	int Faceted;
	const char* ShaderName;
	int OpacityType;
	float Opacity;
	float FilterColor[3];
	int FalloffType;
	float Falloff;
	float IndexOfRefraction;
	float WireSize;
	int WireUnits;
	int ApplyReflectionDimming;
	float DimLevel;
	float ReflectionLevel;
	int PixelSampler;
	float SamplerQuality;
	int SamplerEnable;
	float AdaptiveThreshold;
	int AdaptiveOn;
	int SubSampleTextures;
	int AdvancedOptions;
	const char* SamplerName;
	float OptionalParam0;
	float OptionalParam1;
	int UseGlobalSettings;
	int *MapEnables;
	float* MapAmounts;
	int AmbientDiffuseTextureLock;
	float BounceCoefficient;
	float StaticFriction;
	float SlidingFriction;

	int bDiffuseEnable;
	const char* DiffuseTexture;
	int bSpecularEnable;
	const char* SpecularTexture;
	int bNormalEnable;
	const char* NormalTexture;
	int bDetailBlendEnable;
	const char* DetailBlendTexture;
	int bDiffuseSpecular2Enable;
	const char* DiffuseSpecular2Texture;
	int bDiffuse2Enable;
	const char* Diffuse2Texture;
	int bSpecular2Enable;
	const char* Specular2Texture;
	int bNormal2Enable;
	const char* Normal2Texture;
	int bMaskEnable;
	const char* MaskTexture;
	float f2UVScrolling[4];
	float fGlossiness2;
	float fFresnel2;
	float fGlow2;
	int bEnvironmentEnable;
	const char* EnvironmentTexture;
	int bTwoMaterial;
	int bFresnel;
	int bFresnelMode;
	int bGlowEnable;
	int bOutputGlow;
	int bGlossinessEnable;
	int bAlphaMaskEnable;
	int bAlphaTrasparency;
	int bGrayDiffuse;
	int bTwoSided;
	float fCubeRotation;
	int bSunEnabled;
	float f3SunDirection[3];
	float f3SunColor[3];
	float f3EnvironmentTint[3];
	const char* EffectFile;
	int SoftwareRendering;
	float ShadedColor[3];
	int ForceSoftwareRender;
	int Technique;
	int DoubleQuestionMark;

	float AmbientColor[3];
	float DiffuseColor[3];
	float SpecularColor[3];
	float Shininess;
	float ShininessStrength;
	float Transparency;
	float SelfIllumination;
	int SelfIlluminationIsOn;
	float SelfIlluminationColor[3];
};

struct ESOTexture {
	int fileIndex;
};

#pragma pack(pop)

struct granny_data_type_definition;

extern const granny_data_type_definition ESOLikeVertexType[];
extern const granny_data_type_definition ESOLikeVertexAsideType[];
extern const granny_data_type_definition ESOMeshExtendedDataType[];
extern const granny_data_type_definition ESOMaterialType[];
extern const granny_data_type_definition ESOTextureType[];

#endif
