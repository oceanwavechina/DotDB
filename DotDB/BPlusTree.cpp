/*
 * BPlusTree.cpp
 *
 *  Created on: Apr 20, 2021
 *      Author: liuyanan
 */

#include "BPlusTree.h"
#include <queue>
#include <sstream>
#include <cmath>
#include <functional>
#include <algorithm>


Node::Node(): is_leaf(false), size(0) {
    bzero(ptrs, sizeof(ptrs));
}

bool Node::IsFull()
{
    return size == MAX;
}

bool Node::NeedBorrowOrMerge()
{
    // 这里是一个node要包含的最少的key的个数
    // 少于这个数值就要合并了
    return size < (MAX+1) / 2 && size > 1;
}

void Node::Display(const string& msg)
{
    ostringstream oss;
    cout << msg << ": ";
    for(int i=0; i<size; ++i) {
        cout << key[i] << ",";
    }
    cout << endl;
}

string Node::Keys()
{
    ostringstream oss;
    for(int i=0; i<size; ++i) {
        cout << key[i] << ",";
    }
    
    return oss.str();
}

int Node::FindDataPosAsLeaf(int x)
{
	for(int i=0; i< size; ++i) {
		if(key[i] == x) {
			return i;
		}
	}

	return Node::npos;
}

int Node::InsertDataAsLeaf(int x)
{
	int target_pos = 0;
	// 1. 所有小于 x 的都在 x 的左边
	while(x > key[target_pos] && target_pos < size) {
		++target_pos;
	}

	// 2. 所有大于 x 的 都在 x 的右边
	for(int i = size; i> target_pos; --i) {
		key[i] = key[i-1];
	}

	// 3. 把 x 放到目标位置上
	key[target_pos] = x;
	size += 1;

	// 这里是处理，叶子节点那个链表的关系
	ptrs[size] = ptrs[size-1];
	ptrs[size-1] = nullptr;

	return target_pos;
}

int Node::InsertKeyAsInternal(int x/*要插入的数据*/, Node* p_child/*要插入的孩子节点 */)
{
	int target_pos = 0;
	while(x>key[target_pos] && target_pos < size) {
		++target_pos;
	}

	// 给 key 腾地儿
	for(int j=size; j>target_pos; --j) {
		key[j] = key[j-1];
	}

	// 给child pointer 腾地儿
	for(int j=size+1; j> target_pos+1; --j) {
		ptrs[j] = ptrs[j-1];
	}

	key[target_pos] = x;
	++ size;
	ptrs[target_pos+1] = p_child;

	return target_pos;
}

bool Node::TryBorrowFromLeftSiblingAsLeaf(Node* p_parent, int left_sibling_in_parent)
{
    if(left_sibling_in_parent >= 0) {
            // 要是左边兄弟的节点多，就从左边兄弟 借一个 过来
            // 这里多的判断是 >= (MAX+1) / 2 + 1
        Node* p_left_node = p_parent->ptrs[left_sibling_in_parent];

        if(p_left_node->size >= (MAX+1) / 2 + 1 && p_left_node->size > 1) {

            cout << "will borrow " << key[0] << "from " << p_left_node->Keys() << " left sibling" << endl;
            
            // 从左边拿过来的那个 key，肯定是要放到 key[0] 的位置的
            for(int i=size; i>0; --i) {
                key[i] = key[i-1];
            }

            size += 1;
            // 注意这里 children_or_sibling 实际上是 sibling，因为我们此案在处理的是叶子节点
            ptrs[size] = ptrs[size-1];
            ptrs[size-1] = nullptr;

            // 开始从左边兄弟节点转移数据
            ptrs[0] = p_left_node->ptrs[p_left_node->size-1];
            p_left_node->size -= 1;
            p_left_node->ptrs[p_left_node->size] = this;
            p_left_node->ptrs[p_left_node->size+1] = nullptr;

            // 更新 parent
            p_parent->key[left_sibling_in_parent] = key[0];
            
            return true;
            
        }
    }
    
    return false;
}

