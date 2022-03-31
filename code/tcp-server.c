#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>

int main(int argc, char const **argv)
{
    // TCP 监听地址（数据结构）
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    // str->int->特殊hex
    addr.sin_port = htons(atoi("8888"));
    // 0.0.0.0:8888
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 创建文件描述符（准备通信基础）
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    // 修改文件描述符，使其符合自己的需求
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    listen(sockfd, 10);

    while (1)
    {
        // 用于存储Client的信息
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);

        // 阻塞等待，直到有就绪的连接（会产生新的套接字描述符）
        int new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &len);

        // 输出客户端信息
        char client_ip[16] = "";
        int client_port = ntohs(client_addr.sin_port);
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, 16);
        printf("ip:%s port:%hu\n", client_ip, client_port);

        // 返回信息（可以改成多线程）。
        char buf[128] = "123";
        int ret = recv(new_fd, buf, sizeof(buf), 0);
        send(new_fd, buf, ret, 0);
        
        // 回收套接字描述符
        close(new_fd);
    }
    // 关闭监听套接字
    close(sockfd);
}

// gcc -pthread -o tcp-server tcp-server.c && ./tcp-server
