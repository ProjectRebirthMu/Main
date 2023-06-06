// NewUINameWindow.h: interface for the CNewUINameWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUINAMEWINDOW_H__76B140FF_46CB_4DB6_9DA2_5F84F294D212__INCLUDED_)
#define AFX_NEWUINAMEWINDOW_H__76B140FF_46CB_4DB6_9DA2_5F84F294D212__INCLUDED_

#pragma once

#include "NewUIManager.h"

namespace SEASON3B
{
	// ������ �̸�
	class CNewUINameWindow  : public CNewUIObj  
	{
	public:
		CNewUINameWindow();
		virtual ~CNewUINameWindow();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();

		float GetLayerDepth();		// 1.0f

	private:
		void RenderName();

		CNewUIManager* m_pNewUIMng;		// UI �Ŵ���.
		POINT m_Pos;					// â�� ��ġ.

		bool m_bShowItemName;
	};
	
}

#endif // !defined(AFX_NEWUINAMEWINDOW_H__76B140FF_46CB_4DB6_9DA2_5F84F294D212__INCLUDED_)
