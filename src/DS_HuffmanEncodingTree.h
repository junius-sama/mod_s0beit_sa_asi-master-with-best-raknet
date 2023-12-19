#ifndef __HUFFMAN_ENCODING_TREE
#define __HUFFMAN_ENCODING_TREE

#include "DS_HuffmanEncodingTreeNode.h"
#include "main.h"
#include "DS_LinkedList.h" 

/// This generates special cases of the huffman encoding tree using 8 bit keys with the additional condition that unused combinations of 8 bits are treated as a frequency of 1
class HuffmanEncodingTree
{

public:
	HuffmanEncodingTree();
	~HuffmanEncodingTree();

	/// Pass an array of bytes to array and a preallocated BitStream to receive the output
	/// \param [in] input Array of bytes to encode
	/// \param [in] sizeInBytes size of \a input
	/// \param [out] output The bitstream to write to
	void EncodeArray(unsigned char* input, unsigned sizeInBytes, RakNet::BitStream* output);

	// Decodes an array encoded by EncodeArray()
	unsigned DecodeArray(RakNet::BitStream* input, unsigned sizeInBits, unsigned maxCharsToWrite, unsigned char* output);
	void DecodeArray(unsigned char* input, unsigned sizeInBits, RakNet::BitStream* output);

	/// Given a frequency table of 256 elements, all with a frequency of 1 or more, generate the tree
	void GenerateFromFrequencyTable(unsigned int frequencyTable[256]);

	/// Free the memory used by the tree
	void FreeMemory(void);

private:

	/// The root node of the tree 

	HuffmanEncodingTreeNode* root;

	/// Used to hold bit encoding for one character


	struct CharacterEncoding
	{
		unsigned char* encoding;
		unsigned short bitLength;
	};

	CharacterEncoding encodingTable[256];

	void InsertNodeIntoSortedList(HuffmanEncodingTreeNode* node, DataStructures::LinkedList<HuffmanEncodingTreeNode*>* huffmanEncodingTreeNodeList) const;
};

#endif