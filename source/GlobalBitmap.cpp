// GlobalBitmap.cpp: implementation of the CGlobalBitmap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "GlobalBitmap.h"
#include "./Utilities/Log/ErrorReport.h"
#include "./Utilities/Log/muConsoleDebug.h"

CBitmapCache::CBitmapCache()
{
	memset(m_QuickCache, 0, sizeof(m_QuickCache));
}

CBitmapCache::~CBitmapCache() 
{ 
	Release(); 
}

bool CBitmapCache::Create()
{
	Release();
	QUICK_CACHE cache;

	cache.dwBitmapIndexMin = BITMAP_MAPTILE_BEGIN;
	cache.dwBitmapIndexMax = BITMAP_MAPTILE_END;
	cache.dwRange = cache.dwBitmapIndexMax - cache.dwBitmapIndexMin;
	cache.ppBitmap = new BITMAP_t * [cache.dwRange];
	memset(cache.ppBitmap, 0, cache.dwRange * sizeof(BITMAP_t*));
	m_QuickCache[QUICK_CACHE_MAPTILE] = cache;

	cache.dwBitmapIndexMin = BITMAP_MAPGRASS_BEGIN;
	cache.dwBitmapIndexMax = BITMAP_MAPGRASS_END;
	cache.dwRange = cache.dwBitmapIndexMax - cache.dwBitmapIndexMin;
	cache.ppBitmap = new BITMAP_t * [cache.dwRange];
	memset(cache.ppBitmap, 0, cache.dwRange * sizeof(BITMAP_t*));
	m_QuickCache[QUICK_CACHE_MAPGRASS] = cache;

	cache.dwBitmapIndexMin = BITMAP_WATER_BEGIN;
	cache.dwBitmapIndexMax = BITMAP_WATER_END;
	cache.dwRange = cache.dwBitmapIndexMax - cache.dwBitmapIndexMin;
	cache.ppBitmap = new BITMAP_t * [cache.dwRange];
	memset(cache.ppBitmap, 0, cache.dwRange * sizeof(BITMAP_t*));
	m_QuickCache[QUICK_CACHE_WATER] = cache;

	cache.dwBitmapIndexMin = BITMAP_CURSOR_BEGIN;
	cache.dwBitmapIndexMax = BITMAP_CURSOR_END;
	cache.dwRange = cache.dwBitmapIndexMax - cache.dwBitmapIndexMin;
	cache.ppBitmap = new BITMAP_t * [cache.dwRange];
	memset(cache.ppBitmap, 0, cache.dwRange * sizeof(BITMAP_t*));
	m_QuickCache[QUICK_CACHE_CURSOR] = cache;

	cache.dwBitmapIndexMin = BITMAP_FONT_BEGIN;
	cache.dwBitmapIndexMax = BITMAP_FONT_END;
	cache.dwRange = cache.dwBitmapIndexMax - cache.dwBitmapIndexMin;
	cache.ppBitmap = new BITMAP_t * [cache.dwRange];
	memset(cache.ppBitmap, 0, cache.dwRange * sizeof(BITMAP_t*));
	m_QuickCache[QUICK_CACHE_FONT] = cache;

	cache.dwBitmapIndexMin = BITMAP_INTERFACE_NEW_MAINFRAME_BEGIN;
	cache.dwBitmapIndexMax = BITMAP_INTERFACE_NEW_MAINFRAME_END;
	cache.dwRange = cache.dwBitmapIndexMax - cache.dwBitmapIndexMin;
	cache.ppBitmap = new BITMAP_t * [cache.dwRange];
	memset(cache.ppBitmap, 0, cache.dwRange * sizeof(BITMAP_t*));
	m_QuickCache[QUICK_CACHE_MAINFRAME] = cache;

	cache.dwBitmapIndexMin = BITMAP_INTERFACE_NEW_SKILLICON_BEGIN;
	cache.dwBitmapIndexMax = BITMAP_INTERFACE_NEW_SKILLICON_END;
	cache.dwRange = cache.dwBitmapIndexMax - cache.dwBitmapIndexMin;
	cache.ppBitmap = new BITMAP_t * [cache.dwRange];
	memset(cache.ppBitmap, 0, cache.dwRange * sizeof(BITMAP_t*));
	m_QuickCache[QUICK_CACHE_SKILLICON] = cache;

	cache.dwBitmapIndexMin = BITMAP_PLAYER_TEXTURE_BEGIN;
	cache.dwBitmapIndexMax = BITMAP_PLAYER_TEXTURE_END;
	cache.dwRange = cache.dwBitmapIndexMax - cache.dwBitmapIndexMin;
	cache.ppBitmap = new BITMAP_t * [cache.dwRange];
	memset(cache.ppBitmap, 0, cache.dwRange * sizeof(BITMAP_t*));
	m_QuickCache[QUICK_CACHE_PLAYER] = cache;

	m_pNullBitmap = new BITMAP_t;
	memset(m_pNullBitmap, 0, sizeof(BITMAP_t));

	m_ManageTimer.SetTimer(1500);

	return true;
}

