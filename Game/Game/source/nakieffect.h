/*********************************************************************/
// * \file   nakieffect.h
// * \brief  泣き顔エフェクトクラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "effectbase.h"

class NakiEffect : public EffectBase
{
	typedef EffectBase base;
public:
	NakiEffect();
	~NakiEffect() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	void PlayEffect(const vec::Vec3& pos);		// エフェクト再生
	void ResetEffect();							// エフェクトのリセット

protected:
	bool _isPlay;								// エフェクトが再生中かどうか
};

