#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include "CryptoUtils.cpp"
#include "SBox.cpp"
#include "Permutation.cpp"
#include "keySchedule.cpp"
#include "base/base64.h"

using namespace std;

class SimpleCipher {
private:
    SBox sbox;
    Permutation permutation;
    KeySchedule keySchedule;
    static const int NUM_ROUNDS = 5;

public:
    // Constructor
    SimpleCipher() : sbox(4), keySchedule(NUM_ROUNDS) {
        cout << "Inciando..." << "\n";
    }    
    
    // Cifrar un bloque de 16 bits
    bitset<16> encryptBlock(const bitset<16>& plaintext) {
        bitset<16> state = plaintext;
        
        for (int round = 1; round <= NUM_ROUNDS; round++) {
            // Paso 1: XOR con clave de ronda
            uint16_t roundKey = keySchedule.getRoundKey(round);
            bitset<16> keyBits(roundKey);
            state ^= keyBits;
            
            // Paso 2: Aplicar S-box a cada nibble de 4 bits
            vector<unsigned int> nibbles;
            for (int i = 0; i < 4; i++) {
                bitset<4> nibble = CryptoUtils::separateBitsReverse(state, i * 4);
                unsigned int nibbleValue = static_cast<unsigned int>(nibble.to_ulong());
                unsigned int sboxResult = sbox.applySBox(nibbleValue);
                nibbles.push_back(sboxResult);
            }
            
            state = CryptoUtils::construirBitset(nibbles);
            
            // Paso 3: Aplicar permutación
            state = permutation.applyPermutation(state);
        }
        
        return state;
    }

    // Descifrar un bloque de 16 bits
    bitset<16> decryptBlock(const bitset<16>& ciphertext) {
        bitset<16> state = ciphertext;
        
        for (int round = NUM_ROUNDS; round >= 1; round--) {
            // Paso 1: Aplicar permutación inversa
            state = permutation.applyInversePermutation(state);
            
            // Paso 2: Aplicar S-box inversa a cada nibble
            vector<unsigned int> nibbles;
            for (int i = 0; i < 4; i++) {
                bitset<4> nibble = CryptoUtils::separateBitsReverse(state, i * 4);
                unsigned int nibbleValue = static_cast<unsigned int>(nibble.to_ulong());
                unsigned int inverseSboxResult = sbox.applyInverseSBox(nibbleValue);
                nibbles.push_back(inverseSboxResult);
            }
            
            state = CryptoUtils::construirBitset(nibbles);
            
            // Paso 3: XOR con clave de ronda
            uint16_t roundKey = keySchedule.getRoundKey(round);
            bitset<16> keyBits(roundKey);
            state ^= keyBits;
        }
        
        return state;
    }

    // Cifrar mensaje completo (múltiples bloques)
    vector<bitset<16>> encryptMessage(const vector<bitset<16>>& message) {
        vector<bitset<16>> ciphertext;
        
        for (size_t i = 0; i < message.size(); i++) {
            bitset<16> encryptedBlock = encryptBlock(message[i]);
            ciphertext.push_back(encryptedBlock);
        }
        
        return ciphertext;
    }

    // Descifrar mensaje completo
    vector<bitset<16>> decryptMessage(const vector<bitset<16>>& ciphertext) {
        vector<bitset<16>> plaintext;
        
        for (size_t i = 0; i < ciphertext.size(); i++) {
            bitset<16> decryptedBlock = decryptBlock(ciphertext[i]);
            plaintext.push_back(decryptedBlock);
        }
        
        return plaintext;
    }

};

// Función para convertir string hexadecimal a bitset
bitset<16> hexStringToBitset(const string& hexStr) {
    unsigned long value = stoul(hexStr, nullptr, 16);
    return bitset<16>(value);
}

// Función para convertir bitset a string hexadecimal
string bitsetToHexString(const bitset<16>& bits) {
    stringstream ss;
    ss << hex << uppercase << setfill('0') << setw(4) << bits.to_ulong();
    return ss.str();
}

// Función para convertir string de texto a bloques de 16 bits
vector<bitset<16>> stringToBlocks(const string& text) {
    vector<bitset<16>> blocks;
    
    for (size_t i = 0; i < text.length(); i += 2) {
        uint16_t blockValue = 0;
        
        // Primer carácter (byte alto)
        blockValue |= (static_cast<uint16_t>(text[i]) << 8);
        
        // Segundo carácter (byte bajo), si existe
        if (i + 1 < text.length()) {
            blockValue |= static_cast<uint16_t>(text[i + 1]);
        }
        
        blocks.push_back(bitset<16>(blockValue));
    }
    
    return blocks;
}

