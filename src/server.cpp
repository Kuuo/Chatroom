#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <iostream>
#include <vector>

std::vector<int> clients;

std::string getAllClients(int FD)
{
    std::string ret = "-----CLIENT LIST-----\n";
    for (auto &c : clients)
    {
        ret += std::to_string(c);
        if (c == FD)
        {
            ret += " (you)";
        }
        ret += '\n';
    }
    ret += "-----CLIENT LIST-----\n";
    return ret;
}

void sendMsg2AllClients(const char *content)
{
    for (auto &c : clients)
    {
        send(c, content, strlen(content), 0);
    }
}

void sendMsg2ClientsExcept(const char *content, int exceptFD)
{
    for (auto &c : clients)
    {
        if (c == exceptFD)
            continue;
        send(c, content, strlen(content), 0);
    }
}

void *recvsocket(void *arg) //接收client端socket数据的线程
{
    int st = *(int*)arg;
    char s[1024];

    while (1)
    {
        memset(s, 0, sizeof(s));
        int rc = recv(st, s, sizeof(s), 0);
        if (rc <= 0) //如果recv返回小于等于0，代表socket已经关闭或者出错了
            break;

        if (strcmp(s, "GET_CLIENT_LIST\n") == 0)
        {
            std::string list = getAllClients(st);
            send(st, list.c_str(), list.size(), 0);
            continue;
        }

        char content[1024];
        sprintf(content, "CLINET{%d}: %s", st, s);
        printf("%s", content);
        sendMsg2ClientsExcept(content, st);
    }

    close(st);
}

void *sendsocket(void *arg) //向client端socket发送数据的线程
{
    char s[1024];
    while (1)
    {
        memset(s, 0, sizeof(s));
        read(STDIN_FILENO, s, sizeof(s)); //从键盘读取用户输入信息

        char content[2048];
        sprintf(content, "SERVER: %s", s);
        sendMsg2AllClients(content);
    }
}

int main(int arg, char *args[])
{
    if (arg < 2)
    {
        return -1;
    }

    int port = atoi(args[1]);
    int st = socket(AF_INET, SOCK_STREAM, 0); //初始化socket
    int on = 1;

    //IP可重用，关掉程序还能启动同个IP聊天
    if (setsockopt(st, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
    {
        printf("setsockopt failed %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr; //定义一个IP地址结构
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;                //将addr结构的属性定位为TCP/IP地址
    addr.sin_port = htons(port);              //将本地字节顺序转化为网络字节顺序。
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY代表这个server上所有的地址

    //将IP与server程序绑定
    if (bind(st, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("bind failed %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    //server端开始listen，
    if (listen(st, 20) == -1)
    {
        printf("listen failed %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    pthread_t thread_send;
    pthread_create(&thread_send, NULL, sendsocket, NULL);

    int client_st = 0;              //client端socket
    struct sockaddr_in client_addr; //表示client端的IP地址

    while (1)
    {
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t len = sizeof(client_addr);

        //accept会阻塞，直到有客户端连接过来，accept返回client的socket描述符
        client_st = accept(st, (struct sockaddr *)&client_addr, &len);

        if (client_st == -1)
        {
            printf("Client connection failed %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        printf("New Client from: %s\n", inet_ntoa(client_addr.sin_addr));

        clients.push_back(client_st);

        pthread_t thrd1;
        pthread_create(&thrd1, NULL, recvsocket, &client_st);

        // std::string greeting = "Greeting from server~~ Hello " + client_st + '\n';
        // send(client_st, greeting.c_str(), greeting.size(), 0);
    }

    close(st);
    return EXIT_SUCCESS;
}
