#include "Logger.h"

#include <errno.h>
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <queue>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

using namespace std;

bool is_running;
const int BUF_LEN=4096;
char buf[BUF_LEN];

queue<string> message;
pthread_mutex_t lock_x;
pthread_t tid;

LOG_LEVEL m_level;

//------
int fd;
    struct sockaddr_in servaddr;
    socklen_t addrlen = sizeof(servaddr);

void *recv_func(void *arg);

int InitializeLog(){
    int ret, len;



    //socket create
    fd=socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if(fd<0) {
        cout<<"Cannot create the socket"<<endl;
	    cout<<strerror(errno)<<endl;
	    return -1;
    }
    //address bind
    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    ret = inet_pton(AF_INET, IP_ADDR, &servaddr.sin_addr);
    if(ret==0) {
        cout<<"No such address"<<endl;
	    cout<<strerror(errno)<<endl;
        close(fd);
        return -1;
    }
    servaddr.sin_port = htons(PORT);

    is_running = true;
    //mutex create
    pthread_mutex_init(&lock_x, NULL);

    //thread create
    ret = pthread_create(&tid, NULL, recv_func, &fd);
    if(ret!=0) {
        cout<<"Cannot start thread"<<endl;
	    cout<<strerror(errno)<<endl;
        close(fd);
        return -1;
    }

    return 0;

}

void SetLogLevel(LOG_LEVEL level){
    m_level = level;
}

void Log(LOG_LEVEL level, const char* prog, const char* func, int line, const char* message){
    int len, ret;
    if(level >= m_level){
        time_t now = time(0);
        char *dt = ctime(&now);
        memset(buf, 0, BUF_LEN);
        char levelStr[][16]={"DEBUG", "WARNING", "ERROR", "CRITICAL"};
        //pthread_mutex_lock(&lock_x);
        len = sprintf(buf, "%s %s %s:%s:%d %s\n", dt, levelStr[level], prog, func, line, message)+1;
        //pthread_mutex_unlock(&lock_x);
        buf[len-1]='\0';


        ret = sendto(fd, buf, len, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
        
    }

}

void ExitLog(){
    is_running = false;
    close(fd);
}

void *recv_func(void *arg){
    int fd = *(int *)arg;
    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr);
    char buf[100];
    LOG_LEVEL tem;

    cout << "server: read()" << endl;
    while(is_running) {
        memset(buf,0,100);
        int len = recvfrom(fd, buf, BUF_LEN, 0, (struct sockaddr *)&remaddr, &addrlen);
        if(len > 0) {
            string rec = buf;
            string set = rec.substr(14);
            
            if(strncmp(set.c_str(), "DEBUG", 5) == 0){
                cout << "Set Log Level=<" << set << ">" << endl;
                tem = DEBUG;
                SetLogLevel(tem);
            }else if(strncmp(set.c_str(), "WARNING", 7) == 0){
                cout << "Set Log Level=<" << set << ">" << endl;
                tem = WARNING;
                SetLogLevel(tem);
            }else if(strncmp(set.c_str(), "ERROR", 5) == 0){
                cout << "Set Log Level=<" << set << ">" << endl;
                tem = ERROR;
                SetLogLevel(tem);
            }else if(strncmp(set.c_str(), "CRITICAL", 8) == 0){
                cout << "Set Log Level=<" << set << ">" << endl;
                tem = CRITICAL;
                SetLogLevel(tem);
            }
        } else sleep(1);
    }
    
    pthread_exit(NULL);
}
