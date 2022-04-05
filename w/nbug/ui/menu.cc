// #include "config.h"
#include <nbug/ui/menu.h>
#include <nbug/ui/themes.h>
#include <nbug/input/keyboard.h>
//#include "ComboBox.h"
//#include "Brush.h"
//#include "Mouse.h"

namespace e
{
	MenuItem::MenuItem()
	{
		isSeparator = false;
		checked  = false;
		enabled  = true;
		subMenu = 0;
	}

	MenuItem::~MenuItem()
	{
	}


	MenuItem::MenuItem(const string & _text, const TexRef & _icon, const Callback & _action, const Callback & _update)
	{
		update = _update;
		action = _action;
		isSeparator = false;
		checked = false;
		enabled = true;
		subMenu = 0;
		icon = _icon;
		text = _text;
	}


	Vector2 MenuItem::GetRequestSize(const Menu * const _menu)
	{
		Themes * themes = _menu->themes;
		if(isSeparator)
		{
			Vector2 ret = {themes->itemMargin * 2 + 2, themes->itemMargin * 2 + 2};
			return ret;
		}
		else
		{
			Vector2 sz = {0, 0};// = themes->GetLabelSize(_dc, label, _drawIcon, _drawText);

			//float _w = 0;
			//float _h = 0;
			if(_menu->drawIcon || _menu->toggle)
			{
				sz.x+= themes->iconSize;
				sz.y = themes->iconSize > sz.y ? themes->iconSize : sz.y;
			}

			if(_menu->drawText)
			{
				float w, h;
				themes->GetTextExt(text, w, h);
				sz.x+= w;
				sz.y = h > sz.y ? h : sz.y;
			}

			if((_menu->drawIcon || _menu->toggle) && _menu->drawText)
			{
				sz.x+= themes->itemMargin;
			}
			//sz.w+= itemMargin * 2;
			//sz.h+= itemMargin * 2;
			sz.x+= themes->itemMargin * 4;
			sz.y+= themes->itemMargin * 4;

			if(subMenu)
			{
				sz.x+= sz.y; // space for arrow
			}
			return sz;
		}
	}

	void MenuItem::Draw(float _x, float _y, float _w, float _h, Menu * _menu, bool _hover)
	{
		bool enabled1 = _menu->enabled && enabled;
	
		Themes * themes = _menu->themes;
		themes->GetGraphics()->TranslateMatrix(_x, _y);
		themes->DrawButton(_w, _h, 0, 0);
		themes->DrawLabel(_w, _y, text);
		themes->GetGraphics()->TranslateMatrix(-_x, -_y);
		//themes->PaintMenuItemBackground(_dc, _x, _y, _w, _h,enabled1, false, _hover && !this->isSeparator); // TODO: focus?
		//if(isSeparator)
		//{
		//	themes->PaintMenuSeparator(_dc, _x, _y, _w, _h);
		//}
		//else
		//{
		//	float x = _x + themes->itemMargin;
		//	float y = _y + themes->itemMargin;
		//	float w = _w - themes->itemMargin * 2;
		//	float h = _h - themes->itemMargin * 2;

		//	if(_menu->drawIcon || _menu->toggle)
		//	{

		//		if(icon.IsValid())
		//		{
		//			if(checked)
		//			{
		//				themes->PaintCheckedIconBackground(_dc, x-2, y-2, themes->iconSize+ 4, themes->iconSize + 4);
		//			}
		//			_dc.PaintIcon(icon, x, y);
		//		}
		//		else if(checked)
		//		{
		//			themes->PaintSimpleIcon(_dc, themes->GetFontByState(enabled1, _hover).color , checked ? _T('Y') : _T('N')
		//				, x, y + (h - themes->smallIconSize) / 2, themes->smallIconSize, themes->smallIconSize);
		//		}
		//		x+= themes->iconSize + themes->itemMargin;
		//		w-= themes->iconSize + themes->itemMargin;
		//	}

		//	if(_menu->drawText)
		//	{
		//		//Color c = enabled1 ? (_hover ? themes->selectionTextColor : themes->normalFont.color) : themes->grayFont.color;
		//		//_dc.PaintControlText(themes->GetFontByState(enabled1, _hover), text, x, y, w, h, -1);
		//		themes->PaintMenuItemText(_dc, text, x, y, w, h, enabled1, false, _hover);
		//	}

		//	if(subMenu && enabled1 && ( _hover || _menu->GetVertical()))
		//	{
		//		Rect rect(_x + _w - _h, _y , _h, _h);
		//		if(themes->smallIconSize < _h)
		//		{
		//			rect.Shrink(_h - themes->smallIconSize, _h - themes->smallIconSize);
		//		}
		//		themes->PaintTriangle(_dc, themes->GetFontByState(enabled1, _hover).color,
		//			rect.x, rect.y, rect.w, rect.h, _menu->GetVertical() ? e::Direction::East : e::Direction::South);
		//	}
		//}
	}


