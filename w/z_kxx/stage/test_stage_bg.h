
#pragma once

#include <z_kxx/stage/stage.h>

namespace e
{
	class Object3D : public Object
	{
	public:
		Matrix4 matrix;
		virtual bool Step() = 0;
		virtual void Render() = 0;
	};
	typedef List<Object3D*> Object3DList;
	class Globe;
	class TestStageBG : public StageBG
	{
	public:
		Globe * moon;
		Vector2 prevPlayerCenter;
		Object3DList object_list;
		Matrix4 projectionMatrix3D;
		Matrix4 modelViewMatrix3D;
	//	Matrix4 viewMatrix3D;
		TestStageBG();
		~TestStageBG();
		bool Step() override;
		void Render() override;
		void Calc3DMatries();
	};
}
