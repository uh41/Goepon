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
	
	_handle = MV1LoadModel("res/Tanuki/goepon.mv1");
	_iAttachIndex = -1;
	// �X�e�[�^�X��u�����v�ɐݒ�
	_status = STATUS::NONE;
	// �Đ����Ԃ̏�����
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// �ʒu�A�����̏�����
	//if(vec3::VSize(_vPos) == 0.0f)
	//{
	//	_vPos = vec3::VGet(0.0f, 0.0f, 0.0f); // �����ʒu�����������A�����o���ꏈ���̂������ňʒu�������
	//}
	_vDir = vec3::VGet(0.0f, 0.0f, -1.0f);// �L�������f���̓f�t�H���g��-Z����������Ă���
	// ���ʒu�̐ݒ�
	// �ʒu�A�����̏�����
	_vPos = vec3::VGet(0.0f, 0.0f, 0.0f);
	_vDir = vec3::VGet(0.0f, 0.0f, -1.0f);// �L�������f���̓f�t�H���g��-Z����������Ă���
	// ���ʒu�̐ݒ�
	_fColSubY = 40.0f;
	// �R���W�������a�̐ݒ�
	_fCollisionR = 30.0f;
	_fCollisionWeight = 20.0f;
	_cam = nullptr;
	_fMvSpeed = 10.0f;

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

	// 処理前の位置を保存
	_vOldPos = _vPos;

	// 処理前のステータスを保存しておく
	CharaBase::STATUS old_status = _status;
	//vec::Vec3 v = { 0,0,0 };

	// カメラの向いている角度を取得
	float sx = _cam->_vPos.x - _cam->_vTarget.x;
	float sz = _cam->_vPos.z - _cam->_vTarget.z;
	float camrad = atan2(sz, sx);

	// キャラ移動(カメラ設定に合わせて)
	lStickX = fLx;
	lStickZ = fLz;

	vec::Vec3 inputLocal = vec3::VGet(0.0f, 0.0f, 0.0f);
	if(CheckHitKey(KEY_INPUT_UP))
	{
		lStickZ = -1.0f;
	}
	if(CheckHitKey(KEY_INPUT_DOWN))
	{
		lStickZ = 1.0f;
	}
	if(CheckHitKey(KEY_INPUT_LEFT))
	{
		lStickX = -1.0f;
	}
	if(CheckHitKey(KEY_INPUT_RIGHT))
	{
		lStickX = 1.0f;
	}
	float length = sqrt(lStickX * lStickX + lStickZ * lStickZ);
	float rad = atan2(lStickX, lStickZ);
	if(length < _fAnalogDeadZone)
	{
		length = 0.0f;
	}

	// 入力ベクトルを保存（EscapeCollisionで使用）
	_vInput = inputLocal;

	// カメラ方向に合わせて移動量を計算
	if(length > 0.0f)
	{
		length = _fMvSpeed;
		_v.x = cosf(rad + camrad) * length;
		_v.z = sinf(rad + camrad) * length;

		_vDir = _v;
		_status = STATUS::WALK;
	}
	else
	{
		_v = vec3::VGet(0.0f, 0.0f, 0.0f);
		_status = STATUS::WAIT;
	}

	// アニメーション管理
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

		if(_animId != -1)
		{
			AnimationManager::GetInstance()->Stop(_animId);
			_animId = -1;
		}

		std::string anim_name;
		switch(_status)
		{
			case STATUS::WAIT:
				anim_name = "idle";
				break;
			case STATUS::WALK:
				anim_name = "walk";
				break;
			default:
				anim_name.clear();
		}

		if(!anim_name.empty())
		{
			_animId = AnimationManager::GetInstance()->Play(_handle, anim_name, true);
			_fPlayTime = 0.0f;
			switch(_status)
			{
				case STATUS::WAIT:
					_fPlayTime += rand() % 30;
					break;
			}
			if(_animId != -1)
			{
				AnimationManager::GetInstance()->SetTime(_animId, _fPlayTime);
			}
		}
	}

	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	// --- ここで実際に位置とカメラを移動させる ---
	if(vec3::VSize(_v) > 0.0f)
	{
		// プレイヤーの位置を移動
		_vPos = vec3::VAdd(_vPos, _v);

		// カメラが設定されていればカメラ位置はプレイヤー位置 + オフセットで設定（加算はしない）
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
	//MV1SetAttachAnimTime(_handle, static_cast<int>(_iAttachIndex), static_cast<float>(_fPlayTime));

	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);// ���f�����W���łǂ��������Ă��邩�Ŏ����ς��(�����-z������Ă���ꍇ)

	MATRIX mRotY = MGetRotY(vorty);

	MATRIX mRotZ = MGetRotZ(DX_PI_F * 0.5f); // -90�x�i�K�v�ɉ����ĕ����𔽓]�j

	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));

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