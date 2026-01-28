#include "Treasure.h"

// �w���p�[�֐�
namespace
{
	// ���f���s���K�p���A�K�v�Ȃ�R���W��������X�V����
	inline void ApplyMatrixAndRefreshCollInfo(int handle, int hitFrame, int openFrame, const MATRIX& m)
	{
		// �����`�F�b�N
		if(handle < 0)
		{
			return;
		}

		// ���f���s���K�p����
		MV1SetMatrix(handle, m);

		// �R���W��������\�z����
		if(hitFrame >= 0)
		{
			MV1RefreshCollInfo(handle, hitFrame);
		}
		if(openFrame >= 0)
		{
			MV1RefreshCollInfo(handle, openFrame);
		}
	}
}
// ������
bool Treasure::Initialize()
{
	base::Initialize();

	// ���f���ǂݍ���
	_handle = MV1LoadModel("res/Treasure/tuzura_02.mv1");
	if(_handle < 0) { DxLib::printfDx("Treasure model load failed\n"); return false; }

	// �q�b�g�^�I�[�v���p�t���[���̓��f����`�ǂ���Ɍ���
	_hitCollisionFrame = MV1SearchFrame(_handle, "Collision_04");
	_openCollisionFrame = MV1SearchFrame(_handle, "Collision_05");

	// �G���[�`�F�b�N
	if(_hitCollisionFrame < 0) { DxLib::printfDx("HitFrame not found\n"); return false; }
	// Open �͔C�ӂȂ� <0 �ł���s�B�g���Ȃ�`�F�b�N
	// if (_openCollisionFrame < 0) { DxLib::printfDx("OpenFrame not found\n"); }

	// �A�^�b�`�A�j���[�V�����C���f�b�N�X������
	_attachIndex = -1;
	// �󔠂̏�ԏ�����
	_objStatus = OBJSTATUS::NONE;
	// �����ʒu�E�����ݒ�
	_vPos = vec::Vec3{ 600.0f, 0.0f, 2450.0f };
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };

	// �R���W�������̐���
	MV1SetupCollInfo(_handle, _hitCollisionFrame, 16, 16, 16);
	MV1SetFrameVisible(_handle, _hitCollisionFrame, FALSE);
	// �I�[�v���p�t���[�������ΐݒ�
	if(_openCollisionFrame >= 0)
	{
		MV1SetupCollInfo(_handle, _openCollisionFrame, 16, 16, 16);
		MV1SetFrameVisible(_handle, _openCollisionFrame, FALSE);
	}

	// �󔠂͏�����Ԃŕ��Ă���
	_isOpen    = false;
	_isVisible = true;

	// �������f�i�`��Ɣ���ŋ��ʁj
	ApplyMatrixAndRefreshCollInfo(_handle, _hitCollisionFrame, _openCollisionFrame, MakeModelMatrix());
	return true;
}

// �I��
bool Treasure::Terminate()
{
	MV1DeleteModel(_handle);
	return true;
}

bool Treasure::Process()
{
	base::Process();

	// ����̂��߂ɂ���t���[�����f�i�s��ƃR���W�����𓯊��j
	ApplyMatrixAndRefreshCollInfo(_handle, _hitCollisionFrame, _openCollisionFrame, MakeModelMatrix());

	// �󔠂̊J����
	if(!_isOpen && _objStatus != OBJSTATUS::OPEN)
	{
		if(_attachIndex != -1)
		{
			MV1DetachAnim(_handle, _attachIndex);
			_attachIndex = -1;
		}
		_objStatus = OBJSTATUS::OPEN;
	}
	return true;
}

bool Treasure::Render()
{
	base::Render();

	// 非表示なら描画しない（デバック)
	if (!_isVisible)
	{
		return true; 

	}


	if(_handle >= 0)
	{
		// Process()�ōs��͔��f�ς݂Ȃ̂ŁA�`�悾��
		MV1DrawModel(_handle);
	}

	return true;
}

// ���݂� _vPos/_vDir/_vScale ���烂�f���s��𐶐�
MATRIX Treasure::MakeModelMatrix() const
{
	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);
	MATRIX mRotY = MGetRotY(vorty);
	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));
	MATRIX mScale = MGetScale(VGet(1.2f, 1.2f, 1.2f));
	MATRIX m = MGetIdent();
	m = MMult(m, mScale);
	m = MMult(m, mRotY);
	m = MMult(m, mTrans);
	return m;
}