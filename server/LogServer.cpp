/****************************************************************************************
  // THe LogServer.cpp will host on the another meachine to receive all the datalog that come from the client.
  //** High notice: if the S_IWOTH | S_IROTH connot set the file to rw-for other, umask000 will fix this problem of you setting.
  //    client process use it to send log information.
  //    server can also send information to it to change the monitor level.
  // The LogServer provide an interface that give user three main function
  //    1. give the user to change monitor level (LOG_LEVEL)
  //    2. give the user the ability to look the log file that display the lof file contain
  //    3. manually shut down the server

**************************************************************************************/
#include <arpa/inet.h>
#include <iostream>
#include <queue>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>

using namespace std;

const int PORT=1153;
//---
const char IP_ADDR[]="192.168.1.31";
const char FILENAME[]="logger.txt";
const int BUF_LEN=4096;
bool is_running;
int flag = 1;
    

struct sockaddr_in remaddr;
queue<string> message;
pthread_mutex_t lock_x;


void *recv_func(void *arg);

static void shutdownHandler(int sig);

int main(){
    int fd, ret, len;
    struct sockaddr_in myaddr;
    socklen_t addrlen = sizeof(remaddr);
    char buf[BUF_LEN];

    //socket create
    fd=socket(AF_INET, SOCK_DGRAM, 0);
    if(fd<0) {
        cout<<"Cannot create the socket"<<endl;
	    cout<<strerror(errno)<<endl;
	    return -1;
    }

    //bind IP address
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    ret = inet_pton(AF_INET, IP_ADDR, &myaddr.sin_addr);
    if(ret==0) {
        cout<<"No such address"<<endl;
	    cout<<strerror(errno)<<endl;
        close(fd);
        return -1;
    }
    myaddr.sin_port = htons(PORT);
    ret = bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr));
    if(ret<0) {
        cout<<"Cannot bind the socket to the local address"<<endl;
	cout<<strerror(errno)<<endl;
	return -1;
    }

    //mutex create
    pthread_mutex_init(&lock_x, NULL);

    //thread create
    is_running = true;
    pthread_t tid;
    ret = pthread_create(&tid, NULL, recv_func, &fd);
    if(ret!=0) {
        cout<<"Cannot start thread"<<endl;
        cout<<strerror(errno)<<endl;
        close(fd);
        return -1;
    }

    //interface
    int input=0;
    
    while (flag == 1){

        printf("1. Set the log level\n2. Dump the log file here\n3. Shut down\n");
        cin >> input;
        cout << "xx" << endl;
        if (input == 1){   //reset level
            printf("What level? (0-Debug, 1-Warning, 2-Error, 3-Critical): \n");
            cin >> input;
            string mes;
            if(input == 0)  mes = "DEBUG";
            else if(input == 1) mes = "WARNING";
            else if(input == 2) mes = "ERROR";
            else if(input == 3) mes = "CRITICAL";
            memset(buf, 0, BUF_LEN);
            len=sprintf(buf, "Set Log Level=%s", mes.c_str())+1;
            cout << buf << endl;
            sendto(fd, buf, len, 0, (struct sockaddr *)&remaddr, addrlen);


        }else if(input == 2){ //output file contain and output
            int offset=0;
            memset(buf, 0, BUF_LEN);
            // int rd;
            // rd = open(FILENAME, O_RDONLY);
            // if(rd == -1)
            //     cout << "error read file" << endl;
	        
	        // int ret = pread(rd, buf, BUF_LEN-1, offset);
	        // if(ret>0) {
            //     cout << "1" << endl;
	        //     cout<<buf<<endl;
	        //     offset += ret;
            // }
        
	        // close(rd);
            char c;
            ifstream in(FILENAME);
            while (!in.eof()) {
                in.get(c);
                cout << c;
            }
            in.close();
            string tem;
            cout << endl;
            cout << "Press any key to continue:";
            cin >> tem;
            if(offset > 4096) offset=0;

        }else if(input == 3){ //shut down
            shutdownHandler(SIGINT);
        }

        system("clear");
    }

    return 0;
        
}

void *recv_func(void *arg)
{
    int fd = *(int *)arg;
    socklen_t addrlen = sizeof(remaddr);
    char buf[100];

    // open file
    int wd;
    
    wd = open(FILENAME, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);

    while(is_running) {
        memset(buf,0,100);
        pthread_mutex_lock(&lock_x);
        int len = recvfrom(fd, buf, BUF_LEN, 0, (struct sockaddr *)&remaddr, &addrlen);
        pthread_mutex_unlock(&lock_x);
        
        if(len > 0){
        //write to file
            //cout << buf << endl;
            int ret = write(wd, buf, strlen(buf)-1);
            if(ret < 1){
                cout << "write error" << endl;
            }
        } else sleep(1);
	    
    } 
    close(wd);
    pthread_exit(NULL);

}

static void shutdownHandler(int sig){
    switch (sig)
    {
    case SIGINT:
        cout << "Server Shut down" << endl;
        is_running = false;
        flag = 0;
        break;
    }
}