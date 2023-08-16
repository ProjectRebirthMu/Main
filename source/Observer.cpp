#include "stdafx.h"
#include "Observer.h"

CObserver::CObserver()
{}

CObserver::~CObserver()
{}

CSubject::CSubject()
{}

CSubject::~CSubject()
{}

void CSubject::Attach(CObserver* pObserver)
{
	m_ObserverList.AddTail(pObserver);
}

void CSubject::Detach(CObserver* pObserver)
{
	NODE* pPos = m_ObserverList.Find(pObserver);
	if (pPos)
		m_ObserverList.RemoveAt(pPos);
}

void CSubject::Notify()
{
	CObserver* pObserver;
	NODE* pPos = m_ObserverList.GetHeadPosition();
	while (pPos)
	{
		pObserver = (CObserver*)m_ObserverList.GetNext(pPos);
		pObserver->UpdateData(this);
	}
}