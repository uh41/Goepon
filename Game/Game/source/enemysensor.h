#pragma once
#include "enemybase.h"
#include "playerbase.h"
#include "MapBase.h"


struct DetectionSector
{
	vec::Vec3 center;    
	vec::Vec3 forward;     
	float radius;       
	float angle;        
};


struct DetectionInfo
{
	bool isDetected;        
	float timer;            

	
	bool isChasing;					
	vec::Vec3 lastKnownPlayerPos;	
	float chaseTimer;				
};

class EnemySensor : public EnemyBase
{
	typedef EnemyBase base;

public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();


	void SetDetectionSector(float radius, float angle);	

	
	bool CheckPlayerDetection(PlayerBase* player);

	
	const DetectionInfo& GetDetectionInfo() const { return _detectionInfo; }

	
	void ResetDetection();

	
	bool IsChasing() const { return _detectionInfo.isChasing; } 
	vec::Vec3 GetLastKnownPlayerPosition() const { return _detectionInfo.lastKnownPlayerPos; }
	float GetChaseTimer() const { return _detectionInfo.chaseTimer; }

	
	void RenderDetectionSector() const;

	
	void RenderDetectionUI() const;

	
	void SetSensorEnabled(bool enabled) { _bSensorEnabled = enabled; }
	bool IsSensorEnabled() const { return _bSensorEnabled; }


	void SetMap(MapBase* map) { _map = map; }

	
	bool CheckFloorExistence(const vec::Vec3& position) const;

	
	bool CheckLineOfSight(const vec::Vec3& startPos, const vec::Vec3& endPos) const;

	
	bool GetFloorYCollision(const vec::Vec3& position, float colSubY, float& outY) const;

	
	bool IsPlayerInDetectionRangeWithCapsule(
		const vec::Vec3& playerPos,
		const vec::Vec3& playerCapsuleTop,
		const vec::Vec3& playerCapsuleBottom,
		float playerCapsuleRadius) const;

protected:
	DetectionSector _detectionSector; 
	bool _bHasDetectionSector; 
	bool _bSensorEnabled;          

	DetectionInfo _detectionInfo;      

	
	static constexpr float DETECTION_DISPLAY_TIME = 0.1f;

	
	static constexpr float CHASE_TIME = 5.0f;

	int _detectionFrameCount;
	static constexpr int DetectionFrame = 5;
	bool _CanDetectionResult;

	
	void UpdateDetectionTimer();		  
	vec::Vec3 GetDetectionCenter() const; 

	MapBase* _map;

	// 描画キャッシュ（const メソッドから更新するため mutable）
	mutable std::vector<std::array<VERTEX3D, 6>> _detectionCachedPolygons;
	mutable int _detectionCacheCounter;

	// キャッシュ再計算インターバル（フレーム）
	static constexpr int DetectionSectorCacheInterval = 5;

	// キャッシュを再計算する（描画用。内部で重い判定を実行）
	void RecalculateDetectionSector() const;
};