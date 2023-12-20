#include "utils.h"
#include "errorhamming.h"

// USER DEFINED DATA TYPES

// Client Data Type
typedef struct
{
    int client_fd;
    char user_id[USER_ID_LEN + 1];
    int is_authenticated;
    int is_helper_node;
    struct sockaddr_in client_address;
} Client;

int helper_fd = -1;

// FUNCTION SIGNATURES

Client *accept_incoming_connection(int);
int get_empty_client_index();
int get_global_client_index_based_on_user_id(char *);
void *receive_and_print_text_from_client(Client *);
void accept_all_incoming_connections(int);
void append_text_to_file(char *, char *);
void cleanup(int);
void client_login(char *, Client *);
void client_logout(char *, Client *);
void close_client_connections();
void copy_user_id(Client *, char *);
void decode_client_text(char *, Client *);
void decode_message(char *, Client *);
void handle_info_tag(char *);
void read_from_console_in_new_thread();
void read_from_console();
void receive_text_from_client_in_new_thread(Client *);
void remove_text_files_based_on_user_id(Client *);
void send_clients_list_to_target_client(Client *);
void send_message_to_target_client(Client *, char *);
int check_if_helpernode_exists(char *, Client *);
int is_helper_shut_down(char *);

// GLOBAL VARIABLES

// Variable to store list of clients
Client clients[MAX_USERS];

// Variable to store main socket fd globally
// Declaring this to use in cleanup function
int global_socket_fd;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Invalid arguments\n");
        exit(1);
    }

    // Adding Signal Handler
    signal(SIGINT, cleanup);
    int socket_fd = intialise_socket_with_ipv4_and_tcp();
    global_socket_fd = socket_fd;
    struct sockaddr_in *socket_address = intialise_socket_address("", argv[1]);

    // Binding the socket to the server
    bind_server(socket_fd, socket_address);

    // Listen to Clients
    listen_to_port(socket_fd, MAX_USERS, argv[1]);

    // Send messages from server to user in new thread
    read_from_console_in_new_thread();

    // Accept incoming connections of clients
    accept_all_incoming_connections(socket_fd);

    // Close clients
    close_client_connections();

    // Shutdown client
    shutdown(socket_fd, SHUT_RDWR);

    return 0;
}

// Function to accept all the incoming connections
void accept_all_incoming_connections(int socket_fd)
{
    while (1)
    {
        Client *client = accept_incoming_connection(socket_fd);

        //    Storing client information in clients array
        int empty_client_index = get_empty_client_index();
        if (empty_client_index < 0)
        {
            printf("MAX CLIENTS REACHED\n");
            return;
        }
        clients[empty_client_index] = *client;

        receive_text_from_client_in_new_thread(client);
    }
}

// Function to accept one incoming connection
Client *accept_incoming_connection(int socket_fd)
{
    struct sockaddr_in client_address;
    int client_address_length = sizeof(client_address);
    int client_fd = accept(socket_fd, (struct sockaddr *)&client_address, &client_address_length);
    if (client_fd < 0)
    {
        log_error("Accepting the client connection failed.");
    }
    Client *client = malloc(sizeof(Client));
    client->client_fd = client_fd;
    client->is_authenticated = client_fd >= 0;
    client->client_address = client_address;
    return client;
}

// Function to read text from all the clients
void receive_text_from_client_in_new_thread(Client *client)
{
    pthread_t thread;
    pthread_create(&thread, NULL, (void *)receive_and_print_text_from_client, client);
}

// Function to read text from single client
void *receive_and_print_text_from_client(Client *client)
{

    char message[BUFFER_SIZE];
    while (1)
    {
        ssize_t message_length = recv(client->client_fd, message, BUFFER_SIZE, 0);

        // Printing the message
        if (message_length > 0)
        {
            // check whether the message is helper and set the value for ishelper in clients.
            int has_helper_logged_in = check_if_helpernode_exists(message, client);
            int has_helper_logged_out = is_helper_shut_down(message);
            if (has_helper_logged_in == 0 && has_helper_logged_out == 0)
            {
                decode_client_text(message, client);
            }
        }
        else
        {
            break;
        }
        bzero(message, BUFFER_SIZE);
    }
    close(client->client_fd);
}

// check if the message is helpernode
int check_if_helpernode_exists(char *message_recieved, Client *client)
{
    if (strcmp(message_recieved, HELPERNODE) == 0)
    {
        for (int i = 0; i < MAX_USERS; i++)
        {
            if (client->client_fd == clients[i].client_fd)
            {
                clients[i].is_helper_node = 1;
                helper_fd = client->client_fd;
            }
        }
        return 1;
    }
    return 0;
}

