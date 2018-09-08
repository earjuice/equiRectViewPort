#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/Rand.h"
#include "cinder/GeomIo.h"
#include "cinder/Log.h"
#include "cinder/Utilities.h"


//#define DRAWGUI
#ifdef DRAWGUI
#include "CinderImGui.h"
#endif // DRAWGUI




#define NUMVIEWS 6
#define GRIDSIZE  1024
#define FBOWIDTH GRIDSIZE*3
#define FBOHEIGHT GRIDSIZE*2
#define EQUIWIDTH  4096
#define EQUIHEIGHT 2048

using namespace ci;
using namespace ci::app;
using namespace std;

struct Satellite {
	vec3    mPos;
	Colorf  mColor;
};

class equiRectViewPortApp : public App {
public:
	equiRectViewPortApp();
	//void setup() override;
	void resize() override;
	void update() override;
	void draw() override;

	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;
	void mouseWheel(MouseEvent event) override;

	void setupCamera();
	//	void setupTeapot();
	void setupSatellites();
	void setupSkyBox();
	void setupEquiPlane();

	void drawSatellites();
	void drawSkyBox();

	void drawViews();
	void drawStuff(vector<mat4> mMatrices);

	void drawCube();
	void drawEqui();
	void drawPlane();

	void updateViewports();

#ifdef DRAWGUI
	void drawGui();
#endif // DRAWGUI

	uint8_t					mIndexNew, mIndexOld;
	gl::FboRef				mFbo[2], equiFbo;
	gl::FboCubeMapRef		mDynamicCubeMapFbo;

	CameraPersp				mCam;
	array<mat4, 6>			camMats;
	vector<float>			viewportsf;
	CameraUi				mCameraUi;
	vector<Rectf>			mViewports;

	gl::TextureCubeMapRef	mSkyBoxCubeMap, mSkyBoxCubeMap2;
	std::vector<Satellite>  mSatellites;
	gl::BatchRef			mSatelliteBatch, mSkyBoxBatch;
	//gl::BatchRef		mTeapot;

	gl::VboMeshRef			equiMesh;
	gl::GlslProgRef			mShader;
	//gl::BatchRef		mPlane;

	bool					mDrawCubeMap, bColored, bBalls, bSky, bEquiRect, bWireFrame;
	float					mFrameRate;


};


equiRectViewPortApp::equiRectViewPortApp() :
	mCameraUi(&mCam)
{
	mFrameRate = 0.0f;
	bSky = true;
	bBalls = true;
	bColored = false;
	bEquiRect = true;
	bWireFrame = false;


	mIndexNew = 0;
	mIndexOld = 1;

	mDynamicCubeMapFbo = gl::FboCubeMap::create(GRIDSIZE, GRIDSIZE);

	updateViewports();

	setupCamera();

	// setupTeapot();
	setupSatellites();
	setupSkyBox();
	setupEquiPlane();

#ifdef DRAWGUI
	ui::initialize();
#endif // DRAWGUI
}


void equiRectViewPortApp::setupCamera()
{
	uint8_t dir = 0;
	for (auto &camat : camMats)
	{
		camat = mDynamicCubeMapFbo->calcViewMatrix(GL_TEXTURE_CUBE_MAP_POSITIVE_X + dir, vec3(0));
		dir++;
	}
	mCam = CameraPersp(FBOWIDTH, FBOHEIGHT, 90, 0.1f, 100.0f);
	mCam.setWorldUp(vec3(0., 1., 0.));
	mCam.lookAt(vec3(0.), vec3(0.0f, 0.0f, 10.0f));
	//	mCameraUi.setCamera(&mCam);
}

/*
void equiRectViewPortApp::setupTeapot()
{
// create the teapot batch with our multicast geometry shader
auto shForm = gl::GlslProg::Format().vertex(loadAsset("shader.vert")).geometry(loadAsset("shader.geom")).fragment(loadAsset("shader.frag"));
auto shader = gl::GlslProg::create(shForm);
auto geom = geom::Teapot().subdivisions(10) >> geom::ColorFromAttrib(geom::Attrib::NORMAL, (const std::function<Colorf(vec3)>&) [](vec3 v) { return Colorf(v.x, v.y, v.z); });
mTeapot = gl::Batch::create(geom, shader);

}*/

