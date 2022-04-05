#ifndef E_UI_MENU_H
#define E_UI_MENU_H

#include "IListControl.h"
#include <nbug/gl/tex.h>
#include <nbug/math/math.h>
#include <nbug/ui/pane.h>
#include <nbug/core/callback.h>

namespace e
{
	class Menu;
	class MenuItem : public Object
	{
	public:
		bool isSeparator : 1;
		bool checked  : 1;
		bool enabled  : 1;
		TexRef icon;
		string text;
		string comment;
		string id;
		Menu * subMenu;
		Callback action;
		Callback update;
		MenuItem();
		~MenuItem();
		//MenuItem(const Label & _label);
		MenuItem(const string & _text, const TexRef & _icon = 0, const Callback & _action = 0, const Callback & _update = 0);
		//MenuItem(const string & _text, const Icon & _icon, Menu * _subMenu);
		Vector2 GetRequestSize(const Menu * const _menu);
		void Draw(float _x, float _y, float _w, float _h, Menu * _menu, bool _hover);
	private:
		MenuItem(const MenuItem & _r);
		const MenuItem & operator=(const MenuItem & _r);
	};

	struct IQueryCommand : virtual public Interface
	{
		virtual void QueryCommand(Array<MenuItem> & _ret) = 0;
	};
	class Menu_o;
//	class TrayIcon;
	class Menu
		: public Pane
		, virtual public IListControl
	{
		void _Init(bool _popup);
	public:
		bool pushed;
		Menu(bool _popup);
		~Menu();
		void SetMargin(float _margin);
		void clear();
		void ClearList();
		void insert(int _pos, MenuItem * _item);
		void append(MenuItem * _item);
		MenuItem * Remove(int _pos);
		void AppendSeparator();
		void InsertSeparator(int _pos);
		bool SetActiveItem(int _index);
		int GetActiveItem() const;
		MenuItem * GetItem(int _pos) const;
		int GetItemPos(const string & _text) const;
		int GetItemCount() const;
		int GetItemIndex(MenuItem * _item) const;
		void SetDrawHover(bool _b);
		void SetToggle(bool _b);
		void SetCheckBoxVisible(bool _b);
		void SetMultiCheck(bool _b);
		bool IsMultiCheck() const;
		int GetCheckedItem() const; // TODO: checked or selected?
		void GetCheckedItems(Array<int> & _ret) const;
		void SetPaintText(bool _draw);
		void SetPaintIcon(bool _draw);
		void Arrange();
		Vector2 GetRequestSize() const;
		//void SetBackgroundColor(const Color & _color);
		Callback doubleClick;
		Callback checkItem;
		Callback uncheckItem;
		Callback clickItem;
		Menu_o * Get_o() const
		{ return menu_o; }
		void SetListBoxStyle(bool _listBoxStyle);
		void KeyboardNext();
		void KeyboardPrev();
		void KeyboardCancel();
		//void LoadRecentFiles(const string & _sessionName, const TexRef & _icon, const MenuCallback & _action, const MenuCallback & _update);
	protected:
		void ShowPopup(float _x, float _y, bool _comboPopup) ;
		//void Create(Win * _parent, bool _popup, bool _vertical);
		void OnMouseMove(float _x, float _y);
		void OnLeftDown(float _x, float _y);
		void OnLeftUp(float _x, float _y);
		void OnRightDown(float _x, float _y);
		void OnRightUp(float _x, float _y);
		void OnMouseStop(float _x, float _y);
		void OnLeftDoubleClick(float _x, float _y);
		void Draw() override;
		bool OnKeyDown(int _sym);
		void TriggerItemAction(int _index, bool _mouse);
		bool KeyboardGetPopup();
		bool ParentIsPopupMenu() const;
		void HideAllDescendant(bool _setFocus);
		void UpdateState();
		virtual void PaintBackground();
		//void OnHoverChanged(Widget * _old, Widget * _current) override;
		//void OnKeyboardFocusChanged(Widget * _old, Widget * _current) override;
		void OnTabStop();
	private:
		void UpdateScrollPos();
		Menu_o * menu_o;
		void MouseClickAt(float _x, float _y);
		void HideAll();
		int GetItemIndex(float _x, float _y);
		int GetItemIndexEx(float _x, float _y, Menu * & _menu_ret);
		Rect GetItemRect(int _index);
		//bool OnAltCommand(int _sym);
		void DelayPopupSub(int _index);
		void CancelDelayPopupSub();
		bool toggle : 1;
		bool drawText : 1;
		bool drawIcon : 1;
		bool listBoxStyle : 1;
		friend class MenuItem;
		void OnDelayPopupSubTimer();
		virtual void Step() override;
	};
}
#endif
