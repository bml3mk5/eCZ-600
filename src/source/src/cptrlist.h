/** @file cptrlist.h

	@author Sasaji
	@date   2014.12.01

	@brief 汎用ポインタリスト
*/

#ifndef CPTRLIST_H
#define CPTRLIST_H

#include "common.h"
#include <vector>

/// @brief 汎用ポインタリストクラス
template <class TYPE>
class CPtrList
{
protected:
	std::vector<TYPE *> items;
public:
	CPtrList();
	CPtrList(int alloc_size);
//	CPtrList(const CPtrList<TYPE> &src);
	virtual ~CPtrList();

//	virtual CPtrList<TYPE> &operator=(CPtrList<TYPE> &src);

	virtual void Add(TYPE *value);
	virtual void Delete(int num);
	virtual TYPE *EraseFromList(int num);
	virtual void Insert(int new_num, TYPE *new_item);
	virtual void Replace(int num, TYPE *new_item);
	virtual void Exchange(int num1, int num2);

	virtual void Clear();
	virtual int  Count() const;

	virtual TYPE *Item(int num);
	virtual const TYPE *Item(int num) const;
	virtual TYPE *operator[](int num);
	virtual const TYPE *operator[](int num) const;

protected:
	virtual void CopyData(const TYPE *src_item, TYPE *dst_item);

};

///
template <class TYPE>
CPtrList<TYPE>::CPtrList()
{
}

template <class TYPE>
CPtrList<TYPE>::CPtrList(int alloc_size)
{
	items.reserve(alloc_size);
}

template <class TYPE>
CPtrList<TYPE>::~CPtrList()
{
	Clear();
}

#if 0
/// @brief コピーコンストラクタ
/// @param[in] src コピー元
template <class TYPE>
CPtrList<TYPE>::CPtrList(const CPtrList<TYPE> &src)
{
	this->items = src.items;
}
#endif

#if 0
/// @brief 代入
/// @param[in] src コピー元
/// @return 代入先
template <class TYPE>
CPtrList<TYPE> &CPtrList<TYPE>::operator=(CPtrList<TYPE> &src)
{
	typename std::vector<TYPE *>::iterator it;
	this->Clear();
	for(it = src.items.begin(); it != src.items.end(); it++) {
		TYPE *item = new TYPE(*it);
		this->Add(item);
	}
	return *this;
}
#endif

/// @brief リストに追加する
/// @param[in] value 新値
template <class TYPE>
void CPtrList<TYPE>::Add(TYPE *value)
{
	items.push_back(value);
}

/// @brief リストから削除する
/// @param[in] num インデックス
template <class TYPE>
void CPtrList<TYPE>::Delete(int num)
{
	typename std::vector<TYPE *>::iterator it;
	int i;
	for(i = 0, it = items.begin(); it != items.end(); i++, it++) {
		if (i == num) {
			delete *it;
			it = items.erase(it);
			break;
		}
	}
}

/// @brief リストから削除する＆アイテム自体はdeleteしない
/// @param[in] num インデックス
/// @return 削除したアイテム or NULL
template <class TYPE>
TYPE *CPtrList<TYPE>::EraseFromList(int num)
{
	typename std::vector<TYPE *>::iterator it;
	int i;
	TYPE *p = NULL;
	for(i = 0, it = items.begin(); it != items.end(); i++, it++) {
		if (i == num) {
			p = *it;
			it = items.erase(it);
			break;
		}
	}
	return p;
}

/// @brief リストに指定したインデックスに挿入する
/// @param[in] new_num インデックス
/// @param[in] new_item 新アイテム
template <class TYPE>
void CPtrList<TYPE>::Insert(int new_num, TYPE *new_item)
{
	typename std::vector<TYPE *>::iterator it;
	int i;
	for(i = 0, it = items.begin(); it != items.end(); i++, it++) {
		if (i == new_num) {
			it = items.insert(it, new_item);
			break;
		}
	}
}

/// @brief アイテムを置き換える
/// @param[in] num インデックス
/// @param[in] new_item 新アイテム
template <class TYPE>
void CPtrList<TYPE>::Replace(int num, TYPE *new_item)
{
	TYPE *item = items.at(num);
	CopyData(item, new_item);
	delete item;
	items.at(num) = new_item;
}

/// @brief アイテムを入れ替える
/// @param[in] num1 インデックス
/// @param[in] num2 インデックス
template <class TYPE>
void CPtrList<TYPE>::Exchange(int num1, int num2)
{
	TYPE *item = items.at(num2);
	items.at(num2) = items.at(num1);
	items.at(num1) = item;
}

/// @brief アイテムをコピー(Replaceで呼ばれる)
/// @param[in]     src_item 旧アイテム
/// @param[in,out] dst_item 新アイテム
template <class TYPE>
void CPtrList<TYPE>::CopyData(const TYPE *src_item, TYPE *dst_item)
{
}

/// @brief リストをクリア
template <class TYPE>
void CPtrList<TYPE>::Clear()
{
	typename std::vector<TYPE *>::iterator it;
	for(it = items.begin(); it != items.end(); it++) {
		delete *it;
	}
	items.clear();
}

/// @brief リストの件数を返す
/// @return 件数
template <class TYPE>
int CPtrList<TYPE>::Count() const
{
	return (int)items.size();
}

/// @brief 値を取得
/// @param[in] num インデックス
/// @return 値
template <class TYPE>
TYPE *CPtrList<TYPE>::Item(int num)
{
	return items.at(num);
}

/// @brief 値を取得
/// @param[in] num インデックス
/// @return 値
template <class TYPE>
const TYPE *CPtrList<TYPE>::Item(int num) const
{
	return items.at(num);
}

/// @brief 値を取得
/// @param[in] num インデックス
/// @return 値
template <class TYPE>
TYPE *CPtrList<TYPE>::operator[](int num)
{
	return items.at(num);
}

/// @brief 値を取得
/// @param[in] num インデックス
/// @return 値
template <class TYPE>
const TYPE *CPtrList<TYPE>::operator[](int num) const
{
	return items.at(num);
}

#endif /* CPTRLIST_H */
