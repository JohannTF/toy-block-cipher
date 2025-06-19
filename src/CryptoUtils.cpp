#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <bitset>

using namespace std;

class CryptoUtils {
public:

    // Convert unsigned int to 4-bit bitset
    static bitset<4> convertToBitset(unsigned int a) {
        bitset<4> bits;
        for (int i = 0; i < 4; ++i) {
            bits[i] = (a >> i) & 1;
        }
        return bits;
    }

    // Separate 4 bits from a 16-bit bitset starting at position 'start'
    static bitset<4> separateBits(const bitset<16>& bits, int start) {
        bitset<4> result;
        for (int i = 0; i < 4; ++i) {
            result[3 - i] = bits[15 - (start + i)];
        }
        return result;
    }

    // Separate bits in reverse order
    static bitset<4> separateBitsReverse(const bitset<16>& bits, int start) {
        bitset<4> result;
        for (int i = 0; i < 4; ++i) {
            result[i] = bits[start + i];
        }
        return result;
    }

    // Construct a 16-bit bitset from vector of elements
    static bitset<16> construirBitset(const vector<unsigned int>& elements) {
        bitset<16> bits;
        int pos = 0;
        for (const auto& elem : elements) {
            bitset<4> elemBits = convertToBitset(elem);
            for (int i = 0; i < 4; ++i) {
                bits[pos + i] = elemBits[i];
            }
            pos += 4;
        }
        return bits;
    }    
};