	class Menu_o
	{
	public:
//		TrayIcon * trayIcon;
		Array<MenuItem *> items;
		Array<float> offset;
		float margin;
		int activeIndex;
		int leftDownIndex;
		int rightDownIndex;
		//bool autoPopupSub    : 1;
		bool subVisible      : 1; // always hightlight activeIndex item when sub is visible.
		bool lastIsKeyboard  : 1; // ignore mouse enabled when use focus.
		bool hightlightHover : 1;
		//bool enableCheck     : 1;
		bool showCheckBox    : 1;
		bool multiCheck      : 1;
		bool leftDown : 1;
		bool rightDown : 1;
		RGBA backgroundColor;
		uint delayPopupSubTimer;
		int delayPopupSubInex;
		void SetActive(int _index)
		{
			//for(int i = 0; i < items.size(); i++)
			//{
			//	E_ASSERT(items[i] != 0);
			//	items[i]->hover = false;
			//}
			//if(_index >= 0 && _index <= items.size())
			//{
			//	items[_index]->hover = true;
			//}
			activeIndex = _index;
		}
	};

	void Menu::_Init(bool _popup)
	{
		this->popup = _popup;
		this->listBoxStyle = false;
		this->drawText = true;
		this->drawIcon = _popup;

		this->toggle = _popup;
		this->pushed = false;
		this->enabled = true;
		this->vertical = _popup;
		menu_o = enew Menu_o;
//		menu_o->trayIcon = 0;
		menu_o->activeIndex = -1;
		//menu_o->autoPopupSub = _popup;
		menu_o->subVisible = false;
		menu_o->lastIsKeyboard = false;
		menu_o->hightlightHover = true;
		menu_o->showCheckBox = false;
		menu_o->multiCheck = true;
		menu_o->leftDown = false;
		menu_o->rightDown = false;

		menu_o->backgroundColor = themes->controlColor;
		menu_o->leftDownIndex = -1;
		menu_o->rightDownIndex = -1;
		menu_o->delayPopupSubTimer = 0;
		menu_o->delayPopupSubInex = -1;
		//Themes * themes = Themes::GetCurrent();
		menu_o->margin = themes->controlMargin;
		//SetVertical(_popup);
		//SetBrush(0);
	}

	Menu::Menu(bool _popup)
	{
		_Init(_popup);
	}

	//Menu::Menu(TrayIcon * _owner)
	//	: PopupControl(0, true)
	//{
	//	_Init(true);
	//	menu_o->trayIcon = _owner;
	//}


	Menu::~Menu()
	{
		for(int i=0; i < menu_o->items.size(); ++i)
		{
			delete menu_o->items[i];
		}
		//delete menu_o->delayPopupSubTimer;
		delete menu_o;
	}

	void Menu::SetDrawHover(bool _b)
	{
		menu_o->hightlightHover = _b;
//		Invalidate();
	}

	void Menu::SetCheckBoxVisible(bool _b)
	{
		menu_o->showCheckBox = _b;
//		Invalidate();
	}

	void Menu::SetMultiCheck(bool _b)
	{
		menu_o->multiCheck = _b;
//		Invalidate();
	}

	void Menu::OnMouseStop(float _x, float _y)
	{
		int index = GetItemIndex(_x, _y);
		if(index != -1)
		{
			MenuItem * p = GetItem(index);
			if(p)
			{
				if(!p->comment.empty())
				{
					//ShowToolTip(Translate(p->comment));
				}
				else if(!drawText && !p->text.empty())
				{
					//ShowToolTip(Translate(p->text));
				}
			}
		}
	}


	void Menu::OnRightUp(float _x, float _y)
	{
		bool rightDown = menu_o->rightDown;
		this->pushed = false;
		menu_o->rightDown = false;
		menu_o->lastIsKeyboard = false;
		if(rightDown)
		{
			Menu * p;
			int index = GetItemIndexEx(_x, _y, p);
			if(index >= 0 && index == p->menu_o->rightDownIndex)
			{
				p->CancelDelayPopupSub();
				p->TriggerItemAction(index, true);
//				p->Invalidate();
			}
		}
	}

	void Menu::OnRightDown(float _x, float _y)
	{
		Rect rect = GetRect();
		rect.x = 0;
		rect.y = 0;
		if(rect.Contains(_x, _y))
		{
			SetKeyboardFocus();
			this->pushed = true;
			menu_o->rightDown = true;
			menu_o->rightDownIndex = GetItemIndex(_x, _y);
		}
		else
		{
			Menu * p;
			int index = GetItemIndexEx(_x, _y, p);
			if(p != this && index >=0 && index == p->menu_o->activeIndex)
			{

			}
			//else if(index >= 0 && index == p->menu_o->leftDownIndex)
			//{
			//	E_ASSERT(0);
			//	p->CancelDelayPopupSub();
			//	p->TriggerItemAction(index, true);
			//	p->Invalidate();
			//}
			else if(p)
			{
				p->HideAllDescendant(true);
			}
			else
			{
				HideAll();
//				Mouse::Singleton().EmulateRightDown();
			}
		}
	}

