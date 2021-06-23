/*
 * BPlusTree.h
 *
 *  Created on: Apr 20, 2021
 *      Author: liuyanan
 */

#ifndef BPLUSTREE_H_
#define BPLUSTREE_H_

#include <iostream>
#include <vector>
#include <utility>
#include <tuple>

using namespace std;

/*
	https://iq.opengenus.org/b-tree-search-insert-delete-operations/
	https://cstack.github.io/db_tutorial/parts/part7.html
	https://www.cnblogs.com/nullzx/p/8729425.html
	https://www.programiz.com/dsa/b-plus-tree
    
    可视化的 B+树，方便理每一步流程，帮助debug代码
        https://www.cs.usfca.edu/~galles/visualization/BPlusTree.html

	B+ 树的性质
		1. All leaves are at the same level.
		2. The root has at least two children.	(这里根节点只有 1 个 key)
		3. Each node except root can have a maximum of m children and at least m/2 children.
		4. Each node can contain a maximum of m - 1 keys and a minimum of ⌈m/2⌉ - 1 keys.
 */

#define MAX	 2

class Node {
public:
    Node();

    bool IsFull();
    
    bool NeedBorrowOrMerge();
    
    // debug
    void Display(const string& msg);
    string Keys();
    
    int FindDataPosAsLeaf(int x);

	// 返回插入的位置
    int InsertDataAsLeaf(int x);

	// 返回在 p_parent 中插入的位置
	int InsertKeyAsInternal(int x/*要插入的数据*/, Node* p_child/*要插入的孩子节点 */);
    
    bool TryBorrowFromLeftSibling(Node* p_parent, int left_sibling_in_parent);
    bool TryBorrowFromRightSibling(Node* p_parent, int right_sibling_in_parent);
    
    //   返回 x 的 pos
    int RemoveKeyAndChildAsInternal(int x, Node* p_child);
    
    int TryRemoveKeyAsLeaf(int x);
    
    void MergeToLeft(Node* p_left_node);
    
    void MergeFromRight(Node* p_right_node);
    
    // return right sibling if need to continue to search or nullptr
    Node* SearchBetween(int start, int stop, vector<int>& values);

public:
	static const int npos = -1;

private:
	bool is_leaf;
	int key[MAX];						// all keys
	int size;							// valid keys size
	Node* ptrs[MAX+1];	// 如果是中间节点，那这个就是children; 如果是叶子节点，那这个就是 sibling

	friend class BPlusTree;
};


class BPlusTree {
public:
	BPlusTree();
	virtual ~BPlusTree();

	void Insert(int x);
	bool Search(int x);
    bool Between(int start, int stop);
	void Remove(int x);
	void Display(int n_space = 0);
	Node* Root();

private:
	void _InsertInternal(int x, Node* p_parent, Node* p_child);
	Node* _FindParentRecursively(Node* p_cursor, Node* p_child);
	void _RemoveInternal(int x, Node* p_cursor, Node* p_child);

    tuple<Node*/*target*/, Node*/*parent*/> _FindTargetLeafNodeWithParent(int x);
    tuple<Node*/*target*/, Node*/*parent*/, int/*left_sibling*/, int/*right_sibling*/> _FindTargetLeafNodeWithParentAndBrothers(int x);
    
	Node* _SplitLeafNodeWithInsert(Node* p_cursor_node, int x);

	// 这里 p_parent 是待分裂的Node， 即 p_parent 在插入 x 后 这Node就超过最大容量了
	// 分类后的 Node 包含的key是相对较大的部分
	Node* _SplitInternalNodeWithInsert(Node* p_parent, Node* p_child, int x);

private:
	Node* _root;
};

#endif /* BPLUSTREE_H_ */
