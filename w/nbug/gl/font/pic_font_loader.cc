
#include "../private.h"
#include <nbug/tl/map.h>
#include <nbug/gl/font/font_loader.h>
#include <nbug/core/ini.h>
#include <nbug/core/file.h>
#include <nbug/gl/font/font.h>
#include <nbug/gl/image.h>

namespace e
{
	class ImageFontLoaderImp
	{
	public:
		// FSRef fs;
		Path folder;
		e::Map<Char, string> glyphFileMap;
	};

	ImageFontLoader::ImageFontLoader(const Path & _path)
	{
		imp2 = enew ImageFontLoaderImp;
//		imp2->// fs = _fs;
		imp2->folder = _path;
		spaceWidth = 1;
		lineHeight = 1;
		//glyphFileMap
		IniFile iniFile;
		iniFile.Load(_path | L"picfont.ini");
		string v;
		StringArray keys = iniFile.GetAllKey();
		for(uint i=0; i<keys.size(); i++)
		{
			string & k = keys[i];
			if(k.icompare(L"SPACE_WIDTH") == 0)
			{
				iniFile.Get(k, spaceWidth);
				if(spaceWidth < 1)
				{
					spaceWidth = 1;
				}
			}
			else if(k.icompare(L"LINE_HEIGHT") == 0)
			{
				iniFile.Get(k, lineHeight);
				if(lineHeight < 1)
				{
					lineHeight = 1;
				}
			}
			else if(!k.empty())
			{
				if(iniFile.GetString(k, v))
				{
					imp2->glyphFileMap[k[0]] = v;
				}
			}
		}
	}

	ImageFontLoader::~ImageFontLoader()
	{
		delete imp2;
	}

	bool ImageFontLoader::PathIsImageFontFolder(const Path & _path)
	{
		if(!FS::IsFolder(_path))
		{
			return false;
		}

		return FS::IsFile(_path | "picfont.ini");
	}

	bool ImageFontLoader::LoadGlyph(Char _ch, Image & _pic)
	{

		Image image;
		if(_ch == L' ')
		{
			image.Alloc(spaceWidth, lineHeight);
			memset(image.data, 0xff, image.w * image.h * 4);
		}
		else
		{
			e::Map<Char, string>::iterator it = imp2->glyphFileMap.find(_ch);
			if(it == imp2->glyphFileMap.end())
			{
				message(L"[nb] (WW) ImageFontLoader::MakeGlyph('" + string(_ch) + L"') not found.");
				return false;
			}
			string & fileName = it->second;
			if(!image.Load(imp2->folder | (fileName + L".png")))
			{
				message(L"[nb] (WW) ImageFontLoader::MakeGlyph('" + string(_ch) + L"') filed to load picture.");
				return false;
			}
		}

		int w = image.w;
		int h = image.h;
		if(w * h <= 0 || w > Font::MAX_FONT_SIZE || h > Font::MAX_FONT_SIZE )
		{
			message(L"[nb] (WW) ImageFontLoader::MakeGlyph('" + string(_ch) + L"') size unfit.");
			return false;
		}

		_pic.Alloc(w, h);
		_pic.Fill(0x00ffffff);
		for(int y = 0; y < h; y++)
		{
			for(int x = 0; x < w; x ++)
			{
				uint8 * c = image.Get(x, y);
				int i = ((int)c[0] + (int)c[1] + (int)c[2]) * c[3] / 255;
				uint8 a = (uint8)(255 - i / 3);
				uint8 * p = _pic.Get(x, y);
				p[0] = 255;
				p[1] = 255;
				p[2] = 255;
				p[3] = a;
			}
		}

		return true;
	}
}
