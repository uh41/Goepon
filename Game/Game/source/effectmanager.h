/*********************************************************************/
// * \file   effectmanager.h
// * \brief  エフェクト管理クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "appframe.h"
#include "effectpool.h"
#include "hensineffect.h"
#include "walkeffect.h"
#include "findeffect.h"
#include "hatenaeffect.h"
#include "doyaeffect.h"
#include "treasureeffect.h"
#include "nakieffect.h"
#include "shirimochieffect.h"
#include "stuneffect.h"
#include "savepointeffect.h"
#include "makimonogeteffect.h"
#include "goaleffect.h"
#include "treasureopen.h"

class PlayerBase;
class Goal;
class TreasureBase;
class EnemyBase;

class EffectManager
{
public:
	static bool Initialize(); // 初期化
	static bool Terminate();  // 終了処理

	static WalkEffect* GetWalkEffect(); // 足跡エフェクトを取得
	static DoyaEffect* GetDoyaEffect();   // ドヤ顔エフェクトを取得
	static NakiEffect* GetNakiEffect();   // 泣きエフェクトを取得
	static ShirimochiEffect* GetShirimochiEffect(); // しりもちエフェクトを取得
	static SavePointEffect* GetSavePointEffect(); // セーブポイントエフェクトを取得
	static GoalEffect* GetGoalEffect(); // ゴールエフェクトを取得
	static HatenaEffect* GetHatenaEffect(); // はてなエフェクトを取得
	static TreasureEffect* GetTreasureEffect(); // 宝箱エフェクトを取得
	static ShirimochiEffect* GetEnemyReturnEffect();  // 敵が初期位置に戻るエフェクト
	static StunEffect* GetEnemyStunEffect();// 敵のスタンエフェクト
	static TreasureopenEffect* GetTreasureOpenEffect(); // 宝箱オープンエフェクトを取得

	static HensinEffect* PlayeHensinEffect(const vec::Vec3& pos); // 変身エフェクトを取得
	static FindEffect* PlayFindEffect(const vec::Vec3& pos); // 発見エフェクトを取得
	static HatenaEffect* PlayHatenaEffect(EnemyBase* enemy); // はてなエフェクトを取得
	static StunEffect* PlayStunEffect(const vec::Vec3& pos); // スタンエフェクトを取得
	static MakimonoGetEffect* PlayMakimonoGetEffect(const vec::Vec3& pos); // 巻物ゲットエフェクトを取得
	static TreasureopenEffect* PlayTreasureOpenEffect(const vec::Vec3& pos); // 宝箱オープンエフェクトを取得

	static void UpdatePlayerPosition(PlayerBase* player); // プレイヤーの位置を更新する関数
	static void UpdatePalyerTransformEffect(PlayerBase* player, bool playTransform = true);// プレイヤーの位置と向きにエフェクトを合わせる関数

	static void SetGoal(at::spc<Goal> goal); // ゴールを設定する関数
	static void SetInitialPlayer(PlayerBase* player); // 初期プレイヤーを設定する関数
	static void SetTreasure(const at::vspc<TreasureBase>& treasure); // 宝箱を設定する関数

	static void UpdateAllEffect(); // 全てのエフェクトを更新する関数
	//static void PrintPoolStatus(); // プールの状態を表示する関数

private:
	static at::upc<WalkEffect> _walkEffect; // 足跡エフェクト
	static at::upc<DoyaEffect> _doyaEffect;   // ドヤ顔エフェクト
	static at::upc<NakiEffect> _nakiEffect;   // 泣きエフェクト
	static at::upc<ShirimochiEffect> _shirimochiEffect; // しりもちエフェクト
	static at::upc<SavePointEffect> _savePointEffect; // セーブポイントエフェクト
	static at::upc<GoalEffect> _goalEffect; // ゴールエフェクト
	static at::upc<TreasureEffect> _treasureEffect; // 宝箱エフェクト
	static at::upc<TreasureopenEffect> _treasureOpenEffect; // 宝箱オープンエフェクト

	// プール管理
	static at::upc<EffectPool<HensinEffect>> _hensinEffectPool; // 変身エフェクトを再利用
	static at::upc<EffectPool<FindEffect>> _findEffectPool; // 発見エフェクトを再利用
	static at::upc<EffectPool<HatenaEffect>> _hatenaEffectPool; // はてなエフェクトを再利用
	static at::upc<EffectPool<StunEffect>> _stunEffectPool; // スタンエフェクトを再利用
	static at::upc<EffectPool<MakimonoGetEffect>> _makimonoGetEffectPool; // 巻物ゲットエフェクトを再利用

	static bool InitializeSingletonEffect();// シングルトンエフェクトの初期化
	static bool InitializeMultiInstanceEffect();// 複数インスタンスエフェクトの初期化
	static bool InitializePooledEffect();// プールエフェクトの初期化
	static bool TerminateSingletonEffect();// シングルトンエフェクトの終了処理
	static bool TerminateMultiInstanceEffect();// 複数インスタンスエフェクトの終了処理
	static bool TerminatePooledEffect();// プールエフェクトの終了処理
};

