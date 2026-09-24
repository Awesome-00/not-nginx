#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

int main()
{
    // Create a TCP socket (IPv4)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(1);
    }

    // Set up the address struct: any interface, port 8080
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    // Bind the socket to that address/port
    int result = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (result == -1)
    {
        perror("bind");
        exit(1);
    }

    // Start listening for incoming connections (backlog of 1)
    int result2 = listen(sockfd, 1);
    if (result2 == -1)
    {
        perror("listen");
        exit(1);
    }

    // Handle clients one at a time, forever
    while (true)
    {
        // Block until a client connects; newfd is that client's dedicated socket
        int newfd = accept(sockfd, NULL, NULL);
        if (newfd == -1)
        {
            perror("accept");
            exit(1);
        }

        // Read whatever the client sent
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int bytes = read(newfd, buffer, sizeof(buffer) - 1);
        if (bytes == -1)
        {
            perror("read");
            exit(1);
        }
        printf("Received: %s\n", buffer);
        char *method = strtok(buffer, " ");
        char *path = strtok(NULL, " ");
        printf("method: %s\n", method);
        printf("path: %s\n", path);
        if (strcmp(path, "/") == 0)
        {
            FILE *fp = fopen("index.html", "r");

            if (fp == NULL)
            {
                perror("Error opening file");
            }

            fseek(fp, 0, SEEK_END);
            long bytes = ftell(fp);
            printf("file size: %ld\n", bytes);
            fseek(fp, 0, SEEK_SET);

            char *content = malloc(bytes + 1);

            if (content == NULL)
            {
                perror("malloc");
                fclose(fp);
            }
            int read_bytes = fread(content, 1, bytes, fp);
            fclose(fp);
            printf("%s\n", content);
            char header[256];
            snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", bytes);
            write(newfd, header, strlen(header));
            write(newfd, content, bytes);
            free(content);
        }
        else if (strcmp(path, "/about.html") == 0)
        {
            FILE *fp = fopen("about.html", "r");
            if (fp == NULL)
            {
                perror("Error opening file");
            }
            fseek(fp, 0, SEEK_END);
            long bytes = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            char *content = malloc(bytes + 1);

            if (content == NULL)
            {
                perror("malloc");
                fclose(fp);
            }
            int read_bytes = fread(content, 1, bytes, fp);
            fclose(fp);
            printf("%s\n", content);
            char header[256];
            snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", bytes);
            write(newfd, header, strlen(header));
            write(newfd, content, bytes);
            free(content);
        }
        else
        {
            char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 13\r\n\r\n<h1>Not Found</h1>";
            write(newfd, response, strlen(response));
        }

        // Done with this client — close their socket, loop back for the next one
        close(newfd);
    }
    close(sockfd);

    return 0;
}