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
				//	这里的递归调用是自下而上的分列的，
				//		最下边的先分类，多出来的，放到中坚层
				//		中间层需要的话，在继续分列然后向上传递
				_InsertInternal(p_new_leaf->key[0], p_parent, p_new_leaf);
			}

		}	/* if(p_cursor_node->size < MAX) */

	}	/* if(!_root) */
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

void BPlusTree::Remove(int x)
{
	if(!_root) {
		return;
	}

	cout << "prepare remove: " << x;

	Node* p_cursor_node = _root;
	Node* p_parent = nullptr;
	int left_sibling, right_sibling;

	// 1. 先尝试找到包含 x 的那个叶子节点
	while(!p_cursor_node->is_leaf) {

		for(int i=0; i<p_cursor_node->size; ++i) {
			p_parent = p_cursor_node;
			left_sibling = i-1;
			right_sibling = i+1;

			if(x < p_cursor_node->key[i]) {
				p_cursor_node = p_cursor_node->children_or_sibling[i];
				break;
			}

			if(i == p_cursor_node->size - 1) {
				left_sibling = i;
				right_sibling = i+2;
				p_cursor_node = p_cursor_node->children_or_sibling[i+1];
				break;
			}
		}
	}

	// 2. 看看这个叶子节点是否真的包含 x
	int target_pos = -1;
	for(int pos=0; pos<p_cursor_node->size; ++pos) {
		if(p_cursor_node->key[pos] == x) {
			target_pos = pos;
			break;
		}
	}
	if(!target_pos == -1) {
		cout << "not found" << endl;
	}

	// 3. 先从叶子节点上 删除这个key
	for(int i=target_pos; i<p_cursor_node->size; ++i) {
		p_cursor_node->key[i] = p_cursor_node->key[i+1];
	}
	p_cursor_node->size -= 1;

	// 根节点的特殊处理
	if(p_cursor_node == _root) {
		//
		// 	说一下为什么指针都置空:
		//		当 根节点 是 叶子节点 的时候, 他的孩子节点自然后不存在
		//
		for(int i=0; i<MAX+1; ++i) {
			p_cursor_node->children_or_sibling[i] = nullptr;
		}
		if(p_cursor_node->size == 0) {
			cout << "tree died" << endl;
			delete p_cursor_node;
			_root = nullptr;
		}
	}

	// 处理叶子节点链表的串联关系
	p_cursor_node->children_or_sibling[p_cursor_node->size] = p_cursor_node->children_or_sibling[p_cursor_node->size + 1];
	p_cursor_node->children_or_sibling[p_cursor_node->size + 1] = nullptr;

	if(p_cursor_node->size >= (MAX+1) / 2) {
		// 这里是一个node要包含的最少的key的个数
		// 少于这个数值就要合并了
		return;
	}

	/*
	  如下图所示，children 比 key 要多一个

					  k1 | k2 | k3 | k4 | k5
				    /    |    |    |    |    \
				  ch0   ch1  ch2  ch3  ch4   ch5
	*/

	// 以下是合并 **叶子** 节点
	// 合并的原则是从右往左合并
	// 		left  <--  cursor  <--  right

	if(left_sibling >= 0) {
		// 要是左边兄弟的节点多，就从左边兄弟拿一个过来
		// 这里多的判断是 >= (MAX+1) / 2 + 1

		Node* p_left_node = p_parent->children_or_sibling[left_sibling];

		if(p_left_node->size >= (MAX+1) / 2 + 1) {

			// 从左边拿过来的那个 key，肯定是要放到 key[0] 的位置的
			for(int i=p_cursor_node->size; i>0; --i) {
				p_cursor_node->key[i] = p_cursor_node->key[i-1];
			}

			p_cursor_node->size += 1;
			// 注意这里 children_or_sibling 实际上是 sibling，因为我们此案在处理的是叶子节点
			p_cursor_node->children_or_sibling[p_cursor_node->size] = p_cursor_node->children_or_sibling[p_cursor_node->size-1];
			p_cursor_node->children_or_sibling[p_cursor_node->size-1] = nullptr;

			// 开始从左边兄弟节点转移数据
			p_cursor_node->children_or_sibling[0] = p_left_node->children_or_sibling[p_left_node->size-1];
			p_left_node->size -= 1;
			p_left_node->children_or_sibling[p_left_node->size] = p_cursor_node;
			p_left_node->children_or_sibling[p_left_node->size+1] = nullptr;

			// 更新 parent
			p_parent->key[left_sibling] = p_cursor_node->key[0];

			cout << "transfer " << p_cursor_node->key[0] << "from left sibling of leaf node" << endl;

			return ;
		}
	}

	if(right_sibling <= p_parent->size) {
		// 因为这里的 right_sibling 是 parent 中 children 的 pos
		// 所以要跟 parent 的 size 比较
		// 要是右边兄弟的节点多，就从右边兄弟拿一个过来

		/*
		  如下图所示，children 比 key 要多一个

						     parent
					    /             \
				    left -> cursor -> right
		*/

		Node* p_right_node = p_parent->children_or_sibling[right_sibling];

		if(p_right_node->size >= (MAX+1) / 2 + 1) {

			// 修改链表, cursor 多了一个元素，所以
			p_cursor_node->size += 1;
			p_cursor_node->children_or_sibling[p_cursor_node->size] = p_cursor_node->children_or_sibling[p_cursor_node->size - 1];
			p_cursor_node->children_or_sibling[p_cursor_node->size-1] = nullptr;

			// 从右边转移一个到cursor
			p_cursor_node->key[p_cursor_node->size-1] = p_right_node->key[0];

			// 现在 right_node 的 key 和 children 都变了，要重新移动
			p_right_node->size -= 1;
			p_right_node->children_or_sibling[p_right_node->size] = p_right_node->children_or_sibling[p_right_node->size+1];
			p_right_node->children_or_sibling[p_right_node->size+1] = nullptr;
			for(int i=0; i< p_right_node->size; ++i) {
				p_right_node->key[i] = p_right_node->key[i+1];
			}

			// 更新父节点
			p_parent->key[right_sibling-1] = p_right_node->key[0];

			cout << "transfer " << p_cursor_node->key[p_cursor_node->size-1] << "from right sibling of leaf node" << endl;

			return ;
		}
	}

	// 下边的要合并和删除节点了
	if(left_sibling >= 0) {
		// 把 cursor 中的所有元素都转移到 left_node 中
		// 然后把 cursor node 删除

		Node* p_left_node = p_parent->children_or_sibling[left_sibling];

		for(int i=p_left_node->size, j=0; j< p_cursor_node->size; ++i, ++j) {
			p_left_node->key[i] = p_cursor_node->key[j];
		}
		p_left_node->children_or_sibling[p_left_node->size] = nullptr;
		p_left_node->size += p_cursor_node->size;
		p_left_node->children_or_sibling[p_left_node->size] = p_cursor_node->children_or_sibling[p_cursor_node];

		_RemoveInternal(p_parent->key[left_sibling], p_parent, p_cursor_node);

		delete p_cursor_node;

	} else if(right_sibling <= p_parent->size) {
		// 把右兄弟合并到cursor上边来

		Node* p_right_node = p_parent->children_or_sibling[right_sibling];

		// 右边的 keys 都转移到 cursor 上边来
		for(int i=p_cursor_node->size, j=0; j<p_cursor_node->size; ++i, ++j) {
			p_cursor_node->key[i] = p_right_node->key[j];
		}
		p_cursor_node->children_or_sibling[p_cursor_node->size] = nullptr;
		p_cursor_node->size += p_right_node->size;
		p_cursor_node->children_or_sibling[p_cursor_node->size] = p_right_node->children_or_sibling[p_right_node->size];	// 因为childre要比key多一个

		_RemoveInternal(p_parent->key[right_sibling-1], p_parent, p_right_node);

		delete p_right_node;
	}

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

void BPlusTree::_RemoveInternal(int x, Node* p_cursor, Node* p_child)
{

}



