/*
 * BPlusTree_test.cpp
 *
 *  Created on: Apr 20, 2021
 *      Author: liuyanan
 */

#include "BPlusTree.h"

#include <vector>

using namespace std;

int main(int argc, char **argv)
{
	BPlusTree tree;

	//vector<int> keys = {5, 15, 25, 35, 45};
    vector<int> keys = {25, 15, 35, 45, 5, 20, 30, 40, 55};
    //vector<int> keys = {5, 8, 10, 15, 16, 17, };
	for(int i=0; i< keys.size(); ++i) {
		tree.Insert(keys[i]);
		tree.Display(i);
        tree.Search(keys[i]);
		cout << endl << endl;
	}
    
    tree.Between(15, 40);
    
    tree.Between(-1, 100);
    
    tree.Between(-1, 1);

	vector<int> del_keys = {25, 15, 35, 45, 5, 20, 30, 40, 55};
	for(int i=0; i< del_keys.size(); ++i) {
		tree.Remove(del_keys[i]);
		tree.Display((int)del_keys.size()-i);
		cout << endl << endl;
	}

}
