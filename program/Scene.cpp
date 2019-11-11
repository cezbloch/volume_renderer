#include "stdafx.h"

#include "Scene.h"

#include "ShaderFunctions.h"

Scene::Scene()
//	: mVolume(vec3i(256,256,256), "../debug/kula256.raw", "../debug/kula256.raw.grad","../debug/vol.raw",vec3i(256,256,256)),
//	: mVolume(vec3i(128,128,128), "../debug/kula.raw", "../debug/kula.raw.grad","../debug/vol.raw",vec3i(128,128,128)),
//	: mVolume(vec3i(128,128,128), "kula128.raw", "kula128.raw.grad","vol.raw",vec3i(128,128,128)),
//	: mVolume(vec3i(256,256,256), "../debug/foot.raw.modi", "../debug/foot.raw.grad","../debug/vol.raw",vec3i(128,128,128)),
//	: mVolume(vec3i(256,256,256), "../debug/skull.raw.modi", "../debug/skull.raw.grad","../debug/vol.raw",vec3i(128,128,128)),
//	: mVolume(vec3i(256,256,256), "../debug/volume.raw.modi", "../debug/volume.raw.grad","../debug/vol.raw",vec3i(128,128,128)),
//	: mVolume(vec3i(256,256,256), "../debug/smooth_volume.raw.modi", "../debug/smooth_volume.raw.grad","../debug/vol.raw",vec3i(128,128,128)),
//	: mVolume(vec3i(256,256,256), "../debug/bonsai.raw.modi", "../debug/bonsai.raw.grad","../debug/vol.raw",vec3i(128,128,128)),
  //: mVolume(vec3i(256,256,256), "../debug/engine2.raw.modi", "../debug/engine2.raw.grad","../debug/vol.raw",vec3i(128,128,128)),
  : mVolume(vec3i(256, 256, 256), "../volumetric_data/engine2.raw.modi", "../volumetric_data/engine2.raw.grad", "../volumetric_data/vol.raw", vec3i(128, 128, 128)),
