#pragma once
#include "appframe.h"
#include "objectbase.h"

class Treasure : public ObjectBase
{
	typedef ObjectBase base;
public:

	// �󔠂̏��
	enum class OBJSTATUS
	{
		NONE,
		NOMAR,
		OPEN,
	};
	OBJSTATUS _objStatus;

	// ��b����
	bool Initialize() override; // ������
	bool Terminate()  override; // �I��
	bool Process()    override; // �v�Z����
	bool Render()     override; // �`�揈��

	auto GetPos() const { return _vPos; }
	// �󔠂̓����蔻��p�t���[���擾
	auto GetHitCollisionFrame() const { return _hitCollisionFrame; }
	// �󔠊J�p�t���[���擾
	auto GetOpenCollisionFrame() const { return _openCollisionFrame; }
	// ���f���n���h����擾
	auto GetModelHandle() const { return _handle; }

	// �󔠂��J���Ă��邩�ǂ����擾�E�ݒ�
	bool IsOpen() const { return _isOpen; }
	void SetOpen(bool isOpen) { _isOpen = isOpen; }

	// ���݂� _vPos/_vDir/_vScale ���烂�f���s��𐶐�
	MATRIX MakeModelMatrix() const;

protected:
	int _handle;                 // ���f���n���h��
	int _hitCollisionFrame;      // �󔠂̓����蔻��p�t���[��
	int _openCollisionFrame;     // �󔠊J�p�t���[��
	int _attachIndex;            // �A�^�b�`�A�j���[�V�����C���f�b�N�X
	bool _isOpen;	             // �󔠂��J���Ă��邩�ǂ���
};

