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
#include <algorithm>

struct client_t
{
    int id;
    char nickname[32];
};

std::vector<client_t> clients;

bool removeClient(int id)
{
    for (auto it = clients.begin(); it != clients.end(); it++)
    {
        if ((*it).id == id)
        {
            clients.erase(it);
            return true;
        }
    }

    return false;
}

bool existClient(int id)
{
    for (auto &c : clients)
    {
        if (c.id == id)
            return true;
    }
    return false;
}

std::string getAllClients(int FD)
{
    std::string ret = "-----CLIENT LIST-----\nID\tNickname\n";
    for (auto &c : clients)
    {
        ret += std::to_string(c.id) + '\t';
        ret += std::string(c.nickname);
        if (c.id == FD)
        {
            ret += "\t(you)";
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
        send(c.id, content, strlen(content), 0);
    }
}

void sendMsg2ClientsExcept(const char *content, int exceptFD)
{
    for (auto &c : clients)
    {
        if (c.id == exceptFD)
            continue;
        send(c.id, content, strlen(content), 0);
    }
}

void *recvsocket(void *arg) //接收client端socket数据的线程
{
    client_t *client = (client_t *)arg;
    int st = client->id;

    char s[1024];
    while (1)
    {
        memset(s, 0, sizeof(s));
        int rc = recv(st, s, sizeof(s), 0);

        if (rc <= 0)
            break;

        std::string receivedStr = std::string(s);

        if (receivedStr == "-a\n")
        {
            std::string list = getAllClients(st);
            send(st, list.c_str(), list.size(), 0);
            continue;
        }

        if (receivedStr == "-q\n")
        {
            break;
        }

        if (receivedStr.substr(0, 3) == "-nn")
        {
            std::string newName = receivedStr.substr(4, receivedStr.size() - 5);
            sprintf(client->nickname, "%s", newName.c_str());
            printf("Client{%d} change nickname to %s\n", st, newName.c_str());

            std::string reply = "Change nickname success! (" + newName + ")\n";
            send(st, reply.c_str(), reply.size(), 0);
            continue;
        }

        if (receivedStr.substr(0, 2) == "-p")
        {
            int toid;
            char msg[512];
            sscanf(receivedStr.substr(2).c_str(), "%d %s", &toid, msg);

            if (toid == st)
            {
                send(st, "WARNING: CANNOT SEND MESAGE TO YOURSELF\n", 41, 0);
            }
            else if (existClient(toid))
            {
                std::string msg2send;

                msg2send = "***PRIVATE MESSAGE FROM CLIENT{";
                msg2send += std::string(client->nickname) + "}***\n" + std::string(msg);
                msg2send += "\n***PRIVATE MESSAGE***\n";
                send(toid, msg2send.c_str(), msg2send.size(), 0);
            }
            else
            {
                send(st, "WARNING: INVALID ID\n", 21, 0);
            }
            continue;
        }

        char content[1024];
        sprintf(content, "CLINET{%d}: %s", st, s);
        printf("%s", content);
        sendMsg2ClientsExcept(content, st);
    }

    removeClient(st);
    close(st);
    pthread_cancel(pthread_self());
}

void *sendsocket(void *arg) //向client端socket发送数据的线程
{
    char s[1024];
    while (1)
    {
        memset(s, 0, sizeof(s));
        read(STDIN_FILENO, s, sizeof(s));

        std::string content = "SERVER: " + std::string(s);
        sendMsg2AllClients(content.c_str());
    }
}

int main(int arg, char *args[])
{
    int port = 6666;
    if (arg >= 2)
    {
        port = atoi(args[1]);
    }

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

    // 将IP与server程序绑定
    if (bind(st, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("bind failed %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // server端开始listen
    if (listen(st, 20) == -1)
    {
        printf("listen failed %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    pthread_t thread_send;
    pthread_create(&thread_send, NULL, sendsocket, NULL);

    int client_st = 0;
    struct sockaddr_in client_addr;

    while (1)
    {
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t len = sizeof(client_addr);

        client_st = accept(st, (struct sockaddr *)&client_addr, &len);

        if (client_st == -1)
        {
            printf("Client connection failed %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        printf("New Client from: %s\n", inet_ntoa(client_addr.sin_addr));

        client_t *client = (client_t *)malloc(sizeof(client_t));
        client->id = client_st;
        sprintf(client->nickname, "%d", client_st);

        clients.push_back(*client);

        pthread_t thrd1;
        pthread_create(&thrd1, NULL, recvsocket, client);
    }

    close(st);
    return EXIT_SUCCESS;
}
