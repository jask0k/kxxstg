#ifndef NB_STATIC_3D_H
#define NB_STATIC_3D_H

#include <nbug/core/obj.h>
#include <nbug/core/file.h>

namespace e
{
	class Graphics;
	struct Static3DImp;
	struct Vector3;
	class Static3D : public RefObject
	{
		Static3DImp * imp;
	public:
		Static3D();
		~Static3D();
		void Load(const Path & _path);
		void Save(const Path & _path);
		void Render(Graphics * _g);
		void GetBoundBox(Vector3 &_min, Vector3 & _max);
	};

	typedef Ref<Static3D> Static3DRef;
}

#endif

