#ifndef NB_ASYNC_DELETE
#define NB_ASYNC_DELETE

#include <nbug/core/obj.h>

namespace e
{
	struct IAsyncDelete : public Interface
	{
		virtual bool OnAsyncDelete() = 0;
	};

	void AsyncDelete(IAsyncDelete * _p);
	void AsyncDeleteRoutine(bool _block);
}

#endif
