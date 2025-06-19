#include <iostream>
#include <bitset>
#include <sstream>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include "base64.h"
#include <fstream>
#include <vector>

using namespace std;

bitset<10> getKeyFromUser(){
    cout << "Ingrese la clave: ";
    int intKey;
    cin >> intKey;
    if (intKey < 0 || intKey > 1023)
    { // Verificar que esté en el rango de 10 bits
        throw invalid_argument("El valor entero debe estar entre 0 y 1023.");
    }
    bitset<10> key;
    for (int i = 0; i < 8; i++)
    {
        key[i] = (intKey >> i) & 1;
    }
    return key;
}

// Funcion para convertir texto a su representacion binaria ASCII
string textToBinary(const string &text) {
    string binary;
    for (char c : text) {
        // Convertir cada carácter a su valor ASCII en binario (8 bits)
        binary += bitset<8>(static_cast<unsigned char>(c)).to_string();
    }
    return binary;
}

// Funcion para obtener el nombre base del archivo eliminando extensiones previas
string getBaseFilename(const string &filename, const string &extensionToRemove) {
    size_t pos = filename.rfind(extensionToRemove);
    if (pos != string::npos) {
        return filename.substr(0, pos); // Eliminar la extension previa
    }
    return filename;
}

// Funcion para realizar las permutaciones para obtener las subkeys
uint16_t permuteBits(uint16_t input, const int positions[], int size){
    uint16_t result = 0;
    for (int i = 0; i < size; ++i)
    {
        // Extraer el bit de la posicion especificada y colocarlo en la posicion correspondiente
        result |= ((input >> (10 - positions[i])) & 1) << (size - 1 - i);
    }
    return result;
}

// Funcion para generar subkeys
pair<bitset<8>, bitset<8>> generateSubkeys(bitset<10> key){
    // Permutacion inicial de la llave (3 5 2 7 4 10 1 9 8 6)
    const int initialPermutation[10] = {3, 5, 2, 7, 4, 10, 1, 9, 8, 6};
    uint16_t permutedKey = permuteBits(static_cast<uint16_t>(key.to_ulong()), initialPermutation, 10);

    // Separar en dos mitades de 5 bits
    uint8_t left = (permutedKey >> 5) & 0b11111; // Mitad izquierda
    uint8_t right = permutedKey & 0b11111;       // Mitad derecha

    // Corrimiento circular a la izquierda de 1 bit para ambas mitades
    left = ((left << 1) | (left >> 4)) & 0b11111;
    right = ((right << 1) | (right >> 4)) & 0b11111;

    // Generar la primera subclave K1 (6 3 7 4 8 5 10 9)
    const int permutation8[8] = {6, 3, 7, 4, 8, 5, 10, 9};
    uint16_t combined = (left << 5) | right; // Combinar ambas mitades
    uint8_t K1 = permuteBits(combined, permutation8, 8);

    // Corrimiento circular adicional de 2 bits a la izquierda para ambas mitades
    left = ((left << 2) | (left >> 3)) & 0b11111;
    right = ((right << 2) | (right >> 3)) & 0b11111;

    // Generar la segunda subclave K2 (6 3 7 4 8 5 10 9)
    combined = (left << 5) | right; // Combinar ambas mitades
    uint8_t K2 = permuteBits(combined, permutation8, 8);

    // Retornar las subclaves como bitset<8>
    return {bitset<8>(K1), bitset<8>(K2)};
}

// Codificar una cadena binaria a Base64
string encodeBase64(const string &binaryInput){
    // Calcular la longitud en bits
    int bitLength = binaryInput.size();

    // Convertir la cadena binaria a bytes
    int byteLength = (bitLength + 7) / 8; // Redondear hacia arriba
    unsigned char bytes[byteLength] = {0};

    for (int i = 0; i < bitLength; ++i)
    {
        if (binaryInput[i] == '1')
        {
            bytes[i / 8] |= (1 << (7 - (i % 8))); // Establecer el bit correspondiente
        }
    }

    // Codificar los bytes a Base64
    return base64_encode(bytes, byteLength);
}


// Generar una llave aleatoria para DES y codificarla en Base64
string generateRandomDESKey(){
    const int keySize = 8; // Tamaño de la llave DES en bytes (64 bits)
    unsigned char key[keySize];

    // Generar una llave aleatoria usando OpenSSL
    if (!RAND_bytes(key, keySize))
    {
        throw runtime_error("Error al generar la llave aleatoria");
    }

    // Convertir la llave a una cadena binaria
    string binaryKey;
    for (int i = 0; i < keySize; ++i)
    {
        binaryKey += bitset<8>(key[i]).to_string(); // Convertir cada byte a binario
    }

    // Codificar la llave en Base64
    return encodeBase64(binaryKey);
}

