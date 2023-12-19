
#ifndef __HUFFMAN_ENCODING_TREE_NODE
#define __HUFFMAN_ENCODING_TREE_NODE

struct HuffmanEncodingTreeNode
{
	unsigned char value;
	unsigned weight;
	HuffmanEncodingTreeNode* left;
	HuffmanEncodingTreeNode* right;
	HuffmanEncodingTreeNode* parent;
};

#endif