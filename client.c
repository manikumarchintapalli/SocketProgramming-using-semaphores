#include "utils.h"
#include "errorhamming.h"

// Function Signatures

char *collect_and_validate_user_id(int);
void *receive_and_print_text_from_server(void *);
void cleanup(int);
void listen_messages_from_server_in_new_thread(int);
void send_messages_to_server(char *, int);

// Global variables

// Variable to store the socket fd globally -> to use in cleanup function
int global_socket_fd;

// Main Function
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        log_error("Invalid arguments.");
    }

    // Adding Signal Handler
    signal(SIGINT, cleanup);

    int socket_fd = intialise_socket_with_ipv4_and_tcp();
    global_socket_fd = socket_fd;
    struct sockaddr_in *socket_address = intialise_socket_address(argv[1], argv[2]);

    // Connecting to server
    int connection_result = connect(socket_fd, (struct sockaddr *)socket_address, sizeof(*socket_address));
    if (connection_result < 0)
    {
        log_error("Error connecting to server");
    }
    printf("Connection Successful.\n");

    // Collect user_id
    char *user_id = collect_and_validate_user_id(socket_fd);

    // listen messages
    listen_messages_from_server_in_new_thread(socket_fd);

    // send messages
    send_messages_to_server(user_id, socket_fd);

    close(socket_fd);
    return 0;
}

// Function to send message to server
void send_messages_to_server(char *user_id, int socket_fd)
{
    // char buffer[BUFFER_SIZE];

    while (1)
    {
        char *message = NULL;
        ssize_t message_length = 0;
        ssize_t character_count = getline(&message, &message_length, stdin);
        if (character_count > 0)
        {
            message[character_count - 1] = '\0';

            // sprintf(buffer, "%s := %s", user_id, message);
            ssize_t characters_sent = send(socket_fd, message, strlen(message), 0);
            if (strncmp(message, "<LOGOUT>", 8) == 0)
            {
                break;
            }
        }
    }
}

// Function to collect and validate user id
char *collect_and_validate_user_id(int socket_fd)
{
    printf("Please enter your user id in login tag: ");
    char *user_id = malloc(USER_ID_LEN * USER_ID_LEN * sizeof(char));
    size_t user_id_length = 0;
    ssize_t character_count = getline(&user_id, &user_id_length, stdin);
    if (!isdigit(user_id[7]) && character_count == USER_ID_LEN + 16)
    {
        char message[BUFFER_SIZE];
        user_id[character_count - 1] = '\0';

        // Validating the user on server side
        send(socket_fd, user_id, strlen(user_id), 0);
        ssize_t message_length = recv(socket_fd, message, BUFFER_SIZE, 0);
        message[message_length] = '\0';
        printf("%s\n", message);
        if (strcmp(message, USER_REG_SUCCESS) == 0)
        {
            return user_id;
        }
    }
    printf("Enter valid user id which is of 8 characters starting with a character and encoded as <LOGIN>userid</LOGIN>\n");
    return collect_and_validate_user_id(socket_fd);
}

// Function to intialise thread to listen messages from server
void listen_messages_from_server_in_new_thread(int socket_fd)
{
    pthread_t thread;
    pthread_create(&thread, NULL, receive_and_print_text_from_server, &socket_fd);
}

// Function to print messages received from server
void *receive_and_print_text_from_server(void *socket_fd_address)
{
    int socket_fd = *((int *)socket_fd_address);
    char message[BUFFER_SIZE];

    while (1)
    {
        ssize_t message_length = recv(socket_fd, message, BUFFER_SIZE, 0);

        if (message_length > 0)
        {
            message[message_length] = '\0';
            printf("%s\n", message);
            if (strcmp(message, SERVER_SHUTDOWN_MSG) == 0)
            {
                cleanup(SERVER_ABORT);
            }
        }
        else
        {
            break;
        }
        bzero(message, BUFFER_SIZE);
    }
    close(socket_fd);
}

// Function to logout
void logout(int socket_fd)
{
    char msg[] = "<LOGOUT></LOGOUT>";
    send(socket_fd, msg, strlen(msg), 0);
}

// Cleanup function
void cleanup(int signal_number)
{
    printf("\nShutting down client\n");
    if (global_socket_fd >= 0)
    {
        if (signal_number != SERVER_ABORT)
        {
            logout(global_socket_fd);
        }
        close(global_socket_fd);
    }
    exit(1);
}