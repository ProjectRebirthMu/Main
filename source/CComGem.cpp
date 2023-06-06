#include "stdafx.h"
#include "CComGem.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "wsclientinline.h"
#include "ZzzInventory.h"
#include "NewUIInventoryCtrl.h"

extern DWORD  g_dwActiveUIID;
extern int   InventoryStartX;
extern int   InventoryStartY;

namespace COMGEM
{
	//Locals
	int		iInvWidth = 190.f;
	int		iInvHeight = 433.f;
	int		iUnMixIndex = -1;
	int		iUnMixLevel = -1;

	//Globals
	BOOL	m_bType = ATTACH;
	char	m_cState = STATE_READY;
	char	m_cErr = NOERR;
	char	m_cGemType = -1;
	char	m_cComType = -1;
	BYTE	m_cCount = 0;
	int		m_iValue = 0;
	BYTE	m_cPercent = 0;
	
	CUIUnmixgemList	m_UnmixTarList;
}

void COMGEM::SendReqUnMix()
{
	iUnMixIndex += 12;
	SendRequestGemUnMix(m_cGemType/2, iUnMixLevel, iUnMixIndex);
}

void COMGEM::SendReqMix()
{
	SendRequestGemMix(m_cGemType/2, (m_cComType/10-1));
}

void COMGEM::ProcessCSAction()
{
	if(m_cState == STATE_HOLD || m_cErr != NOERR)
		return;
	SetState(STATE_HOLD);

	if(isComMode())
		SendReqMix();
	else
		SendReqUnMix();
}

void COMGEM::ResetWantedList()
{
	m_UnmixTarList.Clear();
}

bool COMGEM::FindWantedList()
{
	SEASON3B::CNewUIInventoryCtrl * pNewInventoryCtrl = g_pMyInventory->GetInventoryCtrl();
	ITEM * pItem = NULL;

	bool	bReturn			= false;
	int		nInvenMaxIndex	= MAX_MY_INVENTORY_INDEX;
	ResetWantedList();
		
	for (int i = 0; i < nInvenMaxIndex; ++i)
	{
		pItem = pNewInventoryCtrl->GetItem(i);
		if( !pItem ) continue;
		if( isCompiledGem( pItem ) )
		{
			INTBYTEPAIR p;
			int Level = (pItem->Level>>3) & 15;
			p.first = pItem->y*pNewInventoryCtrl->GetNumberOfColumn()+pItem->x;
			p.second = Level;
			m_UnmixTarList.AddText(p.first, p.second);
			bReturn = true;
		}
	}
	return bReturn;		
}

void COMGEM::SelectFromList(int iIndex, int iLevel)
{
	iUnMixIndex = iIndex;
	iUnMixLevel = iLevel;

	if(CheckInv())
	{

	}
}

int COMGEM::GetUnMixGemLevel()
{
	return iUnMixLevel;
}

void COMGEM::MoveUnMixList()
{
	g_dwActiveUIID = m_UnmixTarList.GetUIID();
	m_UnmixTarList.DoAction();
	g_dwActiveUIID = 0;
}

void COMGEM::RenderUnMixList()
{
	m_UnmixTarList.Render();
}

char COMGEM::CheckOneItem(const ITEM* p)
{
	return Check_Jewel(p->Type);
}

bool COMGEM::CheckInv()
{
	if(!CheckMyInvValid())
	{
		switch(GetError())
		{
		case COMERROR_NOTALLOWED:
		g_pChatListBox->AddText("", GlobalText[1817], SEASON3B::TYPE_ERROR_MESSAGE);
			break;
		case DEERROR_NOTALLOWED:
		g_pChatListBox->AddText("", GlobalText[1818], SEASON3B::TYPE_ERROR_MESSAGE);
			break;
		}
		GetBack();
		return false;
	}

	return true;
}

