# Internet log server

>  An embedded logging mechanism in our code with logs being sent to a central server which can keep track of how the code is performing.

## Process Side

### Logger
The logger is the real client that communicate with the server:


- **InitializeLog()** will:
    1. create a non-blocking socket for UDP communications (AF_INET, SOCK_DGRAM).
    2. Set the address and port of the server.
    3. Create a mutex to protect any shared resources.
    4. Start the receive thread and pass the file descriptor to it.
- **SetLogLevel()** will set the filter log level and store in a variable global within Logger.cpp.
- **Log()** will:
    1. compare the severity of the log to the filter log severity. The log will be thrown away if its severity is lower than the filter log severity.
    2. create a timestamp to be added to the log message. Code for creating the log message will look something like:
    ```cpp
        time_t now = time(0);
        char *dt = ctime(&now);
        memset(buf, 0, BUF_LEN);
        char levelStr[][16]={"DEBUG", "WARNING", "ERROR", "CRITICAL"};
        len = sprintf(buf, "%s %s %s:%s:%d %s\n", dt, levelStr[level], file, func, line, message)+1;
        buf[len-1]='\0';
    ```
    3. apply mutexing to any shared resources used within the Log() function.
    4. The message will be sent to the server via UDP sendto().
- **ExitLog()** will stop the receive thread via an is_running flag and close the file descriptor.
- **The receive thread** is waiting for any commands from the server. So far there is only one command from the server: “Set Log Level=<level>”. The receive thread will:
    1. accept the file descriptor as an argument.
    2. run in an endless loop via an is_running flag.
    3. apply mutexing to any shared resources used within the recvfrom() function.
    4. ensure the recvfrom() function is non-blocking with a sleep of 1 second if nothing is received.
    5. act on the command “Set Log Level=<level>” from the server to overwrite the filter log severity. 

## Server Side

### Server
THe LogServer.cpp will host on the company meachine to receive all the datalog that come from the client.


- The server will shutdown gracefully via a ctrl-C via a shutdownHandler.  

- The server’s main() function will create a non-blocking socket for UDP communications (AF_INET, SOCK_DGRAM).
- The server’s main() function will bind the socket to its IP address and to an available port.
- The server’s main() function will create a mutex and apply mutexing to any shared resources.
- The server’s main() function will start a receive thread and pass the file descriptor to it.
- The server’s main() function will present the user with three options via a user menu:
    - Set the log level
        - The user will be prompted to enter the filter log severity.
        - The information will be sent to the logger. Sample code will look something like:
        ```cpp
            memset(buf, 0, BUF_LEN);
            len=sprintf(buf, "Set Log Level=%d", level)+1;
            sendto(fd, buf, len, 0, (struct sockaddr *)&remaddr, addrlen);
        ```
    - Dump the log file here
        - The server will open its server log file for read only.
        - It will read the server’s log file contents and display them on the screen.
        - On completion, it will prompt the user with:
        ```cpp
            "Press any key to continue:"
        ```
    - Shut down
        - The receive thread will be shutdown via an is_running flag.
        - The server will exit its user menu.
        - The server will join the receive thread to itself so it doesn’t shut down before the receive thread does.
 - The server’s receive thread will:
    - open the server log file for write only with permissions rw-rw-rw-
    - run in an endless while loop via an is_running flag.
    - apply mutexing to any shared resources used within the recvfrom() function.
    - ensure the recvfrom() function is non-blocking with a sleep of 1 second if nothing is received.
    - take any content from recvfrom() and write to the server log file.


