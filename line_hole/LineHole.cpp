#include "pch.h"
#include "LineHole.h"
#include "osg/BlendFunc"
#include <osg/PolygonOffset>
#include "ReadJsonFile.h"

namespace util
{
	auto create_texture = [&]() {
		osg::Texture2D* texture2d = new osg::Texture2D;
		texture2d->setTextureSize(TEXTURE_SIZE1, TEXTURE_SIZE2);
		texture2d->setInternalFormat(GL_RGBA);
		texture2d->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
		texture2d->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
		return texture2d;
	};

	auto create_id_texture = [&](int s1 = TEXTURE_SIZE1, int s2 = TEXTURE_SIZE2) {
		osg::Texture2D* texture2d = new osg::Texture2D;
		texture2d->setTextureSize(s1, s2);
		texture2d->setInternalFormat(GL_R32I);
		texture2d->setSourceFormat(GL_R32I);
		texture2d->setSourceType(GL_INT);
		texture2d->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
		texture2d->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
		return texture2d;
	};

	auto create_line_pt_texture = [&]() {
		osg::Texture2D* texture2d = new osg::Texture2D;
		texture2d->setTextureSize(TEXTURE_SIZE1, TEXTURE_SIZE2);
		texture2d->setInternalFormat(GL_RGBA32F_ARB);
		texture2d->setSourceFormat(GL_RGBA);
		texture2d->setSourceType(GL_FLAT);
		texture2d->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
		texture2d->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
		return texture2d;
	};

	auto create_depth_texture = []()
	{
		// Setup shadow texture
		osg::Texture2D* texture = new osg::Texture2D;
		texture->setTextureSize(TEXTURE_SIZE1, TEXTURE_SIZE2);
		texture->setInternalFormat(GL_DEPTH_COMPONENT);
		texture->setShadowComparison(true);
		texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
		texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

		// the shadow comparison should fail if object is outside the texture
		texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
		texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);
		texture->setBorderColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		return texture;
	};

	osg::Camera* createRttCamera(osgViewer::Viewer* viewer)
	{
		osg::Camera* rttCamera = new osg::Camera;
		rttCamera->setClearColor(CLEAR_COLOR);
		g_root->addChild(rttCamera);
		// set up the background color and clear mask.
		//rttCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// set view
		rttCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		// set viewport
		rttCamera->setViewport(0, 0, TEXTURE_SIZE1, TEXTURE_SIZE2);

		// set the camera to render before the main camera.
		rttCamera->setRenderOrder(osg::Camera::PRE_RENDER);

		// tell the camera to use OpenGL frame buffer object where supported.
		rttCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

		osg::Camera* mainCamera = viewer->getCamera();
		rttCamera->setProjectionMatrix(mainCamera->getProjectionMatrix());
		rttCamera->setViewMatrix(mainCamera->getViewMatrix());
		rttCamera->addPreDrawCallback(new CameraPredrawCallback(rttCamera, mainCamera));

		return rttCamera;
	}
}

using namespace util;

void LineHole::createRttCamera(osgViewer::Viewer* viewer, RenderPass& pass)
{
	osg::Camera* rttCamera = util::createRttCamera(viewer);
	pass.rttCamera = rttCamera;
	rttCamera->setRenderOrder(osg::Camera::PRE_RENDER);
	rttCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	pass.baseColorTexture = create_texture();
	pass.idTexture = create_id_texture();
	pass.depthTexture = create_texture();
	pass.linePtTexture = create_line_pt_texture();

	// attach the texture and use it as the color buffer.
	rttCamera->attach(osg::Camera::COLOR_BUFFER0, pass.baseColorTexture, 0, 0, false);
	rttCamera->attach(osg::Camera::COLOR_BUFFER1, pass.idTexture, 0, 0, false);
	rttCamera->attach(osg::Camera::COLOR_BUFFER2, pass.depthTexture, 0, 0, false);
	rttCamera->attach(osg::Camera::COLOR_BUFFER3, pass.linePtTexture, 0, 0, false);
}