	void Menu::OnLeftDown(float _x, float _y)
	{
		// E_STACK_TRACE;
		Rect rect = GetRect();
		rect.x = 0;
		rect.y = 0;
		if(rect.Contains(_x, _y))
		{
			SetKeyboardFocus();
			this->pushed = true;
			menu_o->leftDown = true;
			menu_o->leftDownIndex = GetItemIndex(_x, _y);
		}
		else
		{
			Menu * p;
			int index = GetItemIndexEx(_x, _y, p);
			if(p != this && index >=0 && index == p->menu_o->activeIndex)
			{

			}
			//else if(index >= 0 && index == p->menu_o->leftDownIndex)
			//{
			//	E_ASSERT(0);
			//	p->CancelDelayPopupSub();
			//	p->TriggerItemAction(index, true);
			//	p->Invalidate();
			//}
			else if(p)
			{
				p->HideAllDescendant(true);
			}
			else
			{
				HideAll();
				PaneToWin(_x, _y);
				EmulateLeftDown(_x, _y);
			}
		}
	}

	void Menu::OnLeftUp(float _x, float _y)
	{
		bool leftDown = menu_o->leftDown;
		this->pushed = false;
		menu_o->leftDown = false;
		menu_o->lastIsKeyboard = false;
		if(leftDown)
		{
			Menu * p;
			int index = GetItemIndexEx(_x, _y, p);
			if(p == this && index >= 0 && index == p->menu_o->leftDownIndex)
			{
				p->CancelDelayPopupSub();
				p->TriggerItemAction(index, true);
			//	p->Invalidate();
			}
		}
	}

	void Menu::ShowPopup(float _x, float _y, bool _comboPopup) 
	{
		Vector2 sz = GetRequestSize();
		SetRect(_x, _y, sz.x, sz.y);
		// EnsureInScreen(_comboPopup); // TODO: implement
		SetVisible(true);
		GrabMouse();
	}

	void Menu::TriggerItemAction(int _index, bool _mouse)
	{
		E_TRACE_LINE(string("[e_ui] Menu::TriggerItemAction(): _index=") + string(_index));
		menu_o->lastIsKeyboard = ! _mouse;
		if(_index < 0 || _index >= (int)menu_o->items.size())
		{
			return;
		}
		MenuItem * item = menu_o->items[_index];
		if(!item->enabled || item->isSeparator)
		{
			return;
		}
		menu_o->activeIndex = _index;
		float w = W();
		float h = H();
		if(item->isSeparator)
		{
			return;
		}
		SetKeyboardFocus();
		if(item->subMenu != 0)
		{
			HideAllDescendant(false);
			menu_o->subVisible = true;
//			menu_o->autoPopupSub = true;
			float x, y;
			if(vertical)
			{
				x = w - 3;
				y = menu_o->offset[_index];
			}
			else
			{
				x = menu_o->offset[_index];
				y = h - 3;
			}

			//item->subMenu->SetPosition(Point(x, y));
			item->subMenu->SetParent(this);
			item->subMenu->ShowPopup(x, y, !vertical);
			item->subMenu->Get_o()->SetActive(-1);
			if(!_mouse)
			{
				item->subMenu->KeyboardNext();
			}
			else
			{
				item->subMenu->Get_o()->lastIsKeyboard = true;
			}
		}
		else if(item)
		{
			HideAll();
			int n = _index;
			item->action(item);
		}
		else if(this->toggle)
		{
			int n = _index;
			if(item->checked)
			{
				checkItem(item);
			}
			else
			{
				uncheckItem(item);
			}
//			Invalidate();
		}

		int n = _index;
		clickItem(item);
	}

	void Menu::SetToggle(bool _b)
	{
		this->toggle = _b;
	}

	void Menu::MouseClickAt(float _x, float _y)
	{
		E_TRACE_LINE(string("[e_ui] Menu::MouseClickAt();"));
		menu_o->lastIsKeyboard = false;

		Menu * p;
		int index = GetItemIndexEx(_x, _y, p);
		if(index >= 0)
		{
			p->TriggerItemAction(index, true);
			//Invalidate();
		}
		else
		{
			HideAll();
		}
	}

	void Menu::OnMouseMove(float _x, float _y)
	{
		//E_TRACE_LINE(string("Menu::OnMouseMove();"));
		Menu * p;
		int index = GetItemIndexEx(_x, _y, p);
		if(p == this || index < 0)
		{
			if(index != menu_o->activeIndex && (index >= 0 || !menu_o->lastIsKeyboard)
				&& !(this->listBoxStyle && index < 0))
			{
				menu_o->lastIsKeyboard = false;
				menu_o->SetActive(index);
				//if(GetKeyboardFocus() && index >= 0) // don't popup when menu bar have not focus
				if(index >= 0 && index < menu_o->items.size())
				{
					DelayPopupSub(index);
				}

//				Invalidate();
			}
		}
		else if(p && index != p->menu_o->activeIndex)
		{
			p->HideAllDescendant(true);
		}
	}

