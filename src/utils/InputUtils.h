#ifndef INPUTUTILS_H
#define INPUTUTILS_H

#include <iostream>
#include <string>
#include <bitset>
#include <stdexcept>
#include "CryptoUtils.h"

using namespace std;

// ========== CLASE PARA UTILIDADES DE ENTRADA ==========
class InputUtils {
public:
    // Obtener entrada de texto del usuario
    static string getTextInput(const string& prompt) {
        string text;
        cout << prompt;
        getline(cin, text);
        return text;
    }

    // Obtener clave maestra del usuario
    static string getMasterKeyInput() {
        string keyText = getTextInput("\nIngrese la clave maestra en Base64: ");
        
        if (keyText.empty()) {
            throw invalid_argument("La clave no puede estar vacia");
        }
        
        return keyText;
    }

    // Obtener entrada Base64 para ECB
    static string getBase64InputECB() {
        string base64Text = getTextInput("\nIngrese el mensaje cifrado en Base64 (ECB): ");
        
        if (base64Text.empty()) {
            throw invalid_argument("El mensaje no puede estar vacio");
        }
        
        return base64Text;
    }

    // Obtener entrada Base64 para CBC
    static string getBase64InputCBC() {
        string base64Text = getTextInput("\nIngrese el mensaje cifrado en Base64 (CBC): ");
        
        if (base64Text.empty()) {
            throw invalid_argument("El mensaje no puede estar vacio");
        }
        
        return base64Text;
    }

    // Obtener IV del usuario para descifrado CBC
    static bitset<16> getIVInput() {
        string ivText = getTextInput("\nIngrese el vector de inicializacion (IV) en Base64: ");
        
        if (ivText.empty()) {
            throw invalid_argument("El IV no puede estar vacio");
        }
        
        try {
            return CryptoUtils::base64ToBitset(ivText);
        } catch (const exception& e) {
            throw invalid_argument("Formato de IV invalido. Use formato Base64 valido");
        }
    }
};

#endif