void encryptFileWithDES(const string &base64Key, const string &filename) {
    // Decodificar la clave Base64
    string keyBytes = base64_decode(base64Key);

    // Verificar que la clave tenga el tamaño correcto para DES (8 bytes)
    if (keyBytes.length() != 8) {
        throw runtime_error("La clave DES debe ser de 8 bytes (64 bits)");
    }

    // Leer el archivo de entrada
    ifstream inFile(filename, ios::binary);
    if (!inFile) {
        throw runtime_error("No se puede abrir el archivo: " + filename);
    }

    // Obtener el tamaño del archivo
    inFile.seekg(0, ios::end);
    long fileSize = inFile.tellg();
    inFile.seekg(0, ios::beg);

    // Leer el contenido del archivo
    vector<char> buffer(fileSize);
    inFile.read(buffer.data(), fileSize);
    vector<unsigned char> fileContent(buffer.begin(), buffer.end());

    // Calcular el padding necesario (DES trabaja con bloques de 8 bytes)
    int paddingSize = 8 - (fileSize % 8);
    if (paddingSize == 8)
        paddingSize = 0;

    // Añadir el padding
    for (int i = 0; i < paddingSize; i++) {
        fileContent.push_back(static_cast<unsigned char>(paddingSize));
    }

    // Configurar la clave para DES
    DES_cblock keyblock;
    memcpy(keyblock, keyBytes.c_str(), 8);
    /* keyBytes.c_str(): 
    Es un puntero a los datos de la cadena keyBytes, que contiene la clave DES decodificada desde Base64. 
    El método c_str() devuelve un puntero de tipo const char* a los datos del string */

    DES_key_schedule keysched;
    DES_set_key_unchecked(&keyblock, &keysched);

    // Preparar el buffer para datos cifrados
    vector<unsigned char> encryptedData(fileContent.size());

    // Cifrar los datos usando DES en modo CBC
    DES_cblock ivec = {0}; // Vector de inicializacion (todos ceros)

    DES_ncbc_encrypt(
        fileContent.data(),
        encryptedData.data(),
        fileContent.size(),
        &keysched,
        &ivec,
        DES_ENCRYPT
    );

    // Codificar los datos cifrados en Base64
    string base64EncryptedData = base64_encode(encryptedData.data(), encryptedData.size());

    // Generar el nombre del archivo de salida
    string baseFilename = getBaseFilename(filename, ".txt");
    string outputFilename = baseFilename + "_enc"+ ".txt";

    // Guardar en archivo de salida
    ofstream outFile(outputFilename);
    if (!outFile) {
        throw runtime_error("No se puede crear el archivo de salida: " + outputFilename);
    }

    outFile << base64EncryptedData;
    outFile.close();

    cout << "Archivo cifrado guardado como: " << outputFilename << endl;
}

void decryptFileWithDES(const string &base64Key, const string &filename) {
    // Decodificar la clave Base64
    string keyBytes = base64_decode(base64Key);

    // Verificar que la clave tenga el tamaño correcto para DES (8 bytes)
    if (keyBytes.length() != 8) {
        throw runtime_error("La clave DES debe ser de 8 bytes (64 bits)");
    }

    // Leer el archivo cifrado
    ifstream inFile(filename, ios::binary);
    if (!inFile) {
        throw runtime_error("No se puede abrir el archivo cifrado: " + filename);
    }

    // Leer el contenido del archivo cifrado
    inFile.seekg(0, ios::end);
    long fileSize = inFile.tellg();
    inFile.seekg(0, ios::beg);

    vector<unsigned char> encryptedData(fileSize);
    // Leer el contenido del archivo byte por byte
    for (size_t i = 0; i < fileSize; ++i) {
        char byte;
        inFile.get(byte); // Leer un byte del archivo
        encryptedData[i] = static_cast<unsigned char>(byte); // Convertir a unsigned char y almacenar
    }
    inFile.close();

    // Decodificar el contenido cifrado desde Base64
    string decodedData = base64_decode(string(encryptedData.begin(), encryptedData.end()));

    // Configurar la clave para DES
    DES_cblock keyblock;
    memcpy(keyblock, keyBytes.c_str(), 8);

    DES_key_schedule keysched;
    DES_set_key_unchecked(&keyblock, &keysched);

    // Preparar el buffer para los datos descifrados
    vector<unsigned char> decryptedData(decodedData.size());

    // Descifrar los datos usando DES en modo CBC
    DES_cblock ivec = {0};
    
    DES_ncbc_encrypt(
        reinterpret_cast<const unsigned char *>(decodedData.data()),
        decryptedData.data(),
        decodedData.size(),
        &keysched,
        &ivec,
        DES_DECRYPT
    );

    // Eliminar el padding
    int paddingSize = decryptedData.back();
    if (paddingSize > 0 && paddingSize <= 8) {
        decryptedData.resize(decryptedData.size() - paddingSize);
    }

    // Generar el nombre del archivo de salida
    string baseFilename = getBaseFilename(filename, "_enc.txt");
    string outputFilename = baseFilename + "_dec" + ".txt";

    // Guardar los datos descifrados en un archivo de salida
    ofstream outFile(outputFilename, ios::binary);
    if (!outFile) {
        throw runtime_error("No se puede crear el archivo de salida: " + outputFilename);
    }

    outFile.write(reinterpret_cast<const char *>(decryptedData.data()), decryptedData.size());
    outFile.close();

    cout << "Archivo descifrado guardado como: " << outputFilename << endl;
}