	void Menu::Draw()
	{
		float _w = W();
		float _h = H();
		//Themes * themes = Themes::GetCurrent();

		//Brush brush(menu_o->backgroundColor);
		//_dc.FillRectangle(brush, 0.0f, 0.0f, _w, _h);
		//themes->PaintMenuBackground(_dc, 0.0f, 0.0f, _w, _h/*, Flags()*/);
		PaintBackground();

		float margin = menu_o->margin;
		//bool drawHover = !menu_o->popup && (GetHover() || GetKeyboardFocus()) || menu_o->popup;
		bool drawHover;
		if(popup)
		{
			drawHover = true;
		}
		else
		{
			drawHover = GetMouseHoverPane() == this || menu_o->subVisible || GetKeyboardFocusPane() == this;
		}

		for(int i=0; i < menu_o->items.size(); ++i)
		{
			MenuItem * p = menu_o->items[i];
			if(p->update)
			{
				p->update(p);
			}
		}

		//UIFlags * winFlags = Flags();
		if(vertical)
		{
			float y = margin - ScrollY();
			for(int i=0; i < menu_o->items.size(); ++i)
			{
				MenuItem * p = menu_o->items[i];
				Vector2 sz = p->GetRequestSize(this);
				if(y + sz.y > 0 && y < _h)
				{
					p->Draw(margin, y, _w - margin - margin, sz.y, this, menu_o->activeIndex == i);
				}
				y+= sz.y;
			}
		}
		else
		{
			float x = margin - ScrollX();
			for(int i=0; i < menu_o->items.size(); ++i)
			{
				MenuItem * p = menu_o->items[i];
				Vector2 sz = p->GetRequestSize(this);
				if(x + sz.x > 0 && x < _w)
				{
					p->Draw(x, margin, sz.x, _h - margin - margin, this,  menu_o->activeIndex == i);
				}
				x+= sz.x;
			}
		}
	}

	MenuItem * Menu::GetItem(int _pos) const
	{ return menu_o->items[_pos]; }

	MenuItem * Menu::Remove(int _pos)
	{
//		Invalidate();
		MenuItem * ret = menu_o->items[_pos];
		menu_o->items.remove(_pos);
		return ret;
	}

	int Menu::GetItemIndex(float _x, float _y)
	{
		//E_TRACE_LINE(_T("Menu::GetItem();"));
		float w = W();
		float h = H();
		if(_x < 0 || _x > w || _y < 0 || _y > h)
		{
			return -1;
		}
//		DC & dc = DummyDC();
		float margin = menu_o->margin;
		if(vertical)
		{
			float y = margin - ScrollY();
			for(int i=0; i < menu_o->items.size(); ++i)
			{
				MenuItem * p = menu_o->items[i];
				Vector2 sz1 = p->GetRequestSize(this);

				if(_x >= margin &&  _y >= y && _x < w - 1 && _y < y + sz1.y)
				{
					return i;
				}

				y+= sz1.y;
			}
		}
		else
		{
			float x = margin - ScrollX();
			for(int i=0; i < menu_o->items.size(); ++i)
			{
				MenuItem * p = menu_o->items[i];
				Vector2 sz1 = p->GetRequestSize(this);

				if(_y >= margin &&  _x >= x && _y < h - 1 && _x < x + sz1.x)
				{
					return i;
				}

				x+= sz1.x;
			}
		}

		return -1;
	}

	Rect Menu::GetItemRect(int _index)
	{
		if(_index < 0)
		{
			Rect ret = {0};
			return ret;
		}
		Rect ret;
	//	DC & dc = DummyDC();
		float margin = menu_o->margin;
		if(vertical)
		{
			float y = margin;
			for(int i=0; i < _index; ++i)
			{
				MenuItem * p = menu_o->items[i];
				Vector2 sz1 = p->GetRequestSize(this);
				y+= sz1.y;
			}
			Vector2 sz1 = menu_o->items[_index]->GetRequestSize(this);
			ret.y = y;
			ret.h = sz1.y;
			ret.x = margin;
			ret.w = sz1.x;
		}
		else
		{
			float x = margin;
			for(int i=0; i < _index; ++i)
			{
				MenuItem * p = menu_o->items[i];
				Vector2 sz1 = p->GetRequestSize(this);
				x+= sz1.x;
			}
			Vector2 sz1 = menu_o->items[_index]->GetRequestSize(this);
			ret.y = margin;
			ret.h = sz1.y;
			ret.x = x;
			ret.w = sz1.x;
		}

		return ret;
	}

