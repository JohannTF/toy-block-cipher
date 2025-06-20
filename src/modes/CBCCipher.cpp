#ifndef CBCCIPHER_H
#define CBCCIPHER_H

#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <random>
#include <openssl/rand.h>
#include "SimpleCipher.cpp"

using namespace std;

class CBCCipher {
private:
    SimpleCipher cipher;

    // Generar IV aleatorio de 16 bits
    bitset<16> generateRandomIV() {
        unsigned char randomBytes[2]; // 2 bytes = 16 bits
        
        // Generar bytes aleatorios
        if (RAND_bytes(randomBytes, 2) != 1) {
            // Si falla, usar fallback con random_device
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<uint16_t> dis(0, UINT16_MAX);
            return bitset<16>(dis(gen));
        }
        
        // Convertir los 2 bytes a uint16_t
        uint16_t randomValue = (static_cast<uint16_t>(randomBytes[0]) << 8) | 
                               static_cast<uint16_t>(randomBytes[1]);
        
        return bitset<16>(randomValue);
    }

public:
    CBCCipher() {}
    
    // Obtener la clave maestra en formato Base64
    string getMasterKeyBase64() const {
        return cipher.getMasterKeyBase64();
    }
    
    // Configurar nueva clave desde Base64
    void setMasterKeyFromBase64(const string& base64Key) {
        cipher.setMasterKeyFromBase64(base64Key);
    }

    // Cifrar en modo CBC
    pair<bitset<16>, vector<bitset<16>>> encryptCBC(const vector<bitset<16>>& plaintext) {
        if (plaintext.empty()) {
            return {bitset<16>(0), vector<bitset<16>>()};
        }

        bitset<16> iv = generateRandomIV();
        vector<bitset<16>> ciphertext;
        ciphertext.reserve(plaintext.size());

        bitset<16> previousBlock = iv;

        for (const auto& block : plaintext) {
            // XOR con el bloque anterior (o IV para el primer bloque)
            bitset<16> xorResult = block ^ previousBlock;
            
            // Cifrar el resultado
            bitset<16> encryptedBlock = cipher.encryptBlock(xorResult);
            ciphertext.push_back(encryptedBlock);
            
            // El bloque cifrado se convierte en el "anterior" para la siguiente iteración
            previousBlock = encryptedBlock;
        }

        return {iv, ciphertext};
    }

    // Descifrar en modo CBC
    vector<bitset<16>> decryptCBC(const bitset<16>& iv, const vector<bitset<16>>& ciphertext) {
        if (ciphertext.empty()) {
            return vector<bitset<16>>();
        }

        vector<bitset<16>> plaintext;
        plaintext.reserve(ciphertext.size());

        bitset<16> previousBlock = iv;

        for (const auto& block : ciphertext) {
            // Descifrar el bloque
            bitset<16> decryptedBlock = cipher.decryptBlock(block);
            
            // XOR con el bloque anterior (o IV para el primer bloque)
            bitset<16> originalBlock = decryptedBlock ^ previousBlock;
            plaintext.push_back(originalBlock);
            
            // El bloque cifrado se convierte en el "anterior" para la siguiente iteración
            previousBlock = block;
        }        return plaintext;
    }
};

#endif