void CBitmapCache::Release()
{
	SAFE_DELETE(m_pNullBitmap);
	RemoveAll();

	for (int i = 0; i < NUMBER_OF_QUICK_CACHE; i++)
	{
		SAFE_DELETE_ARRAY(m_QuickCache[i].ppBitmap);
	}

	memset(m_QuickCache, 0, sizeof(QUICK_CACHE) * NUMBER_OF_QUICK_CACHE);
}

void CBitmapCache::Add(GLuint uiBitmapIndex, BITMAP_t* pBitmap)
{
	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		if(uiBitmapIndex > m_QuickCache[i].dwBitmapIndexMin && uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
		{
			DWORD dwVI = uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin;
			if(pBitmap)
				m_QuickCache[i].ppBitmap[dwVI] = pBitmap;
			else
				m_QuickCache[i].ppBitmap[dwVI] = m_pNullBitmap;
			return;
		}
	}
	if(pBitmap)
	{
		if(BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
			m_mapCachePlayer.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
		else if(BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
			m_mapCacheInterface.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
		else if(BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
			m_mapCacheEffect.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
		else
			m_mapCacheMain.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
	}
}

void CBitmapCache::Remove(GLuint uiBitmapIndex)
{
	for (int i = 0; i < NUMBER_OF_QUICK_CACHE; i++)
	{
		if (uiBitmapIndex > m_QuickCache[i].dwBitmapIndexMin && uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
		{
			DWORD dwVI = uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin;
			m_QuickCache[i].ppBitmap[dwVI] = NULL;
			return;
		}
	}
	if (BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && uiBitmapIndex <= BITMAP_PLAYER_TEXTURE_END)
	{
		auto mi = m_mapCachePlayer.find(uiBitmapIndex);
		if (mi != m_mapCachePlayer.end())
			m_mapCachePlayer.erase(mi);
	}
	else if (BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && uiBitmapIndex <= BITMAP_INTERFACE_TEXTURE_END)
	{
		auto mi = m_mapCacheInterface.find(uiBitmapIndex);
		if (mi != m_mapCacheInterface.end())
			m_mapCacheInterface.erase(mi);
	}
	else if (BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && uiBitmapIndex <= BITMAP_EFFECT_TEXTURE_END)
	{
		auto mi = m_mapCacheEffect.find(uiBitmapIndex);
		if (mi != m_mapCacheEffect.end())
			m_mapCacheEffect.erase(mi);
	}
	else
	{
		auto mi = m_mapCacheMain.find(uiBitmapIndex);
		if (mi != m_mapCacheMain.end())
			m_mapCacheMain.erase(mi);
	}
}

void CBitmapCache::RemoveAll()
{
	for (int i = 0; i < NUMBER_OF_QUICK_CACHE; i++)
	{
		memset(m_QuickCache[i].ppBitmap, 0, m_QuickCache[i].dwRange * sizeof(BITMAP_t*));
	}
	m_mapCacheMain.clear();
	m_mapCachePlayer.clear();
	m_mapCacheInterface.clear();
	m_mapCacheEffect.clear();
}

size_t CBitmapCache::GetCacheSize()
{
	return m_mapCacheMain.size() + m_mapCachePlayer.size() +
		m_mapCacheInterface.size() + m_mapCacheEffect.size();
}

void CBitmapCache::Update()
{
	m_ManageTimer.UpdateTime();
	if (m_ManageTimer.IsTime())
	{
		type_cache_map::iterator mi = m_mapCacheMain.begin();
		while (mi != m_mapCacheMain.end())
		{
			BITMAP_t* pBitmap = mi->second;
			if (pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				++mi;
			}
			else
			{
				mi = m_mapCacheMain.erase(mi);
			}
		}

		mi = m_mapCachePlayer.begin();
		while (mi != m_mapCachePlayer.end())
		{
			BITMAP_t* pBitmap = mi->second;

			if (pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				++mi;
			}
			else
			{
				mi = m_mapCachePlayer.erase(mi);
			}
		}

		mi = m_mapCacheInterface.begin();
		while (mi != m_mapCacheInterface.end())
		{
			BITMAP_t* pBitmap = mi->second;
			if (pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				++mi;
			}
			else
			{
				mi = m_mapCacheInterface.erase(mi);
			}
		}

		mi = m_mapCacheEffect.begin();
		while (mi != m_mapCacheEffect.end())
		{
			BITMAP_t* pBitmap = mi->second;
			if (pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				++mi;
			}
			else
			{
				mi = m_mapCacheEffect.erase(mi);
			}
		}
#ifdef DEBUG_BITMAP_CACHE
		g_ConsoleDebug->Write(MCD_NORMAL, "M,P,I,E : (%d, %d, %d, %d)", m_mapCacheMain.size(),
			m_mapCachePlayer.size(), m_mapCacheInterface.size(), m_mapCacheEffect.size());
#endif // DEBUG_BITMAP_CACHE
	}
}

bool CBitmapCache::Find(GLuint uiBitmapIndex, BITMAP_t** ppBitmap)
{
	for (int i = 0; i < NUMBER_OF_QUICK_CACHE; i++)
	{
		if (uiBitmapIndex > m_QuickCache[i].dwBitmapIndexMin &&
			uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
		{
			DWORD dwVI = uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin;
			if (m_QuickCache[i].ppBitmap[dwVI])
			{
				if (m_QuickCache[i].ppBitmap[dwVI] == m_pNullBitmap)
					*ppBitmap = NULL;
				else
				{
					*ppBitmap = m_QuickCache[i].ppBitmap[dwVI];
				}
				return true;
			}
			return false;
		}
	}
	if (BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCachePlayer.find(uiBitmapIndex);
		if (mi != m_mapCachePlayer.end())
		{
			*ppBitmap = mi->second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	else if (BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheInterface.find(uiBitmapIndex);
		if (mi != m_mapCacheInterface.end())
		{
			*ppBitmap = mi->second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	else if (BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheEffect.find(uiBitmapIndex);
		if (mi != m_mapCacheEffect.end())
		{
			*ppBitmap = mi->second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	else
	{
		type_cache_map::iterator mi = m_mapCacheMain.find(uiBitmapIndex);
		if (mi != m_mapCacheMain.end())
		{
			*ppBitmap = mi->second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	return false;
}

CGlobalBitmap::CGlobalBitmap()
{
	Init();
	m_BitmapCache.Create();

#ifdef DEBUG_BITMAP_CACHE
	m_DebugOutputTimer.SetTimer(5000);
#endif // DEBUG_BITMAP_CACHE
}
CGlobalBitmap::~CGlobalBitmap() 
{ 
	UnloadAllImages();
}

void CGlobalBitmap::Init()
{
	m_uiAlternate = 0;
	m_uiTextureIndexStream = BITMAP_NONAMED_TEXTURES_BEGIN;
	m_dwUsedTextureMemory = 0;
}

GLuint CGlobalBitmap::LoadImage(const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	BITMAP_t* pBitmap = FindTexture(filename);
	if (pBitmap && pBitmap->Ref > 0 && _stricmp(pBitmap->FileName, filename.c_str()) == 0)
	{
		pBitmap->Ref++;
		return pBitmap->BitmapIndex;
	}
	else
	{
		GLuint uiNewTextureIndex = GenerateTextureIndex();
		if (LoadImage(uiNewTextureIndex, filename, uiFilter, uiWrapMode))
		{
			m_listNonamedIndex.push_back(uiNewTextureIndex);
			return uiNewTextureIndex;
		}
	}
	return BITMAP_UNKNOWN;
}

bool CGlobalBitmap::LoadImage(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode) 
{
	unsigned int UICLAMP = GL_CLAMP_TO_EDGE;
	unsigned int UIREPEAT = GL_REPEAT;

	if (uiWrapMode != UICLAMP && uiWrapMode != UIREPEAT)
	{
#ifdef _DEBUG
		static unsigned int uiCnt2 = 0;
		int iBuff;
		char szDebugOutput[256];
		iBuff = 0;
		
		iBuff = iBuff + sprintf(szDebugOutput + iBuff, "%d. Call No CLAMP & No REPEAT. \n", uiCnt2++);
		OutputDebugString(szDebugOutput);
#endif
	}

	type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);
	if (mi != m_mapBitmap.end())
	{
		BITMAP_t* pBitmap = mi->second;
		if (pBitmap->Ref > 0 && _stricmp(pBitmap->FileName, filename.c_str()) == 0)
		{
			pBitmap->Ref++;
			return true;
		}
		else
		{
			g_ErrorReport.Write("File not found %s (%d)->%s\r\n", pBitmap->FileName, uiBitmapIndex, filename.c_str());
			UnloadImage(uiBitmapIndex, true);
		}
	}
	
	std::string ext;
	SplitExt(filename, ext, false);

	if (_stricmp(ext.c_str(), "jpg") == 0)
		return OpenJpeg(uiBitmapIndex, filename, uiFilter, uiWrapMode);
	else if (_stricmp(ext.c_str(), "tga") == 0)
		return OpenTga(uiBitmapIndex, filename, uiFilter, uiWrapMode);

	return false;
}

void CGlobalBitmap::UnloadImage(GLuint uiBitmapIndex, bool bForce)
{
	type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);
	if (mi != m_mapBitmap.end())
	{
		BITMAP_t* pBitmap = mi->second;
		if (--pBitmap->Ref == 0 || bForce)
		{
			glDeleteTextures(1, &(pBitmap->TextureNumber));

			m_dwUsedTextureMemory -= static_cast<DWORD>(pBitmap->Width * pBitmap->Height * pBitmap->Components);

			delete[] pBitmap->Buffer;
			delete pBitmap;
			m_mapBitmap.erase(mi);

			if (uiBitmapIndex >= BITMAP_NONAMED_TEXTURES_BEGIN && uiBitmapIndex <= BITMAP_NONAMED_TEXTURES_END)
			{
				m_listNonamedIndex.remove(uiBitmapIndex);
			}
			m_BitmapCache.Remove(uiBitmapIndex);
		}
	}
}

void CGlobalBitmap::UnloadAllImages()
{
#ifdef _DEBUG
	if (!m_mapBitmap.empty())
		g_ErrorReport.Write("Unload Images\r\n");
#endif // _DEBUG
	for (auto& pair : m_mapBitmap)
	{
		BITMAP_t* pBitmap = pair.second;
#ifdef _DEBUG
		if (pBitmap->Ref > 1)
		{
			g_ErrorReport.Write("Bitmap %s(RefCount= %d)\r\n", pBitmap->FileName, pBitmap->Ref);
		}
#endif // _DEBUG
		delete[] pBitmap->Buffer;
		delete pBitmap;
	}

	m_mapBitmap.clear();
	m_listNonamedIndex.clear();
	m_BitmapCache.RemoveAll();

	Init();
}

BITMAP_t* CGlobalBitmap::GetTexture(GLuint uiBitmapIndex)
{
	BITMAP_t* pBitmap = nullptr;
	if (!m_BitmapCache.Find(uiBitmapIndex, &pBitmap))
	{
		auto mi = m_mapBitmap.find(uiBitmapIndex);
		if (mi != m_mapBitmap.end())
			pBitmap = mi->second;
		m_BitmapCache.Add(uiBitmapIndex, pBitmap);
	}
	if (pBitmap == nullptr)
	{
		static BITMAP_t s_Error;
		memset(&s_Error, 0, sizeof(BITMAP_t));
		strcpy(s_Error.FileName, "CGlobalBitmap::GetTexture Error!!!");
		pBitmap = &s_Error;
	}
	return pBitmap;
}

BITMAP_t* CGlobalBitmap::FindTexture(GLuint uiBitmapIndex)
{
	BITMAP_t* pBitmap = NULL;
	if(false == m_BitmapCache.Find(uiBitmapIndex, &pBitmap))
	{
		type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);
		if(mi != m_mapBitmap.end())
			pBitmap = (*mi).second;
		if(pBitmap != NULL)
			m_BitmapCache.Add(uiBitmapIndex, pBitmap);
	}
	return pBitmap;
}

BITMAP_t* CGlobalBitmap::FindTexture(const std::string& filename)
{
	type_bitmap_map::iterator mi = m_mapBitmap.begin();
	for (auto mi = m_mapBitmap.begin(); mi != m_mapBitmap.end(); ++mi)
	{
		BITMAP_t* pBitmap = mi->second;
		if (0 == stricmp(filename.c_str(), pBitmap->FileName))
			return pBitmap;
	}
	return nullptr;
}

BITMAP_t* CGlobalBitmap::FindTextureByName(const std::string& name)
{
	for (auto mi = m_mapBitmap.begin(); mi != m_mapBitmap.end(); ++mi)
	{
		BITMAP_t* pBitmap = mi->second;
		std::string texname;
		SplitFileName(pBitmap->FileName, texname, true);
		if (0 == stricmp(texname.c_str(), name.c_str()))
			return pBitmap;
	}
	return nullptr;
}

DWORD CGlobalBitmap::GetUsedTextureMemory() const
{
	return m_dwUsedTextureMemory;
}

size_t CGlobalBitmap::GetNumberOfTexture() const
{
	return m_mapBitmap.size();
}

void CGlobalBitmap::Manage()
{
#ifdef DEBUG_BITMAP_CACHE
	m_DebugOutputTimer.UpdateTime();
	if (m_DebugOutputTimer.IsTime())
	{
		g_ConsoleDebug->Write(MCD_NORMAL, "CacheSize=%zu(NumberOfTexture=%zu)", m_BitmapCache.GetCacheSize(), GetNumberOfTexture());
}
#endif // DEBUG_BITMAP_CACHE
	m_BitmapCache.Update();
}

GLuint CGlobalBitmap::GenerateTextureIndex()
{
	GLuint uiAvailableTextureIndex = FindAvailableTextureIndex(m_uiTextureIndexStream);
	if (uiAvailableTextureIndex >= BITMAP_NONAMED_TEXTURES_END)
	{
		m_uiAlternate++;
		m_uiTextureIndexStream = BITMAP_NONAMED_TEXTURES_BEGIN;
		uiAvailableTextureIndex = FindAvailableTextureIndex(m_uiTextureIndexStream);
	}
	m_uiTextureIndexStream = uiAvailableTextureIndex;
	return m_uiTextureIndexStream;
}

GLuint CGlobalBitmap::FindAvailableTextureIndex(GLuint uiSeed)
{
	if (m_uiAlternate > 0)
	{
		type_index_list::iterator li = std::find(m_listNonamedIndex.begin(), m_listNonamedIndex.end(), uiSeed + 1);
		if (li != m_listNonamedIndex.end())
			return FindAvailableTextureIndex(uiSeed + 1);
	}
	return uiSeed + 1;
}

bool CGlobalBitmap::OpenJpeg(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	std::string filename_ozj;
	ExchangeExt(filename, "OZJ", filename_ozj);
	FILE* infile = fopen(filename_ozj.c_str(), "rb");
	if (infile == NULL)
	{
		return false;
	}

	fseek(infile, 24, SEEK_SET);	//. Skip Dump

	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer))
	{
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return false;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	(void)jpeg_read_header(&cinfo, TRUE);
	(void)jpeg_start_decompress(&cinfo);

	if (cinfo.output_width <= MAX_WIDTH && cinfo.output_height <= MAX_HEIGHT)
	{
		int Width = 1, Height = 1;
		while (Width < cinfo.output_width)
		{
			Width <<= 1;
		}
		while (Height < cinfo.output_height)
		{
			Height <<= 1;
		}

		BITMAP_t* pNewBitmap = new BITMAP_t;
		memset(pNewBitmap, 0, sizeof(BITMAP_t));

		pNewBitmap->BitmapIndex = uiBitmapIndex;

		strncpy_s(pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, filename.c_str(), MAX_BITMAP_FILE_NAME);

		pNewBitmap->Width = (float)Width;
		pNewBitmap->Height = (float)Height;
		pNewBitmap->Components = 3;
		pNewBitmap->Ref = 1;

		size_t BufferSize = Width * Height * pNewBitmap->Components;
		pNewBitmap->Buffer = new BYTE[BufferSize];
		m_dwUsedTextureMemory += BufferSize;

		int offset = 0;
		int row_stride = cinfo.output_width * cinfo.output_components;
		JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
		while (cinfo.output_scanline < cinfo.output_height)
		{
			if (offset + row_stride > (int)BufferSize)
				break;

			(void)jpeg_read_scanlines(&cinfo, buffer, 1);
			memcpy(pNewBitmap->Buffer + (cinfo.output_scanline - 1) * Width * 3, buffer[0], row_stride);
			offset += row_stride;
		}

		m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, pNewBitmap));

		glGenTextures(1, &(pNewBitmap->TextureNumber));

		glBindTexture(GL_TEXTURE_2D, pNewBitmap->TextureNumber);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pNewBitmap->Buffer);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiFilter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiFilter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uiWrapMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uiWrapMode);
	}
	(void)jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);
	return true;
}

bool CGlobalBitmap::OpenTga(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	std::string filename_ozt;
	ExchangeExt(filename, "OZT", filename_ozt);

	FILE* fp = fopen(filename_ozt.c_str(), "rb");
	if (fp == NULL)
	{
		return false;
	}

	fseek(fp, 0, SEEK_END);
	int Size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	unsigned char* PakBuffer = new unsigned char[Size];
	fread(PakBuffer, 1, Size, fp);
	fclose(fp);
	
	int index = 12;
	index += 4;
	short nx = *((short*)(PakBuffer + index)); index += 2;
	short ny = *((short*)(PakBuffer + index)); index += 2;
	char bit = *((char*)(PakBuffer + index)); index += 1;
	index += 1;

	if (bit != 32 || nx > MAX_WIDTH || ny > MAX_HEIGHT)
	{
		SAFE_DELETE_ARRAY(PakBuffer);
		return false;
	}

	int Width = 0, Height = 0;
	for (int i = 1; i <= MAX_WIDTH; i <<= 1)
	{
		Width = i;
		if (i >= nx) break;
	}
	for (int i = 1; i <= MAX_HEIGHT; i <<= 1)
	{
		Height = i;
		if (i >= ny) break;
	}

	BITMAP_t* pNewBitmap = new BITMAP_t;
	memset(pNewBitmap, 0, sizeof(BITMAP_t));
	
	pNewBitmap->BitmapIndex = uiBitmapIndex;

	filename._Copy_s(pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);

	pNewBitmap->Width = (float)Width;
	pNewBitmap->Height = (float)Height;
	pNewBitmap->Components = 4;
	pNewBitmap->Ref = 1;
	
	size_t BufferSize = Width * Height * pNewBitmap->Components;
	pNewBitmap->Buffer = (unsigned char*)new BYTE[BufferSize];

	m_dwUsedTextureMemory += BufferSize;

    for(int y=0;y<ny;y++)
	{
		unsigned char* src = &PakBuffer[index];
		index += nx * 4;
		unsigned char* dst = &pNewBitmap->Buffer[(ny - 1 - y) * Width * pNewBitmap->Components];

		for (int x = 0; x < nx; x++)
		{
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst[3] = src[3];
			src += 4;
			dst += pNewBitmap->Components;
		}
	}
	SAFE_DELETE_ARRAY(PakBuffer);

	m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, pNewBitmap));

	glGenTextures( 1, &(pNewBitmap->TextureNumber));

	glBindTexture(GL_TEXTURE_2D, pNewBitmap->TextureNumber);

    glTexImage2D(GL_TEXTURE_2D, 0, 4, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pNewBitmap->Buffer);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiFilter);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiFilter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uiWrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uiWrapMode);

	return true;
}

