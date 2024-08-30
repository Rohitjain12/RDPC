#pragma once

#include <cryptopp/gf256.h>
#include <vector>
#include <fstream>
#include <cmath>
#include<bits/stdc++.h>

using namespace CryptoPP;
using namespace std;

GF256 gf256(0x11B);

#define sector 100000

vector<GF256::Element> mac_calculate(const vector<GF256::Element>& alpha, const vector<GF256::Element>& block) {
    const int k = alpha.size();
    const int m = block.size();
    
    //GF256 gf256(0x11B);
    vector<GF256::Element> macDigests(k, GF256::Element(0)); // Initialize MAC digests with zeros
    
    for (int alphaIndex = 0; alphaIndex < k; ++alphaIndex) {
        GF256::Element currentAlpha = alpha[alphaIndex];
        for (int i = 0; i < m; ++i) {
            GF256::Element temp = gf256.Multiply(block[i], currentAlpha);
            macDigests[alphaIndex] = gf256.Add(macDigests[alphaIndex], temp);
            currentAlpha = gf256.Multiply(currentAlpha, alpha[alphaIndex]);
        }
    }

    // for (int alphaIndex = 0; alphaIndex < k; ++alphaIndex) {
    //     cout<<macDigests[alphaIndex];
    // }
     for (int j = 0; j < macDigests.size(); ++j) {
                cout << hex << setw(2) << setfill('0') << static_cast<int>(macDigests[j]);
                if (j+1 != macDigests.size()) {
                    cout << "-";
                }
            }
        //cout << "\n";
    cout<<endl;
    return macDigests;
}

void elementwise_multiply_and_add(const vector<GF256::Element>& a, const vector<GF256::Element>& b, vector<GF256::Element>& c, int row) {
    GF256::Element temp= 0;
    for (size_t i = 0; i < b.size(); ++i) {
        temp = gf256.Multiply(a[row], b[i]);
        c[i] = gf256.Add(c[i], temp);
    }
}


void Perform_block_wise_mac(fstream& file, long long blockSize, const vector<GF256::Element>& alpha) {
    long long m2 = blockSize; // Number of sectors in MAC
    int k = alpha.size(); // Number of keys
    file.seekg(0, std::ios::end);
    long long fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    //GF256 gf256(0x11B);

    // Calculate the number of blocks based on the file size and m1
    long long numBlocks = ceil((double) fileSize/m2);

    // Open an output file stream for writing MACs
    ofstream macFile("mac_.bin", ios::binary);
    if (!macFile.is_open()) {
        cerr << "Error opening mac.bin for writing." << endl;
        return;
    }

    for (long long blockIndex = 0; blockIndex < numBlocks; ++blockIndex) {
        vector<GF256::Element> buffer(m2, 0); // Buffer to store the read bytes

        // Set the file pointer to the beginning of the current block
        file.seekg(blockIndex * m2, ios::beg);

        // Read data from the file and store it in the buffer
        file.read(reinterpret_cast<char*>(buffer.data()), m2);

        // Perform MAC calculation for buffer
        vector<GF256::Element> macResult = mac_calculate(alpha, buffer);

        // Write the MACs to the output file
        for (const auto& mac : macResult) {
            macFile.write(reinterpret_cast<const char*>(&mac), sizeof(mac));
        }
    }

    // Close the output file stream
    macFile.close();
}



void Perform_block_wise_v_mac(fstream& file, long long blockSize,vector<GF256::Element>& result ,vector<GF256::Element>& v, int keys) {
    long long m2 = keys; // Number of sectors in MAC
    
    file.seekg(0, std::ios::end);
    long long fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    //GF256 gf256(0x11B);

    // Calculate the number of blocks based on the file size and m1
    long long numBlocks = ceil((double) fileSize/m2);

    for (long long blockIndex = 0; blockIndex < numBlocks; ++blockIndex) {
        vector<GF256::Element> buffer(m2, 0); // Buffer to store the read bytes
        vector<GF256::Element> extractedData; // Vector to store the extracted data from the buffer

        // Set the file pointer to the beginning of the current block
        file.seekg(blockIndex * m2, ios::beg);

        // Read data from the file and store it in the buffer
        file.read(reinterpret_cast<char*>(buffer.data()), m2);
        
        elementwise_multiply_and_add(v, buffer, result, blockIndex);
        
    }
}


