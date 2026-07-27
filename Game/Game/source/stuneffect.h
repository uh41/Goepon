/*********************************************************************/
// * \file   stuneffect.h
// * \brief  スタンエフェクトクラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "effectbase.h"

class StunEffect : public EffectBase
{
	typedef EffectBase base;
public:
	StunEffect();
	virtual ~StunEffect() = default;
	bool Initialize() override;
	bool Terminate() override;
	bool Process() override;
	bool Render() override;

	virtual void PlayEffect(const vec::Vec3& pos) override;// エフェクト再生
	virtual bool PlayeMultiEffect(const at::vet<vec::Vec3>& positions) override;// 複数エフェクト再生
	virtual bool StopPlaying() override;// エフェクト停止
	virtual bool StopMultiEffect() override;// 複数エフェクト停止
};

