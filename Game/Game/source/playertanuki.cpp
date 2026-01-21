/*********************************************************************/
// * \file   playertanuki.cpp
// * \brief  �K��ԃN���X
// *
// * \author ��ؗT�H
// * \date   2025/12/15
// * \��Ɠ�e: �V�K�쐬 ��ؗT�H�@2025/12/15
/*********************************************************************/

#include "playertanuki.h"
#include "appframe.h"

// ������
bool PlayerTanuki::Initialize()
{
	if(!base::Initialize()) { return false; }
	
	_handle = MV1LoadModel("res/Tanuki/anime_goepon_walk.mv1");
	_iAttachIndex = -1;
	// �X�e�[�^�X��u�����v�ɐݒ�
	_status = STATUS::NONE;
	// �Đ����Ԃ̏�����
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// �ʒu�A�����̏�����
	_vPos = vec3::VGet(0.0f, 0.0f, 0.0f);
	_vDir = vec3::VGet(0.0f, 0.0f, -1.0f);// �L�������f���̓f�t�H���g��-Z����������Ă���
	// ���ʒu�̐ݒ�
	_fColSubY = 40.0f;
	// �R���W�������a�̐ݒ�
	_fCollisionR = 30.0f;
	_fCollisionWeight = 20.0f;
	_cam = nullptr;
	_fMvSpeed = 6.0f;

	_bLand = true;

	return true;
}

// �I��
bool PlayerTanuki::Terminate()
{
	base::Terminate();

	return true;
}

// �v�Z����
bool PlayerTanuki::Process()
{
	base::Process();

	int key = ApplicationBase::GetInstance()->GetKey();

	// �����O�̈ʒu��ۑ�
	_vOldPos = _vPos;

	// �����O�̃X�e�[�^�X��ۑ����Ă���
	CharaBase::STATUS old_status = _status;
	// �ړ���������߂�
	_v = { 0,0,0 };

	// �J�����̌����Ă���p�x��擾
	float sx = _cam->_vPos.x - _cam->_vTarget.x;
	float sz = _cam->_vPos.z - _cam->_vTarget.z;
	float camrad = atan2(sz, sx);
	

	//���X�e�B�b�N�l
	lStickX = fLx;
	lStickZ = fLz;

	// �L�����ړ�(�J�����ݒ�ɍ��킹��)
	VECTOR inputLocal = VGet(0.0f, 0.0f, 0.0f);
	if (CheckHitKey(KEY_INPUT_UP))
	{
		lStickZ = -1.0f;
	}
	if (CheckHitKey(KEY_INPUT_DOWN))
	{
		lStickZ = 1.0f;
	}
	if (CheckHitKey(KEY_INPUT_LEFT))
	{
		lStickX = -1.0f;
	}
	if (CheckHitKey(KEY_INPUT_RIGHT))
	{
		lStickX = 1.0f;
	}

	// ���[�J�����̓x�N�g����v�Z
	float length = sqrt(lStickX * lStickX + lStickZ * lStickZ);
	float rad = atan2(lStickX, lStickZ);

	// �f�b�h�]�[������
	if (length < _fAnalogDeadZone)
	{
		length = 0.0f;
	}

	// �A�i���O�ړ��x�N�g���i�J�������΁j
	if (length > 0.0f)
	{
		// ���x�͈��i�K�v�Ȃ� length �𑬓x�X�P�[���ɂł���j
		length = _fMvSpeed;
		_v.x = cosf(rad + camrad) * length;
		_v.z = sinf(rad + camrad) * length;

		// �����X�V�i�ړ������j
		_vDir = _v;

		_status = STATUS::WALK;
	}
	else
	{
		_status = STATUS::WAIT;
	}

	// �A�j���[�V�����Ǘ�
	if(old_status == _status)
	{
		float anim_speed = 0.5f;
		_fPlayTime += anim_speed;
		switch(_status)
		{
		case STATUS::WAIT:
			_fPlayTime += (float)(rand() % 10) / 100.0f;
			break;
		}
	}
	else
	{
		if(_iAttachIndex != -1)
		{
			MV1DetachAnim(_handle, static_cast<int>(_iAttachIndex));
			_iAttachIndex = -1;
		}
		switch(_status)
		{
		case STATUS::WAIT:
			_iAttachIndex = static_cast<float>(MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "goepon_walk"), -1, FALSE));
			break;
		case STATUS::WALK:
			_iAttachIndex = static_cast<float>(MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "goepon_walk"), -1, FALSE));
			break;
		}
		_fTotalTime = static_cast<float>(MV1GetAttachAnimTotalTime(_handle, static_cast<int>(_iAttachIndex)));
		_fPlayTime = 0.0f;
		switch(_status)
		{
		case STATUS::WAIT:
			_fPlayTime += static_cast<float>(rand() % 30);
			break;
		}
	}

	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	// --- �����Ŏ��ۂɈʒu�ƃJ������ړ������� ---
	if(vec3::VSize(_v) > 0.0f)
	{
		// �v���C���[�̈ʒu��ړ�
		_vPos = vec3::VAdd(_vPos, _v);

		// �J�������ݒ肳��Ă���΃J�����ʒu�̓v���C���[�ʒu + �I�t�Z�b�g�Őݒ�i���Z�͂��Ȃ��j
		if(_cam != nullptr)
		{
			_cam->_vPos = vec3::VAdd(_vPos, _camOffset);
			_cam->_vTarget = vec3::VAdd(_vPos, _camTargetOffset);
		}
	}

	return true;
}

// �`�揈��
bool PlayerTanuki::Render()
{
	base::Render();

	// �Đ����Ԃ�Z�b�g����
		// �Đ����Ԃ�Z�b�g����
	MV1SetAttachAnimTime(_handle, static_cast<int>(_iAttachIndex), static_cast<float>(_fPlayTime));

	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);// ���f�����W���łǂ��������Ă��邩�Ŏ����ς��(�����-z������Ă���ꍇ)

	MATRIX mRotY = MGetRotY(vorty);

	MATRIX mRotZ = MGetRotZ(DX_PI_F * 0.5f); // -90�x�i�K�v�ɉ����ĕ����𔽓]�j

	MATRIX mTrans = MGetTranslate(VectorConverter::VecToDxLib(_vPos));

	MATRIX mScale = MGetScale(VGet(1.0f, 1.0f, 1.0f));

	MATRIX m = MGetIdent();

	//m = MMult(m, mRotZ);
	m = MMult(m, mRotY);
	m = MMult(m, mScale);
	m = MMult(m, mTrans);

	MV1SetMatrix(_handle, m);

	// �`��
	MV1DrawModel(_handle);
	
	return true;
}