void CGlobalBitmap::SplitFileName(const std::string& filepath, std::string& filename, bool bIncludeExt)
{
	char __fname[_MAX_FNAME] = { 0 };
	char __ext[_MAX_EXT] = { 0 };
	_splitpath(filepath.c_str(), nullptr, nullptr, __fname, __ext);
	filename = __fname;
	if (bIncludeExt)
		filename += __ext;
}

void CGlobalBitmap::SplitExt(const std::string& filepath, std::string& ext, bool bIncludeDot)
{
	char __ext[_MAX_EXT] = { 0 };
	_splitpath(filepath.c_str(), nullptr, nullptr, nullptr, __ext);
	if (bIncludeDot) {
		ext = __ext;
	}
	else {
		if ((__ext[0] == '.') && __ext[1])
			ext = __ext + 1;
	}
}

void CGlobalBitmap::ExchangeExt(const std::string& in_filepath, const std::string& ext, std::string& out_filepath)
{
	char __drive[_MAX_DRIVE] = { 0 };
	char __dir[_MAX_DIR] = { 0 };
	char __fname[_MAX_FNAME] = { 0 };
	char __ext[_MAX_EXT] = { 0 };
	_splitpath(in_filepath.c_str(), __drive, __dir, __fname, __ext);
	out_filepath = __drive;
	out_filepath += __dir;
	out_filepath += __fname;
	out_filepath += '.';
	out_filepath += ext;
}