void Perform_block_wise_v_f(fstream& file, long long blockSize, vector<GF256::Element>& result, vector<GF256::Element>& v, vector<int> &block) {
    long long m2 = blockSize; // Number of sectors in MAC
    
    file.seekg(0, std::ios::end);
    long long fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    //GF256 gf256(0x11B);

    // Calculate the number of blocks based on the file size and m1
    long long numBlocks = block.size();

    for (long long blockIndex = 0; blockIndex < numBlocks; ++blockIndex) {
        file.clear();
	vector<GF256::Element> buffer(m2, 0); // Buffer to store the read bytes
        vector<GF256::Element> extractedData; // Vector to store the extracted data from the buffer

        // Set the file pointer to the beginning of the current block
        file.seekg(block[blockIndex] * m2, ios::beg);


        file.read(reinterpret_cast<char*>(buffer.data()), m2);

        elementwise_multiply_and_add(v, buffer, result, blockIndex);
    }
}

void readValuesFromFile(std::vector<GF256::Element>& v) {
    fstream inputFile("xyz", ios::in);  // Use double quotes for the file name
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file: xyz.txt" << std::endl;
        return;  // Return void on failure
    }

    // Read the line from the file
    std::string line;
    std::getline(inputFile, line);

    // Use a stringstream to parse the line
    std::istringstream iss(line);
    int value;

    // Read each value and store it in the vector as GF256 elements
    while (iss >> value) {
        //cout<<value;
       v.push_back(static_cast<GF256::Element>((value))); 
        // cout<<static_cast<int>(v[v.size()-1])<<" ";
    }

    inputFile.close();
}

void generateRandomGF256Values(vector<GF256::Element>& v, int size, int seed) {
    srand(seed);
    for (int i = 0; i < size; ++i) {
        int value = rand() % 256; // Generate random number between 0 and 255
        v.push_back(GF256::Element(value));
        //cout<<value<<" ";
    }
}

bool compareFiles(const std::string& file1, const std::string& file2) {
    std::ifstream fileStream1(file1, std::ios::binary);
    std::ifstream fileStream2(file2, std::ios::binary);

    if (!fileStream1 || !fileStream2) {
        std::cerr << "Error opening files for comparison.\n";
        return false;
    }

    constexpr size_t bufferSize = 4096; // Adjust the buffer size as needed
    std::vector<char> buffer1(bufferSize);
    std::vector<char> buffer2(bufferSize);

    do {
        fileStream1.read(buffer1.data(), bufferSize);
        fileStream2.read(buffer2.data(), bufferSize);

        if (fileStream1.gcount() != fileStream2.gcount() ||
            memcmp(buffer1.data(), buffer2.data(), fileStream1.gcount()) != 0) {
            return false;
        }
    } while (fileStream1 && fileStream2);

    return true;
}

void proofGen(vector<GF256::Element> &v, fstream &file, vector<int> &block) 
{
    //int sector= 100000;    
    long long noc= sector;

    vector<GF256::Element> result(noc, GF256::Element(0));

    Perform_block_wise_v_f(file, noc, result, v, block);

    ofstream resultFile("Mul_VF.bin", ios::out | ios::binary);
    if (resultFile.is_open()) {
        resultFile.write(reinterpret_cast<const char*>(&result[0]), result.size() * sizeof(GF256::Element));
        resultFile.close();
        cout << "Result vector stored in Mul_VF.bin" << endl;
    } else {
        cerr << "Error opening result file for writing." << endl;
    }

}

void tagGen(fstream &file, vector<GF256::Element> &alpha, vector<int> &block)
{
    //int sector= 100000;
    Perform_block_wise_mac(file, sector, alpha);
}

bool verifyProof(vector<GF256::Element> &v, fstream &mu, fstream &tag, vector<GF256::Element> &alpha, vector<int> &block) 
{
    //compute V x Mac(f)
    proofGen(v, tag, block);
    //renaming result.bin to Mul_v_Mac(F)
    if (std::rename("Mul_VF.bin", "Mul_V_Macf.bin") != 0) {
        std::cerr << "Error renaming file\n";
        return 1;
    }

    tagGen(mu, alpha, block);

    if (std::rename("mac.bin", "Tag_VF.bin") != 0) {
        std::cerr << "Error renaming file\n";
        return 1;
    }
 
    bool result = compareFiles("Mul_V_Macf.bin", "Tag_VF.bin");
    return result;
}

