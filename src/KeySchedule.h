#ifndef KEYSCHEDULE_H
#define KEYSCHEDULE_H

#include <iostream>
#include <vector>
#include <bitset>
#include <iomanip>
#include <array>
#include <cstdint>
#include <random>
#include <openssl/rand.h>

using namespace std;

class KeySchedule {
private:
    uint16_t masterKey;
    vector<uint16_t> roundKeys;
    vector<uint16_t> inverseRoundKeys;
    int numRounds;

    // Función para separar en cuatro nibbles de 4 bits
    vector<uint8_t> splitToNibbles(uint16_t value) {
        vector<uint8_t> nibbles(4);
        nibbles[0] = (value >> 12) & 0xF;
        nibbles[1] = (value >> 8) & 0xF;
        nibbles[2] = (value >> 4) & 0xF;
        nibbles[3] = value & 0xF;
        return nibbles;
    }
    
    // Función para combinar cuatro nibbles en un uint16_t
    uint16_t combineNibbles(const vector<uint8_t>& nibbles) {
        return (static_cast<uint16_t>(nibbles[0]) << 12) |
               (static_cast<uint16_t>(nibbles[1]) << 8) |
               (static_cast<uint16_t>(nibbles[2]) << 4) |
               static_cast<uint16_t>(nibbles[3]);
    }

    // Generar clave aleatoria criptográficamente segura
    uint16_t generateSecureRandomKey() {
        unsigned char randomBytes[2];
        
        int result = RAND_bytes(randomBytes, 2);
        if (result != 1) {
            throw runtime_error("Error: No se pudo generar clave criptograficamente segura con OpenSSL");
        }
        
        return (static_cast<uint16_t>(randomBytes[0]) << 8) | 
        static_cast<uint16_t>(randomBytes[1]);
    }

public:
    KeySchedule(uint16_t key, int rounds) : masterKey(key), numRounds(rounds) {
        generateRoundKeys();
        generateInverseRoundKeys();
    }

    // Constructor adicional para generar clave aleatoria
    KeySchedule(int rounds) : numRounds(rounds) {
        masterKey = generateSecureRandomKey();
        generateRoundKeys();
        generateInverseRoundKeys();
    }
    
    // Copy constructor
    KeySchedule(const KeySchedule& other) : masterKey(other.masterKey), numRounds(other.numRounds) {
        roundKeys = other.roundKeys;
        inverseRoundKeys = other.inverseRoundKeys;
    }

    // Precomputar las llaves para cada ronda
    void generateRoundKeys() {
        roundKeys.clear();
        roundKeys.reserve(numRounds);
        
        uint16_t currentKey = masterKey;
        
        for(int round = 1; round <= numRounds; round++) {
            // Separar en cuatro nibbles de 4 bits
            auto nibbles = splitToNibbles(currentKey);
            // Sumar 1 a cada nibble con overflow controlado
            for(int i = 0; i < 4; i++) {
                nibbles[i] = (nibbles[i] + 1) % 16; // Módulo 16 para 4 bits
            }
            
            // Combinar nibbles
            uint16_t tempKey = combineNibbles(nibbles);
            
            // Aplicar rotación circular a la izquierda de (round - 1) posiciones
            uint16_t roundKey = (tempKey << (round - 1)) | (tempKey >> (16 - (round - 1)));
            
            roundKeys.push_back(roundKey);
            
            // Para la siguiente iteración, usar la clave generada
            currentKey = roundKey;
        }
    }
    
    // Generar claves inversas para el proceso de descifrado
    void generateInverseRoundKeys() {
        inverseRoundKeys.clear();
        inverseRoundKeys.reserve(numRounds);

        for(int round = numRounds; round >= 1; round--) {
            uint16_t forwardKey = roundKeys[round - 1];
            
            // Aplicar rotación inversa (derecha) de (round - 1) posiciones
            uint16_t rotatedKey = (forwardKey >> (round - 1)) | (forwardKey << (16 - (round - 1)));
            
            // Separar en nibbles
            auto nibbles = splitToNibbles(rotatedKey);
            
            // Restar 1 de cada nibble (operación inversa de la suma)
            for(int i = 0; i < 4; i++) {
                nibbles[i] = (nibbles[i] - 1 + 16) % 16; // +16 para evitar negativos
            }
            
            // Combinar nibbles para obtener la clave inversa
            uint16_t inverseKey = combineNibbles(nibbles);
            
            inverseRoundKeys.push_back(inverseKey);
        }    
    }
    
    // Mostrar las llaves de cada ronda generadas
    void showAllKeys() {
        cout << "\n=== ALL PRECOMPUTED KEYS ===" << "\n";
        cout << "Master Key: 0x" << hex << uppercase << masterKey 
                  << " (" << bitset<16>(masterKey) << ")" << "\n";
        
        for(size_t i = 0; i < roundKeys.size(); i++) {
            cout << "K" << (i+1) << ": 0x" 
                      << hex << uppercase << roundKeys[i]
                      << " (" << bitset<16>(roundKeys[i]) << ")" << "\n";
        }
    }

    // Mostrar las llaves inversas de cada ronda
    void showInverseKeys() {
        cout << "\n=== ALL INVERSE KEYS ===" << "\n";
        for(size_t i = 0; i < inverseRoundKeys.size(); i++) {
            cout << "IK" << (i+1) << ": 0x" 
                << hex << uppercase << inverseRoundKeys[i]                      
                << " (" << bitset<16>(inverseRoundKeys[i]) << ")" << "\n";
        }
    }
    
    // Obtener clave de ronda (si está precomputada)
    uint16_t getRoundKey(int round) {
        if(round < 1 || round > roundKeys.size()) {
            throw out_of_range("Round number out of range");
        }
        return roundKeys[round - 1];
    }    // Obtener clave inversa de ronda (si está precomputada)
    uint16_t getInverseRoundKey(int round) {
        if(round < 1 || round > inverseRoundKeys.size()) {
            throw out_of_range("Round number out of range");
        }
        return inverseRoundKeys[round - 1];
    }

    // Obtener la clave maestra
    uint16_t getMasterKey() const {
        return masterKey;
    }

};

#endif