bool CGlobalBitmap::Convert_Format(const unicode::t_string& filename)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	::_splitpath(filename.c_str(), drive, dir, fname, ext);

	std::string strPath = drive; strPath += dir;
	std::string strName = fname;

	std::string strSaveName;

	if (_stricmp(ext, ".jpg") == 0)
	{
		strSaveName = strPath + strName + ".OZJ";
		return Save_Image(filename, strSaveName.c_str(), 24);
	}
	else if (_stricmp(ext, ".tga") == 0)
	{
		strSaveName = strPath + strName + ".OZT";
		return Save_Image(filename, strSaveName.c_str(), 4);
	}
	else if (_stricmp(ext, ".bmp") == 0)
	{
		strSaveName = strPath + strName + ".OZB";
		return Save_Image(filename, strSaveName.c_str(), 4);
	}

	return false;
}

bool CGlobalBitmap::Save_Image(const unicode::t_string& src, const unicode::t_string& dest, int cDumpHeader)
{
	FILE* fp = fopen(src.c_str(), "rb");
	if (fp == NULL)
	{
		return false;
	}
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* pTempBuf = new char[size];
	fread(pTempBuf, 1, size, fp);
	fclose(fp);

	fp = fopen(dest.c_str(), "wb");
	if (fp == NULL)
	{
		delete[] pTempBuf;
		return false;
	}

	fwrite(pTempBuf, 1, cDumpHeader, fp);
	fwrite(pTempBuf, 1, size, fp);
	fclose(fp);

	delete[] pTempBuf;

	return true;
}

void CGlobalBitmap::my_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr)cinfo->err;
	(*cinfo->err->output_message)(cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}
