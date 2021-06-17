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

using namespace std;

/*
	https://iq.opengenus.org/b-tree-search-insert-delete-operations/
	https://cstack.github.io/db_tutorial/parts/part7.html
	https://www.cnblogs.com/nullzx/p/8729425.html
	https://www.programiz.com/dsa/b-plus-tree

	B+ 树的性质
		1. All leaves are at the same level.
		2. The root has at least two children.	(这里根节点只有 1 个 key)
		3. Each node except root can have a maximum of m children and at least m/2 children.
		4. Each node can contain a maximum of m - 1 keys and a minimum of ⌈m/2⌉ - 1 keys.
 */

#define MAX	3

class Node {
public:
	Node(): is_leaf(false), size(0) {
		memset(children_or_sibling, 0, sizeof(children_or_sibling));
	}

private:
	bool is_leaf;
	int key[MAX];						// all keys
	int size;							// valid keys size
	Node* children_or_sibling[MAX+1];	// 如果是中间节点，那这个就是children; 如果是叶子节点，那这个就是 sibling

	friend class BPlusTree;
};


class BPlusTree {
public:
	BPlusTree();
	virtual ~BPlusTree();

	void Insert(int x);
	bool Search(int x);
	void Remove(int x);
	void Display();
	Node* Root();

private:
	void _InsertInternal(int x, Node* p_parent, Node* p_child);
	Node* _FindParent(Node* p_cursor, Node* p_child);
	void _RemoveInternal(int x, Node* p_cursor, Node* p_child);

private:
	Node* _root;
};

#endif /* BPLUSTREE_H_ */
