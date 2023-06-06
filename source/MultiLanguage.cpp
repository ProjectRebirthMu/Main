// MultiLanguage.cpp: implementation of the CMultiLanguage class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MultiLanguage.h"


CMultiLanguage* CMultiLanguage::ms_Singleton = nullptr;

CMultiLanguage::CMultiLanguage(std::string strSelectedML)
{
    ms_Singleton = this;
    if (strSelectedML == "ENG")
    {
        byLanguage = 0;
        iCodePage = 1252;
        iNumByteForOneCharUTF8 = 2;
    }
    else if (strSelectedML == "POR")
    {
        byLanguage = 1;
        iCodePage = 1252;
        iNumByteForOneCharUTF8 = 2;
    }
    else if (strSelectedML == "SPN")
    {
        byLanguage = 2;
        iCodePage = 1252;
        iNumByteForOneCharUTF8 = 2;
    }
    else
    {
        byLanguage = 0;
        iCodePage = 1252;
        iNumByteForOneCharUTF8 = 2;
    }
}

BYTE CMultiLanguage::GetLanguage()
{
	return byLanguage;
}

int CMultiLanguage::GetCodePage()
{
	return iCodePage;
}

int CMultiLanguage::GetNumByteForOneCharUTF8()
{
	return iNumByteForOneCharUTF8;
}

BOOL CMultiLanguage::IsCharUTF8(const char* pszText)
{
    if (pszText == nullptr || strlen(pszText) <= 0)
        return TRUE;
    const unsigned char* pbyCurr = reinterpret_cast<const unsigned char*>(pszText);
    BOOL bUTF8 = TRUE;

    while (*pbyCurr != 0x00 && bUTF8)
    {
        // U+0000 ~ U+007F (0 ~ 127)
        if ((*pbyCurr & 0x80) == 0x00)
        {
            pbyCurr++;
        }
        // U+0080 ~ U+07FF (128 ~ 2,047)
        else if ((*pbyCurr & 0xE0) == 0xC0 && (*(pbyCurr + 1) & 0xC0) == 0x80)
        {
            pbyCurr += 2;
        }
        // U+0800 ~ U+FFFF (2,048 ~ 65,535)
        else if ((*pbyCurr & 0xF0) == 0xE0 && (*(pbyCurr + 1) & 0xC0) == 0x80 && (*(pbyCurr + 2) & 0xC0) == 0x80)
        {
            pbyCurr += 3;
        }
        // U+10000 ~ U+10FFFF (65,536 ~ 1,114,111)
        else if ((*pbyCurr & 0xF8) == 0xF0 && (*(pbyCurr + 1) & 0xC0) == 0x80 && (*(pbyCurr + 2) & 0xC0) == 0x80 && (*(pbyCurr + 3) & 0xC0) == 0x80)
        {
            pbyCurr += 4;
        }
        // not UTF-8
        else
        {
            bUTF8 = FALSE;
        }
    }
    return bUTF8;
}

int CMultiLanguage::ConvertCharToWideStr(std::wstring& wstrDest, LPCSTR lpString)
{
    wstrDest.clear();
    if (lpString == nullptr || strlen(lpString) <= 0)
        return 0;

    int nLenOfWideCharStr;
    int iConversionType;

    iConversionType = (IsCharUTF8(lpString)) ? CP_UTF8 : iCodePage;

    // calculate the number of characters needed to hold the wide-character version of the string.
    nLenOfWideCharStr = MultiByteToWideChar(iConversionType, 0, lpString, -1, nullptr, 0);
    if (nLenOfWideCharStr == 0)
        return 0;

    // memory allocation
    wchar_t* pwszStr = new wchar_t[nLenOfWideCharStr];

    // convert the multi-byte string to a wide-character string.
    int result = MultiByteToWideChar(iConversionType, 0, lpString, -1, pwszStr, nLenOfWideCharStr);
    if (result == 0)
    {
        delete[] pwszStr;
        return 0;
    }

    // assign
    wstrDest.assign(pwszStr);

    // release the allocated memory.
    delete[] pwszStr;

    return result - 1;
}

