#include <nbug/gl/static_3d.h>
#include <nbug/gl/graphics.h>
#include <nbug/core/debug.h>
#include <nbug/tl/str_array.h>
#include <nbug/tl/map.h>

namespace e
{
#	define NB_3D_FILE_MAGIC_CODE "\x80NB3D\x0D\x0A\x1A\x0A"
#	define NB_3D_FILE_VERSION 0

	struct GROUP
	{
		int material;
		int primitive_count;
		Array<int> first_vertex_array;
		Array<int> vertex_count_array;
	};

	struct Static3DImp
	{
		Vector3 bound_min;
		Vector3 bound_max;
		Array<Material*> materials;
		Array<VNT> vertices;
		Array<GROUP*> groups;

#ifdef NB_DEBUG
		bool normal_segs_ready;
		Array<VC> normal_segs;
#endif

		void clear()
		{
			bound_min.x = bound_min.y = bound_min.z = 0;
			bound_max.x = bound_max.y = bound_max.z = 0;
			for(int i=0; i<materials.size(); i++)
			{
				delete materials[i];
			}
			materials.clear();
			vertices.clear();
			for(int i=0; i<groups.size(); i++)
			{
				delete groups[i];
			}
			groups.clear();
#ifdef NB_DEBUG
			normal_segs_ready = false;
			normal_segs.clear();
#endif
		}

		Static3DImp()
		{
			bound_min.x = bound_min.y = bound_min.z = 0;
			bound_max.x = bound_max.y = bound_max.z = 0;
#ifdef NB_DEBUG
			normal_segs_ready = false;
#endif
		}

		~Static3DImp()
		{
			clear();
		}

		void CalcBoundBox()
		{
			bound_min.x = bound_min.y = bound_min.z = 0;
			bound_max.x = bound_max.y = bound_max.z = 0;
			for(int i=0; i<vertices.size(); i++)
			{
				VNT & v = vertices[i];
				if(v.v.x < bound_min.x) bound_min.x = v.v.x;
				if(v.v.y < bound_min.y) bound_min.y = v.v.y;
				if(v.v.z < bound_min.z) bound_min.z = v.v.z;
				if(v.v.x > bound_max.x) bound_max.x = v.v.x;
				if(v.v.y > bound_max.y) bound_max.y = v.v.y;
				if(v.v.z > bound_max.z) bound_max.z = v.v.z;
			}
		}

#ifdef NB_DEBUG
		void MakeNormalSegs(Graphics * _g)
		{
			float len = (bound_max - bound_min).length() * 0.05f;
			RGBA ca = {1, 0.5f, 0, 0.8f};
			RGBA cb = {0, 0.5f, 1, 0.8f};
			VC a, b;
			a.c = _g->RGBAtoInternalColor(ca);
			b.c = _g->RGBAtoInternalColor(cb);
			for(int i=0; i<vertices.size(); i++)
			{
				VNT & v = vertices[i];
				a.v = v.v;
				normal_segs.push_back(a);
				b.v = a.v + v.n * len;
				//E_TRACE_LINE(L"n.len= " + string(v.n.length()));
				normal_segs.push_back(b);
			}
			normal_segs_ready = true;
		}
#endif


		void NewGroup()
		{
			GROUP * p = enew GROUP();
			p->material = -1;
			p->primitive_count = 0;
			groups.push_back(p);
		}
		GROUP * LastGroup()
		{
			if(groups.empty())
			{
				NewGroup();
			}
			return groups.back();
		}

