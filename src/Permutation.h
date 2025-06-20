#ifndef PERMUTATION_H
#define PERMUTATION_H

#include <iostream>
#include <vector>
#include <bitset>
#include <algorithm>
#include <numeric>
#include <iomanip>

using namespace std;

class Permutation {
private:
    vector<int> planckDigits;
    vector<int> permutationArray;
    vector<int> inversePermutationArray;

    // Generate the permutation array based on Planck constant
    void generatePermutationArrays() {
        // Step 1: Initialize array with values 0-15
        permutationArray.resize(16);
        for (int i = 0; i < 16; ++i) {
            permutationArray[i] = i;
        }
        
        // Step 2: Shift array by seed value (1): {1, 2, ..., 15, 0}
        rotate(permutationArray.begin(), permutationArray.begin() + 1, permutationArray.end());
        
        // Step 3: For each digit 'd' in planckDigits, swap positions i and (i + d) mod 16
        for (size_t i = 0; i < planckDigits.size(); ++i) {
            int d = planckDigits[i];
            int swapPos = (i + d) % 16;
            swap(permutationArray[i], permutationArray[swapPos]);
        }

        // Generate inverse permutation array
        inversePermutationArray.resize(16);
        for (int i = 0; i < 16; ++i) {
            inversePermutationArray[permutationArray[i]] = i;
        }
    }

public:

    // Constructor
    Permutation() {
        // Planck Digits
        planckDigits = {6, 6, 2, 6, 0, 7, 0, 1, 5};
        generatePermutationArrays();
    }
    
    // Get the permuted position for a given original position
    int getPermutedPosition(int originalPosition) const {
        if (originalPosition < 0 || originalPosition >= 16) {
            throw out_of_range("Position must be between 0 and 15");
        }
        return permutationArray[originalPosition];
    }
    
    // Get the original position for a given permuted position (inverse lookup)
    int getOriginalPosition(int permutedPosition) const {
        if (permutedPosition < 0 || permutedPosition >= 16) {
            throw out_of_range("Position must be between 0 and 15");
        }
        return inversePermutationArray[permutedPosition];
    }
    
    // Apply permutation to a 16-bit bitset
    bitset<16> applyPermutation(const bitset<16>& input) const {
        bitset<16> permutedKey;
        for (size_t i = 0; i < 16; i++) {
            permutedKey[i] = input[permutationArray[i]];
        }
        return permutedKey;
    }
    
    // Apply inverse permutation to a 16-bit bitset
    bitset<16> applyInversePermutation(const bitset<16>& input) const {
        bitset<16> originalKey;
        for (size_t i = 0; i < 16; i++) {
            originalKey[i] = input[inversePermutationArray[i]];
        }
        return originalKey;
    }
    
    // Print the permutation array
    void printPermutationArray() const {
        cout << "\n=== PERMUTATION ARRAY ===" << "\n";
        cout << "Position -> Permuted Position" << "\n";
        cout << "============================" << "\n";
        
        for (int i = 0; i < 16; ++i) {
            cout << "P[" << setw(2) << i << "] = " << setw(2) << permutationArray[i] 
                 << " (0x" << hex << uppercase << i << " -> 0x" << permutationArray[i] << ")" << dec << "\n";
        }
    }
    
    // Print the inverse permutation array
    void printInversePermutationArray() const {
        cout << "\n=== INVERSE PERMUTATION ARRAY ===" << "\n";
        cout << "Permuted Position -> Original Position" << "\n";
        cout << "=====================================" << "\n";
        
        for (int i = 0; i < 16; ++i) {
            cout << "IP[" << setw(2) << i << "] = " << setw(2) << inversePermutationArray[i] 
                 << " (0x" << hex << uppercase << i << " -> 0x" << inversePermutationArray[i] << ")" << dec << "\n";
        }
    }
};

#endif
