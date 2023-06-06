//*****************************************************************************
// File: NewUITrade.h
//
// Desc: interface for the CNewUITrade class.
//		 �ŷ�â Ŭ����.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_NEWUITRADE_H__25FC9B24_8F86_4791_B246_689326623DFB__INCLUDED_)
#define AFX_NEWUITRADE_H__25FC9B24_8F86_4791_B246_689326623DFB__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIMyQuestInfoWindow.h"
#include "NewUIStorageInventory.h"

namespace SEASON3B
{
	class CNewUITrade : public CNewUIObj  
	{
	public:
		enum IMAGE_LIST
		{
			IMAGE_TRADE_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
			IMAGE_TRADE_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
			IMAGE_TRADE_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
			IMAGE_TRADE_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
			IMAGE_TRADE_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,

			IMAGE_TRADE_LINE = CNewUIMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
			IMAGE_TRADE_NICK_BACK = BITMAP_INTERFACE_NEW_TRADE_BEGIN,
			IMAGE_TRADE_MONEY = CNewUIMyInventory::IMAGE_INVENTORY_MONEY,
			IMAGE_TRADE_CONFIRM = BITMAP_INTERFACE_NEW_TRADE_BEGIN + 1,
			IMAGE_TRADE_WARNING_ARROW = BITMAP_CURSOR+7,

			IMAGE_TRADE_BTN_CLOSE = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
			IMAGE_TRADE_BTN_ZEN_INPUT = CNewUIStorageInventory::IMAGE_STORAGE_BTN_INSERT_ZEN,
		};
		
	private:
		enum
		{
			TRADE_WIDTH = 190,
			TRADE_HEIGHT = 429,
			CONFIRM_WIDTH = 36,
			CONFIRM_HEIGHT = 29,
			COLUMN_TRADE_INVEN = 8,
			ROW_TRADE_INVEN = 4,
			MAX_TRADE_INVEN = COLUMN_TRADE_INVEN * ROW_TRADE_INVEN,
		};

		enum TRADE_BUTTON
		{
			BTN_CLOSE = 0,			// â �ݱ�.
			BTN_ZEN_INPUT,			// �� �Է�.
			MAX_BTN
		};

		CNewUIManager*			m_pNewUIMng;			// UI �Ŵ���.
		POINT					m_Pos;					// â�� ��ġ.
		CNewUIButton			m_abtn[MAX_BTN];		// ��ư.
		POINT					m_posMyConfirm;			// �� Ȯ�� ��ư ��ġ.
		CNewUIInventoryCtrl*	m_pYourInvenCtrl;		// ���� ��ǰ ��Ʈ��.
		CNewUIInventoryCtrl*	m_pMyInvenCtrl;			// �� ��ǰ ��Ʈ��.
		ITEM					m_aYourInvenBackUp[MAX_TRADE_INVEN];// ���� ��ǰ ���.

		char					m_szYourID[MAX_ID_SIZE+1];// �ŷ� ������� ���̵�.
		int						m_nYourLevel;			// �ŷ� ������� ����.
		int						m_nYourGuildType;		// ���� ��� Ÿ��.
		int						m_nYourTradeGold;		// ���� �ŷ��� ��.
		int						m_nMyTradeGold;			// �ڽ��� �ŷ��� ��.
		int						m_nTempMyTradeGold;		// �ڽ��� �ŷ��� �� �ӽ� ����.
		bool					m_bYourConfirm;			// ���� �ŷ� ���� ����.
		bool					m_bMyConfirm;			// �ڽ��� �ŷ� ���� ����.
		int						m_nMyTradeWait;			// �ڽ��� �ŷ� ���� ��ư �� ������ �ϴ� ��� �ð�.
		bool					m_bTradeAlert;			// �ŷ��� ���.

	public:
		CNewUITrade();
		virtual ~CNewUITrade();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		
		float GetLayerDepth();	//. 2.1f

		static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);
		
		// (���� �����) ����� �ŷ� ����ǰ ��Ʈ���� ����.
		CNewUIInventoryCtrl* GetYourInvenCtrl() const
		{ return m_pYourInvenCtrl; }
		// (���� �����) �ڽ��� �ŷ� ����ǰ ��Ʈ���� ����.
		CNewUIInventoryCtrl* GetMyInvenCtrl() const
		{ return m_pMyInvenCtrl; }

		void ProcessCloseBtn();
		void ProcessClosing();

		void GetYourID(char* pszYourID);
		void SetYourTradeGold(int nGold){ m_nYourTradeGold = nGold; }

		void SendRequestMyGoldInput(int nInputGold);
		void SendRequestItemToMyInven(ITEM* pItemObj,
			int nTradeIndex, int nInvenIndex);

		void ProcessToReceiveTradeRequest(BYTE* pbyYourID);
		void ProcessToReceiveTradeResult(LPPTRADE pTradeData);
		void ProcessToReceiveYourItemDelete(BYTE byYourInvenIndex);
		void ProcessToReceiveYourItemAdd(BYTE byYourInvenIndex, BYTE* pbyItemPacket);
		void ProcessToReceiveMyTradeGold(BYTE bySuccess);
		void ProcessToReceiveYourConfirm(BYTE byState);
		void ProcessToReceiveTradeExit(BYTE byState);
		void ProcessToReceiveTradeItems(int nIndex, BYTE* pbyItemPacket);

		void AlertTrade();

		int GetPointedItemIndexMyInven();
		int GetPointedItemIndexYourInven();

	private:
		void LoadImages();
		void UnloadImages();

		void RenderBackImage();
		void RenderText();
		void RenderWarningArrow();

		void ProcessMyInvenCtrl();
		bool ProcessBtns();

		void ConvertYourLevel(int& rnLevel, DWORD& rdwColor);

		void InitTradeInfo();
		void InitYourInvenBackUp();
		void BackUpYourInven(int nYourInvenIndex);
		void BackUpYourInven(ITEM* pYourItemObj);
		void AlertYourTradeInven();

		void SendRequestItemToTrade(ITEM* pItemObj, int nInvenIndex, int nTradeIndex);
	};
}

#endif // !defined(AFX_NEWUITRADE_H__25FC9B24_8F86_4791_B246_689326623DFB__INCLUDED_)
