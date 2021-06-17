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

	vector<int> keys = {12, 3, 4, 5, 6, 7, 23, 42, 178, 123, 4324,9, 54365};

	for(const auto x : keys) {
		tree.Insert(x);
		tree.Display();
		cout << endl << endl;
	}

}
