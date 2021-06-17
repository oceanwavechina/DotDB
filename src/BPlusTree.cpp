/*
 * BPlusTree.cpp
 *
 *  Created on: Apr 20, 2021
 *      Author: liuyanan
 */

#include "BPlusTree.h"
#include <queue>
#include <sstream>


BPlusTree::BPlusTree()
: _root(nullptr){
	// TODO Auto-generated constructor stub

}

BPlusTree::~BPlusTree() {
	// TODO Auto-generated destructor stub
}

void BPlusTree::Insert(int x)
{
	//
	//	https://www.programiz.com/dsa/insertion-on-a-b-plus-tree
	//
	//
	//

	cout << "prepare insert: " << x << endl;

	if(!_root) {
		_root = new Node;
		_root->key[0] = x;
		_root->is_leaf = true;
		_root->size = 1;

		cout << "create root" << endl;

	} else  {
		Node* p_cursor_node = _root;
		Node* p_parent = nullptr;

		// 找到目标叶子节点
		while(!p_cursor_node->is_leaf) {
			p_parent = p_cursor_node;

			for(int i=0; i<p_cursor_node->size; ++i) {

				if(x < p_cursor_node->key[i]) {
					p_cursor_node = p_cursor_node->children_or_sibling[i];
					break;
				}

				if(i == p_cursor_node->size-1) {
					p_cursor_node = p_cursor_node->children_or_sibling[i+1];
					break;
				}
			}
		}

		if(p_cursor_node->size < MAX) {
			// 直接在叶子节点上插入

			int target_pos = 0;
			// 1. 所有小于 x 的都在 x 的左边
			while(x > p_cursor_node->key[target_pos] && target_pos < p_cursor_node->size) {
				++target_pos;
			}

			// 2. 所有大于 x 的 都在 x 的右边
			for(int i = p_cursor_node->size; i> target_pos; --i) {
				p_cursor_node->key[i] = p_cursor_node->key[i-1];
			}

			// 3. 把 x 放到目标位置上
			p_cursor_node->key[target_pos] = x;
			++p_cursor_node->size;
			// 这里是处理，叶子节点那个链表的关系
			p_cursor_node->children_or_sibling[p_cursor_node->size] = p_cursor_node->children_or_sibling[p_cursor_node->size-1];
			p_cursor_node->children_or_sibling[p_cursor_node->size-1] = nullptr;

			cout << "Insert at leaf nodes, target pos: " << target_pos << endl;

		} else {
			// 需要 split 叶子节点

			Node* p_new_leaf = new Node;
			p_new_leaf->is_leaf = true;

			// 1. 先创建一个虚拟节点，用来存放key
			int virtual_node[MAX+1];	// 这里只存 p_cursor_node 中的key
			for(int i=0; i<MAX; ++i) {
				virtual_node[i] = p_cursor_node->key[i];
			}

			// 2. 把要插入的 key 放到虚拟节点中正确的位置上
			int target_pos = 0;
			while(x>virtual_node[target_pos] && target_pos < MAX) {
				++ target_pos;
			}
			for(int i=MAX+1; i>target_pos; --i) {
				virtual_node[i] = virtual_node[i-1];
			}
			virtual_node[target_pos] = x;

			// 3. 分配两个叶子节点的数量
			p_cursor_node->size = (MAX + 1) / 2;
			p_new_leaf->size = MAX+1 - p_cursor_node->size;

			// 4. 处理叶子节点的链表关系, 新的叶子节点在右边，也就是新的叶子节点上的数据都是比较大的部分
			p_cursor_node->children_or_sibling[p_cursor_node->size] = p_new_leaf;
			p_new_leaf->children_or_sibling[p_new_leaf->size] = p_cursor_node->children_or_sibling[MAX];
			p_cursor_node->children_or_sibling[MAX] = nullptr;

			// 5. 把virtual_node中的数据，分配到两个叶子节点上
			cout << "on split: old data: ";
			for(int i=0; i<p_cursor_node->size; ++i) {
				p_cursor_node->key[i] = virtual_node[i];
				cout << virtual_node[i] << ",";
			}
			cout << endl;
			cout << "on split: new data: ";
			for(int i=0, j=p_cursor_node->size; i< p_new_leaf->size; ++i, ++j) {
				p_new_leaf->key[i] = virtual_node[j];
				cout << virtual_node[j] << ",";
			}
			cout << endl;

			// 6. 修改父节点, 把拆分的两个节点挂到父节点上
			if(p_cursor_node == _root) {
				Node* p_new_root = new Node;
				p_new_root->key[0] = p_new_leaf->key[0];
				p_new_root->children_or_sibling[0] = p_cursor_node;
				p_new_root->children_or_sibling[1] = p_new_leaf;
				p_new_root->is_leaf = false;
				p_new_root->size = 1;
				_root = p_new_root;
				cout << "create new node" << endl;

			} else {
				_InsertInternal(p_new_leaf->key[0], p_parent, p_new_leaf);
			}

		}

	}
}

