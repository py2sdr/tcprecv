/*
** TCP data receiver
** by Edson Pereira, PY2SDR
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#define BUFSIZE 2048
#define MAX_IP_LEN 16

void usage()
{
    fprintf(stderr, "tcprecv: v1.00 by py2sdr\n");
    fprintf(stderr, "usage: tcprecv server_ip server_port\n");
}

void get_timestamp(char *buffer, size_t size)
{
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (timeinfo == NULL)
    {
        strncpy(buffer, "[timestamp error]", size - 1);
        buffer[size - 1] = '\0';
        return;
    }
    strftime(buffer, size, "%F %X", timeinfo);
}

int validate_ip_address(const char *ip)
{
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) == 1;
}

int main(int argc, char *argv[])
{
    int server_fd, server_port, n;
    struct sockaddr_in serv_addr;
    char timeStamp[80];
    unsigned char buffer[BUFSIZE];
    char server_ip[MAX_IP_LEN];

    // Check if we have all arguments
    if (argc != 3)
    {
        usage();
        return EXIT_FAILURE;
    }

    // Store and validate arguments
    strncpy(server_ip, argv[1], MAX_IP_LEN - 1);
    server_ip[MAX_IP_LEN - 1] = '\0';

    server_port = atoi(argv[2]);

    // Validate IP address format
    if (!validate_ip_address(server_ip))
    {
        fprintf(stderr, "Error: Invalid IP address format\n");
        usage();
        return EXIT_FAILURE;
    }

    // Validate TCP port
    if (server_port < 1024 || server_port > 65535)
    {
        fprintf(stderr, "Error: Port must be between 1024 and 65535\n");
        usage();
        return EXIT_FAILURE;
    }

    // Setup socket structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);

    // Setup structure for socket timeout
    struct timeval socketTimeout;
    socketTimeout.tv_sec = 10;
    socketTimeout.tv_usec = 0;

    while (1)
    {
        // Get socket
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Socket creation failed");
            sleep(5);
            continue;
        }

        // Configure socket options
        if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO,
                       &socketTimeout, sizeof(socketTimeout)) < 0)
        {
            perror("setsockopt failed");
            close(server_fd);
            sleep(5);
            continue;
        }

        // Increase socket buffer size for binary data streaming
        int rcvbuf_size = 32 * 1024;  // 32KB socket receive buffer
        setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF,
                   &rcvbuf_size, sizeof(rcvbuf_size));

        // Get time stamp for connection attempt
        get_timestamp(timeStamp, sizeof(timeStamp));
        fprintf(stderr, "%s Connecting to %s:%d\n",
                timeStamp, server_ip, server_port);

        // Connect to server
        if (connect(server_fd, (struct sockaddr*)&serv_addr,
                    sizeof(serv_addr)) < 0)
        {
            get_timestamp(timeStamp, sizeof(timeStamp));
            fprintf(stderr, "%s Connection failed: %s\n",
                    timeStamp, strerror(errno));
            close(server_fd);
            sleep(5);
            continue;
        }

        // Update timestamp for successful connection
        get_timestamp(timeStamp, sizeof(timeStamp));
        fprintf(stderr, "%s Connected\n", timeStamp);

        // Receive binary data from tcp stream and write to stdout
        int write_error = 0;
        while ((n = read(server_fd, buffer, BUFSIZE)) > 0)
        {
            ssize_t total_written = 0;

            // Ensure all binary data is written
            while (total_written < n)
            {
                ssize_t written = write(STDOUT_FILENO,
                                       buffer + total_written,
                                       n - total_written);
                if (written < 0)
                {
                    if (errno == EINTR)
                        continue;  // Interrupted, retry

                    get_timestamp(timeStamp, sizeof(timeStamp));
                    fprintf(stderr, "%s Write to stdout failed: %s\n",
                            timeStamp, strerror(errno));
                    write_error = 1;
                    break;
                }
                total_written += written;
            }

            if (write_error)
                break;
        }

        // Check for read errors
        if (n < 0)
        {
            get_timestamp(timeStamp, sizeof(timeStamp));
            fprintf(stderr, "%s Read error: %s\n",
                    timeStamp, strerror(errno));
        }
        else if (n == 0 && !write_error)
        {
            get_timestamp(timeStamp, sizeof(timeStamp));
            fprintf(stderr, "%s Connection closed by server\n", timeStamp);
        }

        close(server_fd);
        sleep(5);
    }

    return EXIT_SUCCESS;
}
