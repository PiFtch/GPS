#include "../include/huffman.h"
#include <string>
#include <iostream>
using namespace std;

unsigned char buf[512] = "1234";
unsigned char out[][512];

int main() {
    huffman_encode_memory(buf, 512, out[], (uint32_t)512);
    cout << out[] << endl;
}