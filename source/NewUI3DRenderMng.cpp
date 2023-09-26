// NewUI3DRenderMng.cpp: implementation of the CNewUI3DRenderMng class.
// Rev: 24/09/23 11:25 GMT-3
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUI3DRenderMng.h"
#include "NewUIManager.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUI3DCamera::CNewUI3DCamera() 
{}

SEASON3B::CNewUI3DCamera::~CNewUI3DCamera() 
{ 
	Release(); 
}

bool SEASON3B::CNewUI3DCamera::Create(int iCameraIndex, UINT uiWidth, UINT uiHeight, float fZOrder)
{
	Release();

	m_iCameraIndex = iCameraIndex;
	m_uiWidth = uiWidth;
	m_uiHeight = uiHeight;
	m_fZOrder = fZOrder;

	return true;
}

void SEASON3B::CNewUI3DCamera::Release()
{
	RemoveAll3DRenderObjs();
	m_deque2DEffects.clear();
}

bool SEASON3B::CNewUI3DCamera::IsEmpty()
{ 
	return m_list3DObjs.empty(); 
}

void SEASON3B::CNewUI3DCamera::Add3DRenderObj(INewUI3DRenderObj* pObj)
{
	if(std::find(m_list3DObjs.begin(), m_list3DObjs.end(), pObj) == m_list3DObjs.end())
	{
		m_list3DObjs.push_back(pObj);
	}
}

void SEASON3B::CNewUI3DCamera::Remove3DRenderObj(INewUI3DRenderObj* pObj)
{
	auto vi = std::find(m_list3DObjs.begin(), m_list3DObjs.end(), pObj);
	if (vi != m_list3DObjs.end())
	{
		m_list3DObjs.erase(vi);
	}
}

void SEASON3B::CNewUI3DCamera::RemoveAll3DRenderObjs()
{
	m_list3DObjs.clear();
}

void SEASON3B::CNewUI3DCamera::RenderUI2DEffect(UI_2DEFFECT_CALLBACK pCallbackFunc, LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
	m_deque2DEffects.emplace_back(UI_2DEFFECT_INFO{ pCallbackFunc, pClass, dwParamA, dwParamB });
}

void SEASON3B::CNewUI3DCamera::DeleteUI2DEffectObject(UI_2DEFFECT_CALLBACK pCallbackFunc)
{
	for (auto di = m_deque2DEffects.begin(); di != m_deque2DEffects.end(); )
	{
		if ((*di).pCallbackFunc == pCallbackFunc)
		{
			di = m_deque2DEffects.erase(di);
			break;
		}
		else
		{
			++di;
		}
	}
}

int SEASON3B::CNewUI3DCamera::GetCameraIndex() const
{ 
	return m_iCameraIndex; 
}

float SEASON3B::CNewUI3DCamera::GetLayerDepth()
{
	return m_fZOrder;
}

