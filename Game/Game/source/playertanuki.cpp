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

    // 前フレーム位置を保存（マップ当たり判定などで使用）
    _vOldPos = _vPos;

    // 前フレームのステータス保存
    CharaBase::STATUS old_status = _status;

    // カメラの向き（ターゲットから見た位置ベクトル）
    float sx = _cam->_vPos.x - _cam->_vTarget.x;
    float sz = _cam->_vPos.z - _cam->_vTarget.z;
    float camrad = atan2(sz, sx);   // カメラの「後ろ」方向

    // 左スティック値（どこかで fLx / fLz を更新している前提）
    lStickX = fLx;
    lStickZ = fLz;

    // --- 1. ローカル空間（カメラ基準の前後左右）で入力を作る ---
    vec::Vec3 inputLocal = vec3::VGet(0.0f, 0.0f, 0.0f);

    // キーボード入力（WASD/カーソルキー的な前後左右）
    if(CheckHitKey(KEY_INPUT_UP))
    {
        inputLocal.z -= 1.0f;   // 前
    }
    if(CheckHitKey(KEY_INPUT_DOWN))
    {
        inputLocal.z += 1.0f;   // 後
    }
    if(CheckHitKey(KEY_INPUT_LEFT))
    {
        inputLocal.x -= 1.0f;   // 左
    }
    if(CheckHitKey(KEY_INPUT_RIGHT))
    {
        inputLocal.x += 1.0f;   // 右
    }

    // 左スティック入力
    float stickLen = sqrtf(lStickX * lStickX + lStickZ * lStickZ);
    if(stickLen >= _fAnalogDeadZone)
    {
        // スティック方向を単位ベクトルにして足し込む（キーボードと合成）
        inputLocal.x += lStickX / stickLen;
        inputLocal.z += lStickZ / stickLen;
    }

    // --- 2. ローカル入力ベクトルを元にワールド移動ベクトル _vInput を計算 ---
    _vInput = vec3::VGet(0.0f, 0.0f, 0.0f);
    _v = vec3::VGet(0.0f, 0.0f, 0.0f);  // 最後の移動処理用

    float inputLen = vec3::VSize(inputLocal);
    if(inputLen > 0.0f)
    {
        // ローカル入力方向（正規化）
        vec::Vec3 dirLocal = vec3::VScale(inputLocal, 1.0f / inputLen);

        // ローカルX,Z を角度に変換（右が0度、手前が+90度のDX準拠）
        float rad = atan2f(dirLocal.z, dirLocal.x);

        float speed = _fMvSpeed;

        // カメラ角度を加味してワールド移動量にする
        // camrad は「カメラの後ろ向きベクトル」の角度なので
        // ここで +camrad してキャラの移動方向をカメラに追従させる
        _vInput.x = cosf(rad + camrad) * speed;
        _vInput.z = sinf(rad + camrad) * speed;

        // 実際に使う速度ベクトル
        _v = _vInput;

        // キャラの向きは速度ベクトルを正規化して使う
        if(vec3::VSize(_vInput) > 0.0f)
        {
            _vDir = vec3::VNorm(_vInput);
        }

        _status = STATUS::WALK;
    }
    else
    {
        _vInput = vec3::VGet(0.0f, 0.0f, 0.0f);
        _v = _vInput;
        // 向きはそのまま維持でもよいので _vDir は更新しない方が自然
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

        if(_animId != -1)
        {
            AnimationManager::GetInstance()->Stop(_animId);
            _animId = -1;
        }

        std::string anim_name;
        switch(_status)
        {
        case STATUS::WAIT:
            anim_name = "hensin";
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