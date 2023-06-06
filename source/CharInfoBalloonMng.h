//*****************************************************************************
// File: CharInfoBalloonMng.h
//
// Desc: interface for the CCharInfoBalloonMng class.
//		 ĳ���� ���� ǳ�� ���� Ŭ����.(ĳ���� ���þ����� ����)
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_CHARINFOBALLOONMNG_H__37129186_F7FE_4FBC_87BD_189E01191E8F__INCLUDED_)
#define AFX_CHARINFOBALLOONMNG_H__37129186_F7FE_4FBC_87BD_189E01191E8F__INCLUDED_

#pragma once

class CCharInfoBalloon;

class CCharInfoBalloonMng  
{
protected:
	CCharInfoBalloon*	m_pCharInfoBalloon;	// ĳ���� ���� ǳ�� �迭�� �ּ�.

public:
	CCharInfoBalloonMng();
	virtual ~CCharInfoBalloonMng();

	void Release();
	void Create();
	void Render();
	void UpdateDisplay();
};

#endif // !defined(AFX_CHARINFOBALLOONMNG_H__37129186_F7FE_4FBC_87BD_189E01191E8F__INCLUDED_)