bool SEASON3B::CNewUI3DCamera::Render()
{
	if (m_list3DObjs.empty())
		return true;

	EndBitmap();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glViewport2(0, 0, m_uiWidth, m_uiHeight);
	gluPerspective2(1.f, static_cast<float>(m_uiWidth) / static_cast<float>(m_uiHeight), RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	GetOpenGLMatrix(CameraMatrix);
	EnableDepthTest();
	EnableDepthMask();
	glClear(GL_DEPTH_BUFFER_BIT);

	for (auto li = m_list3DObjs.begin(); li != m_list3DObjs.end(); ++li)
	{
		if ((*li)->IsVisible())
		{
			(*li)->Render3D();
		}
	}
	UpdateMousePositionn();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	BeginBitmap();

	while (!m_deque2DEffects.empty())
	{
		UI_2DEFFECT_INFO& UI2DEffectInfo = m_deque2DEffects.front();
		if (UI2DEffectInfo.pCallbackFunc)
		{
			(*UI2DEffectInfo.pCallbackFunc)(UI2DEffectInfo.pClass, UI2DEffectInfo.dwParamA, UI2DEffectInfo.dwParamB);
		}
		m_deque2DEffects.pop_front();
	}

	return true;
}

bool SEASON3B::CNewUI3DCamera::Update()
{
	//. DOING NOTHING
	return true;
}

bool SEASON3B::CNewUI3DCamera::UpdateMouseEvent()
{
	//. DOING NOTHING
	return true;
}

bool SEASON3B::CNewUI3DCamera::UpdateKeyEvent()
{
	//. DOING NOTHING
	return true;
}

SEASON3B::CNewUI3DRenderMng::CNewUI3DRenderMng() 
{}

SEASON3B::CNewUI3DRenderMng::~CNewUI3DRenderMng() 
{ 
	Release(); 
}

bool SEASON3B::CNewUI3DRenderMng::Create(CNewUIManager* pNewUIMng)
{
	m_pNewUIMng = pNewUIMng;
	return true;
}

void SEASON3B::CNewUI3DRenderMng::Release()
{ 
	RemoveAll3DRenderObjs(); 
}

void SEASON3B::CNewUI3DRenderMng::Add3DRenderObj(INewUI3DRenderObj* pObj, float fZOrder)
{
	CNewUI3DCamera* pCamera = FindCamera(fZOrder);
	if (nullptr == pCamera)
	{
		int iAvailableCameraIndex = FindAvailableCameraIndex();
		if (-1 != iAvailableCameraIndex)
		{
			pCamera = new CNewUI3DCamera;
			pCamera->Create(iAvailableCameraIndex, WindowWidth, WindowHeight, fZOrder);
			pCamera->Add3DRenderObj(pObj);
			m_pNewUIMng->AddUIObj(iAvailableCameraIndex, pCamera);
			m_listCamera.push_back(pCamera);
		}
		else
		{
#ifdef _DEBUG
			__asm { int 3 };
#endif // _DEBUG
		}
	}
	else
	{
		pCamera->Add3DRenderObj(pObj);
	}
}

void SEASON3B::CNewUI3DRenderMng::Remove3DRenderObj(INewUI3DRenderObj* pObj)
{
	for (auto li = m_listCamera.begin(); li != m_listCamera.end(); )
	{
		(*li)->Remove3DRenderObj(pObj);
		if ((*li)->IsEmpty())
		{
			m_pNewUIMng->RemoveUIObj(*li);
			delete (*li);
			li = m_listCamera.erase(li);
		}
		else
		{
			++li;
		}
	}
}

void SEASON3B::CNewUI3DRenderMng::RemoveAll3DRenderObjs()
{
	for (auto li = m_listCamera.begin(); li != m_listCamera.end(); ++li)
	{
		delete (*li);
		m_pNewUIMng->RemoveUIObj(*li);
	}
	m_listCamera.clear();
}

void SEASON3B::CNewUI3DRenderMng::RenderUI2DEffect(float fZOrder, UI_2DEFFECT_CALLBACK pCallbackFunc, LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
	CNewUI3DCamera* pCamera = FindCamera(fZOrder);
	if (pCamera)
	{
		pCamera->RenderUI2DEffect(pCallbackFunc, pClass, dwParamA, dwParamB);
	}
}

void SEASON3B::CNewUI3DRenderMng::DeleteUI2DEffectObject(UI_2DEFFECT_CALLBACK pCallbackFunc)
{
	for (auto li = m_listCamera.begin(); li != m_listCamera.end(); ++li)
	{
		(*li)->DeleteUI2DEffectObject(pCallbackFunc);
	}
}

CNewUI3DCamera* SEASON3B::CNewUI3DRenderMng::FindCamera(float fZOrder)
{
	for (auto li = m_listCamera.begin(); li != m_listCamera.end(); ++li)
	{
		if ((*li)->GetLayerDepth() == fZOrder)
		{
			return (*li);
		}
	}
	return nullptr;
}

int SEASON3B::CNewUI3DRenderMng::FindAvailableCameraIndex()
{
	for (int iIndex = INTERFACE_3DRENDERING_CAMERA_BEGIN; iIndex < INTERFACE_3DRENDERING_CAMERA_END; ++iIndex)
	{
		bool found = false;
		for (auto li = m_listCamera.begin(); li != m_listCamera.end(); ++li)
		{
			if ((*li)->GetCameraIndex() == iIndex)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			return iIndex;
		}
	}
	return -1;
}