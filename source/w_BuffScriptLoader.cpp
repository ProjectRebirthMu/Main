// BuffScriptLoader.cpp: implementation of the CBuffScriptLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ReadScript.h"
#include "UIManager.h"
#include "ItemAddOptioninfo.h"
#include "w_BuffScriptLoader.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace
{
	BYTE bBuxCode[3] = {0xfc,0xcf,0xab};
	void BuxConvert(BYTE *Buffer, int Size)
	{
		for(int i=0;i<Size;i++)
			Buffer[i] ^= bBuxCode[i%3];
	}

	wchar_t wBuxCode[3] = {0xcd56, 0x1d93, 0x5b81};
	void BuxConvertW(wchar_t *Buffer, int Size)
	{
		for(int i=0;i<Size;i++)
			Buffer[i] ^= wBuxCode[i%3];
	}
	
	void CutTokenString( char* pcCuttoken, std::list<std::string>& out )
	{
		if( unicode::_strlen( pcCuttoken ) == 0 ) return;

		int cutpos = 0;

		for( int i = 0; i < MAX_DESCRIPT_LENGTH; ++i )
		{
			if (pcCuttoken[i] == '/' || pcCuttoken[i] == 0 )
			{		
				char Temp[MAX_DESCRIPT_LENGTH] = { 0, };
				strncpy( Temp, pcCuttoken+cutpos, i-cutpos );
				out.push_back( Temp );
				cutpos = i+1;

				if( pcCuttoken[i] == 0 ) return;
			}
		}
	}
}

BuffInfo::BuffInfo() : s_ItemType( 255 ), s_ItemIndex( 255 ), s_BuffIndex( 0 ), 
		s_BuffEffectType( 0 ), s_BuffClassType( 0 ), s_NoticeType( 0 ), s_ClearType( 0 )
{
	memset(&s_BuffName, 0, sizeof(char)*MAX_BUFF_NAME_LENGTH);
	memset(&s_BuffDescript, 0, sizeof(char)*MAX_DESCRIPT_LENGTH);
}

BuffInfo::~BuffInfo()
{

}

BuffScriptLoaderPtr BuffScriptLoader::Make()
{
	BuffScriptLoaderPtr info( new BuffScriptLoader() );
	return info;
}

BuffScriptLoader::BuffScriptLoader()
{
	std::string filename = "data/local/"+g_strSelectedML+"/BuffEffect_"+g_strSelectedML+".bmd";

	if( !Load( filename ) )
	{
		assert(0);
	}
}

BuffScriptLoader::~BuffScriptLoader()
{

}

bool BuffScriptLoader::Load(const std::string& pchFileName)
{
	FILE *fp = fopen( pchFileName.c_str(), "rb" );

	if(fp != NULL)
	{
		DWORD structsize = sizeof(_BUFFINFO);

		DWORD listsize;
		fread( &listsize, sizeof(DWORD), 1, fp );

		BYTE *Buffer = new BYTE [structsize*listsize];
		fread( Buffer, structsize*listsize, 1, fp );

		DWORD dwCheckSum;
		fread( &dwCheckSum, sizeof(DWORD), 1, fp );

		fclose(fp);
		if ( dwCheckSum != GenerateCheckSum2( Buffer, structsize*listsize, 0xE2F1))
		{
			char Text[256];
    		sprintf( Text, "Erro interno.", pchFileName );
			g_ErrorReport.Write( Text );
			MessageBox(g_hWnd,Text,NULL,MB_OK);
			SendMessage(g_hWnd,WM_DESTROY,0,0);
		}
		else
		{
			BYTE *pSeek = Buffer;

			for( DWORD i = 0;i < listsize; i++ )			
			{
				_BUFFINFO tempbuffinfo;

				BuxConvert( pSeek, structsize );
				memcpy( &tempbuffinfo, pSeek, structsize );

				BuffInfo buffinfo;
				buffinfo.s_BuffIndex = tempbuffinfo.s_BuffIndex;
				buffinfo.s_BuffEffectType = tempbuffinfo.s_BuffEffectType;
				buffinfo.s_ItemType = tempbuffinfo.s_ItemType;
				buffinfo.s_ItemIndex = tempbuffinfo.s_ItemIndex;
				memcpy( buffinfo.s_BuffName, tempbuffinfo.s_BuffName, sizeof(char)*MAX_BUFF_NAME_LENGTH );
				buffinfo.s_BuffClassType = tempbuffinfo.s_BuffClassType;
				buffinfo.s_NoticeType = tempbuffinfo.s_NoticeType;
				buffinfo.s_ClearType = tempbuffinfo.s_ClearType;
				memcpy( buffinfo.s_BuffDescript, tempbuffinfo.s_BuffDescript, sizeof(char)*MAX_DESCRIPT_LENGTH );

				CutTokenString( buffinfo.s_BuffDescript, buffinfo.s_BuffDescriptlist );
				m_Info.insert(std::make_pair( static_cast<eBuffState>(buffinfo.s_BuffIndex), buffinfo ) );

				pSeek += structsize;
			}
		}
		delete [] Buffer;
	}
	else
	{
		char Text[256];
    	sprintf( Text, "Erro Interno.", pchFileName );
		g_ErrorReport.Write( Text );
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
	}

	return true;
}

const BuffInfo BuffScriptLoader::GetBuffinfo( eBuffState type ) const
{
	if( type >= eBuff_Count ) return BuffInfo();

	BuffInfoMap::const_iterator iter = m_Info.find( type );

	if( iter != m_Info.end() )
	{
		return (*iter).second;
	}

	return BuffInfo();
}

eBuffClass BuffScriptLoader::IsBuffClass( eBuffState type ) const
{
	if( type >= eBuff_Count ) return eBuffClass_Count;

	BuffInfoMap::const_iterator iter = m_Info.find( type );

	if( iter != m_Info.end() )
	{
		return static_cast<eBuffClass>((*iter).second.s_BuffClassType);
	}
	else
	{
		return eBuffClass_Count;
	}
}

#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
int BuffScriptLoader::GetBuffIndex(int iItemCode)
{
	BuffInfoMap::iterator iter = m_Info.begin();

	int iterItemCode = 0;

	while(iter != m_Info.end())
	{
		iterItemCode = ITEMINDEX(iter->second.s_ItemType, iter->second.s_ItemIndex);
		
		if( iterItemCode == iItemCode )
			return iter->second.s_BuffIndex;

		iter++;
	}

	return -1;
}

int BuffScriptLoader::GetBuffType(int iItemCode)
{
	BuffInfoMap::iterator iter = m_Info.begin();
	
	int iterItemCode = 0;
	
	while(iter != m_Info.end())
	{
		iterItemCode = ITEMINDEX(iter->second.s_ItemType, iter->second.s_ItemIndex);
		
		if( iterItemCode == iItemCode )
			return iter->second.s_BuffEffectType;
		
		iter++;
	}
	
	return -1;
}
#endif // KJH_PBG_ADD_INGAMESHOP_SYSTEM
