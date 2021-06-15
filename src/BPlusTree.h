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
		2. The root has at least two children.
		3. Each node except root can have a maximum of m children and at least m/2 children.
		4. Each node can contain a maximum of m - 1 keys and a minimum of ⌈m/2⌉ - 1 keys.
 */

#define MAX	3

class Node {
public:
	Node(): is_leaf(false), size(0), key(nullptr), children_or_sibling(nullptr) {
		key = new int[MAX];
		children_or_sibling = new Node*[MAX+1];
	}

private:
	bool is_leaf;
	int* key;						// all keys
	int size;						// valid keys size
	Node** children_or_sibling;		// 如果是中间节点，那这个就是children; 如果是叶子节点，那这个就是 sibling

	friend class BPlusTree;
};


class BPlusTree {
public:
	BPlusTree();
	virtual ~BPlusTree();

	void Insert(int x);
	bool Search(int x);
	void Remove(int x);
	void Display(Node* root);
	Node* Root();

private:
	Node* _root;
};

#endif /* BPLUSTREE_H_ */