osg::ref_ptr<osg::TextureBuffer> LineHole::create_tbo(const vector<int>& data)
{
	osg::ref_ptr<osg::Image> image = new osg::Image;
	image->allocateImage(data.size(), 1, 1, GL_R32I, GL_INT);

	for (int i = 0; i < data.size(); i++)
	{
		int* ptr = (int*)image->data(i);
		*ptr = data[i];
	}

	osg::ref_ptr<osg::TextureBuffer> tbo = new osg::TextureBuffer;
	tbo->setImage(image.get());
	tbo->setInternalFormat(GL_R32I);
	return tbo;
}

osg::Camera* LineHole::createHudCamera(osgViewer::Viewer* viewer)
{
	osg::Geode* geode_quat = nullptr;
	osg::Geometry* screenQuat = nullptr;
	osg::Camera* hud_camera_ = new osg::Camera;
	hud_camera_->setNodeMask(NM_HUD);

	float w_ = TEXTURE_SIZE1;
	float h_ = TEXTURE_SIZE2;

	hud_camera_->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	hud_camera_->setProjectionMatrixAsOrtho(0, w_, 0, h_, -1, 1);
	hud_camera_->setViewMatrix(osg::Matrix::identity());
	hud_camera_->setRenderOrder(osg::Camera::POST_RENDER);
	hud_camera_->setClearColor(CLEAR_COLOR);
	hud_camera_->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	hud_camera_->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	osg::Vec3 eye, center, up;
	hud_camera_->getViewMatrixAsLookAt(eye, center, up);

	osg::Camera* mainCamera = viewer->getCamera();
	mainCamera->setClearColor(CLEAR_COLOR);
	osg::Viewport* vp = mainCamera->getViewport();

	//默认相机，视角沿z轴往下看，多张图，要先渲染远处的，才能得到正确结果

#if 1 //面在下，先画
	float faceZ = -0.5;
	float lineZ = -0.3;
	float cableZ = -0.2;

	int backgroundQuatPriority = 3;
	int lineQuatPriority = 5;
	int cableQuatPriority = 6;
#else//面在上，后画
	float faceZ = -0.3;
	float lineZ = -0.5;

	int lineQuatPriority = 1;
	int backgroundQuatPriority = 2;
#endif

	//-----------------------------------------------------------line
	osg::ref_ptr<osg::Program> lineProgram = new osg::Program;
	screenQuat = createFinalHudTextureQuad(lineProgram, osg::Vec3(0, 0, lineZ), osg::Vec3(w_, 0, 0), osg::Vec3(0, h_, 0));
	createTextureQuad(g_linePass, hud_camera_, screenQuat, lineQuatPriority, NM_LINE_PASS_QUAD, lineProgram, viewer);

	//-----------------------------------------------------------background
	osg::ref_ptr<osg::Program> backgroundProgram = new osg::Program;
	screenQuat = createFinalHudTextureQuad(backgroundProgram, osg::Vec3(0, 0, faceZ), osg::Vec3(w_, 0, 0), osg::Vec3(0, h_, 0));
	createTextureQuad(g_backgroundPass, hud_camera_, screenQuat, backgroundQuatPriority, NM_BACKGROUND_PASS_QUAD, backgroundProgram, viewer);

	//-----------------------------------------------------------cable
	osg::ref_ptr<osg::Program> cableProgram = new osg::Program;
	screenQuat = createFinalHudTextureQuad(cableProgram, osg::Vec3(0, 0, cableZ), osg::Vec3(w_, 0, 0), osg::Vec3(0, h_, 0));
	createTextureQuad(g_cablePass, hud_camera_, screenQuat, cableQuatPriority, NM_CABLE_PASS_QUAD, cableProgram, viewer);

	return hud_camera_;
}