bool Node::TryBorrowFromRightSiblingAsLeaf(Node* p_parent, int right_sibling_in_parent)
{
    /*
      如下图所示，children 比 key 要多一个

                         parent
                    /             \
                left -> cursor -> right
    */

    if(right_sibling_in_parent <= p_parent->size) {
        // 因为这里的 right_sibling 是 parent 中 children 的 pos
        // 所以要跟 parent 的 size 比较
        // 要是右边兄弟的节点多，就从右边兄弟 借一个 过来
        Node* p_right_node = p_parent->ptrs[right_sibling_in_parent];

        if(p_right_node->size >= (MAX+1) / 2 + 1 && p_right_node->size > 1) {

            cout << "will borrow " << key[0] << "from " << p_right_node->Keys() << " right sibling" << endl;
            
            // 修改链表, cursor 多了一个元素，所以
            size += 1;
            ptrs[size] = ptrs[size - 1];
            ptrs[size-1] = nullptr;

            // 从右边转移一个到cursor
            key[size-1] = p_right_node->key[0];

            // 现在 right_node 的 key 和 children 都变了，要重新移动
            p_right_node->size -= 1;
            p_right_node->ptrs[p_right_node->size] = p_right_node->ptrs[p_right_node->size+1];
            p_right_node->ptrs[p_right_node->size+1] = nullptr;
            for(int i=0; i< p_right_node->size; ++i) {
                p_right_node->key[i] = p_right_node->key[i+1];
            }

            // 更新父节点
            p_parent->key[right_sibling_in_parent-1] = p_right_node->key[0];

            return true;
            
        }
    }
    
    return false;
}

bool Node::TryBorrowFromLeftSiblingAsInternal(Node* p_parent, int left_sibling_in_parent)
{
    if(left_sibling_in_parent >= 0) {
        Node* p_left_node = p_parent->ptrs[left_sibling_in_parent];

        if(p_left_node->size > (MAX+1) / 2 && p_left_node->size > 1) {
            // 把 cursor key 中的 第 0 号位置空出来
            for(int i=size; i>0; --i) {
                key[i] = key[i-1];
            }

            // transfer key from left sibling to cursor through parent
            // 这里相当于数据转了个圈
            key[0] = p_parent->key[left_sibling_in_parent];
            key[left_sibling_in_parent] = p_left_node->key[p_left_node->size-1];

            // 把 left node 的最后个child的指针 移动到cursor的第0号位置
            for(int i=size+1; i>0; --i) {
                ptrs[i] = ptrs[i-1];
            }
            ptrs[0] = p_left_node->ptrs[p_left_node->size];
            size += 1;
            p_left_node->size -= 1;

            return true;
        }
    }
    
    return false;
}

bool Node::TryBorrowFromRightSiblingAsInternal(Node* p_parent, int right_sibling_in_parent, int cur_pos)
{
    if(right_sibling_in_parent <= p_parent->size) {
        Node* p_right_ndoe = p_parent->ptrs[right_sibling_in_parent];
        
        if(p_right_ndoe->size > (MAX+1) / 2 && p_right_ndoe->size > 1) {
            key[size] = p_parent->key[cur_pos];
            p_parent->key[cur_pos] = p_right_ndoe->key[0];
        
            for(int i=0; i<p_right_ndoe->size-1; ++i) {
                p_right_ndoe->key[i] = p_right_ndoe->key[i+1];
            }

            ptrs[size+1] = p_right_ndoe->ptrs[0];
            for(int i=0; i<p_right_ndoe->size; ++i) {
                p_right_ndoe->ptrs[i] = p_right_ndoe->ptrs[i+1];
            }

            size += 1;
            p_right_ndoe->size -= 1;

            return true;
        }
    }

    return false;
}


int Node::RemoveKeyAndChildAsInternal(int x, Node* p_child)
{
    // 从 cursor 里边删除x
    int pos;
    for(pos=0; pos< size; ++pos) {
        if(key[pos] == x) {
            break;
        }
    }
    for(int i=pos; i< size; ++i) {
        key[i] = key[i+1];
    }
    
    // 删除孩子节点，如果有的话
    for(pos=0; pos<size+1; ++pos) {
        if(ptrs[pos] == p_child) {
            break;
        }
    }
    for(int i=pos; i<size+1; ++i) {
        ptrs[i] = ptrs[i+1];
    }
    
    size -= 1;
    
    return pos;
}

