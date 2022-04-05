
#include <z_kxx/stage/test_stage_bg.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/player/player.h>
//#include <z_kxx/enemy/enemy.h>
//#include <z_kxx/stage/kxx_dialog.h>
//#include <z_kxx/boss/a/boss_a.h>
//#include <z_kxx/boss/a/boss_a_mid.h>
#include <z_kxx/util/util_func.h>
#include <nbug/gl/static_3d.h>
//#include <nbug/core/env.h>

namespace e
{

	class Globe : public Object3D
	{
	public:
		const int na;
		const int nb;

		VNT * north_pole;
		VNT * south_pole;
		VNT * middle;

		Material material;

		Globe(float _r, int _na, int _nb)
			: na(_na)
			, nb(_nb)
		{
			matrix.LoadIdentity();
			material.LoadDefault();
			E_ASSERT(_na >=2 && _nb >= 3);
			north_pole = enew VNT[nb + 2];
			south_pole = enew VNT[nb + 2];
			middle = enew VNT[(nb * 2 + 2) * (na - 2)];

			float da = PI / _na;
			float db = 2 * PI / _nb;
			float x, y;
			float a0, a1, r0, r1, z0, z1;
			float tex_dy = 1.0f / (_na);
			float tex_dx = 1.0f / (_nb);
			
			VNT * p = north_pole;
			p->t.x = 0.5f;
			p->t.y = 0.0f;
			p->n.x = 0;
			p->n.y = 0;
			p->n.z = 1;
			p->v.x = 0;
			p->v.y = 0;
			p->v.z = _r;
			p->n.Normalize();
			p++;

			a1 = PI / 2 - da;
			z1 = sin(a1);
			r1 = cos(a1);
			for(int j = 0; j <= _nb; j++)
			{
				float b = (j == _nb) ? 0 : db * j;
				x = r1 * cos(b);
				y = r1 * sin(b);
				p->t.x = tex_dx * j;
				p->t.y = tex_dy;
				p->n.x = x;
				p->n.y = y;
				p->n.z = z1;
				p->v.x = _r*x;
				p->v.y = _r*y;
				p->v.z = _r*z1;
				p->n.Normalize();
				p++;
			}

			// 中部
			p = middle;
			for(int i = 1; i < _na - 1; i++)
			{
				//p = middle + (nb * 2 + 2) * (i - 1);
				a0 = PI / 2 - da * i;
				a1 = a0 - da;
				z0 = sin(a0);
				z1 = sin(a1);
				r0 = cos(a0);
				r1 = cos(a1);
				for(int j = 0; j <= _nb; j++)
				{
					float b = (j == _nb) ? 0 : db * j;
					x = r0 * cos(b);
					y = r0 * sin(b);

					p->t.x = tex_dx * j;
					p->t.y = tex_dy * i;
					p->n.x = x;
					p->n.y = y;
					p->n.z = z0;
					p->v.x = _r*x;
					p->v.y = _r*y;
					p->v.z = _r*z0;
					p->n.Normalize();
					p++;

					x = r1 * cos(b);
					y = r1 * sin(b);
					p->t.x = tex_dx * j;
					p->t.y = tex_dy * (i+1);
					p->n.x = x;
					p->n.y = y;
					p->n.z = z1;
					p->v.x = _r*x;
					p->v.y = _r*y;
					p->v.z = _r*z1;
					p->n.Normalize();
					p++;
				}
			}

			E_ASSERT(p == middle + (nb * 2 + 2) * (na - 2));

			// 南极
			p = south_pole;
			p->t.x = 0.5f;
			p->t.y = 1.0f;
			p->n.x = 0;
			p->n.y = 0;
			p->n.z = -1;
			p->v.x = 0;
			p->v.y = 0;
			p->v.z = -_r;
			p->n.Normalize();
			p++;		
			
			a0 = PI / 2 - da * (_na - 1);
			z0 = sin(a0);
			r0 = cos(a0);
			for(int j = _nb; j >= 0; j--)
			{
				float b = (j == _nb) ? 0 : db * j;
				x = r0 * cos(b);
				y = r0 * sin(b);

				p->t.x = tex_dx * j;
				p->t.y = 1.0f - tex_dy;
				p->n.x = x;
				p->n.y = y;
				p->n.z = z0;
				p->v.x = _r*x;
				p->v.y = _r*y;
				p->v.z = _r*z0;
				p->n.Normalize();
				p++;

			}
		}