// Function to send message to target client
void send_message_to_target_client(Client *client, char *message)
{
    for (int i = 0; i < MAX_USERS; ++i)
    {
        if ((client->is_authenticated) && (clients[i].client_fd == client->client_fd))
        {
            send(clients[i].client_fd, message, strlen(message), 0);
            break;
        }
    }
}

// Function to close all the client connections
void close_client_connections()
{
    for (int i = 0; i < MAX_USERS; ++i)
    {
        send_message_to_target_client(&clients[i], SERVER_SHUTDOWN_MSG);
        close(clients[i].client_fd);
        remove_text_files_based_on_user_id(&clients[i]);
    }
}

// Function to copy user id into global client object
void copy_user_id(Client *client, char *user_id)
{
    for (int i = 0; i < MAX_USERS; ++i)
    {
        if (clients[i].client_fd == client->client_fd)
        {
            strcpy(clients[i].user_id, user_id);
        }
    }
}

// Function to handle messages and tags from clients
void decode_client_text(char *message, Client *client)
{

    // Handling Login
    if (strlen(client->user_id) == 0)
    {
        client_login(message, client);
        return;
    }

    // Writing client messages to file
    char filename[BUFFER_SIZE];
    snprintf(filename, BUFFER_SIZE, "%s.txt", client->user_id);
    append_text_to_file(filename, message);

    // Handling Logout
    char *logout = extract_text_between_tag(message, "LOGOUT");
    if (logout != NULL)
    {
        free(logout);
        client_logout(message, client);
    }

    // Handling Login List
    char *login_list = extract_text_between_tag(message, "LOGIN_LIST");
    if (login_list != NULL)
    {
        free(login_list);
        send_clients_list_to_target_client(client);
    }

    // Handling Messages
    decode_message(message, client);
}

// Function to return client index based on user id
int get_global_client_index_based_on_user_id(char *user_id)
{
    for (int i = 0; i < MAX_USERS; ++i)
    {
        if (strcmp(clients[i].user_id, user_id) == 0)
        {
            return i;
        }
    }
    return -1;
}

// Function to get empty client index
int get_empty_client_index()
{
    for (int i = 0; i < MAX_USERS; ++i)
    {
        if (!clients[i].is_authenticated)
        {
            return i;
        }
    }
    return -1;
}

// Function to authenticate user
void client_login(char *message, Client *client)
{
    char *user_id = extract_text_between_tag(message, "LOGIN");
    if (user_id != NULL)
    {
        // Checking if user id exists
        int user_exists = get_global_client_index_based_on_user_id(user_id);
        if (user_exists != -1)
        {
            send_message_to_target_client(client, USER_REG_ERROR);
            return;
        }

        // Register User
        copy_user_id(client, user_id);
        strcpy(client->user_id, user_id);
        printf("USER LOGGED IN: %s\n", user_id);
        send_message_to_target_client(client, USER_REG_SUCCESS);
        free(user_id);
    }
}

// Function to logout the user
void client_logout(char *message, Client *client)
{
    int index = get_global_client_index_based_on_user_id(client->user_id);
    if (index < 0)
    {
        return;
    }
    clients[index].is_authenticated = 0;
    if (strlen(clients[index].user_id) != 0)
    {
        printf("USER LOGGED OUT := %s\n", client->user_id);
    }
    close(client->client_fd);

    remove_text_files_based_on_user_id(client);
}

// Function to broadcast list of clients
void send_clients_list_to_target_client(Client *client)
{
    send_message_to_target_client(client, LOGIN_LIST_MSG);
    for (int i = 0; i < MAX_USERS; ++i)
    {
        if (clients[i].is_authenticated)
        {
            sleep(0.1);
            send_message_to_target_client(client, clients[i].user_id);
        }
    }
}