void equiRectViewPortApp::setupSatellites()
{
	// setup satellites (orbiting spheres )
	for (int i = 0; i < 33; ++i) {
		mSatellites.push_back(Satellite());
		float angle = i / 33.0f;
		mSatellites.back().mColor = Colorf(CM_HSV, angle, 1.0f, 1.0f);
		mSatellites.back().mPos = vec3(cos(angle * 2 * M_PI) * 7, 0, sin(angle * 2 * M_PI) * 7);
	}
	auto satForm = gl::GlslProg::Format().vertex(loadAsset("satellite.vert")).geometry(loadAsset("satellite.geom")).fragment(loadAsset("satellite.frag"));
	auto satBoxGlsl = gl::GlslProg::create(satForm);

	mSatelliteBatch = gl::Batch::create(geom::Sphere(), satBoxGlsl);
}


void equiRectViewPortApp::setupSkyBox()
{

	mSkyBoxCubeMap = gl::TextureCubeMap::create(loadImage(loadAsset("env_map.jpg")), gl::TextureCubeMap::Format().mipmap());
	mSkyBoxCubeMap2 = gl::TextureCubeMap::create(loadImage(loadAsset("env_map_color.jpg")), gl::TextureCubeMap::Format().mipmap());

	//	auto envMapGlsl = gl::GlslProg::create(loadAsset("env_map.vert"), loadAsset("env_map.frag"));
	auto skyForm = gl::GlslProg::Format().vertex(loadAsset("sky_box.vert")).geometry(loadAsset("sky_box.geom")).fragment(loadAsset("sky_box.frag"));
	auto skyBoxGlsl = gl::GlslProg::create(skyForm);
	//auto skyBoxGlsl = gl::GlslProg::create(loadAsset("sky_box.vert"), loadAsset("sky_box.frag"));

	mSkyBoxBatch = gl::Batch::create(geom::Cube().size(vec3(100.)), skyBoxGlsl);
	mSkyBoxBatch->getGlslProg()->uniform("uCubeMapTex", 0);

}


void equiRectViewPortApp::setupEquiPlane()
{
	auto plane = geom::Plane().size(vec2(FBOWIDTH, FBOHEIGHT));
	//auto plane = geom::Plane().size(vec2(equiFbo->getWidth(), equiFbo->getHeight()));

	vector<gl::VboMesh::Layout> bufferLayout = {
		gl::VboMesh::Layout().usage(GL_STATIC_DRAW).attrib(geom::Attrib::POSITION, 3),
		gl::VboMesh::Layout().usage(GL_STATIC_DRAW).attrib(geom::Attrib::TEX_COORD_0, 2)
	};
	equiMesh = gl::VboMesh::create(plane, bufferLayout);

	auto equiForm = gl::GlslProg::Format().vertex(loadAsset("equi.vert")).fragment(loadAsset("equi.frag"));
	mShader = gl::GlslProg::create(equiForm);
}





void equiRectViewPortApp::update()
{
	float e = (float)getElapsedSeconds();
	mFrameRate = getAverageFps();

	// move the satellites
	if (bBalls) {
		for (int i = 0; i < 33; ++i) {
			float angle = i / 33.0f;
			mSatellites[i].mPos = vec3(cos(angle * 2 * M_PI) * 7, 6 * sin(getElapsedSeconds() * 2 + angle * 4 * M_PI), sin(angle * 2 * M_PI) * 7);
		}
	}

	drawViews();//draw3x2


	drawCube();//draw nto cube
			   //drawEqui();
	drawPlane();//drawEqui for spout


#ifdef DRAWGUI
	drawGui();
#endif // DRAWGUI

	std::swap(mIndexNew, mIndexOld);

}






void equiRectViewPortApp::draw()
{
	// clear the screen and set matrices
	gl::clear(Color(0, 0, 0));
	gl::ScopedMatrices mMainMats;
	gl::setMatricesWindow(getWindowSize());
	gl::color(Color(1., 1., 1.));
	// render our fbo texture
	if (bEquiRect) {
		gl::draw(equiFbo->getColorTexture(), Rectf(0, getWindowHeight(),getWindowWidth(), 0));
		gl::drawStrokedRect(Rectf(0, getWindowHeight(), getWindowWidth(), 0));
		//	gl::drawEquirectangular(mDynamicCubeMapFbo->getTextureCubeMap(), Rectf(0, 0, getWindowWidth(), getWindowWidth()*.5)); // try this alternative
	}
	else {
		gl::draw(mFbo[mIndexNew]->getColorTexture(), Rectf(0, mFbo[mIndexNew]->getHeight()*.5, mFbo[mIndexNew]->getWidth()*.5, 0));
		// and viewport bounds
		for (auto viewport : mViewports) {
			gl::drawStrokedRect(Rectf(viewport.x1 * .5, viewport.y1 * .5, viewport.x2 * .5, viewport.y2 * .5));
		}
	}


}