	void Menu::HideAll()
	{
		// E_STACK_TRACE;
        //E_TRACE_LINE(_T("[e_ui] Menu::HideAll();"));
		if(!popup)
		{
			menu_o->SetActive(-1);
			UpdateState();
			ReleaseMouse();
			//Win * w = GetParentWin();
			//if(w)
			//{
			//	w->SetKeyboardFocus();
			//}
			//Invalidate();
			return;
		}

        Pane * p = this;
		while(p)
		{
			p->SetVisible(false);
			//p->SetGrabMouse(false);
			Pane * p1 = p->GetParent();
			Menu * p2 = dynamic_cast<Menu*>(p1);
			if(p2)
			{
				p2->Get_o()->SetActive(-1);
				p2->UpdateState();
			}
			else
			{
				//if(p1)
				//{
				//	p1->Invalidate();
				//}
			}
			if(p2 && p2->popup)
			{
				p = p1;
			}
			else
			{
				break;
			}
		}
	}

	bool Menu::OnKeyDown(int _sym)
	{
		//E_TRACE_LINE(string("[e_ui] Menu::OnKeyDown(): _sym=") + string(_sym) + string(" ") + DebugObjectInfo(this));
		//if(GetWinType() == WinType::PopupTool && Menu::OnAltCommand(_sym))
		//{
		//	return true;
		//}
		//else
		//{
		//	switch(_sym)
		//	{
		//	case Keyboard::Left:
		//		if(GetVertical())
		//		{
		//			if(ParentIsPopupMenu())
		//			{
		//				KeyboardCancel();
		//			}
		//			else
		//			{
		//				// try move left in parent
		//				Win * w = GetParentWin();
		//				Menu * m = dynamic_cast<Menu*>(w);
		//				if(m)
		//				{
		//					m->HideAllDescendant(true);
		//					m->KeyboardPrev();
		//					m->KeyboardGetPopup();
		//				}
		//			}
		//		}
		//		else
		//		{
		//			KeyboardPrev();
		//		}
		//		return true;
		//	case Keyboard::Right:
		//		if(GetVertical())
		//		{
		//			if(!KeyboardGetPopup() && !ParentIsPopupMenu())
		//			{
		//				// try move right in parent
		//				Win * w = GetParentWin();
		//				Menu * m = dynamic_cast<Menu*>(w);
		//				if(m)
		//				{
		//					m->HideAllDescendant(true);
		//					m->KeyboardNext();
		//					m->KeyboardGetPopup();
		//				}
		//			}
		//		}
		//		else
		//		{
		//			KeyboardNext();
		//		}
		//		return true;
		//	case Keyboard::Up:
		//		if(GetVertical())
		//		{
		//			KeyboardPrev();
		//		}
		//		else
		//		{
		//			if(ParentIsPopupMenu())
		//			{
		//				KeyboardCancel();
		//			}
		//		}
		//		return true;
		//	case Keyboard::Down:
		//		if(GetVertical())
		//		{
		//			KeyboardNext();
		//		}
		//		else
		//		{
		//			KeyboardGetPopup();
		//		}
		//		return true;
		//	case Keyboard::Enter:
		//		TriggerItemAction(menu_o->activeIndex, false);
		//		break;
		//	case Keyboard::Esc:
		//		KeyboardCancel();
		//		break;
		//	}
		//}

		return false;
	}

	void Menu::KeyboardCancel()
	{
		//E_TRACE_LINE(string("[e_ui] Menu::KeyboardCancel()") );
		//menu_o->lastIsKeyboard = true;
		//if(GetWinType() == WinType::PopupTool)
		//{
		//	SetGrabMouse(false);
		//	Win * w = GetParentWin();
		//	if(w)
		//	{
		//		Menu * l = dynamic_cast<Menu*>(w);
		//		if(l)
		//		{
		//			l->Get_o()->lastIsKeyboard = true;
		//			l->SetGrabMouse(true);
		//			l->UpdateState();
		//		}
		//		w->SetKeyboardFocus();
		//	}
		//	SetVisible(false);
		//}
	}

	void Menu::KeyboardNext()
	{
		//menu_o->lastIsKeyboard = true;
		//if(!menu_o->items.empty())
		//{
		//	int oldHover = menu_o->activeIndex;
		//	do
		//	{
		//		menu_o->activeIndex++;
		//		if(menu_o->activeIndex >= (int)menu_o->items.size())
		//		{
		//			menu_o->activeIndex=0;
		//		}
		//		menu_o->SetActive(menu_o->activeIndex);
		//	}while(oldHover != menu_o->activeIndex /* avoid dead loop*/
		//		&& menu_o->items[menu_o->activeIndex]->isSeparator /*skip separator*/);
		//	UpdateScrollPos();
		//	Invalidate();
		//}
	}

	void Menu::KeyboardPrev()
	{
		//menu_o->lastIsKeyboard = true;
		//if(!menu_o->items.empty())
		//{
		//	int oldHover = menu_o->activeIndex;
		//	do
		//	{
		//		menu_o->activeIndex--;
		//		if(menu_o->activeIndex < 0)
		//		{
		//			menu_o->activeIndex = menu_o->items.size() - 1;
		//		}
		//		menu_o->SetActive(menu_o->activeIndex);
		//	}while(oldHover != menu_o->activeIndex  /* avoid dead loop*/
		//		&& menu_o->items[menu_o->activeIndex]->isSeparator /*skip separator*/);
		//	UpdateScrollPos();
		//	Invalidate();
		//}
	}

