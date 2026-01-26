#pragma once
#include "enemybase.h"
#include "playerbase.h"

class Map; // �O���錾

// ���G�͈͂̏���i�[����\����
struct DetectionSector
{
	vec::Vec3 center;      // ��`�̒��S�_�i�G�̈ʒu�j
	vec::Vec3 forward;     // �G�̐��ʕ���
	float radius;       // ���G�͈͂̔��a
	float angle;        // ���G�p�x�i�x�j
};

// ���o��Ԃ̏��
struct DetectionInfo
{
	bool isDetected;        // ���o����Ă��邩
	float timer;            // ���o�\���^�C�}�[
	int detectorIndex;      // ���o�����G�̃C���f�b�N�X
	vec::Vec3 detectorPos;  // ���o�����G�̈ʒu

	// �ǐՋ@�\�p
	bool isChasing;					// ���ݒǐՒ���
	vec::Vec3 lastKnownPlayerPos;	// �Ō�Ɋm�F���ꂽ�v���C���[�̈ʒu
	float chaseTimer;				// �ǐՌp������
};

class EnemySensor : public EnemyBase
{
	typedef EnemyBase base;

public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	// ���G�͈͂̐ݒ�
	void SetDetectionSector(float radius, float angle);	// ���a�A�p�x

	// �v���C���[�����G�͈͓�ɂ��邩�`�F�b�N
	bool IsPlayerInDetectionRange(const vec::Vec3& playerPos) const;

	// �v���C���[�̌��o�`�F�b�N�i�����̃v���C���[�ɑΉ��j
	bool CheckPlayerDetection(PlayerBase* player);

	// ���o���̎擾
	const DetectionInfo& GetDetectionInfo() const { return _detectionInfo; }

	// ���o��Ԃ̃��Z�b�g
	void ResetDetection();

	// �ǐՋ@�\
	bool IsChasing() const { return _detectionInfo.isChasing; }
	vec::Vec3 GetLastKnownPlayerPosition() const { return _detectionInfo.lastKnownPlayerPos; }
	float GetChaseTimer() const { return _detectionInfo.chaseTimer; }

	// �f�o�b�O�p�F���G�͈͂̕`��
	void RenderDetectionSector() const;

	// ���oUI�\��
	void RenderDetectionUI() const;

	// Map�N���X�ւ̎Q�Ƃ�ݒ�
	void SetMap(Map* map) { _map = map; }

	// ���̑��݂�m�F����֐�
	bool CheckFloorExistence(const vec::Vec3& position) const;

	// �����`�F�b�N - �w�肵��2�_�Ԃŏ��Ȃ��̒n�_�����邩�`�F�b�N
	bool CheckLineOfSight(const vec::Vec3& startPos, const vec::Vec3& endPos) const;

	
protected:
	DetectionSector _detectionSector;  // ���G�͈�
	bool _bHasDetectionSector;         // ���G�͈͂��ݒ肳��Ă��邩
	bool _bSensorEnabled;              // �Z���T�[���L�����ǂ���

	DetectionInfo _detectionInfo;      // ���o��Ԃ̏��

	// ���o�֘A�萔
	static constexpr float DETECTION_DISPLAY_TIME = 3.0f; // ���o�\�����ԁi�b�j

	// �ǐՊ֘A�萔
	static constexpr float CHASE_TIME = 5.0f; // �ǐՌp�����ԁi�b�j

	// ��������p���\�b�h
	void UpdateDetectionTimer();		  // ���o�^�C�}�[�̍X�V
	vec::Vec3 GetDetectionCenter() const; // ���G�͈͂̒��S�ʒu��擾

	// ���̑��݂�m�F����֐�
	bool CheckFloorExistence(const vec::Vec3& position) const;

	// �G����v���C���[�܂ł̌o�H��̏����݂�`�F�b�N����֐�
	bool CheckPathFloorExistence(const vec::Vec3& startPos, const vec::Vec3& endPos) const;

	Map* _map;
};