bool COMGEM::CheckMyInvValid()
{
	m_cPercent = 0;
	m_cCount = 0;

	int nInvenMaxIndex	 = MAX_MY_INVENTORY_INDEX;
	
	if(m_bType == ATTACH)
	{
		SEASON3B::CNewUIInventoryCtrl * pNewInventoryCtrl = g_pMyInventory->GetInventoryCtrl();
		ITEM * pItem = NULL;

		for (int i = 0; i < nInvenMaxIndex; ++i)
		{
			pItem = pNewInventoryCtrl->GetItem(i);
		if( !pItem ) continue;
		if( m_cGemType == Check_Jewel_Unit( pItem->Type ) )	++m_cCount;

			if(m_cCount == m_cComType)
			{
				m_cPercent = 100;
				CalcGen();
				return true;
			}
		}
		if((m_cCount < m_cComType) || (m_cComType == NOCOM))
		{
			m_cPercent = 0;
			m_cErr = COMERROR_NOTALLOWED;
			return false;
		}
	}
	else if(m_bType == DETACH)
	{
		if(iUnMixIndex == -1 || iUnMixIndex >= MAX_INVENTORY)
		{
			m_cErr = DEERROR_NOTALLOWED;
			m_cPercent = 0;
			return false;
		}

		//SEASON3B::CNewUIInventoryCtrl * pNewInventoryCtrl = g_pMyInventory->GetInventoryCtrl();
		//ITEM * pItem = pNewInventoryCtrl->GetItem(iUnMixIndex);
		ITEM * pItem = g_pMyInventory->FindItem(iUnMixIndex);

		if( isCompiledGem( pItem ) )
		
		{
			++m_cCount;
			m_cPercent = 100;
			CalcGen();
			return true;
		}
		else
		{
			m_cErr = DEERROR_NOTALLOWED;
			m_cPercent = 0;
			return false;					
		}		
	}
	m_cErr = ERROR_UNKNOWN;
	return false;
}

void COMGEM::CalcGen()
{
	m_iValue = 0;
	if(m_bType)
	{
		m_iValue = 1000000;
	}
	else
	{
		m_iValue = m_cComType * 50000;
	}
}

char COMGEM::CalcCompiledCount(const ITEM* p)
{
	int Level = (p->Level>>3)&15;

	if( CheckOneItem(p) % 2 )	return (Level+1) * FIRST;
	else						return 0;
}

int	COMGEM::CalcItemValue(const ITEM* p)
{
	int Level = (p->Level>>3)&15;
	switch(CheckOneItem(p))
	{
	case NOGEM:
		return 0;
	case eBLESS_C:
		return 9000000 * (Level+1) * FIRST;
	break;
	case eSOUL_C:
		return 6000000 * (Level+1) * FIRST;
	break;
	case eLIFE_C:
		return 45000000 * (Level+1) * FIRST;
	break;
	case eCREATE_C:
		return 36000000 * (Level+1) * FIRST;
	break;
	case ePROTECT_C:
		return 60000000 * (Level+1) * FIRST;
	break;
	case eCHAOS_C:
		return 810000 * (Level+1) * FIRST;
	break;
	case eGEMSTONE_C:
	case eHARMONY_C:
	case eLOW_C:
	case eUPPER_C:
		return 18600 * (Level+1) * FIRST;
	break;
	default:
		return 0;
	}
}

int COMGEM::CalcEmptyInv()
{
	SEASON3B::CNewUIInventoryCtrl * pNewInventoryCtrl = g_pMyInventory->GetInventoryCtrl();
	return pNewInventoryCtrl->GetEmptySlotCount();
}

void COMGEM::Init()
{
	m_bType = ATTACH;
	m_cState = STATE_READY;
	m_cErr = NOERR;
	m_cGemType = -1;
	m_cComType = -1;
	m_cCount = 0;
	m_iValue = -1;
	m_cPercent = 0;
	iUnMixIndex = -1;
	iUnMixLevel = -1;
}

void COMGEM::GetBack()
{
	if(m_cState == STATE_HOLD)
	{
		m_cState = STATE_READY;
	}
	
	m_cErr = NOERR;
	m_cGemType = NOGEM;

	if(m_bType != ATTACH && m_bType != DETACH)
	{
		Exit();
	}
}

void COMGEM::Exit()
{
	Init();

	SendExitInventory();
}

int	COMGEM::GetJewelRequireCount( int i )
{
	if( i == 0 )	return FIRST;
	if( i == 1 )	return SECOND;
	if( i == 2 )	return THIRD;

	return -1;
}

