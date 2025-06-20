#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include "modes/SimpleCipher.cpp"
#include "modes/CBCCipher.cpp"
#include "base/base64.cpp"

using namespace std;

// Convertir texto a bloques de 16 bits
vector<bitset<16>> stringToBlocks(const string& text) {
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
string blocksToString(const vector<bitset<16>>& blocks) {
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

// Convertir IV y bloques a Base64 (para CBC)
string cbcToBase64(const bitset<16>& iv, const vector<bitset<16>>& blocks) {
    string binaryData;
    
    // Agregar IV al inicio
    uint16_t ivValue = static_cast<uint16_t>(iv.to_ulong());
    binaryData += static_cast<char>((ivValue >> 8) & 0xFF);
    binaryData += static_cast<char>(ivValue & 0xFF);
    
    // Agregar bloques cifrados
    for (const auto& block : blocks) {
        uint16_t value = static_cast<uint16_t>(block.to_ulong());
        binaryData += static_cast<char>((value >> 8) & 0xFF);
        binaryData += static_cast<char>(value & 0xFF);
    }
    
    return base64_encode(binaryData);
}

// Convertir Base64 a IV y bloques (para CBC)
pair<bitset<16>, vector<bitset<16>>> base64ToCBC(const string& base64Data) {
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

// Convertir bloques a Base64 (para ECB)
string blocksToBase64(const vector<bitset<16>>& blocks) {
    string binaryData;
    
    for (const auto& block : blocks) {
        uint16_t value = static_cast<uint16_t>(block.to_ulong());
        binaryData += static_cast<char>((value >> 8) & 0xFF);
        binaryData += static_cast<char>(value & 0xFF);
    }
    
    return base64_encode(binaryData);
}

// Convertir Base64 a bloques (para ECB)
vector<bitset<16>> base64ToBlocks(const string& base64Data) {
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

// Convertir bitset a hexadecimal
string bitsetToHex(const bitset<16>& bits) {
    stringstream ss;
    ss << "0x" << hex << uppercase << setfill('0') << setw(4) << bits.to_ulong();
    return ss.str();
}

// Obtener mensaje de texto del usuario
string getTextInput(const string& prompt) {
    string text;
    cout << prompt;
    cin.ignore();
    getline(cin, text);
    return text;
}

// Validar y obtener entrada Base64 para ECB
string getBase64InputECB() {
    string base64Text = getTextInput("\nIngrese el mensaje cifrado en Base64 (ECB): ");
    
    if (base64Text.empty()) {
        throw invalid_argument("El mensaje no puede estar vacio");
    }
    
    return base64Text;
}

// Validar y obtener entrada Base64 para CBC
string getBase64InputCBC() {
    string base64Text = getTextInput("\nIngrese el mensaje cifrado en Base64 (CBC): ");
    
    if (base64Text.empty()) {
        throw invalid_argument("El mensaje no puede estar vacio");
    }
    
    return base64Text;
}

// Mostrar resultado
void displayResult(const string& title, const string& content) {
    cout << "\n=== " << title << " ===" << endl;
    cout << content << endl;
}

// Mostrar menu principal
void showMainMenu() {
    cout << "\n========================================" << endl;
    cout << "         CIFRADOR SP-NETWORK           " << endl;
    cout << "========================================" << endl;
    cout << "1. Modo ECB (Electronic Codebook)" << endl;
    cout << "2. Modo CBC (Cipher Block Chaining)" << endl;
    cout << "3. Salir" << endl;
    cout << "----------------------------------------" << endl;
    cout << "Seleccione el modo de operacion: ";
}

// Mostrar menu de operaciones
void showOperationMenu(const string& mode) {
    cout << "\n================================" << endl;
    cout << "        MODO " << mode << endl;
    cout << "================================" << endl;
    cout << "1. Cifrar mensaje" << endl;
    cout << "2. Descifrar mensaje" << endl;
    cout << "3. Volver al menu principal" << endl;
    cout << "--------------------------------" << endl;
    cout << "Seleccione una opcion: ";
}

// Procesar cifrado ECB
void processECBEncryption(SimpleCipher& cipher) {
    try {
        string plaintext = getTextInput("\nIngrese el mensaje a cifrar: ");
        
        if (plaintext.empty()) {
            cout << "\nError: El mensaje no puede estar vacio." << endl;
            return;
        }
        
        vector<bitset<16>> textBlocks = stringToBlocks(plaintext);
        vector<bitset<16>> cipherBlocks = cipher.encryptMessage(textBlocks);
        string base64Result = blocksToBase64(cipherBlocks);
        
        displayResult("MENSAJE ORIGINAL", "\"" + plaintext + "\"");
        displayResult("MENSAJE CIFRADO ECB (BASE64)", base64Result);
        
    } catch (const exception& e) {
        cout << "\nError durante el cifrado ECB: " << e.what() << endl;
    }
}

// Procesar descifrado ECB
void processECBDecryption(SimpleCipher& cipher) {
    try {
        string base64Text = getBase64InputECB();
        
        vector<bitset<16>> cipherBlocks = base64ToBlocks(base64Text);
        vector<bitset<16>> plainBlocks = cipher.decryptMessage(cipherBlocks);
        string decryptedText = blocksToString(plainBlocks);
        
        displayResult("MENSAJE CIFRADO ECB (BASE64)", base64Text);
        displayResult("MENSAJE DESCIFRADO", "\"" + decryptedText + "\"");
        
    } catch (const exception& e) {
        cout << "\nError durante el descifrado ECB: " << e.what() << endl;
    }
}

// Procesar cifrado CBC
void processCBCEncryption(CBCCipher& cipher) {
    try {
        string plaintext = getTextInput("\nIngrese el mensaje a cifrar: ");
        
        if (plaintext.empty()) {
            cout << "\nError: El mensaje no puede estar vacio." << endl;
            return;
        }
        
        vector<bitset<16>> textBlocks = stringToBlocks(plaintext);
        auto [iv, cipherBlocks] = cipher.encryptCBC(textBlocks);
        string base64Result = cbcToBase64(iv, cipherBlocks);
        
        displayResult("MENSAJE ORIGINAL", "\"" + plaintext + "\"");
        displayResult("IV GENERADO", bitsetToHex(iv));
        displayResult("MENSAJE CIFRADO CBC (BASE64)", base64Result);
        
    } catch (const exception& e) {
        cout << "\nError durante el cifrado CBC: " << e.what() << endl;
    }
}

// Procesar descifrado CBC
void processCBCDecryption(CBCCipher& cipher) {
    try {
        string base64Text = getBase64InputCBC();
        
        auto [iv, cipherBlocks] = base64ToCBC(base64Text);
        vector<bitset<16>> plainBlocks = cipher.decryptCBC(iv, cipherBlocks);
        string decryptedText = blocksToString(plainBlocks);
        
        displayResult("MENSAJE CIFRADO CBC (BASE64)", base64Text);
        displayResult("IV EXTRAIDO", bitsetToHex(iv));
        displayResult("MENSAJE DESCIFRADO", "\"" + decryptedText + "\"");
        
    } catch (const exception& e) {
        cout << "\nError durante el descifrado CBC: " << e.what() << endl;
    }
}

int main() {
    try {
        SimpleCipher ecbCipher;
        CBCCipher cbcCipher;
        string mainChoice, opChoice;
        
        while (true) {
            showMainMenu();
            cin >> mainChoice;
            
            if (mainChoice == "1") {
                // Modo ECB
                while (true) {
                    showOperationMenu("ECB");
                    cin >> opChoice;
                    
                    if (opChoice == "1") {
                        processECBEncryption(ecbCipher);
                    } 
                    else if (opChoice == "2") {
                        processECBDecryption(ecbCipher);
                    } 
                    else if (opChoice == "3") {
                        break;
                    } 
                    else {
                        cout << "\nOpcion invalida. Por favor, seleccione 1, 2 o 3." << endl;
                    }
                }
            } 
            else if (mainChoice == "2") {
                // Modo CBC
                while (true) {
                    showOperationMenu("CBC");
                    cin >> opChoice;
                    
                    if (opChoice == "1") {
                        processCBCEncryption(cbcCipher);
                    } 
                    else if (opChoice == "2") {
                        processCBCDecryption(cbcCipher);
                    } 
                    else if (opChoice == "3") {
                        break;
                    } 
                    else {
                        cout << "\nOpcion invalida. Por favor, seleccione 1, 2 o 3." << endl;
                    }
                }
            } 
            else if (mainChoice == "3") {
                cout << "\nSaliendo del programa..." << endl;
                break;
            } 
            else {
                cout << "\nOpcion invalida. Por favor, seleccione 1, 2 o 3." << endl;
            }
        }
        
    } catch (const exception& e) {
        cout << "\nError critico: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}