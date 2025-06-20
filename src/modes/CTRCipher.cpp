#ifndef CTRCIPHER_H
#define CTRCIPHER_H

#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <random>
#include <chrono>
#include <openssl/rand.h>
#include "SimpleCipher.cpp"
#include "../utils/CryptoUtils.h"

using namespace std;

class CTRCipher {
private:
    SimpleCipher cipher;

    // Generar IV aleatorio de 16 bits
    bitset<8> generateRandomIV() {
        unsigned char randomBytes[1]; // 1 byte = 8 bits
        
        // Generar bytes aleatorios
        if (RAND_bytes(randomBytes, 1) != 1) {
            // Si falla, usar fallback con random_device
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<uint8_t> dis(0, UINT8_MAX);
            return bitset<8>(dis(gen));
        }
        
        // Convertir los 8 bits a uint8_t
        uint8_t randomValue = static_cast<uint8_t>(randomBytes[0]);
        
        return bitset<8>(randomValue);
    }   

public:
    CTRCipher() {}

    // Obtener la clave maestra en formato Base64
    string getMasterKeyBase64() const {
        return cipher.getMasterKeyBase64();
    }
    
    // Configurar nueva clave desde Base64
    void setMasterKeyFromBase64(const string& base64Key) {
        cipher.setMasterKeyFromBase64(base64Key);
    }

    // Cifrar en modo CTR
    pair<bitset<8>, vector<bitset<16>>> encryptCTR(const vector<bitset<16>>& plaintext) {
        if (plaintext.empty()) {
            return {bitset<8>(0), vector<bitset<16>>()};
        }

        bitset<8> iv = generateRandomIV();
        vector<bitset<16>> ciphertext;
        ciphertext.reserve(plaintext.size());

        bitset<16> counterBits;
        int counter = 0;
        for (const auto& block : plaintext) {
            // Inicializar el contador con el IV
            counterBits = CryptoUtils::counterGenerator(iv, counter);
            // Cifrar el contador
            bitset<16> encryptedCounter = cipher.encryptBlock(counterBits);
            
            // XOR con el bloque de texto plano
            bitset<16> xorResult = block ^ encryptedCounter;
            ciphertext.push_back(xorResult);
            
            // Incrementar el contador
            counter++;
        }

        return {iv, ciphertext};
    }

    // Descifrar en modo CTR
    vector<bitset<16>> decryptCTR(const bitset<8>& iv, const vector<bitset<16>>& ciphertext) {
        if (ciphertext.empty()) {
            return vector<bitset<16>>();
        }

        vector<bitset<16>> plaintext;
        plaintext.reserve(ciphertext.size());

        // Contador inicial
        bitset<16> counterBits;
        int counter = 0;
        for (const auto& block : ciphertext){
            // Inicializar el contador con el IV
            counterBits = CryptoUtils::counterGenerator(iv, counter);
            // Cifrar el contador
            bitset<16> encryptedCounter = cipher.encryptBlock(counterBits);
            
            // XOR con el bloque cifrado
            bitset<16> xorResult = block ^ encryptedCounter;
            plaintext.push_back(xorResult);
            
            // Incrementar el contador
            counter++;
        }

        return plaintext;
    }

};

#endif