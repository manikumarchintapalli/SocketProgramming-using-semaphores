#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <glob.h>
#include <signal.h>
#include <semaphore.h>

#define MAX_USERS 6
#define BUFFER_SIZE 8192
#define USER_ID_LEN 8
#define SERVER_ABORT -9
#define USER_REG_ERROR "USER ID ALREADY TAKEN"
#define USER_REG_SUCCESS "LOGIN SUCCESSFUL, YOU CAN START SENDING MESSAGES!!!"
#define LOGIN_LIST_MSG "BELOW IS THE LIST OF ACTIVE CLIENTS CONNECTED TO SERVER:"
#define SERVER_SHUTDOWN_MSG "SERVER SHUTDOWN!! \n"
#define HELPERNODE "HELPER NODE ACTIVATED"
#define MAXSIZE 5
#define VOWELS "aeiou"
#define SHUTDOWNHELPER "HELPER NODE SHUTDOWN"

// log error
void log_error(char *message)
{
    printf("%s\n", message);
    exit(1);
}

// intialise socket with tcp
int intialise_socket_with_ipv4_and_tcp()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        log_error("Error Intialising the Socket");
    }
    return fd;
}

// intialise socket address
struct sockaddr_in *intialise_socket_address(char *ip, char *port_number)
{
    struct sockaddr_in *socket_address = malloc(sizeof(struct sockaddr_in));
    if (strlen(ip) == 0)
    {
        socket_address->sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        // Converting the string ip to IPV4 format, present to network
        inet_pton(AF_INET, ip, &socket_address->sin_addr.s_addr);
    }
    socket_address->sin_family = AF_INET;
    socket_address->sin_port = htons(atoi(port_number));
    return socket_address;
}

// bind server
int bind_server(int socket_fd, struct sockaddr_in *socket_address)
{

    int server_fd = bind(socket_fd, (struct sockaddr *)socket_address, sizeof(*socket_address));
    if (server_fd < 0)
    {
        log_error("Binding failed");
    }
    printf("Binding successful\n");
    return server_fd;
}

// listen to port
int listen_to_port(int socket_fd, int max_no_of_connections, char *port)
{
    int listen_result = listen(socket_fd, max_no_of_connections);
    if (listen_result < 0)
    {
        log_error("Error Listening to Port");
    }
    printf("Listening on port : %s\n", port);
    return listen_result;
}

// get index of tag in string
int get_index_of_tag_in_string(char *string, char *tag, int start_index)
{
    // if tag length > string length
    if (strlen(tag) > strlen(string))
    {
        return -1;
    }
    // Finding the start index of tag
    for (int i = start_index; i < strlen(string); ++i)
    {
        int count = 0;
        for (int j = 0; j < strlen(tag) && i + j < strlen(string); ++j)
        {
            if (string[i + j] == tag[j])
            {
                count++;
            }
            else
            {
                break;
            }
        }
        if (count == strlen(tag))
        {
            return i;
        }
    }
    return -1;
}

// extract text between tag
char *extract_text_between_tag(char *string, char *tag)
{
    char start_tag[strlen(tag) + 2];
    sprintf(start_tag, "<%s>", tag);
    int start_index = get_index_of_tag_in_string(string, start_tag, 0);

    char end_tag[strlen(tag) + 3];
    sprintf(end_tag, "</%s>", tag);
    int new_start_index = start_index + strlen(start_tag);
    int end_index = get_index_of_tag_in_string(string, end_tag, new_start_index - 1);

    if (start_index == -1 || end_index == -1)
    {
        return NULL;
    }

    char *result_string = malloc((end_index - start_index + 2) * sizeof(char));
    int i;
    for (i = new_start_index; i < end_index; ++i)
    {
        result_string[i - new_start_index] = string[i];
    }
    result_string[i - new_start_index] = '\0';
    return result_string;
}