int COMGEM::Check_Jewel( int _nJewel, int _nType, bool _bModel )
{
	bool	bCom = true;
	bool	bNon = true;

	if( _bModel  )		_nJewel -= MODEL_ITEM;
	if( _nType & 1 )	bCom = false;
	if( _nType & 2 )	bNon = false;

	if( bNon )
	{
		if( _nJewel == ITEM_POTION+13 )		return eBLESS;								
		if( _nJewel == ITEM_POTION+14 )		return eSOUL;
		if( _nJewel == ITEM_POTION+16 )		return eLIFE;
		if( _nJewel == ITEM_POTION+22 )		return eCREATE;
		if( _nJewel == ITEM_POTION+31 )		return ePROTECT;
		if( _nJewel == ITEM_POTION+41 )		return eGEMSTONE;
		if( _nJewel == ITEM_POTION+42 )		return eHARMONY;
		if( _nJewel == ITEM_WING+15 )		return eCHAOS;
		if( _nJewel == ITEM_POTION+43 )		return eLOW;
		if( _nJewel == ITEM_POTION+44 )		return eUPPER;
	}

	if( bCom )
	{
		if( _nJewel == ITEM_WING+30 )	return eBLESS_C;
		if( _nJewel == ITEM_WING+31 )	return eSOUL_C;
		if( _nJewel == ITEM_WING+136 )	return eLIFE_C;
		if( _nJewel == ITEM_WING+137 )	return eCREATE_C;
		if( _nJewel == ITEM_WING+138 )	return ePROTECT_C;
		if( _nJewel == ITEM_WING+139 )	return eGEMSTONE_C;
		if( _nJewel == ITEM_WING+140 )	return eHARMONY_C;
		if( _nJewel == ITEM_WING+141 )	return eCHAOS_C;
		if( _nJewel == ITEM_WING+142 )	return eLOW_C;
		if( _nJewel == ITEM_WING+143 )	return eUPPER_C;
	}
	
	return NOGEM;
}

int COMGEM::GetJewelIndex( int _nJewel, int _nType )
{
	switch( _nJewel )
	{
		case eBLESS:
			if( _nType == eGEM_NAME )		return 1806;
			if( _nType == eGEM_INDEX )		return ITEM_POTION+13;
		break;
		case eBLESS_C:
			if( _nType == eGEM_NAME )		return 1806;
			if( _nType == eGEM_INDEX )		return ITEM_WING+14;
		break;
		case eSOUL:
			if( _nType == eGEM_NAME )		return 1807;
			if( _nType == eGEM_INDEX )		return ITEM_POTION+14;
		break;
		case eSOUL_C:
			if( _nType == eGEM_NAME )		return 1807;
			if( _nType == eGEM_INDEX )		return ITEM_WING+13;
		break;
		case eLIFE:
			if( _nType == eGEM_NAME )		return 3312;
			if( _nType == eGEM_INDEX )		return ITEM_POTION+16;
		break;
		case eLIFE_C:
			if( _nType == eGEM_NAME )		return 3312;
			if( _nType == eGEM_INDEX )		return ITEM_WING+13;
		break;
		case eCREATE:
			if( _nType == eGEM_NAME )		return 3313;
			if( _nType == eGEM_INDEX )		return ITEM_POTION+22;
		break;
		case eCREATE_C:
			if( _nType == eGEM_NAME )		return 3313;
			if( _nType == eGEM_INDEX )		return ITEM_WING+13;
		break;
		case ePROTECT:
			if( _nType == eGEM_NAME )		return 3314;
			if( _nType == eGEM_INDEX )		return ITEM_POTION+31;
		break;
		case ePROTECT_C:
			if( _nType == eGEM_NAME )		return 3314;
			if( _nType == eGEM_INDEX )		return ITEM_WING+13;
		break;
		case eGEMSTONE:
			if( _nType == eGEM_NAME )		return 2081;
			if( _nType == eGEM_INDEX )		return ITEM_POTION+41;
		break;
		case eGEMSTONE_C:
			if( _nType == eGEM_NAME )		return 2081;
			if( _nType == eGEM_INDEX )		return ITEM_WING+13;
		break;
		case eHARMONY:
			if( _nType == eGEM_NAME )		return 3315;
			if( _nType == eGEM_INDEX )		return ITEM_POTION+42;
		break;
		case eHARMONY_C:
			if( _nType == eGEM_NAME )		return 3315;
			if( _nType == eGEM_INDEX )		return ITEM_WING+13;
		break;
		case eCHAOS:
			if( _nType == eGEM_NAME )		return 3316;
			if( _nType == eGEM_INDEX )		return ITEM_WING+15;
		break;
		case eCHAOS_C:
			if( _nType == eGEM_NAME )		return 3316;
			if( _nType == eGEM_INDEX )		return ITEM_WING+13;
		break;
		case eLOW:
			if( _nType == eGEM_NAME )		return 3317;
			if( _nType == eGEM_INDEX )		return ITEM_POTION+43;
		break;
		case eLOW_C:
			if( _nType == eGEM_NAME )		return 3317;
			if( _nType == eGEM_INDEX )		return ITEM_WING+13;
		break;
		case eUPPER:
			if( _nType == eGEM_NAME )		return 3318;
			if( _nType == eGEM_INDEX )		return ITEM_POTION+44;
		break;
		case eUPPER_C:
			if( _nType == eGEM_NAME )		return 3318;
			if( _nType == eGEM_INDEX )		return ITEM_WING+13;
		break;
	}

	return -1;
}