	bool Menu::KeyboardGetPopup()
	{
		//menu_o->lastIsKeyboard = true;
		//if(menu_o->activeIndex >= 0 && menu_o->activeIndex < (int)menu_o->items.size())
		//{
		//	MenuItem * p = menu_o->items[menu_o->activeIndex];
		//	if(p && p->subMenu)
		//	{
		//		TriggerItemAction(menu_o->activeIndex, false);
		//		return true;
		//	}
		//}
		//return false;
		return false;
	}



	Vector2 Menu::GetRequestSize() const
	{
		//Themes * themes = Themes::GetCurrent();
		//_w = themes->itemMargin * 2;
		//_h = _w;
		float w = 0.0f;
		float h = 0.0f;
		//DC & dc = DummyDC();
		for(int i=0; i < menu_o->items.size(); ++i)
		{
			MenuItem * p = menu_o->items[i];
			Vector2 sz = p->GetRequestSize(this);
			if(vertical)
			{
				w = w < sz.x ? sz.x : w;
				h = h + sz.y;
			}
			else
			{
				h = h < sz.y ? sz.y : h;
				w = w + sz.x;
			}
		}

		Pane * p = parent;
		//ComboBox * pComboBox = dynamic_cast<ComboBox *>(p);
		//if(pComboBox)
		//{
		//	float w1 = pComboBox->W();
		//	//float h1 = pComboBox->H();
		//	if(w < w1)
		//	{
		//		w = w1;
		//	}

		//	if(h < themes->itemHeight)
		//	{
		//		h = themes->itemHeight;
		//	}
		//}
		w+= menu_o->margin * 2;
		h+= menu_o->margin * 2;
	//	w+= this->GetYScroll() ? themes->scrollBarSize : 0;
	//	h+= this->GetXScroll() ? themes->scrollBarSize : 0;
		if(popup && vertical)
		{
			w = E_MAX(120, w);
		}
		//h = E_MAX(request_size.h, h);
		Vector2 ret = {w, h};
		return ret;
	}

	void Menu::Arrange()
	{
		menu_o->offset.resize(menu_o->items.size(), 0);

		//float w, h;
		//GetSize(w, h);
	//	DC & dc = DummyDC();
		float margin = menu_o->margin;
		if(vertical)
		{
			float y = margin;
			for(int i=0; i < menu_o->items.size(); ++i)
			{
				MenuItem * p = menu_o->items[i];
				Vector2 sz = p->GetRequestSize(this);
				menu_o->offset[i] = y;
				//p->SetBoundRect(margin, y, W() - margin * 2, sz.h);
				y+= sz.y;
			}
		}
		else
		{
			float x = margin;
			for(int i=0; i < menu_o->items.size(); ++i)
			{
				MenuItem * p = menu_o->items[i];
				Vector2 sz = p->GetRequestSize(this);
				menu_o->offset[i] = x;
				//p->SetBoundRect(x, margin, sz.w, H() - margin * 2);
				x+= sz.x;
			}
		}
		//Themes * themes = Themes::GetCurrent();
		Vector2 sz = GetRequestSize();
		//sz.y-= this->GetYScroll() ? themes->scrollBarSize : 0;
		//sz.x-= this->GetXScroll() ? themes->scrollBarSize : 0;
		//SetScrollSize(sz.w, sz.h);
		//Invalidate();
	}

	//bool Menu::OnAltCommand(int _sym)
	//{
	//	E_TRACE_LINE(string("[e_ui] Menu::OnAltCommand(): _sym=") + string(_sym));
	//	if(_sym == 0)
	//	{
	//		// ALT just down
	//		if(GetKeyboardFocusPane() == this)
	//		{
	//			HideAll();
	//		}
	//		else
	//		{
	//			SetKeyboardFocus();
	//			menu_o->SetActive(-1);
	//			KeyboardNext();
	//		}
	//		return true;
	//	}
	//	else
	//	{
	//		//for(int i=0; i < menu_o->items.size(); ++i)
	//		//{
	//		//	MenuItem * p = menu_o->items[i];
	//		//	if(p->enabled && Control::CheckAltCommand(p->text, _sym))
	//		//	{
	//		//		SetKeyboardFocus();
	//		//		menu_o->SetActive(i);
	//		//		TriggerItemAction(i, false);
	//		//		menu_o->lastIsKeyboard = true;
	//		//		return true;
	//		//	}
	//		//}
	//	}
	//	return false;
	//}

	bool Menu::ParentIsPopupMenu() const
	{
		Pane * w = parent;
		Menu * m = dynamic_cast<Menu*>(w);
		if(m)
		{
			return m->popup;
		}
		return false;
	}

