// CameraMove.h: interface for the CCameraMove class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAMERAMOVE_H__B3723780_481C_43B7_B72C_5D1782D8324C__INCLUDED_)
#define AFX_CAMERAMOVE_H__B3723780_481C_43B7_B72C_5D1782D8324C__INCLUDED_

#pragma once

#include <vector>
#include <string>

class CCameraMove  
{
#pragma pack(push, 1)
	struct WAYPOINT {
		int iIndex;
		
		float fCameraX;
		float fCameraY;
		float fCameraZ;

		int iDelay;
		float fCameraMoveAccel;
		float fCameraDistanceLevel;
	};
#pragma pack(pop)

	typedef std::vector<WAYPOINT*>	t_WayPointList;
	t_WayPointList m_listWayPoint;

	float m_CameraStartPos[3];
	float m_fCameraStartDistanceLevel;
	int m_iDelayCount;
	
	DWORD m_dwCameraWalkState;
	float m_CurrentCameraPos[3];
	float m_fCurrentDistanceLevel;

	DWORD	m_dwCurrentIndex;
	int		m_iSelectedTile;

	void Init();
public:
	enum {
		CAMERAWALK_STATE_READY = 0,
		CAMERAWALK_STATE_MOVE,
		CAMERAWALK_STATE_DONE,
	};
	~CCameraMove();

	bool LoadCameraWalkScript(const std::string& filename);
	void UnLoadCameraWalkScript();
	bool SaveCameraWalkScript(const std::string& filename);

	void AddWayPoint(int iGridX, int iGridY, float fCameraMoveAccel, float fCameraDistanceLevel, int iDelay);
	void RemoveWayPoint(int iGridX, int iGridY);

	void SetCameraMoveAccel(int iTileIndex, float fCameraMoveAccel);
	void SetCameraDistanceLevel(int iTileIndex, float fCameraDistanceLevel);
	void SetDelay(int iTileIndex, int iDelay);
	float GetCameraMoveAccel(int iTileIndex);
	float GetCameraDistanceLevel(int iTileIndex);
	int GetDelay(int iTileIndex);

	bool IsCameraMove() const;
	void UpdateWayPoint();
	void GetCurrentCameraPos(float CameraPos[3]);
	float GetCurrentCameraDistanceLevel() const;

	void PlayCameraWalk(float StartPos[3], float fStartDistanceLevel);
	void StopCameraWalk(bool bDone);
	void UpdateCameraStartPos(float StartPos[3]);

	void SetCameraWalkState(DWORD dwCameraWalkState);
	DWORD GetCameraWalkState() const;

	void RenderWayPoint();
	void SetSelectedTile(int iTileIndex);
	DWORD GetSelectedTile() const;

	static CCameraMove* GetInstancePtr();

public:
	CCameraMove();		//. ban create instance

// tourmode
private:
	BOOL m_bTourMode;
	BOOL m_bTourPause;
	float m_fForceSpeed;
	float m_fTourCameraAngle;
	float m_fTargetTourCameraAngle;
	float m_vTourCameraPos[3];
#ifdef PJH_NEW_SERVER_SELECT_MAP
	float m_fCameraAngle;
	float m_fFrustumAngle;
#endif //PJH_NEW_SERVER_SELECT_MAP
public:
#ifdef PJH_NEW_SERVER_SELECT_MAP
	BOOL SetTourMode(BOOL bFlag, BOOL bRandomStart = FALSE, int _index = 0);
#else //PJH_NEW_SERVER_SELECT_MAP
	BOOL SetTourMode(BOOL bFlag, BOOL bRandomStart = FALSE);
#endif //PJH_NEW_SERVER_SELECT_MAP
	BOOL IsTourMode() { return m_bTourMode; }
	BOOL IsTourPaused() { return m_bTourPause; }
 	void PauseTour(BOOL bFlag);
 	void ForwardTour(float fSpeed);
 	void BackwardTour(float fSpeed);
	void UpdateTourWayPoint();
#ifdef PJH_NEW_SERVER_SELECT_MAP
	void SetAngleFrustum(float _Value);
	const float GetAngleFrustum() const { return m_fCameraAngle; };
	void SetFrustumAngle(float _Value);
	float GetFrustumAngle();
#endif //PJH_NEW_SERVER_SELECT_MAP
	
	float GetCameraAngle() { return m_fTourCameraAngle; }
};

#endif // !defined(AFX_CAMERAMOVE_H__B3723780_481C_43B7_B72C_5D1782D8324C__INCLUDED_)