		int LoadWavefrontMtl(const Path & _path, StringArray &_names)
		{
			FileRef file = FS::OpenFile(_path);
			if(!file)
			{
				return 0;
			}

			int count = 0;
			int line_number = 0;

			stringa line_a;
			StringArray words;
			string material_name;
			Material m;
			m.LoadDefault();
			//memset(&m, 0, sizeof(m));
			while(file->ReadLine(line_a))
			{
				line_number++;
				string line(line_a);
				line.trim();
				if(line.empty() || line[0] == L'#')
				{
					continue;
				}

				words = Split(line, L" ");
				int n = words.size();
				if(n == 0)
				{
					continue;
				}
				string & k = words[0];

				if(k.icompare(L"newmtl") == 0)
				{
					if(n < 2)
					{
						throwf(NB_SRC_LOC "syntax error at line %d", line_number);
					}

					if(!material_name.empty())
					{
						count++;
						_names.push_back(material_name);
						Material * p = enew Material();
						*p = m;
						materials.push_back(p);
						material_name.clear();
						m.LoadDefault();
					}

					material_name = words[1];
				}
				else if(k.icompare(L"Ka") == 0)
				{
					if(n < 4)
					{
						throwf(NB_SRC_LOC "syntax error at line %d", line_number);
					}
					m.ambient.r = words[1].to_float();
					m.ambient.g = words[2].to_float();
					m.ambient.b = words[3].to_float();
					m.ambient.a = 1.0f;
				}
				else if(k.icompare(L"Kd") == 0)
				{
					if(n < 4)
					{
						throwf(NB_SRC_LOC "syntax error at line %d", line_number);
					}
					m.diffuse.r = words[1].to_float();
					m.diffuse.g = words[2].to_float();
					m.diffuse.b = words[3].to_float();
					m.diffuse.a = 1.0f;
				}
				else if(k.icompare(L"Ks") == 0)
				{
					if(n < 4)
					{
						throwf(NB_SRC_LOC "syntax error at line %d", line_number);
					}
					m.specular.r = words[1].to_float();
					m.specular.g = words[2].to_float();
					m.specular.b = words[3].to_float();
					m.specular.a = 1.0f;
				}
				else if(k.icompare(L"illum") == 0)
				{
					if(n < 2)
					{
						throwf(NB_SRC_LOC "syntax error at line %d", line_number);
					}
					m.render_mode = words[1].to_int();
					if(m.render_mode != 1 && m.render_mode != 2)
					{
						message(L"[nb] (WW) LoadWavefrontMtl(): unsupported illum: " + words[1]);
					}
					//0. Color on and Ambient off
					//1. Color on and Ambient on
					//2. Highlight on
					//3. Reflection on and Ray trace on
					//4. Transparency: Glass on, Reflection: Ray trace on
					//5. Reflection: Fresnel on and Ray trace on
					//6. Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
					//7. Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
					//8. Reflection on and Ray trace off
					//9. Transparency: Glass on, Reflection: Ray trace off
					//10. Casts shadows onto invisible surfaces
				}
				else if(k.icompare(L"Ns") == 0)
				{
					if(n < 2)
					{
						throwf(NB_SRC_LOC "syntax error at line %d", line_number);
					}
					m.specular_power = words[1].to_float();
				}
	/*			else if(k.icompare(L"map_Ka") == 0)
				{
				}*/
				else if(k.icompare(L"d") == 0 || k.icompare(L"Tr") == 0)
				{
					if(n < 2)
					{
						throwf(NB_SRC_LOC "syntax error at line %d", line_number);
					}
					m.alpha = words[1].to_float();
					if(k.icompare(L"Tr") == 0)
					{
						m.alpha = 1.0f - m.alpha;
					}
					if(m.alpha<0)
					{
						m.alpha = 0;
					}
					else if(m.alpha > 1.0f)
					{
						m.alpha = 1.0f;
					}
				}
				else if(k.icompare(L"map_Kd") == 0)
				{
					if(n < 2)
					{
						throwf(NB_SRC_LOC "syntax error at line %d", line_number);
					}
					Path tex_path = words[n-1];
					m.tex_name = tex_path.GetBaseName(false);
				}
				//else if(k.icompare(L"map_Ks") == 0)
				//{
				//}
				//else if(k.icompare(L"map_d") == 0)
				//{
				//}
				//else if(k.icompare(L"map_Bump") == 0 || k.icompare(L"bump") == 0)
				//{
				//}
				else
				{
					message(L"[nb] (WW) LoadWavefrontMtl(): unsupported keyword: " + k);
				}

			}
		
			if(!material_name.empty())
			{
				count++;
				_names.push_back(material_name);
				Material * p = enew Material();
				*p = m;
				materials.push_back(p);
			}

			return count;
		}

