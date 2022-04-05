
#ifndef NB_CORE_OBJECT_H
#define NB_CORE_OBJECT_H

#include <nbug/tl/array.h>
#include <nbug/core/str.h>
#include <nbug/core/debug.h>

namespace e
{

#ifdef E_CFG_DYNAMIC_CREATE
	class Object;
	struct Rtti
	{
		virtual Object * CreateObject() = 0;
		virtual stringa GetClassName() = 0;
		static Rtti * Find(const stringa & _className);
	protected:
		static void Register(const stringa & _className, Rtti * _class);
	};

	class Object
	{
    public:
		virtual Rtti * GetRtti() const;
		virtual ~Object() = 0;
	};

#	define E_CLASS(_NameSpace, _ClassName) Rtti * GetRtti() const; static Rtti * GetRttiStatic();
#	define E_CLASS_IMPL(_NameSpace, _ClassName) \
	struct Rtti##_ClassName: public e::Rtti \
	{ \
		Rtti##_ClassName() { Rtti::Register(GetClassName(), this); } \
		Object * CreateObject() {return enew _ClassName();}  \
		stringa GetClassName() { return stringa(#_NameSpace) + stringa(#_ClassName); } \
	} __e__Class_##_ClassName; \
	Rtti * _ClassName::GetRtti() const { return &__e__Class_##_ClassName;} \
	Rtti * _ClassName::GetRttiStatic() { return &__e__Class_##_ClassName;}
#else
	class Object
	{
	public:
		virtual ~Object() = 0;
	};
	typedef Object Interface;
#	define E_CLASS(_NameSpace, _ClassName)
#	define E_CLASS_IMPL(_NameSpace, _ClassName)
#endif // E_CFG_DYNAMIC_CREATE

	class RefObject : public Object
	{
	public:
		void Release()
		{ E_ASSERT(this != 0 && ref_count >= 0) ; if(--ref_count <= 0) delete this; }
		void AddRef()
		{ E_ASSERT(this != 0 && ref_count >= 0); ref_count++; }
		int RefCount() const
		{ return ref_count; }
	protected:
		RefObject() : ref_count(0)
		{}
		virtual ~RefObject()
		{};
	private:
		int ref_count;
	};
}


#endif
