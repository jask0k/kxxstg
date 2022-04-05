#ifndef E_I_LIST_CONTROL_H
#define E_I_LIST_CONTROL_H

#include <nbug/core/obj.h>

namespace e
{
	class MenuItem;
	struct IListControl : virtual public Interface
	{
	public:
		virtual void ClearList() = 0;
		virtual void insert(int _pos, MenuItem * _item) = 0;
		virtual void append(MenuItem * _item) = 0;
		virtual MenuItem * GetItem(int _pos) const = 0;
		virtual int GetItemCount() const = 0;
	};


}

#endif
