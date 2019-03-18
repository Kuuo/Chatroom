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
#include <string>

void *recvsocket(void *arg) //接收server端socket数据的线程
{
    int st = *(int *)arg;

    char s[1024];
    while (1)
    {
        memset(s, 0, sizeof(s));
        int rc = recv(st, s, sizeof(s), 0);
        if (rc <= 0) //如果recv返回小于等于0，代表socket已经关闭或者出错了
            break;
        printf("%s", s);
        fflush(stdin);
    }
}

void *sendsocket(void *arg) //向server端socket发送数据的线程
{
    int st = *(int *)arg;

    // char client_username[100];
    // sprintf(client_username, "NEW_CLIENT_USERNAME %s", "Amy");
    // send(st, client_username, strlen(client_username), 0);

    char s[1024];
    while (1)
    {
        memset(s, 0, sizeof(s));
        read(STDIN_FILENO, s, sizeof(s)); //从键盘读取用户输入信息
        send(st, s, strlen(s), 0);
        fflush(stdin);
    }
}

int main(int arg, char *args[])
{
    std::string ip = "0.0.0.0";
    int port = 6666;

    if (arg >= 3)
    {
        ip = std::string(args[1]);
        port = atoi(args[2]);
    }

    int st = socket(AF_INET, SOCK_STREAM, 0); // 初始化socket，

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 调用connect连接到结构addr指定的IP地址和端口号
    if (connect(st, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("connect failed %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    pthread_t thrd1, thrd2;
    pthread_create(&thrd1, NULL, recvsocket, &st);
    pthread_create(&thrd2, NULL, sendsocket, &st);
    pthread_join(thrd1, NULL);

    close(st);
    return EXIT_SUCCESS;
}
