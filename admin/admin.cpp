#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctime>

#define PORT 8010

void Start_Admin() {
    int server_fd, addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in server_addr, client_addr;
    char buffer[1024] = {0};

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

    if (listen(server_fd, 5) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int client_fd;
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) == -1) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        ssize_t recv_len = recv(client_fd, buffer, sizeof(buffer), 0);
        if (recv_len == -1) {
            perror("recv failed");
            close(client_fd);
            continue;
        }

        printf("Error-Message Received from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        //printf("Current Time: %ld\n", time(NULL)); */
	// Get current time
        time_t now = time(nullptr);
        char time_buffer[80];
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
        printf("Current Time: %s\n", time_buffer);

        std::cout<<"\n\n----------------------------------------------------------------------------------------------------------------------------------\n";

        close(client_fd);
    }

    close(server_fd);
}

int main() {
    Start_Admin();
    return 0;
}