// Function to decode the message and send to target client
void decode_message(char *message, Client *client)
{
    char *msg = extract_text_between_tag(message, "MSG");
    if (msg != NULL)
    {

        char *encode = extract_text_between_tag(msg, "ENCODE");
        // char *encode = NULL;

        char *to = extract_text_between_tag(msg, "TO");
        char *body = extract_text_between_tag(msg, "BODY");
        char buffer[BUFFER_SIZE];

        // check whether the helper is signed in or not
        // printf("%d : %d\n", client->is_helper_logged_in, client->is_authenticated);

        if (to != NULL && body != NULL)
        {
            if (helper_fd != -1)
            {
                // send message body to helper
                send(helper_fd, body, strlen(body), 0);
                printf("Sending messagee and helper fd: %d : %s\n", helper_fd, body);

                // receive message from the helpernode
                int received_message_length = recv(helper_fd, body, strlen(body), 0);
                printf("Helper Node executed\n");
            }
            sprintf(buffer, "%s := %s", client->user_id, body);
            int target_client_index = get_global_client_index_based_on_user_id(to);
            if (target_client_index < 0)
            {
                return;
            }
            char filename[USER_ID_LEN * 3], message_to_be_written[BUFFER_SIZE];
            sprintf(filename, "%s%s.txt", client->user_id, clients[target_client_index].user_id);
            strcpy(message_to_be_written, body);

            if (encode != NULL)
            {
                if ((strcmp(encode, "HAMMING CODE")) == 0)
                {
                    size_t message_length = strlen(body);
                    char encrypt[BUFFER_SIZE];
                    // create_hamming_code(body, encrypt);
                    // get_bit_error(encrypt, 0);
                    if (check_hamming(encrypt))
                    {
                        printf("Error Corrected for Hamming Code message\n");
                    }
                    else
                    {
                        //  error_correction_hamming_code(encrypt);
                    }
                    char *decode = (char *)malloc(sizeof(char) * message_length * 2 + 1);
                    // choose_strings_from_hamming_code(encrypt, decode);

                    // Copy and Free variables
                    if (decode)
                    {
                        strcpy(message_to_be_written, decode);
                        free(decode);
                    }
                }
                if ((strcmp(encode, "CRC")) == 0)
                {
                    printf("No Error detected for CRC message\n");

                    // CRC Handling Messages
                    char binaryString[BUFFERSIZE];
                    char encodeCRC[BUFFERSIZE];
                    string_to_binary(body, binaryString);
                    encode_crc(PolynomialGenerator, binaryString, encodeCRC);
                    int decode_value = decode_crc(PolynomialGenerator, binaryString, encodeCRC);
                    if (!decode_value)
                    {
                        return;
                    }
                }

                // Free variables
                free(encode);
            }

            // Appending/Writing conversation to text files
            append_text_to_file(filename, body);

            // Sending message to target client
            send_message_to_target_client(&clients[target_client_index], buffer);

            // Free variables
            free(to);
            free(body);
        }

        // Free variables
        free(msg);
    }
}

// Function to append/write to file
void append_text_to_file(char *filename, char *message)
{
    FILE *fp = fopen(filename, "a");
    fprintf(fp, "%s\n", message);
    fclose(fp);
}

// Function to remove/delete file
void remove_text_files_based_on_user_id(Client *client)
{
    char **filename;
    glob_t glob_struct;
    char buffer[BUFFER_SIZE];
    if (strlen(client->user_id) == 0)
    {
        return;
    }
    sprintf(buffer, "%s*.txt", client->user_id);

    int r = glob(buffer, GLOB_ERR, NULL, &glob_struct);

    if (r != 0)
    {
        return;
    }

    filename = glob_struct.gl_pathv;
    while (*filename)
    {
        remove(*filename);
        filename++;
    }
}

// Function to read messages from console in new thread
void read_from_console_in_new_thread()
{
    pthread_t thread;
    pthread_create(&thread, NULL, (void *)read_from_console, NULL);
}

// Function to read messages from console
void read_from_console()
{
    char *message = NULL;
    ssize_t message_length = 0;
    char buffer[BUFFER_SIZE];

    while (1)
    {
        ssize_t character_count = getline(&message, &message_length, stdin);
        if (character_count > 0)
        {
            message[character_count - 1] = '\0';

            char *info = extract_text_between_tag(message, "INFO");
            if (info != NULL)
            {
                handle_info_tag(info);
            }
        }
    }
}

// check if helpershutdown
int is_helper_shut_down(char *message_received)
{
    if (strcmp(message_received, SHUTDOWNHELPER) == 0)
    {
        printf("Helper shutdown\n");
        helper_fd = 0;
        for (int i = 0; i < MAX_USERS; i++)
        {
            clients[i].is_helper_node = 0;
        }
        return 1;
    }
    return 0;
}

// Function to handle info tag
void handle_info_tag(char *info_message)
{
    char message[BUFFER_SIZE];
    sprintf(message, "Server := %s", info_message);
    for (int i = 0; i < MAX_USERS; ++i)
    {
        if (clients[i].client_fd != helper_fd)
        {
            send_message_to_target_client(&clients[i], message);
        }
    }
}

// Signal handler function
void cleanup(int signal_number)
{
    printf("\nShutting Down Server!!\n");

    // Closing client file sockets
    close_client_connections();

    // Closing global socket
    if (global_socket_fd >= 0)
    {
        close(global_socket_fd);
        shutdown(global_socket_fd, SHUT_RDWR);
    }

    // exit
    exit(1);
}