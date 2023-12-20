#include "utils.h"

void ServerDecoder();
void ServerEncoder();
void initQueue();
int isFull();
int isEmpty();
void enqueue(char);
char dequeue();
void *writerThread(void *);
void cleanup(int);
void *charAThread(void *);
void *charEThread(void *);
void *charIThread(void *);
void *charOThread(void *);
void *charUThread(void *);

char messageGiven[BUFFER_SIZE];

int global_socket_fd;
sem_t sem_lock;

typedef struct
{
    char buffer[MAXSIZE];
    int head;
    int tail;
    int currSize;
} Queue;

Queue queue;

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
    printf("Helper Connection Successful.\n");

    // Saying it is Helper node to the server
    send(global_socket_fd, HELPERNODE, strlen(HELPERNODE), 0);

    // send and receive messages
    while (1)
    {
        ServerDecoder();
    }

    close(socket_fd);
    return 0;
}
// cleanup code when halted
void cleanup(int signal_number)
{
    printf("\nShutting down Helpernode\n");
    if (global_socket_fd >= 0)
    {
        if (signal_number != SERVER_ABORT)
        {
            send(global_socket_fd, SHUTDOWNHELPER, strlen(SHUTDOWNHELPER), 0);
            // logout(global_socket_fd);
        }
        close(global_socket_fd);
    }
    exit(1);
}

// receive messages from server, convert them to uppercase vowels and send them again
void ServerDecoder()
{
    char message_received[BUFFER_SIZE];
    memset(message_received, 0, BUFFER_SIZE);
    int message_received_length = recv(global_socket_fd, message_received, BUFFER_SIZE, 0);
    if (message_received_length != -1)
    {
        ServerEncoder(message_received);
        send(global_socket_fd, messageGiven, strlen(messageGiven), 0);
    }
    else if (message_received_length == 0)
    {
        printf("Server Logged out\n");
        exit(0);
    }
}

// initialising queue data structure
void initQueue()
{
    queue.currSize = 0;
    queue.head = 0;
    queue.tail = 0;
}

// check if queue is full
int isFull()
{
    return queue.currSize == MAXSIZE;
}

// check if queue is empty
int isEmpty()
{
    return queue.head == queue.tail;
}

// enqueue operation
void enqueue(char vowel)
{
    if (isFull())
    {
        printf("Queue is Full\n");
        return;
    }
    queue.currSize = queue.currSize + 1;
    queue.buffer[queue.tail] = vowel;
    queue.tail = (queue.tail + 1);
}

// dequeue operation
char dequeue()
{
    if (isEmpty())
    {
        printf("Queue is empty, cannot delete\n");
        return 0;
    }

    char dequeueChar = queue.buffer[queue.head];
    queue.head = queue.head + 1;
    queue.currSize = queue.currSize - 1;

    return dequeueChar;
}

// convert vowels to uppercase in the message
void ServerEncoder(char *message_received)
{
    memset(messageGiven, 0, BUFFER_SIZE);
    strcpy(messageGiven, message_received);
    messageGiven[strlen(message_received)] = '\0';

    sem_init(&sem_lock, 0, 1);

    initQueue();

    for (int i = 0; i < MAXSIZE; i++)
    {
        enqueue(VOWELS[i]);
    }
    // printf("%s\n", messageGiven);
    pthread_t pthId;

    // creating the thread for handling the char vowels

    for (int i = 0; i < 5; i++)
    {
        sleep(0.2);
        pthread_create(&pthId, NULL, writerThread, &queue.buffer[i]); //?Create thread for separate vowels and convert to upper case vowels
    }

    for (int i = queue.head; i < queue.tail; i++)
    {
        pthread_join(pthId, NULL);
    }

    sem_destroy(&sem_lock);
}

// char A thread
void *charAThread(void *args)
{
    sem_wait(&sem_lock);

    for (int j = 0; j < strlen(messageGiven); j++)
    {
        if (messageGiven[j] == 'a')
        {
            messageGiven[j] = toupper(messageGiven[j]);
        }
    }

    sem_post(&sem_lock);
}

// char E thread
void *charEThread(void *args)
{
    sem_wait(&sem_lock);
    for (int j = 0; j < strlen(messageGiven); j++)
    {
        if (messageGiven[j] == 'e')
        {
            messageGiven[j] = toupper(messageGiven[j]);
        }
    }

    sem_post(&sem_lock);
}
// char I thread
void *charIThread(void *args)
{
    sem_wait(&sem_lock);
    for (int j = 0; j < strlen(messageGiven); j++)
    {
        if (messageGiven[j] == 'i')
        {
            messageGiven[j] = toupper(messageGiven[j]);
        }
    }

    sem_post(&sem_lock);
}
// char o thread
void *charOThread(void *args)
{
    sem_wait(&sem_lock);
    int i = 0;

    for (int j = 0; j < strlen(messageGiven); j++)
    {
        if (messageGiven[j] == 'o')
        {
            messageGiven[j] = toupper(messageGiven[j]);
        }
    }

    sem_post(&sem_lock);
}
// char U thread
void *charUThread(void *args)
{
    sem_wait(&sem_lock);
    for (int j = 0; j < strlen(messageGiven); j++)
    {
        if (messageGiven[j] == 'u')
        {
            messageGiven[j] = toupper(messageGiven[j]);
        }
    }

    sem_post(&sem_lock);
}

// Writer thread which shares the information
void *writerThread(void *args)
{
    sem_wait(&sem_lock);
    char vowel = *(char *)args;
    printf("char%c:", toupper(vowel));
    for (int i = 0; i < strlen(messageGiven); i++)
    {
        if (vowel == messageGiven[i])
        {
            messageGiven[i] = toupper(messageGiven[i]);
        }
    }
    printf("%s\n", messageGiven);
    sem_post(&sem_lock);
}