void LineHole::createTextureQuad(const RenderPass& pass, osg::Camera* hud_camera_, osg::Geometry* screenQuat,
	int priority, int mask, osg::ref_ptr<osg::Program> program, osgViewer::Viewer* viewer)
{
	osg::Geode* geode_quat = new osg::Geode;
	geode_quat->setNodeMask(mask);
	hud_camera_->addChild(geode_quat);
	screenQuat->setUseVertexBufferObjects(true);
	geode_quat->addChild(screenQuat);
	//geode_quat->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	geode_quat->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

	screenQuat->setDataVariance(osg::Object::DYNAMIC);
	screenQuat->setSupportsDisplayList(false);

	osg::StateSet* ss = geode_quat->getOrCreateStateSet();
	ss->setTextureAttributeAndModes(0, pass.baseColorTexture, osg::StateAttribute::ON);
	ss->setTextureAttributeAndModes(1, pass.depthTexture, osg::StateAttribute::ON);
	ss->setTextureAttributeAndModes(2, pass.idTexture, osg::StateAttribute::ON);
	ss->setTextureAttributeAndModes(3, pass.linePtTexture, osg::StateAttribute::ON);

	if (pass.type == RenderPass::LINE_PASS || pass.type == RenderPass::CABLE_PASS)
	{
		ss->setTextureAttributeAndModes(4, g_textureBuffer1, osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(5, g_textureBuffer2, osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(6, g_idTexture1, osg::StateAttribute::ON);		
	}

	ss->setRenderBinDetails(priority, "RenderBin");
	ss->setAttributeAndModes(new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA));

	if (pass.type == RenderPass::LINE_PASS || pass.type == RenderPass::BACKGROUND_PASS)
	{
		osg::Shader* vert = osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_hole/line_hole_quad.vert");
		osg::Shader* frag = osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_hole/line_hole_quad.frag");

		program->addShader(vert);
		program->addShader(frag);
	}
	else
	{
		osg::Shader* vert = osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_hole/cable_pass_quad.vert");
		osg::Shader* frag = osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_hole/cable_pass_quad.frag");

		program->addShader(vert);
		program->addShader(frag);
	}

	ss->addUniform(new osg::Uniform("baseTexture", 0));
	ss->addUniform(new osg::Uniform("depthTexture", 1));
	ss->addUniform(new osg::Uniform("idTexture", 2));
	ss->addUniform(new osg::Uniform("linePtTexture", 3));
	ss->addUniform(new osg::Uniform("textureBuffer1", 4));
	ss->addUniform(new osg::Uniform("textureBuffer2", 5));
	ss->addUniform(new osg::Uniform("idTexture1", 6));

	osg::Uniform* u_MVP(new osg::Uniform(osg::Uniform::FLOAT_MAT4, "u_MVP"));
	u_MVP->setUpdateCallback(new MVPCallback(hud_camera_));
	ss->addUniform(u_MVP);

	if (pass.type == RenderPass::LINE_PASS || pass.type == RenderPass::CABLE_PASS)
	{
		osg::Uniform* range3 = new osg::Uniform(osg::Uniform::BOOL, "u_line_hole_enable");
		range3->setUpdateCallback(new LineHoleCallback(viewer->getCamera()));
		ss->addUniform(range3);

		osg::Uniform* range = new osg::Uniform(osg::Uniform::FLOAT, "u_out_range");
		range->setUpdateCallback(new OutRangeCallback(viewer->getCamera()));
		ss->addUniform(range);

		osg::Uniform* range2 = new osg::Uniform(osg::Uniform::FLOAT, "u_inner_range");
		range2->setUpdateCallback(new InnerRangeCallback(viewer->getCamera()));
		ss->addUniform(range2);

		osg::Uniform* range4 = new osg::Uniform(osg::Uniform::BOOL, "u_always_dont_connected");
		range4->setUpdateCallback(new AlwaysDontConnectedCallback);
		ss->addUniform(range4);

		osg::Uniform* range5 = new osg::Uniform(osg::Uniform::BOOL, "u_always_intersection");
		range5->setUpdateCallback(new AlwaysIntersectionCallback);
		ss->addUniform(range5);
	}
	else if (pass.type == RenderPass::BACKGROUND_PASS)
	{
		//底图pass, 不需要退让
		osg::Uniform* range3 = new osg::Uniform(osg::Uniform::BOOL, "u_line_hole_enable");
		range3->set(false);
		ss->addUniform(range3);
	}

	ss->setAttributeAndModes(program, osg::StateAttribute::ON);
}

osg::Camera* LineHole::createIDPass()
{
	osg::Camera* camera = new osg::Camera;

	float w_ = 512;
	float h_ = 512;

	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setProjectionMatrixAsOrtho(0, w_, 0, h_, -1, 1);
	camera->setViewport(0, 0, w_, h_);
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setRenderOrder(osg::Camera::POST_RENDER);
	camera->setClearColor(CLEAR_COLOR);
	camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	g_idTexture1 = create_id_texture(w_, h_);
	camera->attach(osg::Camera::COLOR_BUFFER0, g_idTexture1, 0, 0, false);

	osg::ref_ptr<osg::Program> program = new osg::Program;

	osg::Geode* geode = new osg::Geode;
	osg::Geometry* screenQuat = createFinalHudTextureQuad(program, osg::Vec3(0, 0, -0.5), osg::Vec3(w_, 0, 0), osg::Vec3(0, h_, 0));
	geode->addChild(screenQuat);
	camera->addChild(geode);

	osg::StateSet* ss = geode->getOrCreateStateSet();
	ss->setRenderBinDetails(1, "RenderBin");
	ss->addUniform(new osg::Uniform("idTexture", 0));
	ss->setTextureAttributeAndModes(0, g_cablePass.idTexture, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

	osg::Uniform* u_MVP(new osg::Uniform(osg::Uniform::FLOAT_MAT4, "u_MVP"));
	u_MVP->setUpdateCallback(new MVPCallback(camera));
	ss->addUniform(u_MVP);

	osg::Shader* vert = osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_hole/id_pass.vert");
	osg::Shader* frag = osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_hole/id_pass.frag");

	program->addShader(vert);
	program->addShader(frag);
	ss->setAttributeAndModes(program, osg::StateAttribute::ON);

	return camera;
}

void LineHole::setUpHiddenLineStateset(osg::StateSet* ss, osg::Camera* camera)
{
	osg::Depth* depth = new osg::Depth(osg::Depth::GREATER);
	ss->setAttributeAndModes(depth, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
	ss->setAttributeAndModes(new osg::LineWidth(2), osg::StateAttribute::ON);
	ss->setAttributeAndModes(new osg::LineStipple(1, 0x0fff), osg::StateAttribute::ON);

	//------------------------osg::Program-----------------------------
	osg::Program* program = new osg::Program;
	program->setName("line_hole");
	program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_hole/line_hole_dot_line.vert"));
	program->addShader(osgDB::readShaderFile(osg::Shader::GEOMETRY, shader_dir() + "/line_hole/line_hole_dot_line.geom"));
	program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_hole/line_hole_dot_line.frag"));

	ss->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);

	//-----------attribute  addBindAttribLocation
	program->addBindAttribLocation("a_pos", 0);
	program->addBindAttribLocation("a_color", 1);
	program->addBindAttribLocation("a_id", 2);

	//-----------------------------------------------uniform
	ss->addUniform(getOrCreateMVPUniform(camera));
}

void LineHole::setUpStateset(osg::StateSet* ss, osg::Camera* camera, bool isLine)
{
	//如果是less equal, 实线最后画会出问题，会导致有些情况下，虚线的颜色留下来了，id却被实线擦除
	osg::Depth* depth = new osg::Depth(osg::Depth::LESS);
	ss->setAttributeAndModes(depth, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
	ss->setAttributeAndModes(new osg::LineWidth(2), osg::StateAttribute::ON);

	//------------------------osg::Program-----------------------------
	osg::Program* program = new osg::Program;
	program->setName("LINESTRIPE");
	program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_hole/line_hole.vert"));
	program->addShader(osgDB::readShaderFile(osg::Shader::GEOMETRY, shader_dir() + "/line_hole/line_hole.geom"));
	program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_hole/line_hole.frag"));

	ss->setAttributeAndModes(program, osg::StateAttribute::ON);

	//-----------attribute  addBindAttribLocation
	program->addBindAttribLocation("a_pos", 0);
	program->addBindAttribLocation("a_color", 1);
	program->addBindAttribLocation("a_id", 2);

	//-----------------------------------------------uniform
	ss->addUniform(getOrCreateMVPUniform(camera));

	if (isLine)
	{
		ss->addUniform(new osg::Uniform("depthTextureSampler", 0));
		ss->setTextureAttributeAndModes(0, g_backgroundPass.depthTexture, osg::StateAttribute::ON);
	}
}

osg::Geometry* LineHole::createFinalHudTextureQuad(osg::ref_ptr<osg::Program> program, const osg::Vec3& corner,
	const osg::Vec3& widthVec, const osg::Vec3& heightVec, float l /*= 0*/, float b /*= 0*/, float r /*= 1*/, float t /*= 1*/)
{
	using namespace osg;
	Geometry* geom = new Geometry;
	geom->setNodeMask(NM_HUD);
	Vec3Array* coords = new Vec3Array(4);
	(*coords)[0] = corner + heightVec;
	(*coords)[1] = corner;
	(*coords)[2] = corner + widthVec;
	(*coords)[3] = corner + widthVec + heightVec;
	geom->setVertexArray(coords);
	geom->setVertexAttribArray(0, coords, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_pos", 0);

	Vec2Array* tcoords = new Vec2Array(4);
	(*tcoords)[0].set(l, t);
	(*tcoords)[1].set(l, b);
	(*tcoords)[2].set(r, b);
	(*tcoords)[3].set(r, t);
	geom->setVertexAttribArray(1, tcoords, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_uv", 1);

	DrawElementsUByte* elems = new DrawElementsUByte(PrimitiveSet::TRIANGLES);
	elems->push_back(0);
	elems->push_back(1);
	elems->push_back(2);

	elems->push_back(2);
	elems->push_back(3);
	elems->push_back(0);
	geom->addPrimitiveSet(elems);

	return geom;
}

osg::Geometry* LineHole::myCreateTexturedQuadGeometry2(osg::Camera* camera, int id,
	const osg::Vec3& corner, const osg::Vec3& widthVec, const osg::Vec3& heightVec,
	float l /*= 0*/, float b /*= 0*/, float r /*= 1*/, float t /*= 1*/)
{
	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->setName("LINE_HOLE_FACE");
	using namespace osg;
	Geometry* geom = new Geometry;

	Vec3Array* coords = new Vec3Array(4);
	(*coords)[0] = corner + heightVec;
	(*coords)[1] = corner;
	(*coords)[2] = corner + widthVec;
	(*coords)[3] = corner + widthVec + heightVec;
	geom->setVertexArray(coords);
	geom->setVertexAttribArray(0, coords, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_pos", 0);

	Vec4Array* colors = new Vec4Array();
	osg::ref_ptr<osg::UIntArray> a_id = new osg::UIntArray;

	for (int i = 0; i < 4; i++)
	{
		a_id->push_back(id);
		colors->push_back(osg::Vec4(0, 1, 0, 1));
	}

	geom->setVertexAttribArray(1, colors, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_color", 1);

	geom->setVertexAttribArray(2, a_id, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(2, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_id", 2);

	DrawElementsUByte* elems = new DrawElementsUByte(PrimitiveSet::TRIANGLES);
	elems->push_back(0);
	elems->push_back(1);
	elems->push_back(2);

	elems->push_back(2);
	elems->push_back(3);
	elems->push_back(0);
	geom->addPrimitiveSet(elems);

	osg::Shader* vert = osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_hole/line_hole_face.vert");
	osg::Shader* frag = osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_hole/line_hole_face.frag");
	program->addShader(vert);
	program->addShader(frag);

	auto ss = geom->getOrCreateStateSet();
	ss->addUniform(getOrCreateMVPUniform(camera));
	ss->setAttributeAndModes(program, osg::StateAttribute::ON);

	osg::BlendFunc* blend = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
	//ss->setAttributeAndModes(blend, osg::StateAttribute::ON);

	osg::ColorMask* colorMask = new osg::ColorMask(false, false, false, false);
	ss->setAttributeAndModes(colorMask, osg::StateAttribute::ON);

	//让面远离视角
	osg::PolygonOffset* offset = new osg::PolygonOffset(1, 1);
	ss->setAttributeAndModes(offset, osg::StateAttribute::ON);

	return geom;
}

osg::Geometry* LineHole::createTriangles(const std::vector<osg::Vec3>& allPTs, const std::vector<osg::Vec4>& color,
	const std::vector<int>& ids, osg::Camera* camera)
{
	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->setName("LINE_HOLE_FACE");
	osg::Geometry* geom = new osg::Geometry;
	geom->setUserValue("ID", ids.front());
	osg::Vec3Array* coords = new osg::Vec3Array;
	for (auto& pt : allPTs)
	{
		coords->asVector().push_back(pt);
	}

	geom->setVertexArray(coords);
	geom->setVertexAttribArray(0, coords, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_pos", 0);

	osg::Vec4Array* colors = new osg::Vec4Array();
	osg::ref_ptr<osg::Vec4Array> a_id = new osg::Vec4Array;

	for (int i = 0; i < allPTs.size(); i++)
	{
		a_id->push_back(osg::Vec4(ids.back(), 0, 0, 0));
		colors->push_back(color.back());
	}

	geom->setVertexAttribArray(1, colors, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_color", 1);

	geom->setVertexAttribArray(2, a_id, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(2, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_id", 2);

	osg::DrawArrays* drawArray = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, allPTs.size() / 3);
	geom->addPrimitiveSet(drawArray);

	osg::Shader* vert = osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_hole/line_hole_face.vert");
	osg::Shader* frag = osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_hole/line_hole_face.frag");
	program->addShader(vert);
	program->addShader(frag);

	auto ss = geom->getOrCreateStateSet();
	ss->addUniform(getOrCreateMVPUniform(camera));
	ss->setAttributeAndModes(program, osg::StateAttribute::ON);
	//ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	osg::BlendFunc* blend = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
	//ss->setAttributeAndModes(blend, osg::StateAttribute::ON);

	osg::ColorMask* colorMask = new osg::ColorMask(false, false, false, false);
	//ss->setAttributeAndModes(colorMask, osg::StateAttribute::ON);

	//让面远离视角
	//osg::PolygonOffset* offset = new osg::PolygonOffset(1, 1);
	//ss->setAttributeAndModes(offset, osg::StateAttribute::ON);

	return geom;
}

osg::Geometry* LineHole::createLine2(const std::vector<osg::Vec3>& allPTs, const std::vector<osg::Vec4>& colors,
	const std::vector<int>& ids, osg::Camera* camera, osg::PrimitiveSet::Mode mode /*= osg::PrimitiveSet::LINE_LOOP*/)
{
	cout << "osg::getGLVersionNumber" << osg::getGLVersionNumber() << endl;

	//传递给shader
	osg::ref_ptr<osg::Vec4Array> a_color = new osg::Vec4Array;
	osg::ref_ptr<osg::Vec4Array> a_id = new osg::Vec4Array;

	int nCount = allPTs.size();

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();
	pGeometry->setUserValue("ID", ids.front());

	osg::ref_ptr<osg::Vec3Array> a_pos = new osg::Vec3Array;

	for (int i = 0; i < allPTs.size(); i++)
	{
		a_pos->push_back(allPTs[i]);
	}

	osg::ref_ptr<osg::ElementBufferObject> ebo = new osg::ElementBufferObject;
	osg::ref_ptr<osg::DrawElementsUInt>	indices = new osg::DrawElementsUInt(mode);

	for (unsigned int i = 0; i < allPTs.size(); i++)
	{
		indices->push_back(i);

		if (i < colors.size())
			a_color->push_back(colors[i]);
		else
			a_color->push_back(colors.back());

		a_id->push_back(osg::Vec4(ids[0], ids[0], ids[0], ids[0]));
	}

	indices->setElementBufferObject(ebo);
	pGeometry->addPrimitiveSet(indices.get());
	pGeometry->setUseVertexBufferObjects(true);

	//-----------attribute  addBindAttribLocation
	pGeometry->setVertexArray(a_pos);
	pGeometry->setVertexAttribArray(0, a_pos, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);

	pGeometry->setVertexAttribArray(1, a_color, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);

	pGeometry->setVertexAttribArray(2, a_id, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(2, osg::Geometry::BIND_PER_VERTEX);

	return pGeometry.release();
}

osg::ref_ptr<osg::Uniform> s_mvp_uniform;
osg::Uniform* LineHole::getOrCreateMVPUniform(osg::Camera* camera)
{
	if (!s_mvp_uniform)
	{
		s_mvp_uniform = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "u_MVP");
		s_mvp_uniform->setUpdateCallback(new MVPCallback(camera));
	}

	return s_mvp_uniform;
}