/**
 * A C program to demonstrate the Producer and Consumer problem.
 *
 * To properly compile this program see COMPILE:
 *
 * COMPILE: gcc simulator.c -lpthread -o simulator
 *
 * To properly use this program see USAGE:
 *
 * USAGE: ./simulator <PATH_TO_CONFIG_FILE> <MAX_TEST_CASE_DURATION>
 * e.g. ./simulator "config.txt" 10
 *
 * @author Nicholas Adamou
 * @date 12/7/2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

typedef struct TestCase TestCase;

/**
 * Represents a given test case associated with each line within the given configuration file.
 */
struct TestCase
{
    int BSIZE;					 // The maximum size of the buffer.
    int producer_sleep_duration; // The amount of time a producer should sleep for.
    int consumer_sleep_duration; // The amount of time a consumer should sleep for.
    bool terminated;

    int *buf;
    int front;
    int rear;

    pthread_mutex_t producer_lock;
    pthread_mutex_t consumer_lock;
    pthread_cond_t producer_flag;
    pthread_cond_t consumer_flag;
};

int size(int front, int rear, int capacity);

char **split(char *str, char tokens);
char **readFile(char * path, int number_of_lines);
int numberOfLinesInFile(char * path);

void *produce(void *argv);
void *consume(void *argv);
void execute(int test_case_number, int duration, int num_producers, int num_consumers, TestCase *test_case);

/**
 * Determines the size of the given buffer (thread safe).
 *
 * @param front The index of the front element of the buffer.
 * @param rear The index of the rear element of the buffer.
 * @param capacity The capacity of the buffer.
 *
 * @return The size of the buffer.
 */
int size(int front, int rear, int capacity)
{
    if (front == -1)
        return 0;

    if (front >= rear)
        return (rear + capacity) - front;

    return rear - front;
}

/**
 * 'split' splits a string separated by a given char into a character array.
 *
 * WARNING: 'split' malloc()s memory to '**result' which must be freed by
 * the caller.
 *
 * @param str The string to split.
 * @param separator The separator character tto split by.
 *
 * @return The character array containing the each element split via the given separator.
 */
char **split(char *str, const char separator)
{
    char **result = 0;
    size_t count = 0;
    char *tmp = str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = separator;
    delim[1] = 0;

    // Count how many elements will be extracted.
    while (*tmp)
    {
        if (separator == *tmp)
        {
            count++;
            last_comma = tmp;
        }

        tmp++;
    }

    // Add space for trailing token.
    count += last_comma < (str + strlen(str) - 1);

    // Add space for terminating null string so caller
    // knows where the list of returned strings ends.
    count++;

    result = malloc(sizeof(char *) * count);

    if (!result) {
        perror("malloc");
    } else {
        size_t i = 0;
        char *token = strtok(str, delim);

        while (token)
        {
            result[i++] = strdup(token);
            token = strtok(NULL, delim);
        }

        result[i] = 0;
    }

    return result;
}

/**
 * Counts the number of lines within a given file.
 *
 * @param path The path to the given file.
 *
 * @return The number of lines contained within the provided file.
 */
int numberOfLinesInFile(char *path) {
    FILE *file = fopen(path, "r");

    if (!file) {
        fputs("The provided <PATH_TO_CONFIG FILE> does not exist.\n\nSimulator\nUsage: ./simulator <PATH_TO_CONFIG_FILE> <MAX_TEST_CASE_DURATION>\n", stderr);
        return -1;
    }

    int number_of_lines = 0;
    char buf[100];

    while(fgets(buf, 100, file) != NULL) {
        number_of_lines++;
    }

    fclose(file);

    return number_of_lines;
}

/**
 * Reads each line of a given file into a character array.
 *
 * WARNING: 'readFile' malloc()s memory to '**data' which must be freed by
 * the caller.
 *
 * @param path The path to the file.
 * @param number_of_lines The number of lines contained within in the given file.
 *
 * @return The array of lines contained within the provided file.
 */
char **readFile(char *path, int number_of_lines) {
    FILE *file = fopen(path, "r");

    if (!file) {
        fputs("The provided <PATH_TO_CONFIG FILE> does not exist.\n\nSimulator\nUsage: ./simulator <PATH_TO_CONFIG_FILE> <MAX_TEST_CASE_DURATION>\n", stderr);
        return NULL;
    }

    char **data = (char **) malloc(sizeof(char*) * number_of_lines);

    if (!data) {
        perror("malloc");
    }

    for (int i = 0; i < number_of_lines; i++) {
        data[i] = (char *)malloc(sizeof(char) * 100);

        if (!data[i]) {
            perror("malloc");
            continue;
        }

        fgets(data[i], 100, file);
    }

    fclose(file);

    return data;
}

/**
 * The function used with a producer thread.
 *
 * Appends a random number to the end of the queue
 * locks all other producer threads while this producer is producing,
 * signals to the next consumer to start consuming, then unlocks the
 * producer lock so other producers can produce and sleeps for x seconds.
 *
 * @param argv The arguments passed to the producer thread.
 */
void *produce(void *argv)
{
    TestCase *test_case = (TestCase *)argv;
    int *buf = test_case->buf;

    while (!test_case->terminated)
    {
        pthread_mutex_lock(&(test_case->producer_lock));

        if (size(test_case->front, test_case->rear, test_case->BSIZE) == test_case->BSIZE)
        {
            printf("\tQueue is full, cannot produce, waiting for consumer\n");
            pthread_cond_wait(&(test_case->producer_flag), &(test_case->producer_lock));
        }

        if (test_case->terminated) {
            pthread_mutex_unlock(&(test_case->producer_lock));
            break;
        }

        int element = rand() % 201;

        if (size(test_case->front, test_case->rear, test_case->BSIZE) == 0)
        {
            test_case->front = 0;
            test_case->rear = 0;
        }

        buf[test_case->rear++] = element;
        test_case->rear %= test_case->BSIZE;
        printf("\tProducer produces an item %d\n", element);

        pthread_cond_signal(&(test_case->consumer_flag));

        pthread_mutex_unlock(&(test_case->producer_lock));

        sleep(rand() % test_case->producer_sleep_duration);
    }

    pthread_exit(NULL);
}

