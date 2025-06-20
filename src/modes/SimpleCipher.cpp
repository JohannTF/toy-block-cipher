#ifndef SIMPLECIPHER_H
#define SIMPLECIPHER_H

#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include "../CryptoUtils.h"
#include "../SBox.h"
#include "../Permutation.h"
#include "../KeySchedule.h"

using namespace std;

class SimpleCipher {
private:
    SBox sbox;
    Permutation permutation;
    KeySchedule keySchedule;
    static const int NUM_ROUNDS = 5;

public:
    SimpleCipher() : sbox(4), keySchedule(NUM_ROUNDS) {}

    // Cifrar un bloque de 16 bits
    bitset<16> encryptBlock(const bitset<16>& plaintext) {
        bitset<16> state = plaintext;
        
        for (int round = 1; round <= NUM_ROUNDS; round++) {
            uint16_t roundKey = keySchedule.getRoundKey(round);
            bitset<16> keyBits(roundKey);
            state ^= keyBits;
            
            vector<unsigned int> nibbles;
            for (int i = 0; i < 4; i++) {
                bitset<4> nibble = CryptoUtils::separateBitsReverse(state, i * 4);
                unsigned int nibbleValue = static_cast<unsigned int>(nibble.to_ulong());
                unsigned int sboxResult = sbox.applySBox(nibbleValue);
                nibbles.push_back(sboxResult);
            }
            
            state = CryptoUtils::construirBitset(nibbles);
            state = permutation.applyPermutation(state);
        }
        
        return state;
    }

    // Descifrar un bloque de 16 bits
    bitset<16> decryptBlock(const bitset<16>& ciphertext) {
        bitset<16> state = ciphertext;
        
        for (int round = NUM_ROUNDS; round >= 1; round--) {
            state = permutation.applyInversePermutation(state);
            
            vector<unsigned int> nibbles;
            for (int i = 0; i < 4; i++) {
                bitset<4> nibble = CryptoUtils::separateBitsReverse(state, i * 4);
                unsigned int nibbleValue = static_cast<unsigned int>(nibble.to_ulong());
                unsigned int inverseSboxResult = sbox.applyInverseSBox(nibbleValue);
                nibbles.push_back(inverseSboxResult);
            }
            
            state = CryptoUtils::construirBitset(nibbles);
            
            uint16_t roundKey = keySchedule.getRoundKey(round);
            bitset<16> keyBits(roundKey);
            state ^= keyBits;
        }
        
        return state;
    }

    // Cifrar mensaje completo (modo ECB básico)
    vector<bitset<16>> encryptMessage(const vector<bitset<16>>& message) {
        vector<bitset<16>> ciphertext;
        ciphertext.reserve(message.size());
        
        for (const auto& block : message) {
            ciphertext.push_back(encryptBlock(block));
        }
        
        return ciphertext;
    }

    // Descifrar mensaje completo (modo ECB básico)
    vector<bitset<16>> decryptMessage(const vector<bitset<16>>& ciphertext) {
        vector<bitset<16>> plaintext;
        plaintext.reserve(ciphertext.size());
        
        for (const auto& block : ciphertext) {
            plaintext.push_back(decryptBlock(block));
        }
          return plaintext;
    }
};

#endif
