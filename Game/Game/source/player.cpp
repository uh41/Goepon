/*********************************************************************/
// * \file   player.cpp
// * \brief  人狸状態クラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//			  : hp追加	鈴木裕稀 2026/01/06
/*********************************************************************/

#include "player.h"
#include "appframe.h"


// ヘルパー関数
namespace
{
	// 空白文字判定
	static bool IsSpace(unsigned char character)
	{
		// 
		return std::isspace(character) != 0 || character == '\r' || character == '\n';
	}

	// std::stringの前方トリム
	static void LTrim(std::string& text)
	{
		size_t i = 0;
		const size_t n = text.size();
		while(i < n && IsSpace(static_cast<unsigned char>(text[i])))++i;
		if(i > 0) text.erase(0, i);
	}
	// std::stringの後方トリム
	static void RTrim(std::string& text)
	{
		if (text.empty()) return;
		size_t i = text.size();
		// 後ろから見て、空白である間だけカウントを下げる
		while (i > 0 && IsSpace(static_cast<unsigned char>(text[i - 1])))
		{
			--i; // 正解はマイナスです
		}
		// 残った文字数（i）以降をすべて削除する
		if (i < text.size())
		{
			text.erase(i);
		}
	}
	// 前後トリム
	static void Trim(std::string& str)
	{
		LTrim(str);
		RTrim(str);
	}
	// 小文字変換
	static std::string ToLower(std::string& input)
	{
		std::string dst = input;
		// 1文字ずつ小文字化
		for(size_t i = 0; i < dst.size(); ++i)
		{
			dst[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(dst[i])));
		}
		return dst;
	}

	//// テキストファイルを key=value 形式で読み込む
	//static std::unordered_map<std::string, std::string> LoadConfigKeyValue(const std::string& path)
	//{
	//	// 最終的なデータを詰め込む変数(result)
	//	std::unordered_map <std::string, std::string> result;
	//	std::ifstream ifs(path);
	//	if(!ifs)return result;

	//	std::string line;
	//	// 1行ずつ読み込み
	//	while(std::getline(ifs, line))
	//	{
	//		Trim(line); // 行の前後にある余計なスペースを消す
	//		// 空行は無視
	//		if(line.empty()) continue;
	//		char first = line[0]; // その行の1番最初の文字を確認
	//		// コメント行は無視
	//		if(first == '#' || first == ';') continue;

	//		size_t pos = line.find('=');
	//		// '='が無い行は無視
	//		if(pos == std::string::npos) continue;     

	//		std::string key   = line.substr(0, pos);
	//		std::string value = line.substr(pos + 1);
	//		Trim(key);
	//		Trim(value);
	//		// keyかvalueが空なら無視
	//		if(key.empty() || value.empty()) continue;

	//		// keyを小文字化して登録
	//		result[ToLower(key)] = value; 
	//	}
	//	return result;
	//}
	// 文字列-> floatの安全バース
	static bool TryParseFloat(const std::string& text, float& outValue)
	{
		if (text.empty()) return false;
		try
		{
			// 第2引数(idx)を使わず、単に数値変換を試みる
			// これにより、後ろに改行やスペースがあっても数値部分だけを抽出してくれます
			outValue = std::stof(text);
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	// 文字列からkey=value をパースして map を返す(CFilと組み合わせて使う){パーサー処理｝
	static std::unordered_map < std::string, std::string> ParseKeyValueConfig(const std::string& content)
	{
		std::unordered_map<std::string, std::string> result;
		std::istringstream iss(content);
		std::string line;
		while (std::getline(iss, line))
		{
			Trim(line);
			if (line.empty()) continue;
			if (line[0] == '#' || line[0] == ';') continue;

			size_t pos = line.find('=');
			if (pos == std::string::npos) continue;

			std::string key = line.substr(0, pos);
			std::string val = line.substr(pos + 1);

			Trim(key);
			Trim(val);
			while (!val.empty() && (val.back() == '\r' || val.back() == '\n')) {
				val.pop_back();
			}

			// keyかvalが空ならスキップ
			if (key.empty() || val.empty()) continue;

			// ここで map に登録されるはずです！
			result[ToLower(key)] = val;
		}
		return result;
	}
}
// 初期化
bool Player::Initialize()
{
	if(!base::Initialize()) { return false; }
	_iHandle = MV1LoadModel("res/Tanuhuman/TanuHuman_Base2.mv1");
	_iAttachIndex = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// 位置、向きの初期化
	_vPos = vec3::VGet(0.0f, 0.0f, 0.0f); // 初期位置が同じだが、押し出され処理のおかげで位置がずれる
	_vDir = vec3::VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
	// 腰位置の設定
	_fColSubY = 40.0f;
	// コリジョン半径の設定
	_fCollisionR = 30.0f;
	_fCollisionWeight = 20.0f;
	_cam = nullptr;
	// 移動速度設定
	_fMvSpeed = 6.0f;

	_iAxisHoldCount = 0;		// 十字キー水平入力保持カウント初期化
	_bAxisUseLock = false;			// 十字キー水平入力ロックフラグ初期化
	_iAxisThreshold = 15;		// 十字キー水平入力保持閾値設定

	_vAxisLockDir = vec3::VGet(0.0f, 0.0f, -1.0f);

	// カメラオフセット初期化
	_camOffset = vec3::VGet(0.0f, 0.0f, 0.0f);
	_camTargetOffset = vec3::VGet(0.0f, 0.0f, 0.0f);

	// 円形移動用パラメータ初期化
	_arc_pivot_dist = 50.0f;    // 回転中心までの距離（調整可）
	_arc_turn_speed = 1.0f;     // 円形移動速度係数（調整可）
	
	_bLand = true;
	
	_fHp = 20.0f; // 初期体力設定

	// 設定ファイルから上書き読み込み
	CFile cfgFile("res/Player/player_config.txt");
	if(cfgFile.Success())
	{
		auto config = ParseKeyValueConfig(cfgFile.DataStr());
		// 移動速度
		auto it = config.find("speed");
		if(it != config.end())
		{
			float val;
			if(TryParseFloat(it->second, val))
			{
				_fMvSpeed = val;
			}
		}
		// 体力
		it = config.find("hp");
		if(it != config.end())
		{
			float val;
			if(TryParseFloat(it->second, val))
			{
				_fHp = val;
			}
		}
	}
	return true;
}

// 終了
bool Player::Terminate()
{
	base::Terminate();
	return true;
}

// 計算処理
bool Player::Process()
{
	base::Process();
	int key = ApplicationBase::GetInstance()->GetKey();
	int trg = ApplicationBase::GetInstance()->GetTrg();

	// 処理前の位置を保存
	_vOldPos = _vPos;

	// 処理前のステータスを保存しておく
	CharaBase::STATUS old_status = _status;
	vec::Vec3 v = { 0,0,0 };
	float length = 0.0f;

	// カメラの向いている角度を取得
	float sx = _cam->_vPos.x - _cam->_vTarget.x;
	float sz = _cam->_vPos.z - _cam->_vTarget.z;
	float camrad = atan2(sz, sx);
	float rad = 0.0f;

	if((key & (PAD_INPUT_7 | PAD_INPUT_8)) == 0)
	{
		// キャラ移動(カメラ設定に合わせて)

        // ローカル入力を取得
        vec::Vec3 inputLocal = vec3::VGet(0.0f, 0.0f, 0.0f);
        if(key & PAD_INPUT_DOWN) { inputLocal.x = 1; }
        if(key & PAD_INPUT_UP)   { inputLocal.x = -1; }
        if(key & PAD_INPUT_LEFT) { inputLocal.z = -1; }
        if(key & PAD_INPUT_RIGHT){ inputLocal.z = 1; }

        // 回転前のローカル入力を保存（斜め判定に使う）
        _vInput = inputLocal;

        // 十字キー保持での軸ロック開始判定
		if(key & PAD_INPUT_6)
		{
			_iAxisHoldCount++;
			if(_iAxisHoldCount >= _iAxisThreshold)
			{
				// ロックに入る瞬間に一度だけロック方向をキャプチャする
				if(!_bAxisUseLock)
				{
					_bAxisUseLock = true;
					
					// 現在の向きをロック方向として保存
					_vAxisLockDir = _vDir;
					_vAxisLockDir.y = 0.0f;
					if(vec3::VSize(_vAxisLockDir) > 0.0f)
					{
						_vAxisLockDir = vec3::VNorm(_vAxisLockDir);
					}
					else
					{
						// 向きが不定の場合はデフォルト方向を採用
						_vAxisLockDir = vec3::VGet(0.0f, 0.0f, -1.0f);
					}
				}
			}
		}
		else
		{
			_iAxisHoldCount = 0;
			_bAxisUseLock = false;
		}

		// 十字キー水平入力ロック処理
		if(_bAxisUseLock)
		{
			// 軸ロック中の移動処理（向き固定で前後左右に移動）
			vec::Vec3 axis_lock_input = vec3::VGet(0.0f, 0.0f, 0.0f);

			// 軸ロック専用の入力を取得
			if(key & PAD_INPUT_DOWN) {
				axis_lock_input.x = 0.5f;
			}
			if(key & PAD_INPUT_UP) {
				axis_lock_input.x = -0.5f;
			}
			if(key & PAD_INPUT_LEFT) {
				axis_lock_input.z = -0.5f;
			}
			if(key & PAD_INPUT_RIGHT) {
				axis_lock_input.z = 0.5f;
			}

			_vInput = axis_lock_input;

			// 入力があれば軸ロック移動を計算
			if(vec3::VSize(axis_lock_input) > 0.0f)
			{
				vec::Vec3 forward = _vAxisLockDir;
				forward.y = 0.0f;
				if(vec3::VSize(forward) > 0.0f)
				{
					forward = vec3::VNorm(forward);
				}
				// 右ベクトル（XZ平面で前方の90度回転）
				VECTOR right = VGet(forward.z, 0.0f, -forward.x);

				// ローカル入力 axis_lock_input.x = 前後(UP/DOWN), axis_lock_input.z = 左右(LEFT/RIGHT)
				float forwardInput = -axis_lock_input.x; // UP = 前進, DOWN = 後退
				float sideInput = axis_lock_input.z;     // RIGHT = 右移動, LEFT = 左移動

				// 移動ベクトルを計算（前方向 + 横方向）
				vec::Vec3 moveDir = vec3::VGet(0.0f, 0.0f, 0.0f);
				moveDir.x = forward.x * forwardInput + right.x * sideInput;
				moveDir.z = forward.z * forwardInput + right.z * sideInput;

				// 移動ベクトルを正規化してから速度を掛ける
				if(vec3::VSize(moveDir) > 0.0f)
				{
					moveDir = vec3::VNorm(moveDir);
					v.x = moveDir.x * _fMvSpeed;
					v.z = moveDir.z * _fMvSpeed;
				}


				// 向きは固定
				_vDir = forward;
			}

		}
        else
        {
            // 通常：カメラ方向に合わせて回転して移動（inputLocal はローカル入力）
            // vをrad分回転させる（ローカル入力の角度）
            if(vec3::VSize(inputLocal) > 0.0f)
            {
                length = _fMvSpeed;
                float localRad = atan2(inputLocal.z, inputLocal.x);
                v.x = cos(localRad + camrad) * length;
                v.z = sin(localRad + camrad) * length;
            }

        }
	}

	// 地上移動
	if(vec3::VSize(v) > 0.0f)
	{
		if(_bAxisUseLock)
		{
			// 向きを固定（移動は v のまま）
			_vDir.x = _vAxisLockDir.x;
			_vDir.z = _vAxisLockDir.z;
		}
		else
		{
			_vDir = v;
		}

		_status = STATUS::WALK;
	}
	else
	{
		_status = STATUS::WAIT;
	}

	// アニメーション時間・アタッチ管理
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
			MV1DetachAnim(_iHandle, static_cast<int>(_iAttachIndex));
			_iAttachIndex = -1;
		}
		switch(_status)
		{
		case STATUS::WAIT:
			_iAttachIndex = static_cast<float>(MV1AttachAnim(_iHandle, MV1GetAnimIndex(_iHandle, "mot_attack_charge_loop"), -1, FALSE));
			break;
		case STATUS::WALK:
			_iAttachIndex = static_cast<float>(MV1AttachAnim(_iHandle, MV1GetAnimIndex(_iHandle, "mot_move_run"), -1, FALSE));
			break;
		}
		_fTotalTime = static_cast<float>(MV1GetAttachAnimTotalTime(_iHandle, static_cast<int>(_iAttachIndex)));
		_fPlayTime = 0.0f;
		switch(_status)
		{
		case STATUS::WAIT:
			_fPlayTime += rand() % 30;
			break;
		}
	}
	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	return true;
}

// 描画処理
bool Player::Render()
{
	base::Render();
	// 再生時間をセットする
	MV1SetAttachAnimTime(_iHandle, static_cast<int>(_iAttachIndex), _fPlayTime);
	
	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);// モデルが標準でどちらを向いているかで式が変わる(これは-zを向いている場合)

	MATRIX mRotY = MGetRotY(vorty);

	//MATRIX mRotZ = MGetRotZ(DX_PI_F * 0.5f); // -90度（必要に応じて符号を反転）

	MATRIX mTrans = MGetTranslate(VectorConverter::VecToDxLib(_vPos));

	MATRIX mScale = MGetScale(VGet(1.0f, 1.0f, 1.0f));

	MATRIX m = MGetIdent();

	//m = MMult(m, mRotZ);
	m = MMult(m, mRotY);
	m = MMult(m, mScale);
	m = MMult(m, mTrans);

	MV1SetMatrix(_iHandle, m);

	// 描画
	MV1DrawModel(_iHandle);

	return true;

}