		void LoadWavefrontObj(const Path & _path)
		{
			clear();

			FileRef file = FS::OpenFile(_path);
			if(!file)
			{
				throw(NB_SRC_LOC "failed open file.");
			}
			uint64 sz = file->GetSize();
			Array<Vector3> tmp_v;
			Array<Vector2> tmp_t;
			Array<Vector3> tmp_n;
			StringArray material_names;

			int line_number = 0;
			stringa line_a;
			StringArray words;
			StringArray words1;
			while(file->ReadLine(line_a))
			{
				line_number++;
#ifdef NB_DEBUG
				if(line_number %50000 == 49999)
				{
					int64 pos = file->Tell();
					E_TRACE_LINE(L"    " + string(int(pos*100/sz)) + L"%");
				}
#endif
				string line(line_a);
				line.trim();
				if(line.empty() || line[0] == L'#')
				{
					continue;
				}
				words = Split(line, L" ");
				int n = words.size();
				if(n == 0)
				{
					continue;
				}
				string & k = words[0];

				if(k.icompare(L"v") == 0)
				{
					if(n < 4)
					{
						throwf(NB_SRC_LOC "syntax error at line %d", line_number);
					}
					Vector3 v;
					v.x = words[1].to_float();
					v.y = words[2].to_float();
					v.z = words[3].to_float();
					tmp_v.push_back(v);
				}
				else if(k.icompare(L"vt") == 0)
				{
					if(n < 3)
					{
						throwf(NB_SRC_LOC "syntax error at line %d", line_number);
					}
					Vector2 v;
					v.x = words[1].to_float();
					v.y = words[2].to_float();
					tmp_t.push_back(v);
				}
				else if(k.icompare(L"vn") == 0)
				{
					if(n < 4)
					{
						throwf(NB_SRC_LOC "syntax error at line %d", line_number);
					}
					Vector3 v;
					v.x = words[1].to_float();
					v.y = words[2].to_float();
					v.z = words[3].to_float();
					tmp_n.push_back(v);
				}
				else if(k.icompare(L"f") == 0)
				{
					int first = vertices.size();
					int count = n - 1;
					if(count > 0)
					{
						for(int i=1; i<n; i++)
						{
							int vi=0, ti=0, ni=0;
							bool bv, bt, bn;
							string & vw = words[i];
							words1 = Split(vw, L"/", 0, -1, false);
							int m = words1.size();
							bv = m > 0 && !words1[0].empty();
							bt = m > 1 && !words1[1].empty();
							bn = m > 2 && !words1[2].empty();
							if(!bv)
							{
								throwf(NB_SRC_LOC "invalid vertex index at line %d, vertex %d. no geometric vertex.", line_number, i-1);
							}
							vi = words1[0].to_int();
							if(bt)
							{
								ti = words1[1].to_int();
							}
							if(bn)
							{
								ni = words1[2].to_int();
							}
							if(vi < 0)
							{
								vi = (int)tmp_v.size() - vi;
							}
							if(ti < 0)
							{
								ti = (int)tmp_t.size() - ti;
							}
							if(ni < 0)
							{
								ni = (int)tmp_n.size() - ni;
							}

							if(vi == 0 || bt && ti == 0 || bn && ni == 0 ||
								vi > tmp_v.size() || bt && ti > tmp_t.size() || bn && ni > tmp_n.size())
							{
								throwf(NB_SRC_LOC "invalid vertex index at line %d, vertex %d", line_number, i-1);
							}

							VNT vnt;
							vnt.v = tmp_v[vi-1];
							if(bt)
							{
								vnt.t = tmp_t[ti-1];
							}
							else
							{
								vnt.t.x = 0;
								vnt.t.y = 0;
							}

							if(bn)
							{
								vnt.n = tmp_n[ni-1];
							}
							else
							{
								vnt.n.x = 1;
								vnt.n.y = 0;
								vnt.n.z = 0;
							}

							vertices.push_back(vnt);
						}
						GROUP * group = LastGroup();
						group->first_vertex_array.push_back(first);
						group->vertex_count_array.push_back(count);
						group->primitive_count++;
					}
				}
				else if(k.icompare(L"g") == 0)
				{
					NewGroup();
				}
				else if(k.icompare(L"mtllib") == 0)
				{
					if(n < 2)
					{
						throwf(NB_SRC_LOC "syntax error at line %d", line_number);
					}
					Path path = _path;
					path.UpOneLevel();
					path = path | words[1];
					LoadWavefrontMtl(path, material_names);
				}
				else if(k.icompare(L"usemtl") == 0)
				{
					if(n < 2)
					{
						throwf(NB_SRC_LOC "syntax error at line %d", line_number);
					}
					int index = -1;
					for(int i=0; i<material_names.size(); i++)
					{
						if(material_names[i].icompare(words[1]) == 0)
						{
							index = i;
							break;
						}
					}

					if(index >= 0)
					{
						GROUP * group = LastGroup();
						group->material = index;
					}
					else
					{
						message(L"[nb] (WW) LoadWavefrontObj(): material not found: " + words[1]);
					}
				}
				else if(k.icompare(L"o") == 0)
				{
					message(L"[nb] LoadWavefrontObj(): ignore keyword: " + k);
				}
				else
				{
					message(L"[nb] (WW) LoadWavefrontObj(): unkown keyword: " + k);
				}
			}
			E_TRACE_LINE(L"    100%");
			CalcBoundBox();
		}

