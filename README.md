# Producer Consumer Simulator

## What is '_Producer Consumer Simulator_'? 🤔

An assignment I had to complete that involved demonstrating the Producer Consumer problem.

## Problem Statement

Design and implement a system in which demonstrates the Producer Consumer problem.

In computing, the producer–consumer problem (also known as the bounded-buffer problem) is a classic example of a multi-process synchronization problem. The problem describes two processes, the producer and the consumer, who share a common, fixed-size buffer used as a queue. The producer's job is to generate data, put it into the buffer, and start again. At the same time, the consumer is consuming the data (i.e., removing it from the buffer), one piece at a time. The problem is to make sure that the producer won't try to add data into the buffer if it's full and that the consumer won't try to remove data from an empty buffer.

The solution for the producer is to either go to sleep or discard data if the buffer is full. The next time the consumer removes an item from the buffer, it notifies the producer, who starts to fill the buffer again. In the same way, the consumer can go to sleep if it finds the buffer empty. The next time the producer puts data into the buffer, it wakes up the sleeping consumer. The solution can be reached by means of inter-process communication, typically using semaphores. An inadequate solution could result in a deadlock where both processes are waiting to be awakened. The problem can also be generalized to have multiple producers and consumers.

⚠️  _**Note**_: Please see [Producer–consumer problem](https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem) for more details on the solution to the Producer Consumer problem.

## Compiling

```shell script
gcc simulator.c -lpthread -o simulator
```

## Running

The program requires a `config.txt` file to be present in the root of the project. The program parses this file for simulation configuration. The `config.txt` file contains the following content.

```shell script
# <BSIZE> <PRODUCER_WAIT_TIME> <CONSUMER_WAIT_TIME> <NUM_PRODUCERS> <NUM_CONSUMERS>
```

Each additional row depicts single simulation.

```shell script
# ./simulator <PATH_TO_CONFIG_FILE> <MAX_TEST_CASE_DURATION>
./simulator "config.txt" 10
```

As denoted by `<MAX_TEST_CASE_DURATION>` each simulation runs for a designated amount of time. This time is denoted by the second parameter passed to the `simulator` program. This amount of time is in terms of _seconds_. 

## License

producer-consumer-simulator is © Nicholas Adamou.

It is free software, and may be redistributed under the terms specified in the [LICENSE] file.

[LICENSE]: LICENSE
