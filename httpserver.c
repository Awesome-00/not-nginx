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
    memset(&addr, 0, sizeof(addr)); // Sets memory to 0, clearing out old bits
    addr.sin_family = AF_INET; // Tells OS that this is an IPV4 address
    addr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any nerwork interface
    addr.sin_port = htons(8080); // Listen on port 8080; arranges bytes into n/w byte order

    // Bind the socket to that address/port
    int result = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)); // bind() expects a generic sockaddr*; Tyecastcasted to sockaddr to fit any family 
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
        memset(buffer, 0, sizeof(buffer)); // Sets memory to 0, clearing out old bits
        int bytes = read(newfd, buffer, sizeof(buffer) - 1); // sizeof(buffer) - 1 allows to read at most 1023 bytes; leaving space for /0
        if (bytes == -1)
        {
            perror("read");
            exit(1);
        }
        printf("Received: %s\n", buffer);
        char *method = strtok(buffer, " "); // Gets first token, up to first space (the HTTP method)
        char *path = strtok(NULL, " "); // Continues from last token, gets the path

        printf("method: %s\n", method);
        printf("path: %s\n", path);

        if (strcmp(path, "/") == 0)
        {
            FILE *fp = fopen("index.html", "r"); // Opens file in read mode

            if (fp == NULL) // error check
            {
                perror("Error opening file");
                char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 13\r\n\r\n<h1>Not Found</h1>";
                write(newfd, response, strlen(response));
                close(newfd);
                continue;
            }

            fseek(fp, 0, SEEK_END); // Heads to the end of file
            long bytes = ftell(fp); // calculates number of bytes upto eof
            fseek(fp, 0, SEEK_SET); // Heads back to beginning of file

            char *content = malloc(bytes + 1); // includes one extra byte for /0

            if (content == NULL) // error check
            {
                perror("malloc");
                fclose(fp);
            }
            int read_bytes = fread(content, 1, bytes, fp); //Reads 'bytes' number of 1-byte chunks from fp into content
            fclose(fp);
            char header[256];
            snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", bytes); // Allows for variable size in read file
            write(newfd, header, strlen(header)); // send status line + headers (safe as a real string)
            write(newfd, content, bytes); // send raw file bytes by exact count, not by strlen — avoids
                                          // breaking early if the file contains a 0 byte somewhere
            free(content);
        }

        else if (strcmp(path, "/about.html") == 0)
        {
            FILE *fp = fopen("about.html", "r");
            if (fp == NULL) // error check
            {
                perror("Error opening file");
                char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 13\r\n\r\n<h1>Not Found</h1>";
                write(newfd, response, strlen(response));
                close(newfd);
                continue;
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