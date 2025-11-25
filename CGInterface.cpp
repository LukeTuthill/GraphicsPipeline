//#define GEOM_SHADER

#include "CGInterface.h"
#include "scene.h"

#include <iostream>

using namespace std;

CGInterface::CGInterface() {};

void CGInterface::PerSessionInit() {

  glEnable(GL_DEPTH_TEST);

  CGprofile latestVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
#ifdef GEOM_SHADER
  CGprofile latestGeometryProfile = cgGLGetLatestProfile(CG_GL_GEOMETRY);
#endif
  CGprofile latestPixelProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

#ifdef GEOM_SHADER
  if (latestGeometryProfile == CG_PROFILE_UNKNOWN) {
	  cerr << "ERROR: geometry profile is not available" << endl;
    exit(0);
  }

  cgGLSetOptimalOptions(latestGeometryProfile);
  CGerror Error = cgGetError();
  if (Error) {
	  cerr << "CG ERROR: " << cgGetErrorString(Error) << endl;
  }

  cout << "Info: Latest GP Profile Supported: " << cgGetProfileString(latestGeometryProfile) << endl;

  geometryCGprofile = latestGeometryProfile;
#endif

  cout << "Info: Latest VP Profile Supported: " << cgGetProfileString(latestVertexProfile) << endl;
  cout << "Info: Latest FP Profile Supported: " << cgGetProfileString(latestPixelProfile) << endl;

  vertexCGprofile = latestVertexProfile;
  pixelCGprofile = latestPixelProfile;
  cgContext = cgCreateContext();


}

bool ShaderOneInterface::PerSessionInit(CGInterface *cgi) {

	cerr << "INFO: entering ShaderOneInterface::PerSessionInit" << endl;
#ifdef GEOM_SHADER
  geometryProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE, 
    "CG/shaderOne.cg", cgi->geometryCGprofile, "GeometryMain", NULL);
  if (geometryProgram == NULL)  {
    CGerror Error = cgGetError();
    cerr << "Shader One Geometry Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
    cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
    return false;
  }
#endif

  vertexProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE, 
    "CG/shaderOne.cg", cgi->vertexCGprofile, "VertexMain", NULL);
  if (vertexProgram == NULL) {
    CGerror Error = cgGetError();
    cerr << "Shader One Vertex Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
    cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
    return false;
  }

  fragmentProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE, 
    "CG/shaderOne.cg", cgi->pixelCGprofile, "FragmentMain", NULL);
  if (fragmentProgram == NULL)  {
    CGerror Error = cgGetError();
    cerr << "Shader One Fragment Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
    cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
    return false;
  }

	// load programs
#ifdef GEOM_SHADER
	cgGLLoadProgram(geometryProgram);
#endif
	cgGLLoadProgram(vertexProgram);
	cgGLLoadProgram(fragmentProgram);

	// build some parameters by name such that we can set them later...
  vertexModelViewProj = cgGetNamedParameter(vertexProgram, "modelViewProj" );
  vertexOCenter = cgGetNamedParameter(vertexProgram, "oCenter");
  vertexSRadius = cgGetNamedParameter(vertexProgram, "sRadius");
  vertexMFraction = cgGetNamedParameter(vertexProgram, "mFraction");
  fragmentEye = cgGetNamedParameter(fragmentProgram, "eye");
  
  // Get billboard parameters
  fragmentBillboards = cgGetNamedParameter(fragmentProgram, "billboards");
  fragmentBillboardTextures = cgGetNamedParameter(fragmentProgram, "billboardTextures");
  fragmentNumBillboards = cgGetNamedParameter(fragmentProgram, "numBillboards");
  
#ifdef GEOM_SHADER
  geometryModelViewProj = cgGetNamedParameter(geometryProgram, "modelViewProj" );
#endif
  return true;

}

void ShaderOneInterface::PerFrameInit() {

	//set parameters
	cgGLSetStateMatrixParameter(vertexModelViewProj, 
		CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	V3 oCenter = scene->tms[0].get_center();
	cgGLSetParameter3fv(vertexOCenter, (const float*)&oCenter);
	float sRadius = 35.0f;
	cgGLSetParameter1f(vertexSRadius, sRadius);
	cgGLSetParameter1f(vertexMFraction, scene->mFraction);
	cgGLSetParameter3fv(fragmentEye, (const float*)&(scene->ppc->C));
	
	// Set billboard parameters from hardware framebuffer
	if (scene->hw_fb && scene->hw_fb->num_billboards > 0) {
		// Set billboard vertex data (V0, V1, V3 for each billboard)
		cgGLSetParameterArray3f(fragmentBillboards, 0, scene->hw_fb->num_billboards * 3, 
			(const float*)scene->hw_fb->billboard_vertices);
		
		// Set number of billboards
		cgSetParameter1i(fragmentNumBillboards, scene->hw_fb->num_billboards);
		
		// Bind billboard textures to texture units sequentially
		// For texture arrays in Cg, we need to bind each texture to a separate texture unit
		for (int i = 0; i < scene->hw_fb->num_billboards && i < 8; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, scene->hw_fb->billboard_texture_ids[i]);

			// Get the array element parameter
			char param_name[64];
			sprintf_s(param_name, "billboardTextures[%d]", i);
			CGparameter texParam = cgGetNamedParameter(fragmentProgram, param_name);
			
			if (texParam) {
				cgGLSetTextureParameter(texParam, scene->hw_fb->billboard_texture_ids[i]);
				cgGLEnableTextureParameter(texParam);
			}
			else {
				cerr << "WARNING: Could not get parameter for " << param_name << endl;
			}
		}
		glActiveTexture(GL_TEXTURE0);
	}

#ifdef GEOM_SHADER
  cgGLSetStateMatrixParameter(
    geometryModelViewProj, 
	  CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
#endif

}

void ShaderOneInterface::PerFrameDisable() {
	// Disable billboard textures
	if (scene->hw_fb && scene->hw_fb->num_billboards > 0) {
		for (int i = 0; i < scene->hw_fb->num_billboards && i < 8; i++) {
			// Get the array element parameter
			char param_name[64];
			sprintf_s(param_name, "billboardTextures[%d]", i);
			CGparameter texParam = cgGetNamedParameter(fragmentProgram, param_name);
			
			if (texParam) {
				cgGLDisableTextureParameter(texParam);
			}

			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glActiveTexture(GL_TEXTURE0);
	}
}


void ShaderOneInterface::BindPrograms() {

#ifdef GEOM_SHADER
  cgGLBindProgram(geometryProgram);
#endif
  cgGLBindProgram(vertexProgram);
  cgGLBindProgram(fragmentProgram);

}

void CGInterface::DisableProfiles() {

  cgGLDisableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
  cgGLDisableProfile(geometryCGprofile);
#endif
  cgGLDisableProfile(pixelCGprofile);

}

void CGInterface::EnableProfiles() {

  cgGLEnableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
  cgGLEnableProfile(geometryCGprofile);
#endif
  cgGLEnableProfile(pixelCGprofile);

}

