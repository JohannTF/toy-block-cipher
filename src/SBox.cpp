#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <bitset>

using namespace std;

class SBox {
private:
    int fieldSize;
    
    // GF(2^n) multiplication
    unsigned int multiplyGF2N(unsigned int a, unsigned int b) {
        unsigned int mask = (1 << fieldSize) - 1;
        unsigned int irreducible = 0;
        
        // Select irreducible polynomial based on field size
        if (fieldSize == 4) irreducible = 0x1F;
        else if (fieldSize == 8) irreducible = 0x11B;
        else irreducible = (1 << fieldSize) | 0x3;
        
        unsigned int result = 0;
        
        // Correct implementation of GF(2^n) multiplication
        for (int i = 0; i < fieldSize; i++) {
            if (b & (1 << i)) {
                result ^= (a << i);
            }
            
            // Modular reduction after each iteration
            for (int j = 2 * fieldSize - 2; j >= fieldSize; j--) {
                if (result & (1 << j)) {
                    result ^= (irreducible << (j - fieldSize));
                }
            }
        }
        
        result &= mask;
        return result;
    }

    // Extended GCD algorithm
    int extendedGCD(int a, int n, int &x1, int &x2) {
        int u = a, v = n;
        x1 = 1, x2 = 0;

        if (a % n == 0) {
            x1 = 0;
            x2 = 1;
            return n;
        }
        
        if (n % a == 0) {
            x1 = 1;
            x2 = 0;
            return a;
        }

        while (u != 0 && u != 1) {
            int q = v / u;
            int r = v % u;
            int x = x2 - q * x1;
            v = u;
            u = r;
            x2 = x1;
            x1 = x;

            if (u == 0) {
                return v;
            }
        }
        return u;
    }

    // Find multiplicative inverse in finite field
    unsigned int findInverse(int a, int n) {
        int x1, x2;
        int gcdResult = extendedGCD(a, n, x1, x2);
        if (gcdResult == 1) {
            return (x1 % n + n) % n;
        }
        return 0;
    }

    // Convert result to hexadecimal representation
    string toHexadecimal(unsigned int r) {
        stringstream ss;
        ss << hex << uppercase << r;
        return ss.str();
    }

public:
    // Constructor
    SBox(int n) : fieldSize(n) {
        if (n <= 0 || n > 32) {
            throw invalid_argument("Field size must be between 1 and 32");
        }
    }

    // Apply S-box transformation to a single element
    unsigned int applySBox(unsigned int element) {
        // S-box operation: (input XOR 5) * 7 XOR 10
        unsigned int firstXor = 5 ^ element;
        unsigned int multiplyResult = multiplyGF2N(firstXor, 7);
        unsigned int result = multiplyResult ^ 10;
        return result;
    }

    // Apply inverse S-box transformation to a single element
    unsigned int applyInverseSBox(unsigned int element) {
        // Inverse S-box operation: ((input XOR 10) * 7^-1) XOR 5
        unsigned int firstXor = 10 ^ element; 
        unsigned int inverse = findInverse(7, 31);
        unsigned int multiplyResult = multiplyGF2N(firstXor, inverse);
        unsigned int result = multiplyResult ^ 5; 
        return result;
    }

    // Apply S-box to a vector of elements
    vector<unsigned int> applySBoxToVector(const vector<unsigned int>& input) {
        vector<unsigned int> output;
        for (unsigned int element : input) {
            output.push_back(applySBox(element));
        }
        return output;
    }

    // Apply inverse S-box to a vector of elements
    vector<unsigned int> applyInverseSBoxToVector(const vector<unsigned int>& input) {
        vector<unsigned int> output;
        for (unsigned int element : input) {
            output.push_back(applyInverseSBox(element));
        }
        return output;
    }

    // Print S-box transformation table
    void printSBoxTable() {
        cout << "\n=== S-BOX TRANSFORMATION TABLE ===" << "\n";
        cout << "Input -> Output" << "\n";
        cout << "===============" << "\n";
        
        unsigned int maxElement = (1 << fieldSize) - 1;
        
        for (unsigned int i = 0; i <= maxElement; ++i) {
            unsigned int output = applySBox(i);
            cout << "S-box[" << toHexadecimal(i) << "] = " 
                     << toHexadecimal(output) << "\n";
        }
    }

    // Print inverse S-box transformation table
    void printInverseSBoxTable() {
        cout << "\n=== INVERSE S-BOX TRANSFORMATION TABLE ===" << "\n";
        cout << "Input -> Output" << "\n";
        cout << "===============" << "\n";
        
        unsigned int maxElement = (1 << fieldSize) - 1;
        
        for (unsigned int i = 0; i <= maxElement; ++i) {
            unsigned int output = applyInverseSBox(i);
            cout << "S-boxReverse[" << toHexadecimal(i) << "] = " 
                     << toHexadecimal(output) << "\n";
        }
    }
};


// int main() {
//     cout << "=== PRUEBA DE LA CLASE SUBSTITUTION BOX ===" << "\n";
    
//     try {
//         // Crear una instancia de SBox para GF(2^4)
//         SBox sbox(4);
        
//         cout << "\nProbando transformación S-box individual:" << "\n";
//         cout << "===========================================" << "\n";
//         vector<unsigned int> testValues = {0, 1, 5, 10, 15};
//         for (unsigned int val : testValues) {
//             unsigned int sboxResult = sbox.applySBox(val);
//             unsigned int inverseResult = sbox.applyInverseSBox(sboxResult);

//             cout << "Original: 0x" << toHexadecimal(val) 
//                  << " -> S-box: 0x" << toHexadecimal(sboxResult)
//                  << " -> Inversa: 0x" << toHexadecimal(inverseResult)
//                  << "\n";
//         }
        

//         cout << "\nProbando transformación de vectores:" << "\n";
//         cout << "=======================================" << "\n";
//         vector<unsigned int> inputVector = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6};
//         cout << setw(15) << "Original: ";
//         for (auto val : inputVector) {
//             cout << "0x" << toHexadecimal(val) << " ";
//         }
//         cout << "\n";

        
//         // Aplicar S-box al vector
//         auto sboxVector = sbox.applySBoxToVector(inputVector);
//         cout << setw(15) << "S-box: ";
//         for (auto val : sboxVector) {
//             cout << "0x" << toHexadecimal(val) << " ";
//         }
//         cout << "\n";

//         // Aplicar S-box inversa
//         auto recoveredVector = sbox.applyInverseSBoxToVector(sboxVector);
//         cout << setw(15) << "S-box inversa: ";
//         for (auto val : recoveredVector) {
//             cout << "0x" << toHexadecimal(val) << " ";
//         }
//         cout << "\n";
        

//         // Mostrar tablas de S-box
//         sbox.printSBoxTable();
//         sbox.printInverseSBoxTable();

//     } catch (const exception& e) {
//         cout << "Error: " << e.what() << "\n";
//         return 1;
//     }
    
//     return 0;
// }