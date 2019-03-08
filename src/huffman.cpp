#include <map>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <string>
#include <bitset>
using namespace std;
#define MAXLEN 512
// unordered_map<char, double> charCount;
unordered_map<char, double> charWeight;
unordered_map<char, string> charToBinStr;

struct huffmanNode {
	char ch;
	double data;
	huffmanNode *left, *right;
};
vector<huffmanNode *> tree;
huffmanNode *root;

bool cmp(huffmanNode *node1, huffmanNode *node2) {
	return node1->data < node2->data;
}

void buildHuffmanTree(char *text, int len) {
	huffmanNode *node;
	// 计算权重，构造初始森林
	for (int i = 0; i < len; i++) {
		charWeight[text[i]]++;	// 字符计数
	}
	unordered_map<char, double>::iterator it;
	for (it = charWeight.begin(); it != charWeight.end(); it++) {
		it->second = it->second / (double)len;	// 计算概率
		// 构造初始huffmanNode
		node = new huffmanNode;
		node->ch = it->first;
		node->data = it->second;
		node->left = node->right = NULL;
		tree.push_back(node);
	}

	// construct huffman tree
	huffmanNode *temp1, *temp2;
	while (tree.size() > 1) {
		sort(tree.begin(), tree.end(), cmp);	// low to high
		// 合并最小的两个结点
		node = new huffmanNode;
		temp1 = *tree.begin();
		temp2 = *(tree.begin() + 1);
		// cout << temp1->data << ' ' << temp2->data << endl;
		node->data = temp1->data + temp2->data;
		node->left = temp1;
		node->right = temp2;
		tree.erase(tree.begin(), tree.begin() + 2);
		tree.push_back(node);
	}
	root = tree[0];
	tree.clear();
	charWeight.clear();
}

void dfs(huffmanNode *root, string str) {
	string temp;
	if (root->left != NULL) {
		temp = str + "0";
		dfs(root->left, temp);
	}
	if (root->right != NULL) {
		temp = str + "1";
		dfs(root->right, temp);
	}
	if (root->left == NULL && root->right == NULL) {
		// cout << root->ch << ' ' << root->data << endl;
		charToBinStr[root->ch] = str;
		// cout << str << endl;
	}
}

string STRING;
int compressedSizeUlong(char *srcbuf, int len) {
	buildHuffmanTree(srcbuf, len);
	string str("");
	dfs(root, str);
	for (int i = 0; i < len; i++) {
		str.append(charToBinStr[srcbuf[i]]);
	}
	STRING = str;
	// cout << str << endl;
	int length = str.length() / 32;
	if (str.length() % 32 != 0) {
		length += str.length() % 32;
	}
	return length;
}

void encode(unsigned long *dstbuf) {
	// cout << str << endl;
	int length = STRING.length();
	// cout << "length: " << length << endl;
	int i;
	for (i = 0; i + 31 < length; i+=32) {
		string temp = STRING.substr(i, 32);
		bitset<32> bits(temp);
		dstbuf[i / 32] = bits.to_ulong();
		// cout << bits.to_ulong() << endl;
	}
	if (i < length) {	// 末尾几位
		int shift = i + 32 - length;
		string temp = STRING.substr(i, length - i);
		bitset<32> bits(temp);
		unsigned long last = bits.to_ulong();
		// cout << last;
		last <<= shift;
		dstbuf[i / 32] = last;
	}
}
/*
int main() {
	// charWeight['a']++;
	// cout << charWeight['a'] << endl;
	char buf[20] = "abcddefg";

	unsigned long *result = new unsigned long[compressedSizeUlong(buf, strlen(buf))];
	// cout << strlen(buf) << endl;
	// buildHuffmanTree(buf, strlen(buf));
	// for (vector<huffmanNode *>::iterator it = tree.begin(); it != tree.end(); it++) {
	// 	cout << (*it)->ch << ' ' << (*it)->data << endl;
	// }
	// dfs(*tree.begin());
	encode(result);
	// cout << charToBinStr['c'] << endl;
	// cout << sizeof(unsigned long) << endl;	// 4bytes
	// for (int i = 0; i < MAXLEN; i++) {
	// 	cout << result[i];
	// }
	for (int i = 0; i < 3; i++) {
		printf("%ul ", result[i]);
	}
	bitset<MAXLEN> temp(string(result));
	// cout << temp << endl;
	// cout << (unsigned long)(*result) << endl;
}
*/