bool BPlusTree::Search(int x)
{
	//
	//	https://www.programiz.com/dsa/b-plus-tree
	//
	//	1. 每个 Node 中的 keys 都是从小到大排序的
	//	2. 我们要查找的数据只能在叶子节点中，中间节点都是索引节点，不包含数据
	//  3. 当 x < keys[i] 时，下一次查找的 node，必然是children[i]； 否则在children[i+1， ...] 里边
	//

	if(!_root) {
		return false;
	}

	bool found = false;

	Node* p_cur_node = _root;

	// 现在中间层中找
	while(! p_cur_node->is_leaf) {

		// 当前层级中查找
		// 最终的数据都在叶子节点中，所以我们要遍历整个层级
		for(int i=0; i<p_cur_node->size; ++i) {

			if(x < p_cur_node->key[i]) {
				// 在左边孩子中
				p_cur_node = p_cur_node->children_or_sibling[i];
				break;
			}

			// 如果执行到这里说明要继续往右遍历

			if(i == p_cur_node->size -1) {
				// 最后一个的处理
				p_cur_node = p_cur_node->children_or_sibling[i+1];
			}
		}
	}

	// 然后在数据层中找
	for(int i=0; i< p_cur_node->size; ++i) {

		if(p_cur_node->key[i] == x) {
			found = true;
			break;
		}
	}

	return found;
}

void BPlusTree::Remove(int x) {
}

void BPlusTree::Display()
{
	ostringstream oss;
	oss << "display:\n";

	if(!_root) {
		return;
	}

	queue<Node*> the_queue;
	the_queue.push(_root);

	int cur_lvl_cnt = 1;
	int next_lvl_cnt = 0;

	while(!the_queue.empty()) {
		Node* tmp = the_queue.front();
		the_queue.pop();

		// 先输出当前层的数据
		for(int i=0; i<tmp->size; ++i) {
			oss << tmp->key[i] << ",";
		}
		oss << "    ";
		cur_lvl_cnt -= 1;

		// 把孩子节点入队
		for(int i=0; i< tmp->size+1; ++i) {
			if(tmp->children_or_sibling[i] && !tmp->is_leaf) {
				the_queue.push(tmp->children_or_sibling[i]);
				next_lvl_cnt += 1;
			}
		}

		if(cur_lvl_cnt == 0) {
			cur_lvl_cnt = next_lvl_cnt;
			next_lvl_cnt = 0;
			oss << "\n";
		}
	}
	cout << oss.str() << endl;
}

Node* BPlusTree::Root()
{
	return _root;
}

void BPlusTree::_InsertInternal(int x, Node* p_parent, Node* p_child)
{
	//
	//  x 是要插入的key，
	//

	if(p_parent->size < MAX) {
		// 中间节点不需要分裂, 只需要在 keys 的合适位置插入

		int target_pos = 0;
		while(x>p_parent->key[target_pos] && target_pos < p_parent->size) {
			++target_pos;
		}

		// 给 key 腾地儿
		for(int j=p_parent->key[target_pos]; j>target_pos; --j) {
			p_parent->key[j] = p_parent->key[j-1];
		}

		// 给child pointer 腾地儿
		for(int j=p_parent->size+1; j> target_pos+1; --j) {
			p_parent->children_or_sibling[j] = p_parent->children_or_sibling[j-1];
		}

		p_parent->key[target_pos] = x;
		++ p_parent->size;
		p_parent->children_or_sibling[target_pos+1] = p_child;

	} else {
		// 需要分裂中间节点

		// 1. 先准备待分裂的数据，包括key和children
		int virtual_keys[MAX+1];
		Node* virtual_ptrs[MAX+2];
		for(int i=0; i<MAX; ++i) {
			virtual_keys[i] = p_parent->key[i];
		}
		for(int i=0; i<MAX+1; ++i) {
			virtual_ptrs[i] = p_parent->children_or_sibling[i];
		}

		// 2. 把要要插入的key和新node， 放到virtual的合适位置
		int i=0, j;
		while(x>virtual_keys[i] && i<MAX) {
			++i;
		}
		for(int j=MAX+1; j>i; --j) {
			virtual_keys[j] = virtual_keys[j-1];
		}
		virtual_keys[i] = x;

		for(int j=MAX+2; j>i+1; j--) {
			virtual_ptrs[j] = virtual_ptrs[j-1];
		}
		virtual_ptrs[i+1] = p_child;


		// 创建新的节点
		Node* p_new_internal = new Node;
		p_new_internal->is_leaf = false;

		// 分配下，每个节点存多少个数据
		p_parent->size = (MAX+1) / 2;
		p_new_internal->size = MAX - p_parent->size;

		// 开始拷贝数据
		for(int i=0, j=p_parent->size+1; i<p_new_internal->size; ++i, ++j) {
			p_new_internal->key[i] = virtual_keys[j];
		}
		for(int i=0, j=p_parent->size+1; i< p_new_internal->size+1; ++i, ++j) {
			p_new_internal->children_or_sibling[i] = virtual_ptrs[j];
		}

		if(p_parent == _root) {
			// 根节点直接插入就行
			Node* p_new_root = new Node;
			p_new_root->key[0] = p_parent->key[p_parent->size];
			p_new_root->children_or_sibling[0] = p_parent;
			p_new_root->children_or_sibling[1] = p_new_internal;
			p_new_root->is_leaf = false;
			p_new_root->size = 1;
			_root = p_new_root;

		} else {
			// 递归调用
			_InsertInternal(p_parent->key[p_parent->size], _FindParent(_root, p_parent), p_new_internal);
		}
	}
}

Node* BPlusTree::_FindParent(Node* p_cursor, Node* p_child)
{
	Node* p_parent;

	if(p_cursor->is_leaf || p_cursor->children_or_sibling[0]->is_leaf) {
		return nullptr;
	}

	for(int i=0; i<p_cursor->size+1; ++i) {
		if(p_cursor->children_or_sibling[i] == p_child) {
			p_parent = p_cursor;
			return p_parent;
		} else {
			p_parent = _FindParent(p_cursor->children_or_sibling[i], p_child);
			if(p_parent) {
				return p_parent;
			}
		}
	}

	return nullptr;
}