void equiRectViewPortApp::drawStuff(vector<mat4> mMatrices) {

	if (bSky)
	{
		gl::ScopedModelMatrix mmMat;
		mSkyBoxBatch->getGlslProg()->uniform("uMatrices", &mMatrices[0], camMats.size());
		drawSkyBox();
	}
	if (bBalls)
	{
		mSatelliteBatch->getGlslProg()->uniform("uMatrices", &mMatrices[0], camMats.size());
		drawSatellites();
	}


}


void equiRectViewPortApp::drawViews()
{
	// bind the fbo and enable depth testing
	gl::ScopedFramebuffer scopedFbo(mFbo[mIndexNew]);
	gl::ScopedDepth scopedDepthTest(true);
	gl::ScopedMatrices dMat;
	glViewportArrayv(0, 7, &viewportsf[0]);
	// clear our fbo
	gl::clear(Color(0, 0, 0));
	// create an array with the different cameras matrices
	vector<mat4> matrices;
	uint8_t dir = 0;
	for (auto cammat : camMats) {
		matrices.push_back(mCam.getProjectionMatrix() * camMats[dir] * mCam.getViewMatrix());
		dir++;
	}

	// and send it to the shader
	//mTeapot->getGlslProg()->uniform("uMatrices", &matrices[0], camMats.size());
	// render the teapot once (the geometry shader takes care of the rest)
	//	mTeapot->draw();
	drawStuff(matrices);

}

void equiRectViewPortApp::drawCube()
{
	gl::ScopedMatrices equiMat;
	uint8_t dir = 0;
	gl::context()->pushFramebuffer();
	for (auto viewport : mViewports) {
		mDynamicCubeMapFbo->bindFramebufferFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X + dir);
		gl::clear();
		gl::draw(mFbo[mIndexOld]->getColorTexture(), vec4(-viewport.x1, -viewport.y1, -viewport.x2, -viewport.y2));
		dir++;
	}
	gl::context()->popFramebuffer();
}

void equiRectViewPortApp::drawPlane()
{
	gl::ScopedMatrices plMat;
	gl::ScopedFramebuffer efb(equiFbo);
	gl::ScopedViewport peView(equiFbo->getSize());
	gl::clear(Color(0, 0, 1));
	gl::setMatricesWindow(equiFbo->getSize());
	mDynamicCubeMapFbo->bindTexture(0);
	gl::ScopedGlslProg glslScope(mShader);

	gl::rotate(toRadians(-90.), vec3(1.f, 0.f, 0.f));
	gl::translate(vec3(equiFbo->getWidth() / 2, 0, equiFbo->getHeight() / 2));
	gl::scale(vec3(1.5, 1., 1.));

	mShader->uniform("iResolution", vec2(equiFbo->getWidth(), equiFbo->getHeight()));
	mShader->uniform("uAspectRatio", equiFbo->getAspectRatio());
	mShader->uniform("uTex0", 0);

	gl::draw(equiMesh);

}
void equiRectViewPortApp::drawEqui()
{
	gl::ScopedMatrices mainMat;
	gl::ScopedViewport cubeView(mDynamicCubeMapFbo->getSize());
	gl::ScopedFramebuffer efb(equiFbo);
	gl::clear(Color(1, 0, 0));
	//	gl::setMatricesWindow(getWindowSize());

	gl::setMatricesWindow(equiFbo->getSize());
	gl::drawEquirectangular(mDynamicCubeMapFbo->getTextureCubeMap(), Rectf(0, 0, equiFbo->getWidth(), equiFbo->getHeight())); // try this alternative
}

void equiRectViewPortApp::drawSatellites()
{
	for (const auto &satellite : mSatellites) {

		gl::pushModelMatrix();
		gl::translate(satellite.mPos);
		//	gl::ScopedColor(ColorA(satellite.mColor,1.0));
		gl::color(satellite.mColor);
		mSatelliteBatch->draw();
		gl::popModelMatrix();
	}
	gl::color(ColorA(1., 1., 1., 1.));

}

void equiRectViewPortApp::drawSkyBox()
{
	(bColored ? mSkyBoxCubeMap2 : mSkyBoxCubeMap)->bind();
	gl::pushMatrices();
	gl::scale(GRIDSIZE, GRIDSIZE, GRIDSIZE);
	mSkyBoxBatch->draw();
	gl::popMatrices();
}

