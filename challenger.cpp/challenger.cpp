#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include "MycryptoLib.h"

#define PORT 8005
#define PORT2 8010
#define ADMIN_IP "172.16.15.7" // Replace with the IP address of the Admin

using namespace std;

//GF256 gf256(0x11B);

bool check(int seed, float p1)
{
    string fname= "datafile.bin";
    fstream file(fname, ios::in | ios::binary);
    if (!file.is_open()) {
        cerr << "Error opening file d.\n";
        return 1;
    }
    //int sector= 100000;

    long long noc = sector;    
    file.seekg(0, std::ios::end);
    long long fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    long long n = ceil((double)fileSize / sector);

    int c=0;
    c=ceil(float(p1*n)/100.0);

    long long m1= c;
    vector<int> block;

    srand(seed+1);

    unordered_map<int, int> hash;

    while(block.size() < c){
        unsigned int randNum =rand()%n;

        if(hash.find(randNum) == hash.end()){
            hash[randNum]= 1;
            block.push_back(randNum);
        }
    }
    //sort(block.begin(), block.end());

    std::vector<GF256::Element> v;          

    generateRandomGF256Values(v, m1, seed);

    std::vector<GF256::Element> alpha(keySize);       //It will store keys 
    int tempKeys[] = key;
    for (int i = 0; i < alpha.size(); ++i) {
        alpha[i] = static_cast<GF256::Element>(tempKeys[i]);
    }
    
    //IF you want to generate MAC of file then uncomment the below lines
    /*std::cout<<"\nCalculalting MAC of datafile.bin"<<std::endl;
    vector<int> tempBlock(n);
    for(int i=0;i<n;i++)tempBlock[i]=i;
    tagGen(file, alpha, tempBlock);  

    if (std::rename("mac.bin", "macFile.bin") != 0) {
        std::cerr << "Error renaming file\n";
        return 1;
    } */

    fstream macFile("macFile.bin", ios::in | ios::binary);          //This is the MAC of datafile.bin for the above key
    if (!macFile.is_open()) {
        cerr << "Error opening file d.\n";
        return 1;
    }
    
    tagPick(macFile, alpha, block);
    //return 0;
    if (std::rename("mac.bin", "tag.bin") != 0) {
        std::cerr << "Error renaming file\n";
        return 1;
    }

    //cout<<"result stored in tag.bin\n";
    fstream mu("response.bin", ios::in | ios::binary);
    fstream tag("tag.bin", ios::in | ios::binary);

    if (!mu.is_open()) {
    cerr << "Error: Unable to open response.bin" << endl;
    // Handle the error or return an error code
}

if (!tag.is_open()) {
    cerr << "Error: Unable to open tag.bin" << endl;
    // Handle the error or return an error code
}


    return verifyProof(v, mu, tag, alpha, block);
}

void Receive_Binary_File(int client_fd) {
    std::ofstream outputFile("response.bin", std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }

    char buffer[1024];
    ssize_t recv_size;
    while ((recv_size = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        outputFile.write(buffer, recv_size);
    }

    if (recv_size == -1) {
        perror("recv");
    }

    outputFile.close();
}

void Send_Challenge(const char *prover_ip) {
    int client_fd;
    struct sockaddr_in prover_addr, admin_addr;
    char buffer[1024] = {0};

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return;
    }

    prover_addr.sin_family = AF_INET;
    prover_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, prover_ip, &prover_addr.sin_addr) <= 0) {
        perror("inet_pton");
        return;
    }

    if (connect(client_fd, (struct sockaddr *)&prover_addr, sizeof(prover_addr)) == -1) {
        perror("connect");
        close(client_fd);
        return;
    }

    std::cout << "Sending Challenge-Message to Prover at IP: " << prover_ip << std::endl;

    // Sending seed value
    // std::random_device rd;
    srand(time(0));

    int seed = rand();
    if(seed < 0) seed *= -1;
    float p1=0;

    cout<<"Seed Gnerated: "<<seed<<"\n";
    cout<<"Enter % of Blocks: ";
    cout<<"p:";
    cin>>p1;
    //cout<<"p:"<<p1<<"%"<<endl;
    send(client_fd, &seed, sizeof(seed), 0);

    send(client_fd, &p1, sizeof(p1), 0);

    // Receiving the binary file
    Receive_Binary_File(client_fd);
    printf("Binary file received from Prover at %s:%d\n", prover_ip, ntohs(prover_addr.sin_port));

    // Saving response to a binary file
    bool result = check(seed, p1);
    
    //Handle response
    if (result == true) {
        std::cout << "Verification Successful. File is NOT Modified" << std::endl;
    } else {
        cout<<"\nFile has been Modified. Sending error message to admin\n";
        // Send error message to admin
        admin_addr.sin_family = AF_INET;
        admin_addr.sin_port = htons(PORT2);
        inet_pton(AF_INET, ADMIN_IP, &admin_addr.sin_addr);

        int admin_fd;
        if ((admin_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            close(client_fd);
            return;
        }

        if (connect(admin_fd, (struct sockaddr *)&admin_addr, sizeof(admin_addr)) == -1) {
            perror("connect");
            close(client_fd);
            close(admin_fd);
            return;
        }

        send(admin_fd, "Error-Message", strlen("Error-Message"), 0);
        printf("Error message sent to Admin at IP: %s\n", ADMIN_IP);

        std::cout<<"\n\n----------------------------------------------------------------------------------------------------------------------------------------------------\n";

        close(admin_fd);
    }
    
    close(client_fd);
}

int main() {
    std::string IP = "172.16.15.6";
    while (true) {

        if (IP == "quit") {
            break;
        }

        Send_Challenge(IP.c_str());
        cout<<"\n------------------------------------------------------------------------------------------------------------------------------------------------------------\n";
    }

    return 0;
}