		void LoadNbug3D(const Path & _path)
		{
			clear();
			FileRef file = FS::OpenFile(_path);
			if(!file)
			{
				throw(NB_SRC_LOC "failed open file.");
			}
			char buf[9];
			if(!file->Read(buf, 9) 
				|| memcmp(buf, NB_3D_FILE_MAGIC_CODE, 9) != 0)
			{
				throw(NB_SRC_LOC "unkown format");
			}
			uint32 version;
			if(!file->Read(&version, 4) 
				|| version != 0)
			{
				throw(NB_SRC_LOC "unkown version");
			}

			if(!file->Read(&bound_min, sizeof(bound_min))
				|| !file->Read(&bound_max, sizeof(bound_max)))
			{
				throw(NB_SRC_LOC "corrupted file");
			}

			uint32 count;
			if(!file->Read(&count, 4))
			{
				throw(NB_SRC_LOC "corrupted file");
			}

			stringa name;
			for(int i=0; i<count; i++)
			{
				Material * m = enew Material();
				if(!file->Read(m, sizeof(RAW_MATERIAL)))
				{
					delete m;
					throw(NB_SRC_LOC "corrupted file");
				}
				uint32 len;
				if(!file->Read(&len, 4))
				{
					delete m;
					throw(NB_SRC_LOC "corrupted file");
				}
				if(len)
				{
					name.reserve(len);
					if(!file->Read(name.c_str(), len))
					{
						delete m;
						throw(NB_SRC_LOC "corrupted file");
					}
					name[len] = 0;
					m->tex_name = string(name, CHARSET_UTF8);
				}
				materials.push_back(m);
			}

			if(!file->Read(&count, 4))
			{
				throw(NB_SRC_LOC "corrupted file");
			}

			if(count)
			{
				vertices.resize(count);
				if(!file->Read(&vertices[0], sizeof(VNT) * count))
				{
					throw(NB_SRC_LOC "corrupted file");
				}
			}

			if(!file->Read(&count, 4))
			{
				throw(NB_SRC_LOC "corrupted file");
			}
			for(int i=0; i<count; i++)
			{
				GROUP * group = enew GROUP();
				uint32 tmp, tmp1, tmp2;
				if(!file->Read(&tmp1, 4)
					|| !file->Read(&tmp, 4))
				{
					delete group;
					throw(NB_SRC_LOC "corrupted file");
				}
				group->material = tmp1;
				group->primitive_count = tmp;
				for(int j=0; j<tmp; j++)
				{
					if(!file->Read(&tmp1, 4) 
						||!file->Read(&tmp2, 4) )
					{
						delete group;
						throw(NB_SRC_LOC "corrupted file");
					}
					group->first_vertex_array.push_back(tmp1);
					group->vertex_count_array.push_back(tmp2);
				}
				groups.push_back(group);
			}
		}