		~Globe()
		{
			delete[] north_pole;
			delete[] south_pole;
			delete[] middle;
		}

		bool Step() override
		{
			matrix.RotateZ(0.01f);
			return true;
		}

		void Render() override
		{
			graphics->PushMatrix();
			graphics->MultMatrix(matrix);
			graphics->SetMaterial(material);
			graphics->SetVertexSource(north_pole, sizeof(VNT), VNT::FORMAT, nb+2);
			graphics->DrawPrimitive(E_TRIANGLEFAN, 0, nb+2);
			graphics->SetVertexSource(south_pole, sizeof(VNT), VNT::FORMAT, nb+2);
			graphics->DrawPrimitive(E_TRIANGLEFAN, 0, nb+2);
			if(na > 2)
			{
				graphics->SetVertexSource(middle, sizeof(VNT), VNT::FORMAT, (nb * 2 + 2) * (na - 2));
				for(int n=0; n<na-2; n++)
				{
					graphics->DrawPrimitive(E_TRIANGLESTRIP, (nb * 2 + 2) * n, nb * 2 + 2);
				}
			}
			graphics->PopMatrix();
		}
	};


	class Static3DObject : public Object3D
	{
		Static3D object;
	public:
		Static3DObject()
		{
			matrix.LoadIdentity();
		}
		void ScaleTo(float _size)
		{
			Vector3 min, max;
			object.GetBoundBox(min, max);
			float r = (max-min).length();
			float s = _size / r;
			matrix.Scale(s, s, s);
		}

		void Load(const Path & _path)
		{
			object.Load(_path);
		}

		bool Step() override
		{
			matrix.RotateZ(0.01f);
			return true;
		}

		void Render() override
		{
			graphics->PushMatrix();
			graphics->MultMatrix(matrix);
			object.Render(graphics);
			graphics->PopMatrix();
		}
	};

	TestStageBG::TestStageBG()
	{
		opacity = false;

		//bgTex0 = kxxwin->LoadTex(L"stage-" + string(_stage_id) + "-terrain");

		//Static3DObject * p = enew Static3DObject();
		//p->Load(kxxwin->ResFolder() | L"3d/greek_vase2.nb3");
		//p->matrix.Translate(50, -50, -50);
		//p->ScaleTo(200);
		//object_list.push_back(p);

		// p = enew Static3DObject();
		//p->Load(kxxwin->ResFolder() | L"3d/bed2.nb3");
		//p->matrix.Translate(-100, 0, -100);
		//p->ScaleTo(120);
		//object_list.push_back(p);

		//Globe * earth = enew Globe(100, 64, 64);
		//earth->material.tex_name="earth";
		//earth->material.specular.r = earth->material.specular.g = earth->material.specular.b = 0.1f;
		//earth->material.specular_power = 10;
		//earth->matrix.Translate(0, 150, 0);
		//object_list.push_back(earth);

		//moon = enew Globe(25,32, 32);
		//moon->material.tex_name="moon";
		//moon->matrix.Translate(150, 0, 0);
		//object_list.push_back(moon);

		prevPlayerCenter = kxxwin->player->pos;
		Calc3DMatries();
	}

	TestStageBG::~TestStageBG()
	{
		Object3DList::iterator it = object_list.begin();
		while(it != object_list.end())
		{
			Object3D * p = *it;
			delete p;
			it = object_list.erase(it);
		}
	}

	bool TestStageBG::Step() 
	{
		Object3DList::iterator it = object_list.begin();
		while(it != object_list.end())
		{
			Object3D * p = *it;
			if(p->Step())
			{
				++it;
			}
			else
			{
				delete p;
				it = object_list.erase(it);
			}
		}
		return true;
	}

