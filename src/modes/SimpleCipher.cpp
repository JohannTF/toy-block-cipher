#ifndef SIMPLECIPHER_H
#define SIMPLECIPHER_H

#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include "../utils/CryptoUtils.h"
#include "../SBox.h"
#include "../Permutation.h"
#include "../KeySchedule.h"
#include "../base/base64.h"

using namespace std;

class SimpleCipher {
private:
    SBox sbox;
    Permutation permutation;
    KeySchedule* keySchedule;
    static const int NUM_ROUNDS = 5;

public:
    SimpleCipher() : sbox(4), keySchedule(nullptr) {
        // Generar clave aleatoria por defecto
        keySchedule = new KeySchedule(NUM_ROUNDS);
    }
    
    // Constructor con clave específica
    SimpleCipher(uint16_t masterKey) : sbox(4), keySchedule(nullptr) {
        keySchedule = new KeySchedule(masterKey, NUM_ROUNDS);
    }
    
    // Destructor
    ~SimpleCipher() {
        delete keySchedule;
    }
    
    // Copy constructor
    SimpleCipher(const SimpleCipher& other) : sbox(4), keySchedule(nullptr) {
        if (other.keySchedule) {
            keySchedule = new KeySchedule(*other.keySchedule);
        }
    }
    
    // Assignment operator
    SimpleCipher& operator=(const SimpleCipher& other) {
        if (this != &other) {
            delete keySchedule;
            keySchedule = nullptr;
            if (other.keySchedule) {
                keySchedule = new KeySchedule(*other.keySchedule);
            }
        }
        return *this;
    }
    
    // Obtener la clave maestra en formato Base64
    string getMasterKeyBase64() const {
        if (!keySchedule) return "";
        
        uint16_t key = keySchedule->getMasterKey();
        unsigned char keyBytes[2];
        keyBytes[0] = (key >> 8) & 0xFF;
        keyBytes[1] = key & 0xFF;
        
        string binaryData(reinterpret_cast<char*>(keyBytes), 2);
        return base64_encode(binaryData);
    }
    
    // Configurar nueva clave desde Base64
    void setMasterKeyFromBase64(const string& base64Key) {
        string decodedData = base64_decode(base64Key);
        
        if (decodedData.length() < 2) {
            throw invalid_argument("Clave Base64 invalida: datos insuficientes");
        }
        
        uint16_t key = (static_cast<uint16_t>(static_cast<unsigned char>(decodedData[0])) << 8) |
                       static_cast<uint16_t>(static_cast<unsigned char>(decodedData[1]));
        
        delete keySchedule;
        keySchedule = new KeySchedule(key, NUM_ROUNDS);
    }    // Cifrar un bloque de 16 bits
    bitset<16> encryptBlock(const bitset<16>& plaintext) {
        bitset<16> state = plaintext;
        
        for (int round = 1; round <= NUM_ROUNDS; round++) {
            uint16_t roundKey = keySchedule->getRoundKey(round);
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
            
            uint16_t roundKey = keySchedule->getRoundKey(round);
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
