# Toy Block Cipher

Implementación de un cifrado por bloques con soporte para modos de operación (ECB, CBC, CTR). Este proyecto se desarrolló para la materia de Introduction to Cryptography, utilizando una red de sustitución-permutación (SP-Network) de 16 bits con manejo seguro de claves e IVs en formato Base64.

## Estructura del Proyecto
```
Proyecto/
├── main.cpp                    # Archivo principal
├── src/
│   ├── base/
│   │   ├── base64.h           # Header de Base64
│   │   └── base64.cpp         # Implementación de Base64
│   ├── utils/
│   │   ├── CryptoUtils.h      # Utilidades criptográficas
│   │   ├── InputUtils.h       # Utilidades de entrada
│   │   └── UIUtils.h          # Utilidades de interfaz
│   ├── modes/
│   │   ├── SimpleCipher.cpp   # Modo ECB
│   │   ├── CBCCipher.cpp      # Modo CBC
│   │   └── CTRCipher.cpp      # Modo CTR
│   ├── keySchedule.cpp        # Generación de claves
│   ├── Permutation.cpp        # Operaciones de permutación
│   └── SBox.cpp               # Operaciones de S-Box
```

## Compilación

```powershell
g++ -std=c++17 -o cifrador main.cpp -lssl -lcrypto
```

## Ejecución
```powershell
./cifrador
```