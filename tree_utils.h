/**
 * @brief Compute the huffman tree.
 * 
 * @param data array of character
 * @param freq array of corresponding frequences
 * @param size size of the previous arrays
 * @return the tree as MinHeapNode structure
 */
struct MinHeapNode *HuffmanCodes(char data[], int freq[], int size);

/** 
 * @brief Utility function to check if this node is leaf
 * 
 * @param root the node
 * 
 * @return 1 if is a leaf, 0 otherwise
 */
int isLeaf(struct MinHeapNode *root);