void equiRectViewPortApp::updateViewports()
{
	// update the viewport array

	mViewports = {
		Rectf(vec2(0.0f, 0.0f), vec2(GRIDSIZE, GRIDSIZE)),
		Rectf(vec2(GRIDSIZE, 0.0f), vec2(FBOWIDTH - GRIDSIZE, GRIDSIZE)),
		Rectf(vec2(FBOWIDTH - GRIDSIZE, 0.0f), vec2(FBOWIDTH, GRIDSIZE)),
		Rectf(vec2(0.0f,GRIDSIZE), vec2(GRIDSIZE, FBOHEIGHT)),
		Rectf(vec2(GRIDSIZE, GRIDSIZE), vec2(FBOWIDTH - GRIDSIZE, FBOHEIGHT)),
		Rectf(vec2(FBOWIDTH - GRIDSIZE , GRIDSIZE), vec2(FBOWIDTH, FBOHEIGHT)),

	};
	viewportsf.clear();
	// transform it to a float array that opengl can understand
	// the first viewport will be the main one used in the draw function
	viewportsf.push_back(0.0f);
	viewportsf.push_back(0.0f);
	viewportsf.push_back(getWindowWidth());
	viewportsf.push_back(getWindowHeight());
	for (auto viewport : mViewports) {
		viewportsf.push_back(viewport.getX1());
		viewportsf.push_back(FBOHEIGHT - viewport.getY2());
		viewportsf.push_back(viewport.getWidth());
		viewportsf.push_back(viewport.getHeight());
	}

	// upload the viewport array
}


#ifdef DRAWGUI
void equiRectViewPortApp::drawGui()
{
	static bool showDemoWindow = false;
	static bool showWindowWithMenu = true;
	/*
	// create the main menu bar
	{
	ui::ScopedMainMenuBar menuBar;
	// add a file menu
	if (ui::BeginMenu("File")) {
	ui::MenuItem("Open");
	ui::MenuItem("Save");
	ui::MenuItem("Save As");
	if (ui::MenuItem("quit"))quit();
	ui::EndMenu();
	}
	// and a view menu
	if (ui::BeginMenu("View")) {
	ui::MenuItem("TestWindow", nullptr, &showDemoWindow);
	ui::MenuItem("Window with Menu", nullptr, &showWindowWithMenu);
	ui::EndMenu();
	}
	}
	*/
	if (showWindowWithMenu) {
		ui::ScopedWindow window("Window with Menu", ImGuiWindowFlags_MenuBar);
		ui::Text("Frame rate %f", mFrameRate);
		if (ui::Button("draw equirectangular"))bEquiRect = !bEquiRect;
		//	if (ui::Button("wireframe"))	bWireFrame = !bWireFrame;
		ui::Separator();

#ifdef USE_SPOUT
		if (ui::Button("Spout Send"))bSpoutOut = !bSpoutOut;
		ui::Separator();
#endif // USE_SPOUT

		if (ui::Button("quit"))quit();
	}
	if (showDemoWindow)ui::ShowDemoWindow();
}

#endif // DRAWGUI




void equiRectViewPortApp::mouseDown(MouseEvent event)
{
	mCameraUi.mouseDown(event);
}

void equiRectViewPortApp::mouseDrag(MouseEvent event)
{
	mCameraUi.mouseDrag(event);
}
void equiRectViewPortApp::mouseUp(MouseEvent event)
{
	mCameraUi.mouseUp(event);
}
void equiRectViewPortApp::mouseWheel(MouseEvent event)
{
	mCameraUi.mouseWheel(event);
}

void equiRectViewPortApp::resize()
{
	// update each aspect/resolution dependant objects
	equiFbo = gl::Fbo::create(EQUIWIDTH, EQUIHEIGHT, gl::Fbo::Format().samples(8));
	mFbo[mIndexNew] = gl::Fbo::create(FBOWIDTH, FBOHEIGHT, gl::Fbo::Format().samples(8));
	mFbo[mIndexOld] = gl::Fbo::create(FBOWIDTH, FBOHEIGHT, gl::Fbo::Format().samples(8));
	mCameraUi.setWindowSize(vec2(FBOWIDTH, FBOHEIGHT) / vec2(3, 2));
	mCam.setAspectRatio(GRIDSIZE / GRIDSIZE);

	// and recreate/upload each viewports
	updateViewports();
}


CINDER_APP(equiRectViewPortApp, RendererGl/*(RendererGl::Options().version(4, 3).coreProfile())*/, [](App::Settings* settings)
{
	settings->prepareWindow(Window::Format().size(vec2(FBOWIDTH / 2, FBOHEIGHT / 2)).title("Point Cloud App"));
	settings->setFrameRate(60.0f);
})
