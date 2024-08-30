#include <iostream>
#include <fstream>
#include <bitset>
#include <random>
#include <unordered_set>
#include <ctime>
#include <cstdlib>

// Function to generate a random position
long long generateRandomPosition(long long fileSize, std::unordered_set<long long>& generatedPositions) {
    long long position;
    while (true) {
        position = rand() % (fileSize * 8);
        if (generatedPositions.find(position) == generatedPositions.end()) {
            generatedPositions.insert(position);
            break;
        }
    }
    return position;
}

// Function to modify a bit in the file at a given position
void modifyBitInFile(const std::string& filename, long long position) {
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return;
    }

    file.seekg(position / 8, std::ios::beg);

    char byte;
    file.read(&byte, 1);

    std::bitset<8> bitset(byte);
    bitset.set(7 - (position % 8),(~(bitset[7 - (position % 8)])) ); // invert the position because bitset is reversed

    file.seekp(position / 8, std::ios::beg);

       file.write(reinterpret_cast<char*>(&bitset), 1);

    file.close();
}

// Function to modify x% bits in the file at random positions
void modifyBitsRandomly(const std::string& filename, double percent) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return;
    }

    file.seekg(0, std::ios::end);
    long long fileSize = file.tellg();
    file.close();

    std::cout<<"File Size: "<<fileSize<<"\n";
    long long numBitsToModify = static_cast<long long>(fileSize * 8 * (percent / 100.0));
    std::cout<<"Bits Modified: "<<numBitsToModify<<"\n";
    std::unordered_set<long long> generatedPositions;

	double status= 0.1;
	std::cout<<"\nModifying Status: ";
	std::cout.flush();	    
    for (long long i = 0; i < numBitsToModify; ++i) {
        //long long position = position = rand() % (fileSize * 8);
        long long position = generateRandomPosition(fileSize, generatedPositions);
        //bool newValue = rand() % 2; // randomly set bit to 0 or 1

        modifyBitInFile(filename, position);
	long long check = numBitsToModify*status;
	//std::cout<<check<<std::endl;
	if(check == i+1) {
		
		std::cout.flush();
		std::cout<<status*100<<"% ";
		std::cout.flush();
		status += 0.1;
	}
    }
}

int main() {
    srand(time(nullptr)); // seed the random number generator

    const std::string filename = "datafile.bin";
    double changePercent = 1; // 

        // Modify bits randomly
    modifyBitsRandomly(filename, changePercent);

    std::cout << "\nChanges made to " << changePercent << "% of the bits in file \"" << filename << "\"" << std::endl;

    return 0;
}