/**
 * The function used with a consumer thread.
 *
 * Removes the first item in the buffer and
 * locks all other consumer threads while this consumer is consuming,
 * signals to the next producer to start producing, then unlocks the
 * consumer lock so other consumers can produce and sleeps for y seconds.
 *
 * @param argv The arguments passed to the consumer thread.
 */
void *consume(void *argv)
{
    TestCase *test_case = (TestCase *)argv;
    int *buf = test_case->buf;

    while (!test_case->terminated)
    {
        pthread_mutex_lock(&(test_case->consumer_lock));

        if (size(test_case->front, test_case->rear, test_case->BSIZE) == 0)
        {
            printf("\tQueue is empty, cannot consume, waiting for producer\n");
            pthread_cond_wait(&(test_case->consumer_flag), &(test_case->consumer_lock));
        }

        if (test_case->terminated) {
            pthread_mutex_unlock(&(test_case->consumer_lock));
            break;
        }

        int element = buf[test_case->front++];
        test_case->front %= test_case->BSIZE;
        printf("\tConsumer consumes an item %d\n", element);

        if (test_case->front == test_case->rear)
            test_case->front = -1;

        pthread_cond_signal(&(test_case->producer_flag));

        pthread_mutex_unlock(&(test_case->consumer_lock));

        sleep(rand() % test_case->consumer_sleep_duration);
    }

    pthread_exit(NULL);
}

/**
 * Executes the simulation of the producer and consumer problem based on a given test case.
 *
 * @param test_case_number The current test case number.
 * @param duration The maximum duration of each test case.
 * @param num_producers The number of producers.
 * @param num_consumers The number of consumers.
 * @param test_case The structure holding the test case parameters.
 */
void execute(int test_case_number, int test_case_duration, int num_producers, int num_consumers, TestCase *test_case)
{
    printf("Test Case %d\n", test_case_number);
    printf("\tbufferSize = %d, producer_sleep_duration = %d, consumer_sleep_duration = %d, num_producers = %d, num_consumers = %d \n", test_case->BSIZE, test_case->producer_sleep_duration, test_case->consumer_sleep_duration, num_producers, num_consumers);

    pthread_mutex_init(&(test_case->producer_lock), NULL);
    pthread_mutex_init(&(test_case->consumer_lock), NULL);
    pthread_cond_init(&(test_case->producer_flag), NULL);
    pthread_cond_init(&(test_case->consumer_flag), NULL);

    pthread_t producers[num_producers];
    pthread_t consumers[num_consumers];

    for (int i = 0; i < num_producers; i++)
        pthread_create(&producers[i], NULL, produce, (void *)test_case);

    for (int i = 0; i < num_consumers; i++)
        pthread_create(&consumers[i], NULL, consume, (void *)test_case);

    sleep(test_case_duration);
    test_case->terminated = true;

    for (int i = 0; i < num_producers; i++)
        pthread_cond_signal(&(test_case->producer_flag));

    for (int i = 0; i < num_consumers; i++)
        pthread_cond_signal(&(test_case->consumer_flag));

    for (int i = 0; i < num_producers; i++)
        pthread_join(producers[i], NULL);

    for (int i = 0; i < num_consumers; i++)
        pthread_join(consumers[i], NULL);

    pthread_mutex_destroy(&(test_case->producer_lock));
    pthread_mutex_destroy(&(test_case->consumer_lock));
}

int main(int argc, char **argv)
{
    srand(time(0));

    char *PATH_TO_CONFIG_FILE;
    int MAX_TEST_CASE_DURATION = 0;

    if (argc < 3)
    {
        fputs("Incorrect number of arguments.\n\nProducerConsumerTests\nUsage: ./ProducerConsumerTests <PATH_TO_CONFIG_FILE> <MAX_TEST_CASE_DURATION>\n", stderr);
        exit(1);
    }

    PATH_TO_CONFIG_FILE = argv[1];
    MAX_TEST_CASE_DURATION = atoi(argv[2]);

    int number_of_lines = numberOfLinesInFile(PATH_TO_CONFIG_FILE);
    char **lines = readFile(PATH_TO_CONFIG_FILE, number_of_lines);

    for (int test_case_number = 0; test_case_number < number_of_lines; test_case_number++) {
        char **data = split(lines[test_case_number], ',');

        TestCase *test_case = (TestCase *)malloc(sizeof(TestCase));

        if (!test_case) {
            perror("malloc");
            continue;
        }

        test_case->BSIZE = atoi(data[0]);
        test_case->producer_sleep_duration = atoi(data[1]);
        test_case->consumer_sleep_duration = atoi(data[2]);
        test_case->terminated = false;

        test_case->buf = (int *)malloc(sizeof(int) * test_case->BSIZE);
        test_case->front = -1;
        test_case->rear = -1;

        int num_producers = atoi(data[3]);
        int num_consumers = atoi(data[4]);

        execute(
                test_case_number + 1,
                MAX_TEST_CASE_DURATION,
                num_producers,
                num_consumers,
                test_case);

        printf("\n");

        free(test_case->buf);
        free(test_case);
        free(data);
    }

    free(lines);

    return 0;
}
