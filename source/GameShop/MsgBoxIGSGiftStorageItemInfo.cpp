// MsgBoxIGSGiftStorageItemInfo.cpp: implementation of the CMsgBoxIGSGiftStorageItemInfo class.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"


#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "MsgBoxIGSGiftStorageItemInfo.h"

#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "MsgBoxIGSUseItemConfirm.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMsgBoxIGSGiftStorageItemInfo::CMsgBoxIGSGiftStorageItemInfo()
{
	m_iStorageSeq	= 0;		// ������ ����
	m_iStorageItemSeq = 0;		// ������ ��ǰ ����
	m_wItemCode		= -1;		// ������ �ڵ�
	
	m_szName[0]		= '\0';		// ������ �̸�
	m_szNum[0]		= '\0';
	m_szPeriod[0]	= '\0';
	m_szItemType	= '\0';		// ��ǰ���� (C : ĳ��, P : ��ǰ)
	
	m_szIDInfo[0]	= '\0';		// �������� ĳ����ID
	m_szMessage[0]	= '\0';		// �������� �޼���
}

CMsgBoxIGSGiftStorageItemInfo::~CMsgBoxIGSGiftStorageItemInfo()
{
	Release();
}

//--------------------------------------------
// Create
bool CMsgBoxIGSGiftStorageItemInfo::Create(float fPriority)
{
	LoadImages();
	SetAddCallbackFunc();
	
	CNewUIMessageBoxBase::Create((IMAGE_IGS_WINDOW_WIDTH/2)-(IMAGE_IGS_FRAME_WIDTH/2), 
		(IMAGE_IGS_WINDOW_HEIGHT/2)-(IMAGE_IGS_FRAME_HEIGHT/2), 
		IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT, fPriority);
	
	if(g_pNewUI3DRenderMng)
	{
		g_pNewUI3DRenderMng->Add3DRenderObj(this);
	}

	SetButtonInfo();

	SetMsgBackOpacity();

	// �޼��� Input Box
	m_MessageInputBox.SetMultiline(TRUE);
	m_MessageInputBox.Init(g_hWnd, IMAGE_IGS_FRAME_WIDTH-30, 100, 50);
	m_MessageInputBox.SetPosition(GetPos().x+22, GetPos().y+IGS_MESSAGE_INPUT_TEXT_POS_Y+96);
	m_MessageInputBox.SetUseScrollbar(FALSE);
	m_MessageInputBox.SetTextLimit(MAX_GIFT_MESSAGE_SIZE);
	m_MessageInputBox.SetFont(g_hFont);
	//m_MessageInputBox.SetOption(UIOPTION_NULL);
	m_MessageInputBox.SetBackColor(0, 0, 0, 0);
	m_MessageInputBox.SetState(UISTATE_NORMAL);
	m_MessageInputBox.SetTextColor(255, 0, 0, 0);
	
	return true;
}

//--------------------------------------------
// IsVisible
bool CMsgBoxIGSGiftStorageItemInfo::IsVisible() const
{
	return true;
}


//--------------------------------------------
// Initialize
void CMsgBoxIGSGiftStorageItemInfo::Initialize(int iStorageSeq, int iStorageItemSeq, WORD wItemCode, unicode::t_char szItemType, 
											   unicode::t_char* pszID, unicode::t_char* pszMessage, unicode::t_char* pszName, 
											   unicode::t_char* pszNum, unicode::t_char* pszPeriod)
{
	m_iStorageSeq		= iStorageSeq;
	m_iStorageItemSeq	= iStorageItemSeq;
	m_wItemCode			= wItemCode;
	m_szItemType		= szItemType;
	
	// Name
	strcpy(m_szName, pszName);

	// Num
	sprintf(m_szNum, GlobalText[3040], pszNum);		// "���� : %s"

	// Period
	sprintf(m_szPeriod, GlobalText[3039], pszPeriod);	// "�Ⱓ : %s"

	// ID Info
	// "\'%s\' ���� ���� �����Դϴ�."
	sprintf(m_szIDInfo, GlobalText[3041], pszID);

	m_MessageInputBox.SetText(pszMessage);
}