int Node::TryRemoveKeyAsLeaf(int x)
{
    int target_pos = FindDataPosAsLeaf(x);
    if(target_pos == Node::npos) {
        cout << "not found" << endl;
        return npos;
    }

    // 3. 先从叶子节点上 删除这个key
    for(int i=target_pos; i<size; ++i) {
        key[i] = key[i+1];
    }
    size -= 1;

    // 处理叶子节点链表的串联关系, 这里的size已经是 缩小1 以后的
    ptrs[size] = ptrs[size + 1];
    ptrs[size + 1] = nullptr;

    return target_pos;
}

void Node::MergeToLeftAsLeaf(Node* p_left_node)
{
    // 当 left 兄弟存在时，则把数据合并到 left 兄弟， 然后把 cursor node 删除

    /*
      如下图所示，children 比 key 要多一个

                        parent                                parent
                    /      |      \         ->         /             \
                left -> cursor -> right             left+cursor   -> right
    */

    for(int i=p_left_node->size, j=0; j<size; ++i, ++j) {
        p_left_node->key[i] = key[j];
    }
    p_left_node->ptrs[p_left_node->size] = nullptr;
    p_left_node->size += size;
    p_left_node->ptrs[p_left_node->size] = ptrs[size];
}

void Node::MergeFromRightAsLeaf(Node* p_right_node)
{
    // 右边的 keys 都转移到 cursor 上边来
    for(int i=size, j=0; j<size; ++i, ++j) {
        key[i] = p_right_node->key[j];
    }
    ptrs[size] = nullptr;
    size += p_right_node->size;
    ptrs[size] = p_right_node->ptrs[p_right_node->size];    // 因为childre要比key多一个
}

void Node::MergeToLeftAsInternal(int left_sibling_in_parent, Node* p_parent, int cur_pos)
{
    //leftnode + parent key + cursor
    Node* p_left_node = p_parent->ptrs[left_sibling_in_parent];
    p_left_node->key[p_left_node->size] = p_parent->key[left_sibling_in_parent];
    
    for(int i = p_left_node->size+1, j = 0; j < size; j++) {
        p_left_node->key[i] = key[j];
    }
    for(int i = p_left_node->size+1, j = 0; j < size+1; j++) {
        p_left_node->ptrs[i] = ptrs[j];
        ptrs[j] = NULL;
    }

    p_left_node->size += size+1;
    size = 0;
}

void Node::MergeFromRightAsInternal(int right_sibling_in_parent, Node* p_parent, int cur_pos)
{
    //cursor + parent key + rightnode
    Node *p_right_sibling = p_parent->ptrs[right_sibling_in_parent];
    key[size] = p_parent->key[right_sibling_in_parent-1];
    for(int i = size+1, j = 0; j < p_right_sibling->size; j++) {
        key[i] = p_right_sibling->key[j];
    }
    for(int i = size+1, j = 0; j < p_right_sibling->size+1; j++) {
        ptrs[i] = p_right_sibling->ptrs[j];
        p_right_sibling->ptrs[j] = NULL;
    }
    size += p_right_sibling->size + 1;
    p_right_sibling->size = 0;
}

