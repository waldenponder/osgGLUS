#pragma once

osg::Node* readCube()
{
	osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
	//osg::Node*						n	= osgDB::readNodeFile(shader_dir() + "/cube.obj");
	//pat->addChild(n);

	// pat->setAttitude(osg::Quat(osg::PI_2f, osg::X_AXIS));

	//cube ԭ��Y����

	osg::Geode* geode = new osg::Geode;
	pat->addChild(geode);
	osg::Geometry* geometry = new osg::Geometry;
	geode->addDrawable(geometry);
	geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, 36));
	osg::Vec3Array* arr = new osg::Vec3Array;
	geometry->setVertexArray(arr);
	geometry->setUseVertexBufferObjects(true);

	// back face
	arr->push_back(osg::Vec3(-1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(-1.0f, 1.0f, -1.0f));

	//front face
	arr->push_back(osg::Vec3(-1.0f, -1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, 1.0f));

	// left face
	arr->push_back(osg::Vec3(-1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, 1.0f, 1.0f));

	// right face
	arr->push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, 1.0f));

	// bottom face
	arr->push_back(osg::Vec3(-1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, -1.0f));

	//top face
	arr->push_back(osg::Vec3(-1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(-1.0f, 1.0f, 1.0f));

	return pat;
}

class CameraPostdrawCallback : public osg::Camera::DrawCallback
{
public:
	virtual void operator()(osg::RenderInfo& renderInfo) const
	{
		g::draw_once_group->setNodeMask(0);
	}
};

class EventCallback : public osgGA::GUIEventHandler
{
public:
	mutable float rot_ = 0;
	float		  delta = .05;

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/, osg::Object* object, osg::NodeVisitor* nv)
	{
		osg::PositionAttitudeTransform* scene = g::scene;

		if (g::rotX)
		{
			rot_ += delta;
			scene->setAttitude(osg::Quat(rot_, osg::X_AXIS));
		}
		else if (g::rotY)
		{
			rot_ += delta;
			scene->setAttitude(osg::Quat(rot_, osg::Y_AXIS));
		}
		else if (g::rotZ)
		{
			rot_ += delta;
			scene->setAttitude(osg::Quat(rot_, osg::Z_AXIS));
		}

		if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
		{
			if (ea.getKey() == osgGA::GUIEventAdapter::KEY_X)
			{
				g::rotX = g::rotY = g::rotZ = false;
				g::rotX = true;
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Y)
			{
				g::rotX = g::rotY = g::rotZ = false;
				g::rotY = true;
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Z)
			{
				g::rotX = g::rotY = g::rotZ = false;
				g::rotZ = true;
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Q)
			{
				g::scene->setAttitude(osg::Quat(0, osg::Z_AXIS));
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Space)
			{
				g::rotX = g::rotY = g::rotZ = false;
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_O)
			{
				static bool flag = true;
				flag = !flag;
				g::skybox->setNodeMask(flag ? ~0 : 0);
			}
		}
		return false;
	}
};

struct ComputeBoundingSphereCallback : public osg::Node::ComputeBoundingSphereCallback
{
	virtual osg::BoundingSphere computeBound(const osg::Node&) const
	{
		osg::BoundingSphere bs;
		return bs;
	}
};

void cubeTextureAndViewMats(osg::TextureCubeMap* cube_texture, vector<osg::Matrix>& view_mats, int size)
{
	cube_texture->setTextureSize(size, size);
	cube_texture->setInternalFormat(GL_RGB16F_ARB);
	cube_texture->setSourceFormat(GL_RGB);
	cube_texture->setSourceType(GL_FLOAT);
	cube_texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	cube_texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	cube_texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
	cube_texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	cube_texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

	float DISTANCE = 73.f;

	view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(-DISTANCE, 0.0f, 0.0f), osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
	view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(DISTANCE, 0.0f, 0.0f), osg::Vec3(-1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));

	view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, DISTANCE, 0.0f), osg::Vec3(0.0f, 1.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f)));
	view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, -DISTANCE, 0.0f), osg::Vec3(0.0f, -1.0f, 0.0f), osg::Vec3(0.0f, 0.0f, -1.0f)));

	view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, DISTANCE), osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
	view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, -DISTANCE), osg::Vec3(0.0f, 0.0f, -1.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
}

