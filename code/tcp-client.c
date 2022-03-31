#include<stdio.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>

int main(int argc, char **argv){
    // TCP 目标地址（数据结构）
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    // str->int->特殊十六进制数
    addr.sin_port = htons(atoi("8888"));
    // 0.0.0.0:8888
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 创建套接字描述符
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    // 修改套接字
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    // 读取数据或发送数据
    char buffer[] = "Hello";
    write(sock, buffer, sizeof(buffer) - 1 );

    close(sock);
    return 0;
}

// gcc -pthread -o tcp-client tcp-client.c && ./tcp-client