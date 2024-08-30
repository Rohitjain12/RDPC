#include <iostream>
#include <thread>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include "modMycryptoLib.h"

#define PORT 8005
#define CHUNK_SIZE 1024

void sendFile(int socket, const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    char buffer[CHUNK_SIZE];
    while (!file.eof()) {
        file.read(buffer, CHUNK_SIZE);
        ssize_t bytesRead = file.gcount();
        if (bytesRead <= 0)
            break;

        if (send(socket, buffer, bytesRead, 0) == -1) {
            std::cerr << "Error sending file data." << std::endl;
            break;
        }
    }

    file.close();
}

void Start_Prover() {
    int server_fd, addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in server_addr, client_addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen)) == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        int seed;
        ssize_t recv_len = recv(new_socket, &seed, sizeof(seed), 0);
        if (recv_len == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

	float p;
	recv_len = recv(new_socket, &p, sizeof(p), 0);
        if (recv_len == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        printf("Seed Received from %s:%d is : %d \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), seed);
	printf("p Received from %s:%d is : %f \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), p);

        string fname= "datafile.bin";
        fstream file(fname, ios::in | ios::binary);
        if (!file.is_open()) {
            cerr << "Error opening file.\n";
            return;
        }

        //int sector= 100000;
        long long noc = sector;    
        file.seekg(0, std::ios::end);
        long long fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        long long m1 = ceil((double)fileSize / sector);
        int n= m1;
	long long c= ceil((p/100.0)*m1);
	m1= c;
    	vector<int> block;

    	srand(seed+1);
    //vector<int> hash1(256, 0);
    	unordered_map<int, int> hash;
    //int count = 0;
	//n--;
    	while(block.size() < c){
        	unsigned int randNum =rand()%n;
        	if(hash.find(randNum) == hash.end()){
            	hash[randNum] = 1;
		block.push_back(randNum);
		//cout<<randNum<<" ";
        	}
    	}
	//sort(block.begin(), block.end());
	//cout<<endl<<c<<" "<<block.size()<<endl;
	std::vector<GF256::Element> v;
	
	//cout<<fileSize<<" "<<m1<<endl;
        generateRandomGF256Values(v, m1, seed);
	//cout<<v.size()<<endl;
        proofGen(v, file, block);

        // Send the binary file
        sendFile(new_socket, "Mul_VF.bin");
        
        printf("Binary file sent to %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        close(new_socket);
        
        std::cout<<"\n\n----------------------------------------------------------------------------------------------------------------------------------------------------------------------\n";
    }

    close(server_fd);
}

int main() {
    Start_Prover();
    return 0;
}