int main()
{
    try
    {
        int opcion;
        cout << "1. Generar subclaves\n";
        cout << "2. Cifrar archivo con DES\n";
        cout << "3. Decifrar archivo con DES\n";
        cout << "4. Generar clave DES aleatoria\n";
        cout << "5. Codificar cadena binaria a Base64\n";
        cout << "6. Decodificar texto en Base64\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;
        cout << "\n";
        
        if (opcion == 1)
        {
            // Obtener la clave de 10 bits del usuario
            bitset<10> testKey = getKeyFromUser();

            // Generar las subclaves
            auto [K1, K2] = generateSubkeys(testKey);

            // Mostrar resultados
            cout << "Clave original: " << testKey << endl;
            cout << "-> Subclave K1: " << K1 << endl;
            cout << "-> Subclave K2: " << K2 << endl;
            cout << "\n";
        }
        else if (opcion == 2)
        {
            string filename, base64Key;

            cout << "Ingrese la ruta del archivo a cifrar: ";
            cin.ignore(); // Limpiar el buffer
            getline(cin, filename);

            cout << "1. Usar clave aleatoria\n";
            cout << "2. Ingresar clave en formato Base64\n";
            cout << "Seleccione una opcion: ";
            int keyOption;
            cin >> keyOption;
            cout << "\n";

            if (keyOption == 1)
            {
                base64Key = generateRandomDESKey();
                cout << "Clave generada en base64: " << base64Key << endl;
            }
            else
            {
                cout << "Ingrese la clave en formato Base64: ";
                cin.ignore();
                getline(cin, base64Key);
            }

            encryptFileWithDES(base64Key, filename);
        }
        else if (opcion == 3) 
        {
            string filename, base64Key;
        
            cout << "Ingrese la ruta del archivo cifrado: ";
            cin.ignore();
            getline(cin, filename);
        
            cout << "Ingrese la clave en formato Base64: ";
            getline(cin, base64Key);
        
            decryptFileWithDES(base64Key, filename);
        }
        else if (opcion == 4)
        {
            string base64Key = generateRandomDESKey();
            cout << "Clave DES aleatoria generada en Base64: " << base64Key << endl;
        }
        else if (opcion == 5)
        {
            string textInput;
            cout << "Ingrese una cadena de texto: ";
            cin.ignore();
            getline(cin, textInput);

            // Convertir texto a binario (ASCII)
            string binaryRepresentation = textToBinary(textInput);
            
            // Codificar binario a Base64
            string base64Encoded = encodeBase64(binaryRepresentation);
            
            cout << "-> Texto original: " << textInput << endl;
            cout << "-> Representacion binaria: " << binaryRepresentation << endl;
            cout << "-> Cadena codificada en Base64: " << base64Encoded << endl;
        }
        else if (opcion == 6)
        {
            string base64Input;
            cout << "Ingrese un texto en Base64: ";
            cin.ignore();
            getline(cin, base64Input);

            string decodedText = base64_decode(base64Input);
            cout << "-> Texto decodificado: " << decodedText << endl;
        }
        else
        {
            cout << "== Opcion no valida ==" << endl;
        }
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