int CMultiLanguage::ConvertWideCharToStr(std::string& strDest, LPCWSTR lpwString, int iConversionType)
{
    strDest.clear();
    if (lpwString == nullptr || wcslen(lpwString) <= 0)
        return 0;

    int nLenOfWideCharStr;

    // calculate the number of characters needed to hold the wide-character version of the string.
    nLenOfWideCharStr = WideCharToMultiByte(iConversionType, 0, lpwString, -1, nullptr, 0, 0, 0);
    if (nLenOfWideCharStr == 0)
        return 0;

    // memory allocation
    char* pszStr = new char[nLenOfWideCharStr];

    // convert the wide-character string to a multi-byte string.
    int result = WideCharToMultiByte(iConversionType, 0, lpwString, -1, pszStr, nLenOfWideCharStr, 0, 0);
    if (result == 0)
    {
        delete[] pszStr;
        return 0;
    }

    // assign
    strDest.assign(pszStr);

    // release the allocated memory.
    delete[] pszStr;

    return result - 1;
}

void CMultiLanguage::ConvertANSIToUTF8OrViceVersa(std::string& strDest, LPCSTR lpString)
{
    std::wstring wstrUTF16;
    int iDestType = (IsCharUTF8(lpString)) ? CP_ACP : CP_UTF8;
    ConvertCharToWideStr(wstrUTF16, lpString);
    ConvertWideCharToStr(strDest, wstrUTF16.c_str(), iDestType);
}


int CMultiLanguage::GetClosestBlankFromCenter(const std::wstring wstrTarget)
{
    int iLength = wstrTarget.length();
    std::wstring wstrText1 = wstrTarget.substr(iLength / 2, std::wstring::npos);
    std::wstring wstrText2 = wstrTarget.substr(0, iLength / 2);

    int iPosLastBlankFromFirstHalf = wstrText2.find_last_of(L" ");
    int iPosFirstBlankFromSecondHalf = wstrText1.find_first_of(L" ", 1);
    int iClosestBlankFromCenter = 0;

    if (iPosLastBlankFromFirstHalf == std::wstring::npos && iPosFirstBlankFromSecondHalf == std::wstring::npos)
    {
        iClosestBlankFromCenter = iLength / 2;
    }
    else if (iPosLastBlankFromFirstHalf == std::wstring::npos)
    {
        iClosestBlankFromCenter = iPosFirstBlankFromSecondHalf + iLength / 2;
    }
    else if (iPosFirstBlankFromSecondHalf == std::wstring::npos)
    {
        iClosestBlankFromCenter = iPosLastBlankFromFirstHalf;
    }
    else if (iPosFirstBlankFromSecondHalf <= iLength / 2 - iPosLastBlankFromFirstHalf)
    {
        iClosestBlankFromCenter = iPosFirstBlankFromSecondHalf + iLength / 2;
    }
    else
    {
        iClosestBlankFromCenter = iPosLastBlankFromFirstHalf;
    }

    return iClosestBlankFromCenter;
}

BOOL CMultiLanguage::_GetTextExtentPoint32(HDC hdc, LPCWSTR lpString, int cbString, LPSIZE lpSize)
{
    return GetTextExtentPoint32W(hdc, lpString, cbString, lpSize);
}

BOOL CMultiLanguage::_GetTextExtentPoint32(HDC hdc, LPCSTR lpString, int cbString, LPSIZE lpSize)
{
    std::wstring wstrText;
    ConvertCharToWideStr(wstrText, lpString);
    return GetTextExtentPoint32W(hdc, wstrText.c_str(), static_cast<int>(wstrText.length()), lpSize);
}

BOOL CMultiLanguage::_TextOut(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, int cbString)
{
    return TextOutW(hdc, nXStart, nYStart, lpString, cbString);
}

BOOL CMultiLanguage::_TextOut(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cbString)
{
    std::wstring wstrText = L"";
    ConvertCharToWideStr(wstrText, lpString);
    return TextOutW(hdc, nXStart, nYStart, wstrText.c_str(), static_cast<int>(wstrText.length()));
}

WPARAM CMultiLanguage::ConvertFulltoHalfWidthChar(DWORD wParam)
{
    wchar_t Char = static_cast<wchar_t>(wParam);
    if (Char >= L'！' && Char <= L'～')
        wParam -= 0xFEE0;
    else if (Char == L'　')
        wParam = L' ';

    return wParam;
}