//	: mVolume(vec3i(256,256,256), "../debug/teapot2.raw.modi", "../debug/teapot2.raw.grad","../debug/vol.raw",vec3i(128,128,128)),
//	: mVolume(vec3i(256,256,256), "../debug/head.modi", "../debug/head.grad","../debug/vol.raw",vec3i(128,128,128)),
//	: mVolume(vec3i(256,256,256), "../debug/new_mri.raw.modi", "../debug/new_mri.raw.grad","../debug/vol.raw",vec3i(128,128,128)),
//	: mVolume(vec3i(64,64,64), "../debug/fuel.raw.modi", "../debug/fuel.raw.grad","../debug/vol.raw",vec3i(128,128,128)),
	  mTexCoordShader(0), mTracerShader(0),mRealShader(0),
	  texture_data(512*512*3+54),
	  tf(0),
	  texture(0),
	  tf_data(256*4+44)
{
//read 2D texture file
	std::ifstream ts("t.bmp", std::ios::in | std::ios::binary);
	if (!ts.is_open())
		throw std::runtime_error("could not open ");
	if( ts.bad() ) std::cerr << "Error reading data" << std::endl;
	std::cout<<"stream opened"<<std::endl;

	ts.read((char*)(&texture_data[0]), 512*512*3+54);
	if (ts.gcount() != (512*512*3+54))
		throw std::runtime_error("unexpected end of file in ");
	std::cout<<"file read correctly"<<std::endl;


	const GLenum target = GL_TEXTURE_2D;

	if (!texture)
		glGenTextures(1, &texture);
	glBindTexture(target, texture);

	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(target, 0, 3,512,512, 0,GL_BGR, GL_UNSIGNED_BYTE, &texture_data[54]);

	if (glGetError())throw std::runtime_error("error creating pattern texture");

//read 1D tranfer function
	std::ifstream tfs("../debug/tf.tga", std::ios::in | std::ios::binary);
	if (!tfs.is_open())
		throw std::runtime_error("could not open tranfer function file");
	if( tfs.bad() ) std::cerr << "Error reading data" << std::endl;
	std::cout<<"stream opened"<<std::endl;

	tfs.read((char*)(&tf_data[0]), 256*4+44);
	if (tfs.gcount() != (256*4+44))
		throw std::runtime_error("unexpected end of file in ");
	std::cout<<"file read correctly"<<std::endl;


	const GLenum tar = GL_TEXTURE_1D;

	if (!tf)
		glGenTextures(1, &tf);
	glBindTexture(tar, tf);

	glTexParameteri(tar, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(tar, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(tar, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	glTexParameteri(tar, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(tar, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	glTexImage1D(tar, 0, 4,256, 0,GL_BGRA, GL_UNSIGNED_BYTE, &tf_data[18]);

	if (glGetError())throw std::runtime_error("error creating tf texture");

//shader stuff
	mTexCoordShader = loadProgram("texCoordVertex.glsl", "texCoordFragment.glsl");
	mTracerShader = loadProgram("tracerVertex.glsl", "tracerFragment.glsl");
	//mRealShader = loadProgram("realVertex.glsl", "realFragment.glsl");
	mFrontLoc = glGetUniformLocation(mTracerShader,"OutCoords");
	mVolLoc = glGetUniformLocation(mTracerShader,"mVolTex");
	mGradLoc = glGetUniformLocation(mTracerShader,"mGradTex");
	mMiniLoc = glGetUniformLocation(mTracerShader,"mMiniTex");
	mThresholdLoc = glGetUniformLocation(mTracerShader,"threshold");
	mDivCoefLoc = glGetUniformLocation(mTracerShader,"div_coef");
	mLightLoc = glGetUniformLocation(mTracerShader,"light");
	mProjLoc = glGetUniformLocation(mTracerShader,"projection");
	mWinSizeLoc = glGetUniformLocation(mTracerShader,"window_size");
	mStepLoc = glGetUniformLocation(mTracerShader,"step");
	mEyeLoc = glGetUniformLocation(mTracerShader,"eye");
	mShineLoc = glGetUniformLocation(mTracerShader,"shininess");
	mTextureLoc = glGetUniformLocation(mTracerShader,"mTex");
	RatioLoc = glGetUniformLocation(mTracerShader,"ratio");
	mTfLoc = glGetUniformLocation(mTracerShader,"TF");

	glActiveTexture(GL_TEXTURE0);
	mFrontTexture = createTexCoordTexture();

	glActiveTexture(GL_TEXTURE0+1);
	glBindTexture(GL_TEXTURE_3D, mVolume.gradientTexture);

	glActiveTexture(GL_TEXTURE0+2);
	glBindTexture(GL_TEXTURE_3D, mVolume.texture);

	glActiveTexture(GL_TEXTURE0+4);
	glBindTexture(GL_TEXTURE_2D, texture);

	glActiveTexture(GL_TEXTURE0+3);
	glBindTexture(GL_TEXTURE_3D, mVolume.miniTexture);

	glActiveTexture(GL_TEXTURE0+5);
	glBindTexture(GL_TEXTURE_1D, tf);
}

void Scene::draw(float threshold,float div_coef,bool framebuffer,
				 GLuint fbo_front,GLuint fbo_back,GLuint front_tex,GLuint back_tex,
				 bool light,int projection,float step,vec3 eye,float shininess,
				 vec3 rotation,int btn,float bubble_scale)
{
	//glColor3f(1,1,1);
	vec3 scale(1.0,1.0,1.0);
	glUseProgram(mTexCoordShader);	
	glEnable(GL_CULL_FACE);
	//glTranslatef(0,0, zoom);
	if(framebuffer==1)
	{
		glCullFace(GL_FRONT);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_front);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawCube(scale);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, front_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else
	{
		glCullFace(GL_FRONT);
		drawCube(scale);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, mFrontTexture);
		glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB,0,0,mViewportSize.x,mViewportSize.y,0);
		glDisable(GL_TEXTURE_2D);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glCullFace(GL_BACK);
		drawCube(scale);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, back_tex);
		glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB,0,0,mViewportSize.x,mViewportSize.y,0);
		glDisable(GL_TEXTURE_2D);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}	

	glDisable(GL_CULL_FACE);
	glUseProgram(mTracerShader);
	glUniform1i(mFrontLoc,0);
	glUniform1i(mGradLoc,1);
	glUniform1i(mVolLoc,2);
	glUniform1i(mMiniLoc,3);
	glUniform1i(mTextureLoc,4);
	glUniform1i(mTfLoc,5);
	glUniform1f(mThresholdLoc,threshold);
	glUniform1f(mStepLoc,step);
	glUniform1f(mDivCoefLoc,div_coef);
	glUniform1i(mLightLoc,light);
	glUniform1i(mProjLoc,projection);
	glUniform2f(mWinSizeLoc,float(mViewportSize.x),float(mViewportSize.y));
	glUniform1f(mShineLoc,shininess);
	glUniform1f(RatioLoc,256.0/bubble_scale);
	glUniform3f(mEyeLoc,eye.x,eye.y,eye.z);
	if(projection==2)
	{
	glBindTexture(GL_TEXTURE_3D, mVolume.gradientTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_3D, mVolume.texture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_3D, mVolume.miniTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);//GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
	}
	else if(projection==0)
	{
	glBindTexture(GL_TEXTURE_3D, mVolume.gradientTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_3D, mVolume.texture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_3D, mVolume.miniTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else if(projection==1)
	{
	glBindTexture(GL_TEXTURE_3D, mVolume.gradientTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_3D, mVolume.texture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_3D, mVolume.miniTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);		
	}


	drawCube(scale);
	glUseProgram(0);

}

void Scene::resize(vec2i viewportSize)
{
	mViewportSize = viewportSize;
}

GLuint Scene::createTexCoordTexture()
{
	const GLenum target = GL_TEXTURE_2D;
	GLuint name = 0;
	glGenTextures(1, &name);
	glBindTexture(target, name);

	// set texture parameters
	// (clamp to edge in order to get non-power of two support on ATI cards)
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_NEAREST
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return name;
}

void Scene::drawCube(vec3 scale)
{
	vec3 size = vec3(mVolume.size);

	glPushMatrix();
	glScalef(size.x / mVolume.size.x, size.y / mVolume.size.x, size.z / mVolume.size.x);
	glTranslatef(-scale.x/2, -scale.y/2, -scale.z/2);

	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);					// Normal Pointing Towards Viewer
		glColor3f( 0.0f, 0.0f, 1.0f); glVertex3f( 0.0f, 0.0f, scale.z);	// Point 1 (Front)
		glColor3f( 1.0f, 0.0f, 1.0f); glVertex3f( scale.x, 0.0f, scale.z);	// Point 2 (Front)
		glColor3f( 1.0f, 1.0f, 1.0f); glVertex3f( scale.x, scale.y, scale.z);	// Point 3 (Front)
		glColor3f( 0.0f, 1.0f, 1.0f); glVertex3f( 0.0f, scale.y, scale.z);	// Point 4 (Front)
		// Back Face
		glNormal3f( 0.0f, 0.0f,-1.0f);					// Normal Pointing Away From Viewer
		glColor3f( 0.0f, 0.0f, 0.0f); glVertex3f( 0.0f, 0.0f, 0.0f);	// Point 1 (Back)
		glColor3f( 0.0f, 1.0f, 0.0f); glVertex3f( 0.0f, scale.y, 0.0f);	// Point 2 (Back)
		glColor3f( 1.0f, 1.0f, 0.0f); glVertex3f( scale.x, scale.y, 0.0f);	// Point 3 (Back)
		glColor3f( 1.0f, 0.0f, 0.0f); glVertex3f( scale.x, 0.0f, 0.0f);	// Point 4 (Back)
		// Top Face
		glNormal3f( 0.0f, 1.0f, 0.0f);					// Normal Pointing Up
		glColor3f( 0.0f, 1.0f, 0.0f); glVertex3f( 0.0f, scale.y, 0.0f);	// Point 1 (Top)
		glColor3f( 0.0f, 1.0f, 1.0f); glVertex3f( 0.0f, scale.y, scale.z);	// Point 2 (Top)
		glColor3f( 1.0f, 1.0f, 1.0f); glVertex3f( scale.x, scale.y, scale.z);	// Point 3 (Top)
		glColor3f( 1.0f, 1.0f, 0.0f); glVertex3f( scale.x, scale.y, 0.0f);	// Point 4 (Top)
		// Bottom Face
		glNormal3f( 0.0f,-1.0f, 0.0f);					// Normal Pointing Down
		glColor3f( 0.0f, 0.0f, 0.0f); glVertex3f( 0.0f, 0.0f, 0.0f);	// Point 1 (Bottom)
		glColor3f( 1.0f, 0.0f, 0.0f); glVertex3f( scale.x, 0.0f, 0.0f);	// Point 2 (Bottom)
		glColor3f( 1.0f, 0.0f, 1.0f); glVertex3f( scale.x, 0.0f, scale.z);	// Point 3 (Bottom)
		glColor3f( 0.0f, 0.0f, 1.0f); glVertex3f( 0.0f, 0.0f, scale.z);	// Point 4 (Bottom)
		// Right face
		glNormal3f( 1.0f, 0.0f, 0.0f);					// Normal Pointing Right
		glColor3f( 1.0f, 0.0f, 0.0f); glVertex3f( scale.x, 0.0f, 0.0f);	// Point 1 (Right)
		glColor3f( 1.0f, 1.0f, 0.0f); glVertex3f( scale.x, scale.y, 0.0f);	// Point 2 (Right)
		glColor3f( 1.0f, 1.0f, 1.0f); glVertex3f( scale.x, scale.y, scale.z);	// Point 3 (Right)
		glColor3f( 1.0f, 0.0f, 1.0f); glVertex3f( scale.x, 0.0f, scale.z);	// Point 4 (Right)
		// Left Face
		glNormal3f(-1.0f, 0.0f, 0.0f);					// Normal Pointing Left
		glColor3f( 0.0f, 0.0f, 0.0f); glVertex3f( 0.0f, 0.0f, 0.0f);	// Point 1 (Left)
		glColor3f( 0.0f, 0.0f, 1.0f); glVertex3f( 0.0f, 0.0f, scale.z);	// Point 2 (Left)
		glColor3f( 0.0f, 1.0f, 1.0f); glVertex3f( 0.0f, scale.y, scale.z);	// Point 3 (Left)
		glColor3f( 0.0f, 1.0f, 0.0f); glVertex3f( 0.0f, scale.y, 0.0f);	// Point 4 (Left)
	glEnd();	

/*	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);					// Normal Pointing Towards Viewer
		glColor3f( 0.0f, 0.0f, 1.0f); glVertex3f( 0.0f, 0.0f, 1.0f);	// Point 1 (Front)
		glColor3f( 1.0f, 0.0f, 1.0f); glVertex3f( 1.0f, 0.0f, 1.0f);	// Point 2 (Front)
		glColor3f( 1.0f, 1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 1.0f);	// Point 3 (Front)
		glColor3f( 0.0f, 1.0f, 1.0f); glVertex3f( 0.0f, 1.0f, 1.0f);	// Point 4 (Front)
		// Back Face
		glNormal3f( 0.0f, 0.0f,-1.0f);					// Normal Pointing Away From Viewer
		glColor3f( 0.0f, 0.0f, 0.0f); glVertex3f( 0.0f, 0.0f, 0.0f);	// Point 1 (Back)
		glColor3f( 0.0f, 1.0f, 0.0f); glVertex3f( 0.0f, 1.0f, 0.0f);	// Point 2 (Back)
		glColor3f( 1.0f, 1.0f, 0.0f); glVertex3f( 1.0f, 1.0f, 0.0f);	// Point 3 (Back)
		glColor3f( 1.0f, 0.0f, 0.0f); glVertex3f( 1.0f, 0.0f, 0.0f);	// Point 4 (Back)
		// Top Face
		glNormal3f( 0.0f, 1.0f, 0.0f);					// Normal Pointing Up
		glColor3f( 0.0f, 1.0f, 0.0f); glVertex3f( 0.0f, 1.0f, 0.0f);	// Point 1 (Top)
		glColor3f( 0.0f, 1.0f, 1.0f); glVertex3f( 0.0f, 1.0f, 1.0f);	// Point 2 (Top)
		glColor3f( 1.0f, 1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 1.0f);	// Point 3 (Top)
		glColor3f( 1.0f, 1.0f, 0.0f); glVertex3f( 1.0f, 1.0f, 0.0f);	// Point 4 (Top)
		// Bottom Face
		glNormal3f( 0.0f,-1.0f, 0.0f);					// Normal Pointing Down
		glColor3f( 0.0f, 0.0f, 0.0f); glVertex3f( 0.0f, 0.0f, 0.0f);	// Point 1 (Bottom)
		glColor3f( 1.0f, 0.0f, 0.0f); glVertex3f( 1.0f, 0.0f, 0.0f);	// Point 2 (Bottom)
		glColor3f( 1.0f, 0.0f, 1.0f); glVertex3f( 1.0f, 0.0f, 1.0f);	// Point 3 (Bottom)
		glColor3f( 0.0f, 0.0f, 1.0f); glVertex3f( 0.0f, 0.0f, 1.0f);	// Point 4 (Bottom)
		// Right face
		glNormal3f( 1.0f, 0.0f, 0.0f);					// Normal Pointing Right
		glColor3f( 1.0f, 0.0f, 0.0f); glVertex3f( 1.0f, 0.0f, 0.0f);	// Point 1 (Right)
		glColor3f( 1.0f, 1.0f, 0.0f); glVertex3f( 1.0f, 1.0f, 0.0f);	// Point 2 (Right)
		glColor3f( 1.0f, 1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 1.0f);	// Point 3 (Right)
		glColor3f( 1.0f, 0.0f, 1.0f); glVertex3f( 1.0f, 0.0f, 1.0f);	// Point 4 (Right)
		// Left Face
		glNormal3f(-1.0f, 0.0f, 0.0f);					// Normal Pointing Left
		glColor3f( 0.0f, 0.0f, 0.0f); glVertex3f( 0.0f, 0.0f, 0.0f);	// Point 1 (Left)
		glColor3f( 0.0f, 0.0f, 1.0f); glVertex3f( 0.0f, 0.0f, 1.0f);	// Point 2 (Left)
		glColor3f( 0.0f, 1.0f, 1.0f); glVertex3f( 0.0f, 1.0f, 1.0f);	// Point 3 (Left)
		glColor3f( 0.0f, 1.0f, 0.0f); glVertex3f( 0.0f, 1.0f, 0.0f);	// Point 4 (Left)
	glEnd();	*/
	glPopMatrix();
}


/*
		more=less=curr_coord;
		less.x=curr_coord.x-unit;
		more.x=curr_coord.x+unit;
		if(less.x<=0.0)grad.x=0.0;
		else if(more.x>=1.0)grad.x=0.0;
		grad.x=texture3D(mVolTex,more).r-texture3D(mVolTex,less).r;

		more=less=curr_coord;
		less.y=curr_coord.y-unit;
		more.y=curr_coord.y+unit;
		if(less.y<=0.0)grad.y=0.0;
		else if(more.y>=1.0)grad.y=0.0;
		else grad.y=texture3D(mVolTex,more).r-texture3D(mVolTex,less).r;

		more=less=curr_coord;
		less.z=curr_coord.z-unit;
		more.z=curr_coord.z+unit;
		if(less.z<=0.0)grad.z=0.0;
		else if(more.z>=1.0)grad.z=0.0;
		else grad.z=texture3D(mVolTex,more).r-texture3D(mVolTex,less).r;
*/