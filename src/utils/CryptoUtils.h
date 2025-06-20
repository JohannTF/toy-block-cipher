#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <bitset>
#include <cstdint>
#include "../base/base64.cpp"

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

    // ========== FUNCIONES DE CONVERSIÓN ==========

    // Convertir texto a bloques de 16 bits
    static vector<bitset<16>> stringToBlocks(const string& text) {
        vector<bitset<16>> blocks;
        
        for (size_t i = 0; i < text.length(); i += 2) {
            uint16_t blockValue = 0;
            
            blockValue |= (static_cast<uint16_t>(text[i]) << 8);
            
            if (i + 1 < text.length()) {
                blockValue |= static_cast<uint16_t>(text[i + 1]);
            }
            
            blocks.push_back(bitset<16>(blockValue));
        }
        
        return blocks;
    }

    // Convertir bloques de 16 bits a texto
    static string blocksToString(const vector<bitset<16>>& blocks) {
        string result;
        
        for (const auto& block : blocks) {
            uint16_t value = static_cast<uint16_t>(block.to_ulong());
            
            char highByte = static_cast<char>((value >> 8) & 0xFF);
            if (highByte != 0) {
                result += highByte;
            }
            
            char lowByte = static_cast<char>(value & 0xFF);
            if (lowByte != 0) {
                result += lowByte;
            }
        }
        
        return result;
    }

    // Convertir bloques a Base64
    static string blocksToBase64(const vector<bitset<16>>& blocks) {
        string binaryData;
        
        for (const auto& block : blocks) {
            uint16_t value = static_cast<uint16_t>(block.to_ulong());
            binaryData += static_cast<char>((value >> 8) & 0xFF);
            binaryData += static_cast<char>(value & 0xFF);
        }
        
        return base64_encode(binaryData);
    }

    // Convertir Base64 a bloques
    static vector<bitset<16>> base64ToBlocks(const string& base64Data) {
        vector<bitset<16>> blocks;
        string decodedData = base64_decode(base64Data);
        
        for (size_t i = 0; i < decodedData.length(); i += 2) {
            uint16_t blockValue = 0;
            
            blockValue |= (static_cast<uint16_t>(static_cast<unsigned char>(decodedData[i])) << 8);
            
            if (i + 1 < decodedData.length()) {
                blockValue |= static_cast<uint16_t>(static_cast<unsigned char>(decodedData[i + 1]));
            }
            
            blocks.push_back(bitset<16>(blockValue));
        }
        
        return blocks;
    }

    // Convertir bitset a Base64
    static string bitsetToBase64(const bitset<16>& bits) {
        uint16_t value = static_cast<uint16_t>(bits.to_ulong());
        unsigned char bytes[2];
        bytes[0] = (value >> 8) & 0xFF;
        bytes[1] = value & 0xFF;
        
        string binaryData(reinterpret_cast<char*>(bytes), 2);
        return base64_encode(binaryData);
    }

    // Convertir Base64 a bitset
    static bitset<16> base64ToBitset(const string& base64Data) {
        string decodedData = base64_decode(base64Data);
        
        if (decodedData.length() < 2) {
            throw invalid_argument("Base64 invalido: datos insuficientes");
        }
        
        uint16_t value = (static_cast<uint16_t>(static_cast<unsigned char>(decodedData[0])) << 8) |
                         static_cast<uint16_t>(static_cast<unsigned char>(decodedData[1]));
        
        return bitset<16>(value);
    }
    
    // Convertir bitset a hexadecimal
    static string bitsetToHex(const bitset<16>& bits) {
        stringstream ss;
        ss << "0x" << hex << uppercase << setfill('0') << setw(4) << bits.to_ulong();
        return ss.str();
    }

    // ========== FUNCIONES CBC GENERALIZADAS ==========

    // Convertir IV y bloques a Base64 (versión generalizada)
    static string ivAndBlocksToBase64(const bitset<16>& iv, const vector<bitset<16>>& blocks) {
        string binaryData;
        
        // Agregar IV al inicio
        uint16_t ivValue = static_cast<uint16_t>(iv.to_ulong());
        binaryData += static_cast<char>((ivValue >> 8) & 0xFF);
        binaryData += static_cast<char>(ivValue & 0xFF);
        
        // Agregar bloques
        for (const auto& block : blocks) {
            uint16_t value = static_cast<uint16_t>(block.to_ulong());
            binaryData += static_cast<char>((value >> 8) & 0xFF);
            binaryData += static_cast<char>(value & 0xFF);
        }
        
        return base64_encode(binaryData);
    }

    // Convertir Base64 a IV y bloques (versión generalizada)
    static pair<bitset<16>, vector<bitset<16>>> base64ToIvAndBlocks(const string& base64Data) {
        string decodedData = base64_decode(base64Data);
        
        if (decodedData.length() < 2) {
            throw invalid_argument("Datos insuficientes para extraer IV");
        }
        
        // Extraer IV (primeros 2 bytes)
        uint16_t ivValue = (static_cast<uint16_t>(static_cast<unsigned char>(decodedData[0])) << 8) |
                           static_cast<uint16_t>(static_cast<unsigned char>(decodedData[1]));
        bitset<16> iv(ivValue);
        
        // Extraer bloques restantes
        vector<bitset<16>> blocks;
        for (size_t i = 2; i < decodedData.length(); i += 2) {
            uint16_t blockValue = 0;
            
            blockValue |= (static_cast<uint16_t>(static_cast<unsigned char>(decodedData[i])) << 8);
            
            if (i + 1 < decodedData.length()) {
                blockValue |= static_cast<uint16_t>(static_cast<unsigned char>(decodedData[i + 1]));
            }
            
            blocks.push_back(bitset<16>(blockValue));
        }
        
        return {iv, blocks};
    }
};

#endif