	void Menu::HideAllDescendant(bool _setFocus)
	{
		E_ASSERT(this);
		//E_TRACE_LINE(_T("[e_ui] Menu::HideAllDescendant();"));
		for(int i=0; i < menu_o->items.size(); ++i)
		{
			MenuItem * p = menu_o->items[i];
			//MenuItem * popup = dynamic_cast<MenuItem*>(p);
			if(p->subMenu)
			{
				p->subMenu->HideAllDescendant(false);
				p->subMenu->SetVisible(false);
			//	p->subMenu->GrabMouse();
			}
		}
		menu_o->subVisible = false;
		if(_setFocus)
		{
			if(popup)
			{
				GrabMouse();
			}
			SetKeyboardFocus();
		}
		//Invalidate();
	}

	void Menu::UpdateState()
	{
		bool subVisible1 = false;;
		for(int i=0; i < menu_o->items.size(); ++i)
		{
			MenuItem * p = menu_o->items[i];

			if(p->subMenu)
			{
				if(p->subMenu->IsVisible())
				{
					subVisible1 = true;
					break;
				}
			}
		}
		if(menu_o->subVisible != subVisible1)
		{
			menu_o->subVisible = subVisible1;
			//Invalidate();
		}
	}

	int Menu::GetItemIndexEx(float _x, float _y, Menu * & _menu_ret)
	{

		Menu * m = this;
		while(m)
		{
			float x = _x;
			float y = _y;
			if(m != this)
			{
				//E_TRACE_LINE(string("from x=") + string(x) + string(", y=") + string(y));
				//this->ClientToScreen(x, y);
				//this->ConvertCoord(x, y, Coord::SelfPixel, Coord::Screen);
				//E_TRACE_LINE(string("screen x=") + string(x) + string(", y=") + string(y));
				//m->ScreenToClient(x, y);
				//m->ConvertCoord(x, y, Coord::Screen, Coord::SelfPixel);
				//E_TRACE_LINE(string("to x=") + string(x) + string(", y=") + string(y));
				this->PaneToWin(x, y);
				m->WinToPane(x, y);
			}
			int i = m->GetItemIndex(x, y);
			if(i>=0)
			{
				_menu_ret = m;
				return i;
			}
			m = dynamic_cast<Menu*>(m->parent);
		}

		_menu_ret = 0;
		return -1;
	}

	int Menu::GetCheckedItem() const
	{
		if(menu_o->multiCheck)
		{
			//E_THROW(_T(""));
			E_ASSERT(0);
		}

		for(int i=0; i < menu_o->items.size(); ++i)
		{
			MenuItem * p = menu_o->items[i];
			if(p->checked)
			{
				return i;
			}
		}

		return -1;
	}

	void Menu::GetCheckedItems(Array<int> & _ret) const
	{
		_ret.clear();
		for(int i=0; i < menu_o->items.size(); ++i)
		{
			MenuItem * p = menu_o->items[i];
			if(p->checked)
			{
				_ret.push_back(i);
			}
		}
	}


	bool Menu::IsMultiCheck() const
	{
		return menu_o->multiCheck;
	}

	void Menu::clear()
	{
		for(int i=0; i < menu_o->items.size(); ++i)
		{
			delete menu_o->items[i];
		}
		menu_o->items.clear();
		menu_o->SetActive(-1);
//		Invalidate();
	}

	void Menu::ClearList()
	{
		clear();
	}

	void Menu::SetPaintText(bool _draw)
	{
		this->drawText = _draw;
		if(!_draw && !this->toggle)
		{
			this->drawIcon = true;
		}
		Arrange();
	}

	void Menu::SetPaintIcon(bool _draw)
	{
		this->drawIcon = _draw;
		if(!_draw && !this->toggle)
		{
			this->drawText = true;
		}
		Arrange();
	}

	void Menu::insert(int _pos, MenuItem * _item)
	{
		menu_o->items.push_back(_item); // TODO: implement 
		Arrange();
	}

	void Menu::InsertSeparator(int _pos)
	{
		MenuItem * p = enew MenuItem();
		p->isSeparator = true;
		menu_o->items.push_back(p); // TODO: implement
		Arrange();
	}

	void Menu::AppendSeparator()
	{
		InsertSeparator(-1);
	}

	int Menu::GetItemCount() const
	{
		return menu_o->items.size();
	}

	//void Menu::SetBackgroundColor(const Color & _color)
	//{
	//	menu_o->backgroundColor = _color;
	//	Invalidate();
	//}

	void Menu::OnLeftDoubleClick(float _x, float _y)
	{
		E_TRACE_LINE(L"[e_ui] Menu::OnLeftDoubleClick(float _x, float _y)");
		int index = GetItemIndex(_x, _y);
		//MouseClickAt(_x, _y);
		if(index >= 0)
		{
			doubleClick(GetItem(index));
		}
	}

	int Menu::GetItemPos(const string & _text) const
	{
		for(int i=0; i < menu_o->items.size(); ++i)
		{
			if(menu_o->items[i]->text == _text)
			{
				return i;
			}
		}
		return -1;
	}

