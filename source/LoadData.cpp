// LoadData.cpp: implementation of the CLoadData class.
// Revisado: 14/07/23 19:39 GMT-3
// By: Qubit
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoadData.h"
#include "GlobalBitmap.h"
#include "ZzzBMD.h"
#include "ZzzTexture.h"

CLoadData gLoadData;

CLoadData::CLoadData() // OK
{}

CLoadData::~CLoadData() // OK
{}

void CLoadData::AccessModel(int Type, char* Dir, char* FileName, int i)
{
    char Name[64];
    if (i == -1)
        sprintf(Name, "%s.bmd", FileName);
    else if (i < 10)
        sprintf(Name, "%s0%d.bmd", FileName, i);
    else
        sprintf(Name, "%s%d.bmd", FileName, i);

    bool Success = Models[Type].Open2(Dir, Name);

    if (!Success && (strcmp(FileName, "Monster") == 0 || strcmp(FileName, "Player") == 0 || strcmp(FileName, "PlayerTest") == 0 || strcmp(FileName, "Angel") == 0))
    {
        char Text[256];
        sprintf(Text, "%s file does not exist.", Name);
        MessageBoxA(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
    }
}

void CLoadData::OpenTexture(int Model, char* SubFolder, int Wrap, int Type, bool Check)
{
    BMD* pModel = &Models[Model];
    for (int i = 0; i < pModel->NumMeshs; i++)
    {
        Texture_t* pTexture = &pModel->Textures[i];

        char szFullPath[256] = { 0 };
        strcpy(szFullPath, "Data\\");
        strcat(szFullPath, SubFolder);
        strcat(szFullPath, pTexture->FileName);

        char __ext[_MAX_EXT] = { 0 };
        _splitpath(pTexture->FileName, NULL, NULL, NULL, __ext);

        if (tolower(__ext[1]) == 't')
            pModel->IndexTexture[i] = Bitmaps.LoadImage(szFullPath, GL_NEAREST, Wrap);
        else if (tolower(__ext[1]) == 'j')
            pModel->IndexTexture[i] = Bitmaps.LoadImage(szFullPath, Type, Wrap);

        if (pModel->IndexTexture[i] == BITMAP_UNKNOWN)
        {
            BITMAP_t* pBitmap = Bitmaps.FindTextureByName(pTexture->FileName);
            if (pBitmap)
            {
                Bitmaps.LoadImage(pBitmap->BitmapIndex, pBitmap->FileName);
                pModel->IndexTexture[i] = pBitmap->BitmapIndex;
            }
            else
            {
                char szErrorMsg[256] = { 0 };
                sprintf(szErrorMsg, "OpenTexture Failed: %s of %s", szFullPath, pModel->Name);
                MessageBoxA(g_hWnd, szErrorMsg, NULL, MB_OK);
                break;
            }
        }
    }
}
