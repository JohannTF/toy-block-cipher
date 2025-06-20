#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include "utils/CryptoUtils.h"
#include "utils/InputUtils.h"
#include "utils/UIUtils.h"
#include "modes/SimpleCipher.cpp"
#include "modes/CBCCipher.cpp"

using namespace std;

// ========== OPERACIONES ECB ==========

void processECBEncryption() {
    try {
        cin.ignore(); // Limpiar buffer después de leer opción del menú
        string plaintext = InputUtils::getTextInput("\nIngrese el mensaje a cifrar: ");
        
        if (plaintext.empty()) {
            UIUtils::showSimpleError("El mensaje no puede estar vacio.");
            return;
        }
        
        // Crear nueva instancia para generar clave aleatoria fresca
        SimpleCipher cipher;
        
        vector<bitset<16>> textBlocks = CryptoUtils::stringToBlocks(plaintext);
        vector<bitset<16>> cipherBlocks = cipher.encryptMessage(textBlocks);
        string base64Result = CryptoUtils::blocksToBase64(cipherBlocks);
        string masterKeyBase64 = cipher.getMasterKeyBase64();
        
        UIUtils::displayResult("MENSAJE ORIGINAL", "\"" + plaintext + "\"");
        UIUtils::displayResult("CLAVE MAESTRA (BASE64)", masterKeyBase64);
        UIUtils::displayResult("MENSAJE CIFRADO ECB (BASE64)", base64Result);
        
    } catch (const exception& e) {
        UIUtils::showError("el cifrado ECB", e.what());
    }
}

void processECBDecryption() {
    try {
        cin.ignore(); // Limpiar buffer después de leer opción del menú
        string masterKeyBase64 = InputUtils::getMasterKeyInput();
        string base64Text = InputUtils::getBase64InputECB();
        
        // Crear cipher con la clave proporcionada
        SimpleCipher cipher;
        cipher.setMasterKeyFromBase64(masterKeyBase64);
        
        vector<bitset<16>> cipherBlocks = CryptoUtils::base64ToBlocks(base64Text);
        vector<bitset<16>> plainBlocks = cipher.decryptMessage(cipherBlocks);
        string decryptedText = CryptoUtils::blocksToString(plainBlocks);
        
        UIUtils::displayResult("CLAVE MAESTRA (BASE64)", masterKeyBase64);
        UIUtils::displayResult("MENSAJE CIFRADO ECB (BASE64)", base64Text);
        UIUtils::displayResult("MENSAJE DESCIFRADO", "\"" + decryptedText + "\"");
        
    } catch (const exception& e) {
        UIUtils::showError("el descifrado ECB", e.what());
    }
}

// ========== OPERACIONES CBC ==========

void processCBCEncryption() {
    try {
        cin.ignore(); // Limpiar buffer después de leer opción del menú
        string plaintext = InputUtils::getTextInput("\nIngrese el mensaje a cifrar: ");
        
        if (plaintext.empty()) {
            UIUtils::showSimpleError("El mensaje no puede estar vacio.");
            return;
        }
        
        // Crear nueva instancia para generar clave aleatoria fresca
        CBCCipher cipher;
        
        vector<bitset<16>> textBlocks = CryptoUtils::stringToBlocks(plaintext);
        auto [iv, cipherBlocks] = cipher.encryptCBC(textBlocks);
        string base64Result = CryptoUtils::blocksToBase64(cipherBlocks);
        string masterKeyBase64 = cipher.getMasterKeyBase64();
        string ivBase64 = CryptoUtils::bitsetToBase64(iv);
        
        UIUtils::displayResult("MENSAJE ORIGINAL", "\"" + plaintext + "\"");
        UIUtils::displayResult("CLAVE MAESTRA (BASE64)", masterKeyBase64);
        UIUtils::displayResult("IV GENERADO (BASE64)", ivBase64);
        UIUtils::displayResult("MENSAJE CIFRADO CBC (BASE64)", base64Result);
        
    } catch (const exception& e) {
        UIUtils::showError("el cifrado CBC", e.what());
    }
}

void processCBCDecryption() {
    try {
        cin.ignore(); // Limpiar buffer después de leer opción del menú
        string masterKeyBase64 = InputUtils::getMasterKeyInput();
        bitset<16> iv = InputUtils::getIVInput();
        string base64Text = InputUtils::getBase64InputCBC();
        
        // Crear cipher con la clave proporcionada
        CBCCipher cipher;
        cipher.setMasterKeyFromBase64(masterKeyBase64);
        
        vector<bitset<16>> cipherBlocks = CryptoUtils::base64ToBlocks(base64Text);
        vector<bitset<16>> plainBlocks = cipher.decryptCBC(iv, cipherBlocks);
        string decryptedText = CryptoUtils::blocksToString(plainBlocks);
        string ivBase64 = CryptoUtils::bitsetToBase64(iv);
        
        UIUtils::displayResult("CLAVE MAESTRA (BASE64)", masterKeyBase64);
        UIUtils::displayResult("IV UTILIZADO (BASE64)", ivBase64);
        UIUtils::displayResult("MENSAJE CIFRADO CBC (BASE64)", base64Text);
        UIUtils::displayResult("MENSAJE DESCIFRADO", "\"" + decryptedText + "\"");
        
    } catch (const exception& e) {
        UIUtils::showError("el descifrado CBC", e.what());
    }
}

// ========== CONTROLADORES DE MENÚ ==========

void handleECBMenu() {
    string opChoice;
    while (true) {
        UIUtils::showOperationMenu("ECB");
        cin >> opChoice;
        if (opChoice == "1") {
            processECBEncryption();
        } 
        else if (opChoice == "2") {
            processECBDecryption();
        }
        else if (opChoice == "3") {
            break;
        } 
        else {
            UIUtils::showSimpleError("Opcion invalida. Por favor, seleccione 1, 2 o 3.");
        }
    }
}

void handleCBCMenu() {
    string opChoice;
    while (true) {
        UIUtils::showOperationMenu("CBC");
        cin >> opChoice;
        if (opChoice == "1") {
            processCBCEncryption();
        } 
        else if (opChoice == "2") {
            processCBCDecryption();
        }
        else if (opChoice == "3") {
            break;
        } 
        else {
            UIUtils::showSimpleError("Opcion invalida. Por favor, seleccione 1, 2 o 3.");
        }
    }
}

int main() {
    try {
        string mainChoice;
        
        while (true) {
            UIUtils::showMainMenu();
            cin >> mainChoice;
            
            if (mainChoice == "1") {
                // Modo ECB
                handleECBMenu();
            } 
            else if (mainChoice == "2") {
                // Modo CBC
                handleCBCMenu();
            } 
            else if (mainChoice == "3") {
                cout << "\nSaliendo del programa..." << endl;
                break;
            } 
            else {
                UIUtils::showSimpleError("Opcion invalida. Por favor, seleccione 1, 2 o 3.");
            }
        }
        
    } catch (const exception& e) {
        cout << "\nError critico: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}