	bool Menu::SetActiveItem(int _index)
	{
		if(_index >= 0 && _index <= menu_o->items.size())
		{
			menu_o->activeIndex = _index;
			UpdateScrollPos();
			//Invalidate();
			return true;
		}
		else
		{
			menu_o->activeIndex = -1;
			UpdateScrollPos();
			//Invalidate();
			return false;
		}
	}

	int Menu::GetActiveItem() const
	{
		return menu_o->activeIndex;
	}


	void Menu::append(MenuItem * _item)
	{
		insert(-1, _item);
	}

	//void Menu::AppendSeparator()
	//{
	//	MenuItem * item = enew MenySeparator();
	//	append(_item);
	//}

	void Menu::SetMargin(float _margin)
	{
		menu_o->margin = _margin;
	}

	void Menu::DelayPopupSub(int _index)
	{
		// TODO: bugfix
		//if(menu_o->delayPopupSubTimer)
		//{
		//	CancelDelayPopupSub();
		//}
		//menu_o->delayPopupSubTimer = enew Timer(Callback(*this, &::OnDelayPopupSubTimer), 400, 0); // TODO: themes it
		//menu_o->delayPopupSubInex = _index;
		//menu_o->delayPopupSubTimer->Start();
	}

	void Menu::CancelDelayPopupSub()
	{
		//delete menu_o->delayPopupSubTimer;
		menu_o->delayPopupSubTimer = 0;
	}
	
	void Menu::Step()
	{
		if(menu_o->delayPopupSubTimer)
		{
			if(--menu_o->delayPopupSubTimer == 0)
			{
				OnDelayPopupSubTimer();
			}
		}
	}

	void Menu::OnDelayPopupSubTimer()
	{
		//CancelDelayPopupSub();
		if(menu_o->delayPopupSubInex>=0 && menu_o->delayPopupSubInex < menu_o->items.size())
		{
			MenuItem * p = menu_o->items[menu_o->delayPopupSubInex];
			if(p->subMenu)
			{
				TriggerItemAction(menu_o->delayPopupSubInex, true);
			}
			menu_o->delayPopupSubInex = -1;
		}
	}

	void Menu::SetListBoxStyle(bool _listBoxStyle)
	{
		listBoxStyle = _listBoxStyle;
		if(listBoxStyle)
		{
			//SetVertical(true);
			//SetYScroll(true);
			vertical = true;
		}
	}

	void Menu::PaintBackground()
	{
		//if(widgetBrush)
		//{
		//	_dc.FillRectangle(*widgetBrush, 0, 0, W(), H());
		//}
		//else
		//{
		//	Themes * themes = Themes::GetCurrent();
		//	if(listBoxStyle)
		//	{
		//		themes->PaintListBoxBackground(_dc, 0.0f, 0.0f, W(), H(), GetEnabled(), GetHover() || GetKeyboardFocus());
		//	}
		//	else
		//	{
		//		themes->PaintMenuBackground(_dc, 0.0f, 0.0f, W(), H(), GetEnabled(), GetHover() || GetKeyboardFocus());
		//	}
		//}
	}

	//void Menu::OnHoverChanged(Widget * _old, Widget * _current)
	//{
	//	if(_old == this)
	//	{
	//		if(!menu_o->subVisible)
	//		{
	//			menu_o->SetActive(-1);
	//		}
	//		CancelDelayPopupSub();
	//	}
	//	Invalidate();
	//}

	//void Menu::OnKeyboardFocusChanged(Widget * _old, Widget * _current)
	//{
	//	PopupControl::OnKeyboardFocusChanged(_old, _current);
	//	if(_old == this)
	//	{
	//		CancelDelayPopupSub();
	//	}
	//	Invalidate();
	//}

	void Menu::UpdateScrollPos()
	{
		// E_STACK_TRACE;
		//if(GetXScroll() || GetYScroll())
		//{
		//	Rect rect = GetItemRect(menu_o->activeIndex);
		//	SetScrollRect(rect.x, rect.y, rect.w, rect.h, true);
		//}
	}

	//void Menu::LoadRecentFiles(const string & _sessionName, const Icon & _icon, const MenuCallback & _action, const MenuCallback & _update)
	//{
	//	Array<Path> paths;
	//	Application::LoadRecentFiles(_sessionName, paths);
	//	for(int i = 0; i < paths.size(); i++)
	//	{
	//		string text =  paths[i].GetString();
	//		MenuItem * item = enew MenuItem(_T("&") +string(i+1) + _T(" ") + text, _icon, _action, _update);
	//		item->comment = text;
	//		append(item);
	//	}
	//}

	//void Menu::OnTabStop()
	//{
	//	if(menu_o->activeIndex == -1)
	//	{
	//		SetActiveItem(0);
	//	}
	//}

	int Menu::GetItemIndex(MenuItem * _item) const
	{
		for(int i=0; i< menu_o->items.size(); i++)
		{
			if(_item == menu_o->items[i])
			{
				return i;
			}
		}
		return -1;
	}
}