//--------------------------------------------
// Release
void CMsgBoxIGSGiftStorageItemInfo::Release()
{
	CNewUIMessageBoxBase::Release();
	
	if(g_pNewUI3DRenderMng)
		g_pNewUI3DRenderMng->Remove3DRenderObj(this);

	UnloadImages();
}

//--------------------------------------------
// Update
bool CMsgBoxIGSGiftStorageItemInfo::Update()
{
	m_BtnUse.Update();
	m_BtnCancel.Update();
	
	return true;
}

//--------------------------------------------
// Render
bool CMsgBoxIGSGiftStorageItemInfo::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	RenderMsgBackColor(true);

	RenderFrame();
	RenderTexts();
	RenderButtons();

	m_MessageInputBox.Render();

	DisableAlphaBlend();
	return true;
}

//--------------------------------------------
// Render3D
void CMsgBoxIGSGiftStorageItemInfo::Render3D()
{
	if(m_wItemCode == 65535)
		return;

	RenderItem3D(GetPos().x+IGS_3DITEM_POS_X, GetPos().y+IGS_3DITEM_POS_Y, 
					IGS_3DITEM_WIDTH, IGS_3DITEM_HEIGHT, m_wItemCode, 0, 0, 0, true);
}

//--------------------------------------------
// SetAddCallbackFunc
void CMsgBoxIGSGiftStorageItemInfo::SetAddCallbackFunc()
{
	AddCallbackFunc(CMsgBoxIGSGiftStorageItemInfo::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
	AddCallbackFunc(CMsgBoxIGSGiftStorageItemInfo::OKButtonDown, MSGBOX_EVENT_USER_COMMON_OK);
	AddCallbackFunc(CMsgBoxIGSGiftStorageItemInfo::CancelButtonDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

//--------------------------------------------
// LButtonUp
CALLBACK_RESULT CMsgBoxIGSGiftStorageItemInfo::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	CMsgBoxIGSGiftStorageItemInfo* pOwnMsgBox = dynamic_cast<CMsgBoxIGSGiftStorageItemInfo*>(pOwner);

	if(pOwnMsgBox)
	{
		if(pOwnMsgBox->m_BtnUse.IsMouseIn() == true)
		{
			g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
			return CALLBACK_BREAK;
		}

		if(pOwnMsgBox->m_BtnCancel.IsMouseIn() == true)
		{
			g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
			return CALLBACK_BREAK;
		}
	}

	return CALLBACK_CONTINUE;
}

//--------------------------------------------
// OKButtonDown
CALLBACK_RESULT CMsgBoxIGSGiftStorageItemInfo::OKButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	CMsgBoxIGSGiftStorageItemInfo* pOwnMsgBox = dynamic_cast<CMsgBoxIGSGiftStorageItemInfo*>(pOwner);

	// ����ϱ� Ȯ�� â
	CMsgBoxIGSUseItemConfirm* pMsgBox = NULL;
	CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSUseItemConfirmLayout), &pMsgBox);
	pMsgBox->Initialize(pOwnMsgBox->m_iStorageSeq, pOwnMsgBox->m_iStorageItemSeq, 
							pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_szItemType, pOwnMsgBox->m_szName);

	PlayBuffer(SOUND_CLICK01);
	g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

	return CALLBACK_BREAK;
}

//--------------------------------------------
// CancelButtonDown
CALLBACK_RESULT CMsgBoxIGSGiftStorageItemInfo::CancelButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	PlayBuffer(SOUND_CLICK01);
	g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
	
	return CALLBACK_BREAK;
}

