/*********************************************************************/
// * \file   playertanuki.cpp
// * \brief  �K��ԃN���X
// *
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
	//_vDir = vec3::VGet(0.0f, 0.0f, -1.0f);// �L�������f���̓f�t�H���g��-Z����������Ă���
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
	if(!_cam) return false;
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
		inputLocal.z = 1.0f;
	}
	if(CheckHitKey(KEY_INPUT_DOWN))
	{
		inputLocal.z = -1.0f;
	}
	if(CheckHitKey(KEY_INPUT_LEFT))
	{
		inputLocal.x = -1.0f;
	}
	if(CheckHitKey(KEY_INPUT_RIGHT))
	{
		inputLocal.x = 1.0f;
	}

	// �X�e�B�b�N�̌X������ړ��ʂ�v�Z
	//_vInput = inputLocal;


	//float length = sqrtf(lStickX * lStickX + lStickZ * lStickZ);



	// �X�e�B�b�N�̌X������ړ��ʂ�v�Z

	// ��i�ŎQ�Ƃ��Ă����`�ɂȂ�Ȃ��悤�ɁA�����Ő錾���Ă���
		// �X�e�B�b�N�̌X������ړ��ʂ�v�Z
	float length = sqrtf(lStickX * lStickX + lStickZ * lStickZ);

	// ��i�ŎQ�Ƃ��Ă����`�ɂȂ�Ȃ��悤�ɁA�����Ő錾���Ă���
	float localRad = 0.0f;

	// �f�b�h�]�[����ׂ�����u�|���������v�Ɉړ��A�f�b�h�]�[�������Ȃ�~�܂�
	if(length >= _fAnalogDeadZone)
	{
		// �X�e�B�b�N�����i���[�J���j
		// �E=+X�A��=+Z �ɂȂ�悤�ɕϊ��iY���͎g��Ȃ��j
		// ���㉺���]�������ꍇ�� -lStickZ �� +lStickZ �ɕς��Ă�������
		const float moveX = lStickZ;
		const float moveZ = lStickX;

		// ���͕���(����)��ۑ��iCharaBase �� GetInputVector �p�j
		_vInput = vec3::VGet(moveX, 0.0f, moveZ);

		// ���͕����̊p�x�i���[�J����ԁj
		localRad = atan2f(moveZ, moveX);

		// ��葬�x�ňړ��i�|����ő��x��ς������Ȃ� length ��g���j
		const float speed = _fMvSpeed;

		// �J�����p�ŉ�]�������[���h�ړ���
		_v.x = cosf(localRad + camrad) * length;
		_v.z = sinf(localRad + camrad) * length;

		_vDir = _v;
		_status = STATUS::WALK;
	}
	else
	{
		// �f�b�h�]�[���F�����Ȃ�
		_v = { 0.0f, 0.0f, 0.0f };
		_vInput = vec3::VGet(0.0f, 0.0f, 0.0f);
		_status = STATUS::WAIT;
	}

	// �����͏�ňړ��v�Z�ς݂Ȃ̂Łu��d�v�Z�v����Ȃ�
	// �i�㉺���]�⏟��ɍ��W�����������ɂȂ��Ă����j
#if 0
	// �J���������ɍ��킹�Ĉړ��ʂ�v�Z
	if(length > 0.0f)
	{
		float localRad = atan2f(inputLocal.z, inputLocal.x);

		length = _fMvSpeed;
		_v.x = cosf(localRad + camrad) * length;
		_v.z = sinf(localRad + camrad) * length;

		_vDir = _v;
		_status = STATUS::WALK;
	}
	else
	{
		_v = vec3::VGet(0.0f, 0.0f, 0.0f);
		_status = STATUS::WAIT;
	}
#endif


	//float length = sqrt(lStickX * lStickX + lStickZ * lStickZ);

	//if(length < _fAnalogDeadZone)
	//{
	//	length = 0.0f;
	//}

	// �J���������ɍ��킹�Ĉړ��ʂ�v�Z
	// �J���������ɍ��킹�Ĉړ��ʂ�v�Z
	//if(length > 0.0f)
	//{
	//	// localRad������`�̂��߁A�����ōČv�Z����
	//	float localRad = atan2f(inputLocal.z, inputLocal.x);

	//	length = _fMvSpeed;
	//	_v.x = cosf(localRad + camrad) * length;
	//	_v.z = sinf(localRad + camrad) * length;

	//	_vDir = _v;
	//	_status = STATUS::WALK;
	//}
	//else
	//{
	//	_status = STATUS::WAIT;
	//}

	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	// �A�j���[�V�������擾�p�����_
	auto GetAnimName = [this](STATUS name) -> std::string
		{
			switch(name)
			{
			case STATUS::WAIT:
				return "taiki";
			case STATUS::WALK:
				return "walk";
			default:
				return std::string();
			}
		};

	// �A�j���[�V�����Đ��p�����_
	auto PlayAnim = [&](bool change)
		{
			std::string name = GetAnimName(_status);
			if(name.empty()) { return; }

			_animId = AnimationManager::GetInstance()->Play(_handle, name, true);
			_fPlayTime = 0.0f;

			// �X�e�[�^�X�ύX���̓����_���ōĐ����Ԃ���炷
			if(change)
			{
				switch(_status)
				{
				case STATUS::WAIT:
					_fPlayTime += rand() % 30;
					break;
				}
			}

			if(_animId != -1)
			{
				AnimationManager::GetInstance()->SetTime(_animId, _fPlayTime);
			}
		};

	// --- �A�j���[�V�����Ǘ� ---
	// �Đ����̃A�j���[�V�������I�����Ă��邩�`�F�b�N�i�񃋁[�v�Đ��� AnimationManager ���C���X�^���X��폜����j
	if(_animId != -1 && !AnimationManager::GetInstance()->IsPlaying(_animId))
	{
		_animId = -1;
		PlayAnim(false);
	}

	//�����̃A�j���Ǘ��i�X�e�[�^�X�ω����̏����j
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

		PlayAnim(true);// �X�e�[�^�X�ύX���Ȃ̂�true
	}

	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}


	// --- �����Ŏ��ۂɈʒu�ƃJ������ړ������� ---
	//if(vec3::VSize(_v) > 0.0f)
	//{
	//	//// �v���C���[�̈ʒu��ړ�
	//	_vPos = vec3::VAdd(_vPos, _v);

	//	//// �J�������ݒ肳��Ă���΃J�����ʒu�̓v���C���[�ʒu + �I�t�Z�b�g�Őݒ�i���Z�͂��Ȃ��j
	//	//if(_cam != nullptr)
	//	//{
	//	//	_cam->_vPos = vec3::VAdd(_vPos, _camOffset);
	//	//	_cam->_vTarget = vec3::VAdd(_vPos, _camTargetOffset);
	//	//}
	//}
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

	DrawFormatString(10, 90, GetColor(255, 255, 0), "fLx=%.3f fLz=%.3f", fLx, fLz);

	return true;
}