Node* Node::SearchBetween(int start, int stop, vector<int>& values)
{
    bool ended = false;
    
    for(int i=0; i< size; ++i) {
        if(key[i] >= start && key[i] <= stop) {
            values.push_back(key[i]);
        }
        
        if(key[i] > stop) {
            ended = true;
        }
    }
    
    return ended ? nullptr : ptrs[size];
}

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

	cout << "prepare insert: " << x << endl;

	if(!_root) {
		_root = new Node;
		_root->key[0] = x;
		_root->is_leaf = true;
		_root->size = 1;

		cout << "create initial root:" << x << endl;

	} else  {
        Node* p_target_leaf;
        Node* p_parent;
        std::tie(p_target_leaf, p_parent) = _FindTargetLeafNodeWithParent(x);

		if( ! p_target_leaf->IsFull() ) {
			// 情况一： 叶子节点没有满， 直接在 **叶子节点** 上插入
			int target_pos = p_target_leaf->InsertDataAsLeaf(x);
			cout << "Insert at leaf nodes, target pos: " << target_pos << endl;

		} else {
            // 情况二： 叶子节点满了，需要分裂 **叶子节点**
			// 这个函数会把 p_cursor 中的元素 和 x 放到一起（有序），然后分裂成两个
			//    1. p_new_leaf 在 p_cursor 的右边，
			//    2. p_new_leaf 中的元素 比 p_cursor 中的元素大
            //    3. p_cursor 中的元素个数 小于等于 p_new_leaf 中的元素个数
			Node* p_new_leaf = _SplitLeafNodeWithInsert(p_target_leaf, x);

			// 当被拆分的 p_cursor 是根节点的时候
			//     这时候要重新创建一个根节点，把拆分的两个节点挂到父节点上
			//     也就是说这时候的树长高了
			if(p_target_leaf == _root) {
				Node* p_new_root = new Node;
				p_new_root->key[0] = p_new_leaf->key[0];	// p_new_leaf中是比较大的部分，我们取第0号元素，就能满足B+树的性质
				p_new_root->ptrs[0] = p_target_leaf;				// p_cursor 是小的那个node，放到左边
				p_new_root->ptrs[1] = p_new_leaf;			// p_new_leaf 是大的那个node，放到右边
				p_new_root->is_leaf = false;
				p_new_root->size = 1;
				_root = p_new_root;
                
				cout << "create new root after split leaf node, with median: " << p_new_leaf->key[0] << endl;

			} else {
				//	这里的递归调用是自下而上的分列的，
				//    1. 最下边的先分裂(leaf node)，然后被分裂的两个 Node 的 median，插入到 parent 中
				//    2. 中间层需要的话，在继续分列然后向上传递
				//    3. 第一个参数是新节点中最小的那个值，也就是被分裂的两个 Node 的 median 我们需要更新到parent中
				//        同时还要吧 p_new_leaf 挂到parent里边
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

    Node* p_cur_node;
    Node* p_parent;
	std::tie(p_cur_node, p_parent) = _FindTargetLeafNodeWithParent(x);

	// 然后看看目标节点中是否真的有这个数据
	return p_cur_node->FindDataPosAsLeaf(x) != Node::npos;
}

bool BPlusTree::Between(int start, int stop)
{
    //
    //    https://www.programiz.com/dsa/b-plus-tree
    //
    //    1. 每个 Node 中的 keys 都是从小到大排序的
    //    2. 我们要查找的数据只能在叶子节点中，中间节点都是索引节点，不包含数据
    //  3. 当 x < keys[i] 时，下一次查找的 node，必然是children[i]； 否则在children[i+1， ...] 里边
    //

    if(!_root) {
        return false;
    }

    cout << "search between:[" << start << "," << stop << "]: ";
    
    Node* p_cur_node;
    Node* p_parent;
    std::tie(p_cur_node, p_parent) = _FindTargetLeafNodeWithParent(start);
    
    vector<int> result;
    while (p_cur_node) {
        p_cur_node = p_cur_node->SearchBetween(start, stop, result);
    }
    
    for(auto item : result) {
        cout << item << ",";
    }
    cout << "\n" << endl;
    
    return true;
}

void BPlusTree::Remove(int x)
{
	if(!_root) {
		return;
	}

	cout << "prepare remove: " << x << endl;

    // 1. 先找到可能包含 x 的那个叶子节点
	Node* p_cursor = _root;
	Node* p_parent = nullptr;
    int left_sibling_in_parent = -1;
    int right_sibling_in_parent = INT_MAX;
    tie(p_cursor, p_parent, left_sibling_in_parent, right_sibling_in_parent) = _FindTargetLeafNodeWithParentAndBrothers(x);
    
	// 2. 尝试现在叶节点删除这个元素，如果没有，直接返回
    int target_pos = p_cursor->TryRemoveKeyAsLeaf(x);
    if(target_pos == Node::npos) {
        cout << "not found" << endl;
        return;
    }
    
	// 根节点的特殊处理
	if(p_cursor == _root) {
		//
		// 	说一下为什么指针都置空:
		//		当 根节点 是 叶子节点 的时候, 他的孩子节点自然后不存在
		//
		for(int i=0; i<MAX+1; ++i) {
			p_cursor->ptrs[i] = nullptr;
		}

		if(p_cursor->size == 0) {
			// 这里的情况是被删除的 x 是根节点中惟一的一个元素
			// 	当x被删除后，整棵树的元素就为0了
			cout << "tree died" << endl;
			delete p_cursor;
			_root = nullptr;
		}
	}

	// 判断当前这个叶子节点需不需要合并
	if( ! p_cursor->NeedBorrowOrMerge()) {
        
        cout << "no need to borrow or merge, stop here" << endl;
		return;
	}
    

	/*
	  如下图所示，children 比 key 要多一个

					  k1 | k2 | k3 | k4 | k5
				    /    |    |    |    |    \
				  ch0   ch1  ch2  ch3  ch4   ch5
	*/

	// 以下是从 兄弟(叶子)节点中 **借一个** 元素过来
	// 合并的原则是从右往左合并
	// 		left  <--  cursor  <--  right

    if(p_cursor->TryBorrowFromLeftSiblingAsLeaf(p_parent, left_sibling_in_parent)) {
        return;
    }

    if(p_cursor->TryBorrowFromRightSiblingAsLeaf(p_parent, right_sibling_in_parent)) {
        return;
    }

    assert(left_sibling_in_parent >= 0 || right_sibling_in_parent <= p_parent->size);
    
	// 当不能从两边的兄弟节点都不能借到元素时，说明要跟其合并才能满足 B+树 的性质
	// 下边就是合并的流程
	if(left_sibling_in_parent >= 0) {
		// 当 left 兄弟存在时，则把数据合并到 left 兄弟，然后把当前删除

		Node* p_left_node = p_parent->ptrs[left_sibling_in_parent];

        cout << "will merge [" << p_cursor->Keys() << "] to left sibling [" << p_left_node->Keys() << "]" << endl;
        
        p_cursor->MergeToLeftAsLeaf(p_left_node);
        
        cout << "\t after merge, left sibling: " << p_left_node->Keys() << endl;
        cout << "will remove interval: " << p_parent->key[left_sibling_in_parent] << endl;

		_RemoveInternal(p_parent->key[left_sibling_in_parent], p_parent, p_cursor);

		delete p_cursor;

	} else if(right_sibling_in_parent <= p_parent->size) {
        // 当 right 兄弟存在时，则把right兄弟合并到当前节点，然后把当前删除
        
		Node* p_right_node = p_parent->ptrs[right_sibling_in_parent];
        
        cout << "will merge " << p_cursor->Keys() << " to right sibling " << p_right_node->Keys() << endl;

        p_cursor->MergeFromRightAsLeaf(p_right_node);
        
        cout << "\t after merge, right sibling: " << p_right_node->Keys() << endl;

		_RemoveInternal(p_parent->key[right_sibling_in_parent-1], p_parent, p_right_node);

		delete p_right_node;
	}
}

void space_prefix(ostringstream& oss, int n)
{
	for(int i=0; i< n; ++i) {
		oss << '\t';
	}
}

void BPlusTree::Display(int n_space)
{
	ostringstream oss;
	oss << "display:\n\n";

	if(!_root) {
		return;
	}

	queue<Node*> the_queue;
	the_queue.push(_root);

	int cur_lvl_cnt = 1;
	int next_lvl_cnt = 0;

	space_prefix(oss, n_space--);
	while(!the_queue.empty()) {
		Node* tmp = the_queue.front();
		the_queue.pop();

		// 先输出当前层的数据
		for(int i=0; i<tmp->size; ++i) {
			oss << tmp->key[i];
            if(i != tmp->size-1) {
                oss << ",";
            }
		}
		oss << "      ";
        
		cur_lvl_cnt -= 1;

		// 把孩子节点入队
		for(int i=0; i< tmp->size+1; ++i) {
			if(tmp->ptrs[i] && !tmp->is_leaf) {
				the_queue.push(tmp->ptrs[i]);
				next_lvl_cnt += 1;
			}
		}

		if(cur_lvl_cnt == 0) {
			cur_lvl_cnt = next_lvl_cnt;
			next_lvl_cnt = 0;
			oss << "\n\n";
			space_prefix(oss, n_space--);
		}
	}

	cout << oss.str() << endl;
}


Node* BPlusTree::Root()
{
	return _root;
}

void BPlusTree::_InsertInternal(int x/*注意当p_child是叶节点与否，而传入的值不同*/, Node* p_cursor, Node* p_child)
{
	//
	//  x 是要插入的key，也就是下一层中的那个 median
	//
	if(!p_cursor->IsFull()) {
		// 中间节点不需要分裂, 只需要在 keys 的合适位置插入
        cout << "will insert " << x << " into inner(none full) " << endl;
		p_cursor->InsertKeyAsInternal(x, p_child);
        p_cursor->Display("\tafter insert into inner");

	} else {
		// 需要分裂中间节点

		// p_new_internal 里边的数据是比较大的部分
		Node* p_new_internal = _SplitInternalNodeWithInsert(p_cursor, p_child, x);

		if(p_cursor == _root) {
            
			// 被分类的节点是根节点，也就是根节点分裂成两个了
            // 这时候，要创建一个新的根节点，放在他俩的上边
			Node* p_new_root = new Node;
			p_new_root->key[0] = p_cursor->key[p_cursor->size]; // 注意这里用的是size, 而不是size-1， 因为parent是较小的那个，size就 median
			p_new_root->ptrs[0] = p_cursor;
			p_new_root->ptrs[1] = p_new_internal;
			p_new_root->is_leaf = false;
			p_new_root->size = 1;
			_root = p_new_root;
            
			cout << "create new root after split internal node" << endl;

		} else {
			// 当前是中间节点，被分裂后，需要把 median 插入到上一层中
			// 注意下这里的第一个参数，我们这次传的是较小的里边的 end+1 位置上的元素，为什么会是这样？
            //    1. 首先，不会内存溢出，因为分裂后，p_cursor 中的元素个数肯定小于树的度
            //    2. size+1 的位置，实际上就是被分裂的两个 Node 的 median， 为什么？
            //        因为 **中间节点不保存这个median** ，而是要插入到上一层
            cout << "InsertInternal will insert median " << p_cursor->key[p_cursor->size] << " into upper level" << endl;
            _InsertInternal(p_cursor->key[p_cursor->size], _FindParentRecursively(_root, p_cursor), p_new_internal);
		}
	}
}

Node* BPlusTree::_FindParentRecursively(Node* p_cursor, Node* p_child)
{
	Node* p_parent = nullptr;

	if(p_cursor->is_leaf || p_cursor->ptrs[0]->is_leaf) {
		return nullptr;
	}

	for(int i=0; i<p_cursor->size+1; ++i) {
		if(p_cursor->ptrs[i] == p_child) {
			p_parent = p_cursor;
			return p_parent;
		} else {
			p_parent = _FindParentRecursively(p_cursor->ptrs[i], p_child);
			if(p_parent) {
				return p_parent;
			}
		}
	}

	return p_parent;
}

tuple<int/*left-sibling*/, int /*cur-pos*/, int/*right-sibling*/> BPlusTree::_FindBrothersPosInParent(Node* p_parent, Node* p_cursor)
{
    int left_sibling_in_parent = -1;
    int right_sibling_in_parent = INT_MAX;
    int cursor_pos = 0;
    for(cursor_pos=0; cursor_pos<p_parent->size+1; ++cursor_pos) {
        if(p_parent->ptrs[cursor_pos] == p_cursor) {
            left_sibling_in_parent = cursor_pos - 1;
            right_sibling_in_parent = cursor_pos + 1;
            break;
        }
    }

    return make_tuple(left_sibling_in_parent, cursor_pos, right_sibling_in_parent);
}

void BPlusTree::_RemoveInternal(int x, Node* p_cursor, Node* p_child /*to be deleted*/)
{
	if(p_cursor == _root) {
		/*
		  如下图所示，children 比 key 要多一个

						 [0] cursor [1]
					     /            \
				      left    ->     right
		*/

		if(p_cursor->size == 1) {
			// p_child 是 cursor 的孩子节点，且是要被删除的节点, 如果 p_child 被删除了，那cursor就剩一个孩子了
			// 根据 B+ 树的规则，根节点最少有两个孩子节点

			if(p_cursor->ptrs[1] == p_child) {
				delete p_child;
				_root = p_cursor->ptrs[0];
				delete p_cursor;
				return ;
			} else if (p_cursor->ptrs[0] == p_child) {
				delete p_child;
				_root = p_cursor->ptrs[1];
				delete p_cursor;
				return ;
			}
		}
	}

	// 从 cursor 里边删除x
    cout << "will remove internal key: " << x << " and child:[" << p_child->Keys() << "] from " << p_cursor->Keys() << endl;
    p_cursor->RemoveKeyAndChildAsInternal(x, p_child);
    cout << "after remove internal key: [" << p_cursor->Keys() << "]" << endl;

	// 当前节点满足 B+ 树的要求了，到此为止
	// 根节点除外
    if(p_cursor->size >= 1 /*(MAX + 1) / 2 - 1*/) {
		return ;
	}
	if(p_cursor == _root) {
		return;
	}

    // 找到 p_cursor 的左右兄弟节点
    int cursor_pos_in_parent = -1;
    int left_sibling_in_parent = -1;
    int right_sibling_in_parent = INT_MAX;
	Node* p_parent = _FindParentRecursively(_root, p_cursor);
    tie(left_sibling_in_parent, cursor_pos_in_parent, right_sibling_in_parent) = _FindBrothersPosInParent(p_parent, p_cursor);

    // 从左边兄弟节点借一个
    if(p_cursor->TryBorrowFromLeftSiblingAsInternal(p_parent, left_sibling_in_parent)) {
        return ;
    }

    // 从右边兄弟节点借一个
    if(p_cursor->TryBorrowFromRightSiblingAsInternal(p_parent, right_sibling_in_parent, cursor_pos_in_parent)) {
        return ;
    }

    //transfer wasnt posssible hence do merging
	if(left_sibling_in_parent >= 0) {
		
        p_cursor->MergeToLeftAsInternal(left_sibling_in_parent, p_parent, cursor_pos_in_parent);

		//delete cursor
		_RemoveInternal(p_parent->key[left_sibling_in_parent], p_parent, p_cursor);

	} else if(right_sibling_in_parent <= p_parent->size) {

        p_cursor->MergeFromRightAsInternal(right_sibling_in_parent, p_parent, cursor_pos_in_parent);

		//delete cursor
		_RemoveInternal(p_parent->key[right_sibling_in_parent-1], p_parent, p_parent->ptrs[right_sibling_in_parent]);
	}
}

void BPlusTree::_ReplaceParentKeyWith(int x, Node* p_parent)
{
    // 1. 更新父节点的key
    //      注意，如果父节点中要替换的key比x大，那我们就不替换了
    // 2. 继续上溯知道根节点
}

tuple<Node*/*target*/, Node*/*parent*/> BPlusTree::_FindTargetLeafNodeWithParent(int x)
{
    Node* p_cursor = _root;
    Node* p_parent = nullptr;

    // 先找到目标节点
    while(! p_cursor->is_leaf) {
        
        p_parent  = p_cursor;
        
        // 当前层级中查找
        // 最终的数据都在叶子节点中，所以我们要遍历整个层级
        for(int i=0; i<p_cursor->size; ++i) {

            if(x < p_cursor->key[i]) {
                // 在左边孩子中
                p_cursor = p_cursor->ptrs[i];
                break;
            }

            // 如果执行到这里，说明遍历完了，我们直接取右下角那个node
            if(i == p_cursor->size -1) {
                // 最后一个的处理
                p_cursor = p_cursor->ptrs[i+1];
                break;
            }
        }
    }
    
    return make_tuple(p_cursor, p_parent);
}

tuple<Node*/*target*/, Node*/*parent*/, int/*left_sibling*/, int/*right_sibling*/> BPlusTree::_FindTargetLeafNodeWithParentAndBrothers(int x)
{
    Node* p_cursor = _root;
    Node* p_parent = nullptr;
    int left_sibling_in_parent = -1;
    int right_sibling_in_parent = INT_MAX;

    // 1. 先尝试找到包含 x 的那个叶子节点
    while(!p_cursor->is_leaf) {

        for(int i=0; i<p_cursor->size; ++i) {
            p_parent = p_cursor;
            left_sibling_in_parent = i-1;
            right_sibling_in_parent = i+1;

            if(x < p_cursor->key[i]) {
                p_cursor = p_cursor->ptrs[i];
                break;
            }

            if(i == p_cursor->size - 1) {
                left_sibling_in_parent = i;
                right_sibling_in_parent = i+2;
                p_cursor = p_cursor->ptrs[i+1];
                break;
            }
        }
    }
    
    return make_tuple(p_cursor, p_parent, left_sibling_in_parent, right_sibling_in_parent);
}

Node* BPlusTree::_SplitLeafNodeWithInsert(Node* p_cursor, int x)
{
    p_cursor->Display("split leaf node: ");
	Node* p_new_leaf = new Node;
	p_new_leaf->is_leaf = true;

	// 1. 先创建一个虚拟节点，用来存放key
	int virtual_node[MAX+1];	// 这里只存 p_cursor 中的key
	for(int i=0; i<MAX; ++i) {
		virtual_node[i] = p_cursor->key[i];
	}

	// 2. 把要插入的 key 放到虚拟节点中正确的位置上
	int target_pos = 0;
	while(x > virtual_node[target_pos] && target_pos < MAX) {
		++target_pos;
	}
	for(int i=MAX; i>target_pos; --i) {
		virtual_node[i] = virtual_node[i-1];
	}
	virtual_node[target_pos] = x;

	// 3. 分配两个叶子节点的数量
	p_cursor->size = (MAX + 1) / 2;
	p_new_leaf->size = MAX + 1 - p_cursor->size;

	// 4. 处理叶子节点的链表关系, 新的叶子节点在右边，也就是新的叶子节点上的数据都是比较大的部分
    //      p_cursor -> p_new_leaf
    p_cursor->ptrs[p_cursor->size] = p_new_leaf;
    p_new_leaf->ptrs[p_new_leaf->size] = p_cursor->ptrs[MAX];
    p_cursor->ptrs[MAX] = nullptr;
	
	// 5. 把virtual_node中的数据，分配到两个叶子节点上
	for(int i=0; i<p_cursor->size; ++i) {
		p_cursor->key[i] = virtual_node[i];
	}
    p_cursor->Display("\tafter split leaf, left node: ");
    
	for(int i=0, j=p_cursor->size; i< p_new_leaf->size; ++i, ++j) {
		p_new_leaf->key[i] = virtual_node[j];
	}
    p_new_leaf->Display("\tafter split leaf, right node: ");

	return p_new_leaf;
}

Node* BPlusTree::_SplitInternalNodeWithInsert(Node* p_cursor, Node* p_child, int x)
{
	// 我们要分裂 parent

    p_cursor->Display("split inner node: ");
    
	// 1. 先准备待分裂的数据，包括key和children
	int virtual_keys[MAX+1];
	Node* virtual_ptrs[MAX+2];
	for(int i=0; i<MAX; ++i) {
		virtual_keys[i] = p_cursor->key[i];
	}
	for(int i=0; i<MAX+1; ++i) {
		virtual_ptrs[i] = p_cursor->ptrs[i];
	}

	// 2. 把要要插入的key和新node， 放到virtual的合适位置
	int i=0;
	while(x>virtual_keys[i] && i<MAX) {
		++i;
	}
	for(int j=MAX; j>i; --j) {
		virtual_keys[j] = virtual_keys[j-1];
	}
	virtual_keys[i] = x;

	for(int j=MAX+1; j>i+1; --j) {
		virtual_ptrs[j] = virtual_ptrs[j-1];
	}
	virtual_ptrs[i+1] = p_child;


	// 创建新的节点
	Node* p_new_internal = new Node;
	p_new_internal->is_leaf = false;

	// 分配下，每个节点存多少个数据
	p_cursor->size =  (MAX+1) / 2;
	p_new_internal->size = MAX - p_cursor->size;

	// 开始拷贝数据
    
    for(int i=0; i<p_cursor->size+1; ++i) {
        p_cursor->key[i] = virtual_keys[i];
    }
    for(int i=0; i<p_cursor->size+1; ++i) {
        p_cursor->ptrs[i] = virtual_ptrs[i];
    }
    p_cursor->Display("\tafter split inner, left node: ");
    
	for(int i=0, j=p_cursor->size+1; i<p_new_internal->size; ++i, ++j) {
		p_new_internal->key[i] = virtual_keys[j];
	}
	for(int i=0, j=p_cursor->size+1; i< p_new_internal->size+1; ++i, ++j) {
		p_new_internal->ptrs[i] = virtual_ptrs[j];
	}
    p_new_internal->Display("\tafter split inner, right node: ");
    
    cout << "\tafter split inner, median: " << p_cursor->key[p_cursor->size] << endl;
    
	return p_new_internal;
}