		void SaveNbug3D(const Path & _path)
		{
			CalcBoundBox();
			FileRef file = FS::OpenFile(_path, true);
			if(!file)
			{
				throw(NB_SRC_LOC "failed open file.");
			}
			file->SetSize(0);
			file->Write(NB_3D_FILE_MAGIC_CODE, 9);
			uint32 version = NB_3D_FILE_VERSION;
			file->Write(&version, 4);

			file->Write(&bound_min, sizeof(bound_min));
			file->Write(&bound_max, sizeof(bound_max));

			// materials
			uint32 count = materials.size();
			file->Write(&count, 4);
			for(int i=0; i<count; i++)
			{
				Material * m = materials[i];
				file->Write(m, sizeof(RAW_MATERIAL));
				stringa name(m->tex_name, CHARSET_UTF8);
				uint32 len = name.length();
				file->Write(&len, 4);
				if(len)
				{
					file->Write(name.c_str(), len);
				}
			}
			// vertices
			count = vertices.size();
			file->Write(&count, 4);
			if(count)
			{
				file->Write(&vertices[0], sizeof(VNT) * count);
			}
			// groups
			count = groups.size();
			file->Write(&count, 4);
			for(int i=0; i<count; i++)
			{
				GROUP * group = groups[i];
				uint32 tmp = group->material;
				file->Write(&tmp, 4);
				tmp = group->primitive_count;
				file->Write(&tmp, 4);
				for(int j=0; j<tmp; j++)
				{
					uint32 tmp1;
					tmp1 = group->first_vertex_array[j];
					file->Write(&tmp1, 4);
					tmp1 = group->vertex_count_array[j];
					file->Write(&tmp1, 4);
				}
			}
		}
	};

	Static3D::Static3D()
	{
		imp = enew Static3DImp;
	}

	Static3D::~Static3D()
	{
		delete imp;
	}

	void Static3D::Load(const Path & _path)
	{
		if(_path.CheckExtension(L"obj"))
		{
			imp->LoadWavefrontObj(_path);
		}
		else
		{
			imp->LoadNbug3D(_path);
		}
	}

	void Static3D::Save(const Path & _path)
	{
		imp->SaveNbug3D(_path);
	}

	void Static3D::Render(Graphics * _g)
	{
		if(imp->vertices.empty())
		{
			return;
		}

		_g->SetVertexSource(&imp->vertices[0], sizeof(VNT), VNT::FORMAT, imp->vertices.size());

		for(int i=0; i<imp->groups.size(); i++)
		{
			GROUP * group = imp->groups[i];
			if(group->primitive_count)
			{
				if(group->material >= 0 && group->material < imp->materials.size())
				{
					Material * m = imp->materials[group->material];
					//m->specular.r = m->specular.r * 0.01f;
					_g->SetMaterial(*m);
				}
				_g->DrawMultiPrimitive(E_TRIANGLEFAN, &group->first_vertex_array[0], &group->vertex_count_array[0], group->primitive_count);
			}
		}

#ifdef NB_DEBUG
		bool lighting = _g->GetRenderState().lighting;
		_g->Disable(GS_LIGHTING);
		if(_g->GetRenderState().wire_frame_mode)
		{
			if(!imp->normal_segs_ready)
			{
				imp->MakeNormalSegs(_g);
			}

			int n = imp->normal_segs.size();
			if(n > 0)
			{
				_g->SetTexMode(TM_DISABLE);
				_g->SetVertexSource(&imp->normal_segs[0], sizeof(VC), VC::FORMAT, n);
				_g->DrawPrimitive(E_LINELIST, 0, n);
			}
		}
		if(lighting)
		{
			_g->Enable(GS_LIGHTING);
		}
#endif

		_g->SetTexMode(TM_MODULATE);
		_g->BindTex(0);

	}

	void Static3D::GetBoundBox(Vector3 &_min, Vector3 & _max)
	{
		_min = imp->bound_min;
		_max = imp->bound_max;
	}

}