	void TestStageBG::Calc3DMatries()
	{
		graphics->CalcPerspective(projectionMatrix3D, PI * 0.3f, 1.0f, 1, 1000);
		float a0 = (prevPlayerCenter.x / K_GAME_W) * 2 * PI;
		float dz = (1.0f - prevPlayerCenter.y / K_GAME_H);
		float a1 =  (dz - 0.5f) * PI;
		float r  = 300.0f + 300.0f * dz;

		float r1 = r * cos(a1);
		float x = r1 * cos(a0);
		float y = r1 * sin(a0);
		float z = -r * sin(a1);

		Vector3 eye = {x, y, z};
		Vector3 lookAt = {0, 0, 0};
		Vector3 up = {0, 0, -1};
		graphics->CalcLookAt(modelViewMatrix3D, eye, lookAt, up);
//		graphics->CalcLookAt1(viewMatrix3D, eye, lookAt, up);
	}

	void TestStageBG::Render()
	{
		if(!kxxwin->isPause && kxxwin->GetRenderTimer() % 2 == 0 && prevPlayerCenter != kxxwin->player->pos)
		{
			PositionApproach(prevPlayerCenter, kxxwin->player->pos, 5);
			Calc3DMatries();
		}

		graphics->SetProjectViewMatrix(projectionMatrix3D, modelViewMatrix3D);
		//graphics->SetActiveMatrixStack(MT_PROJECTION);
		//graphics->SetMatrix(projectionMatrix3D);
		//graphics->SetActiveMatrixStack(MT_MODELVIEW);
	//	graphics->SetMatrix(modelViewMatrix3D);
		//graphics->SetTexMode(TM_REPLACE);
		//Matrix4 identity;
		//identity.LoadIdentity();
		//graphics->SetMatrix(identity);
		//graphics->SetViewMatrix(modelViewMatrix3D);

		graphics->ClearBuffer(false, true, false);

		graphics->Enable(GS_DEPTH_TEST);
		if(kxxwin->debugMode)
		{
			graphics->DrawHalfAxisGrid(1000);
		}

		graphics->Enable(GS_LIGHTING);
		graphics->Enable(GS_CULL_FACE);

		//material.LoadDefault();
		//material.tex_name = "earth";
		//graphics->SetMaterial(material);

		RGBA ambient = {0.2f, 0.2f, 0.2f, 1};
		graphics->SetAmbient(ambient);

		Light light = 
		{
			1,		//uint32 type; // 1 = point, 2 = spot, 3 = directional
			{0.95f, 0.9f, 0.85f, 1.0f},	//RGBA diffuse;
			{0.95f, 0.9f, 0.85f, 1.0f},	//RGBA specular;
			{0.95f, 0.9f, 0.85f, 1.0f},	//RGBA ambient;
			{0, -50, 0},	//Vector3 position;
			{2, -0.7f, -1},	//Vector3 direction;
			
			{1, 0, 0}, // attenuation
		};


		graphics->SetLight(0, &light);

		//light.diffuse.r = 0.3f;
		//light.diffuse.g = 0.4f;
		//light.diffuse.b = 1.0f;
		//light.specular = light.diffuse;
		//light.direction.x = 0;
		//light.direction.y = 1;
		//light.direction.z = 0;
		//light.direction = modelViewMatrix3D * light.direction;

		//graphics->SetLight(1, &light);

		//graphics->Disable(GS_DEPTH_TEST);

		//for(int i=0; i<25; i++)
		{
			Object3DList::iterator it = object_list.begin();
			while(it != object_list.end())
			{
				Object3D * p = *it;
				p->Render();
				++it;
			}
		}

		graphics->Disable(GS_DEPTH_TEST);
		graphics->Disable(GS_CULL_FACE);
		graphics->Disable(GS_LIGHTING);

		graphics->SetTexMode(TM_MODULATE);
		//graphics->SetBlendMode(BM_NORMAL);
		graphics->SetColor(1, 1, 1, 1);
//		graphics->SetViewMatrix(identity);

		//graphics->SetFogParam(fog_color.r, fog_color.g, fog_color.b, fog_color.a, 180, 400, 0);
		//graphics->Enable(GS_FOG);

		//graphics->Disable(GS_FOG);
	}
}