// Función para convertir bloques de 16 bits a string de texto
string blocksToString(const vector<bitset<16>>& blocks) {
    string result;
    
    for (const auto& block : blocks) {
        uint16_t value = static_cast<uint16_t>(block.to_ulong());
        
        // Extraer byte alto (primer carácter)
        char highByte = static_cast<char>((value >> 8) & 0xFF);
        if (highByte != 0) {  // Solo agregar si no es carácter nulo
            result += highByte;
        }
        
        // Extraer byte bajo (segundo carácter)
        char lowByte = static_cast<char>(value & 0xFF);
        if (lowByte != 0) {  // Solo agregar si no es carácter nulo
            result += lowByte;
        }
    }
    
    return result;
}

// Función para convertir bloques cifrados a Base64
string blocksToBase64(const vector<bitset<16>>& blocks) {
    string binaryData;
    
    for (const auto& block : blocks) {
        uint16_t value = static_cast<uint16_t>(block.to_ulong());
        
        // Agregar byte alto
        binaryData += static_cast<char>((value >> 8) & 0xFF);
        // Agregar byte bajo
        binaryData += static_cast<char>(value & 0xFF);
    }
    
    return base64_encode(binaryData);
}

// Función para convertir Base64 a bloques de 16 bits
vector<bitset<16>> base64ToBlocks(const string& base64Data) {
    vector<bitset<16>> blocks;
    string decodedData = base64_decode(base64Data);
    
    for (size_t i = 0; i < decodedData.length(); i += 2) {
        uint16_t blockValue = 0;
        
        // Primer byte (byte alto)
        blockValue |= (static_cast<uint16_t>(static_cast<unsigned char>(decodedData[i])) << 8);
        
        // Segundo byte (byte bajo), si existe
        if (i + 1 < decodedData.length()) {
            blockValue |= static_cast<uint16_t>(static_cast<unsigned char>(decodedData[i + 1]));
        }
        
        blocks.push_back(bitset<16>(blockValue));
    }
    
    return blocks;
}

// Función para obtener mensaje de texto del usuario
vector<bitset<16>> getUserTextMessage() {
    string text;
    
    cout << "\nIngresa tu mensaje: ";
    cin.ignore(); // Limpiar buffer
    getline(cin, text);
    
    if (text.empty()) {
        cout << "Error: El mensaje no puede estar vacío." << "\n";
        return vector<bitset<16>>();
    }
    
    return stringToBlocks(text);
}

// Función para obtener datos Base64 del usuario (para descifrar)
vector<bitset<16>> getUserBase64Message() {
    string base64Text;
    
    cout << "\nIngresa el mensaje cifrado en Base64: ";
    cin.ignore(); // Limpiar buffer
    getline(cin, base64Text);
    
    if (base64Text.empty()) {
        cout << "Error: El mensaje no puede estar vacío." << "\n";
        return vector<bitset<16>>();
    }
    
    try {
        return base64ToBlocks(base64Text);
    } catch (const exception& e) {
        cout << "Error: Formato Base64 inválido." << "\n";
        return vector<bitset<16>>();
    }
}

// Función para mostrar mensaje de texto
void displayTextMessage(const vector<bitset<16>>& message, const string& title) {
    cout << "\n=== " << title << " ===" << "\n";
    string textResult = blocksToString(message);
    cout << "\"" << textResult << "\"" << "\n";
}

int main() {    
    try {
        cout << "=== CIFRADOR SP-NETWORK ===" << "\n";
        
        SimpleCipher cipher;

        while (true) {
            cout << "\n=== MENÚ ===" << "\n";
            cout << "1. Cifrar" << "\n";
            cout << "2. Descifrar" << "\n";
            cout << "3. Salir" << "\n";
            cout << "\nOpción: ";
            
            string choice;
            cin >> choice;
            
            if (choice == "1") {
                vector<bitset<16>> plaintext = getUserTextMessage();
                if (plaintext.empty()) continue;
                
                displayTextMessage(plaintext, "MENSAJE ORIGINAL");
                vector<bitset<16>> ciphertext = cipher.encryptMessage(plaintext);
                
                cout << "\n=== MENSAJE CIFRADO (BASE64) ===" << "\n";
                string base64Result = blocksToBase64(ciphertext);
                cout << base64Result << "\n";
                
            } else if (choice == "2") {
                vector<bitset<16>> ciphertext = getUserBase64Message();
                if (ciphertext.empty()) continue;
                
                cout << "\n=== MENSAJE CIFRADO ===" << "\n";
                string cipherResult = blocksToString(ciphertext);
                cout << "\"" << cipherResult << "\"" << "\n";
                
                vector<bitset<16>> plaintext = cipher.decryptMessage(ciphertext);
                
                cout << "\n=== MENSAJE DESCIFRADO ===" << "\n";
                string decryptedText = blocksToString(plaintext);
                cout << "\"" << decryptedText << "\"" << "\n";
                
            } else if (choice == "3") {
                cout << "\nSaliendo..." << "\n";
                break;
                
            } else {
                cout << "\nOpción inválida." << "\n";
            }
        }
        
    } catch (const exception& e) {
        cout << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}