//--------------------------------------------
// SetButtonInfo
void CMsgBoxIGSGiftStorageItemInfo::SetButtonInfo()
{
	// Ȯ�� ��ư
	m_BtnUse.SetInfo(IMAGE_IGS_BUTTON, GetPos().x+IGS_BTN_OK_POS_X, GetPos().y+IGS_BTN_POS_Y+102, 
						IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
	m_BtnUse.MoveTextPos(0, -1);
	m_BtnUse.SetText(GlobalText[228]);	

	// ��� ��ư
	m_BtnCancel.SetInfo(IMAGE_IGS_BUTTON, GetPos().x+IGS_BTN_CANCEL_POS_X, GetPos().y+IGS_BTN_POS_Y+102, 
							IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
	m_BtnCancel.MoveTextPos(0, -1);
	m_BtnCancel.SetText(GlobalText[229]);	
}

//--------------------------------------------
// RenderFrame
void CMsgBoxIGSGiftStorageItemInfo::RenderFrame()
{
	RenderImage(IMAGE_IGS_FRAME, GetPos().x, GetPos().y, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT);
}

//--------------------------------------------
// RenderTexts
void CMsgBoxIGSGiftStorageItemInfo::RenderTexts()
{
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetFont(g_hFontBold);

	// Title "���� ����â" 
	g_pRenderText->RenderText(GetPos().x, GetPos().y+IGS_TEXT_TITLE_POS_Y, GlobalText[3048], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);	

	// Item Name
	g_pRenderText->SetTextColor(255, 255, 0, 255);
	g_pRenderText->RenderText(GetPos().x, GetPos().y+IGS_TEXT_ITEM_NAME_POS_Y, m_szName, IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

	// ID Info
	g_pRenderText->SetTextColor(0, 0, 0, 255);
	g_pRenderText->RenderText(GetPos().x, GetPos().y+IGS_TEXT_ID_INFO_POS_Y, m_szIDInfo, IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);
	
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	
	// Item Info
 	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_ITEM_INFO_POS_X, GetPos().y+IGS_TEXT_ITEM_INFO_NUM_POS_Y, m_szNum, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);
	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_ITEM_INFO_POS_X, GetPos().y+IGS_TEXT_ITEM_INFO_PERIOD_POS_Y, m_szPeriod, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);
	

#ifdef FOR_WORK
	// debug
	unicode::t_char szText[256] = {0,};
	g_pRenderText->SetTextColor(255, 0, 0, 255);
	if( m_wItemCode == 65535 )
	{
		sprintf(szText, "�������ڵ尡 �����ϴ�.");
	}
	else
	{
		sprintf(szText, "ItemCode : %d (%d, %d)", m_wItemCode, m_wItemCode/MAX_ITEM_INDEX, m_wItemCode%MAX_ITEM_INDEX);
	}
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+10, szText, 150, 0, RT3_SORT_LEFT);
	sprintf(szText, "Storage Seq : %d", m_iStorageSeq);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+20, szText, 150, 0, RT3_SORT_LEFT);
	sprintf(szText, "Storage ItemSeq : %d", m_iStorageItemSeq);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+30, szText, 150, 0, RT3_SORT_LEFT);
#endif // FOR_WORK
}

//--------------------------------------------
// RenderButtons
void CMsgBoxIGSGiftStorageItemInfo::RenderButtons()
{
	m_BtnUse.Render();
	m_BtnCancel.Render();
}

//--------------------------------------------
// LoadImages
void CMsgBoxIGSGiftStorageItemInfo::LoadImages()
{
	LoadBitmap("Interface\\InGameShop\\Ingame_Bt03.tga",	IMAGE_IGS_BUTTON, GL_LINEAR);
	LoadBitmap("Interface\\InGameShop\\ingame_Box_List_B.tga",	IMAGE_IGS_FRAME, GL_LINEAR);
}

//--------------------------------------------
// UnloadImages
void CMsgBoxIGSGiftStorageItemInfo::UnloadImages()
{
	DeleteBitmap(IMAGE_IGS_BUTTON);
	DeleteBitmap(IMAGE_IGS_FRAME);
}

////////////////////////////////////////////////////////////////////
// LayOut
////////////////////////////////////////////////////////////////////
bool CMsgBoxIGSGiftStorageItemInfoLayout::SetLayout()
{
	CMsgBoxIGSGiftStorageItemInfo* pMsgBox = GetMsgBox();
	if(false == pMsgBox)
		return false;
	
	if(false == pMsgBox->Create())
		return false;
	
	return true;
}

#endif KJH_ADD_INGAMESHOP_UI_SYSTEM
