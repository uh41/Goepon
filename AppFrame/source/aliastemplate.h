/*********************************************************************/
// * \file   applicationbase.h
// * \brief  アプリケーションベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#pragma once
#include "container.h"

// エイリアステンプレート
namespace at
{
	template<typename C>
	using vspc = std::vector<std::shared_ptr<C>>;	// ベクターとスマートポインタとクラス

	template<typename C>
	using msc = std::map<std::string, C*>;			// マップとストリングとクラス

	template<typename C>
	using spc = std::shared_ptr<C>;					//スマートポインタとクラス

	template<typename C>
	using upc = std::unique_ptr<C>;				// ユニークポインタとクラス

	template<typename C>
	using vupc = std::vector<std::unique_ptr<C>>; // ベクターとユニークポインタとクラス

	template<typename T, typename C>
	using umtc = std::unordered_map<T, C>;	// アンオーダードマップとストリングとクラス

	template<typename T, typename U>
	using umtt = std::unordered_map<T, U>;	// アンオーダードマップとテンプレート(型)

	template<typename T>
	using ust = std::unordered_set<T>;			// アンオーダードセットとテンプレート(型)

	template<typename T, typename U>
	using umss = std::unordered_map<std::string, std::string>; // アンオーダードマップとストリングとストリング 

	template<typename C, size_t s>
	using arc = std::array<C, s>;					// アレイとクラス

	template<typename T, size_t s>
	using art = std::array<T, s>;					// アレイとテンプレート(型)

	template<typename C, typename D>
	using vpcc = std::vector<std::pair<C, D>>;	// ベクターとペアとクラス

	template<typename C>
	using vec = std::vector<C>;					// ベクターとクラス

	template<typename T>
	using mst = std::map<std::string, T>;		// マップとストリングとテンプレート(型)

	template<typename T>
	using vet = std::vector<T>;					// ベクターとテンプレート(型)

	template<typename C>
	using fc = std::function<C>;				// ファンクションとクラス


}

template<typename T, typename U>
T StCas(U&& u)
{
	return static_cast<T>(std::forward<U>(u));
}
