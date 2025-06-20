#ifndef UIUTILS_H
#define UIUTILS_H

#include <iostream>
#include <string>

using namespace std;

// ========== CLASE PARA UTILIDADES DE INTERFAZ ==========
class UIUtils {
public:
    // Mostrar resultado formateado
    static void displayResult(const string& title, const string& content) {
        cout << "\n=== " << title << " ===" << endl;
        cout << content << endl;
    }

    // Mostrar menu principal
    static void showMainMenu() {
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
    static void showOperationMenu(const string& mode) {
        cout << "\n================================" << endl;
        cout << "        MODO " << mode << endl;
        cout << "================================" << endl;
        cout << "1. Cifrar mensaje" << endl;
        cout << "2. Descifrar mensaje" << endl;
        cout << "3. Volver al menu principal" << endl;
        cout << "--------------------------------" << endl;
        cout << "Seleccione una opcion: ";
    }

    // Mostrar mensaje de error
    static void showError(const string& operation, const string& errorMsg) {
        cout << "\nError durante " << operation << ": " << errorMsg << endl;
    }

    // Mostrar mensaje de error simple
    static void showSimpleError(const string& errorMsg) {
        cout << "\nError: " << errorMsg << endl;
    }
};

#endif
