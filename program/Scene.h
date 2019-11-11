#pragma once

#include "Volume.h"
#include "vec2.h"
#include "vec3.h"
#include <vector>
#include <string>

class Scene {
	vec2i mViewportSize;
	Volume mVolume;
	GLuint mFrontTexture,texture,tf;
	std::vector<unsigned char> texture_data,tf_data;

	GLuint mTexCoordShader, mTracerShader,mRealShader;

	GLint mFrontLoc,mBackLoc,mVolLoc,mGradLoc,mThresholdLoc,
		mDivCoefLoc,mLightLoc,mProjLoc,mWinSizeLoc,mStepLoc,
		mEyeLoc,mShineLoc,mMiniLoc,mTextureLoc,RatioLoc,mTfLoc;	//locations for uniform variables for textures

	GLuint createTexCoordTexture();
	void drawCube(vec3 scale);
public:

	Scene();
	~Scene();
	int tmp;
	void draw(float threshold,float div_coef,bool framebuffer,
		GLuint fbo_front,GLuint fbo_back,GLuint front_tex,GLuint back_tex,
		bool light,int projection,float step,vec3 eye,float shininess,
		vec3 rotation,int btn,float bubble_scale);
	void resize(vec2i viewportSize);
};
