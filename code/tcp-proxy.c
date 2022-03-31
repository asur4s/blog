/*
author: Asura
References：
- https://github.com/bovine/datapipe
*/

#include <stdio.h>
#include <time.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
// 300 second
#define TIMEOUT 300
#define MAXCLIENTS 20
#define h_addr h_addr_list[0]
#define recv(x, y, z, a) read(x, y, z)
#define send(x, y, z, a) write(x, y, z)
typedef int SOCKET;

struct client_t
{
    int isused;
    SOCKET csock, rsock;
    time_t activity;
};

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s, listenhost listenport remotehost remoteport");
        // 使用 echo $? 可以查看。
        return 30;
    }

    // 创建 Client 并且初始化
    struct client_t client[MAXCLIENTS];
    for (int i = 0; i < MAXCLIENTS; i++)
    {
        client[i].isused = 0;
    }

    // 创建监听socket
    struct sockaddr_in laddr;
    bzero(&laddr, sizeof(laddr));
    laddr.sin_family = AF_INET;
    laddr.sin_addr.s_addr = inet_addr(argv[1]);
    // string -> int -> hex
    laddr.sin_port = htons(atol(argv[2]));
    // 异常处理
    if (!laddr.sin_port)
    {
        fprintf(stderr, "invalid listener port\n");
        return 20;
    }
    if (laddr.sin_addr.s_addr == INADDR_NONE)
    {
        struct hostent *n;
        // 默认为 INADDR_NONE，如果填写的 hostname，那么前面的 inet_add 就无法转换，就需要利用这个来处理。
        if ((n = gethostbyname(argv[1])) == NULL)
        {
            perror("gethostbyname");
            return 20;
        }
        bcopy(n->h_addr, (char *)&laddr.sin_addr, n->h_length);
    }

    // 创建发送 socket
    struct sockaddr_in raddr;
    bzero(&raddr, sizeof(raddr));
    raddr.sin_family = AF_INET;
    raddr.sin_addr.s_addr = inet_addr(argv[3]);
    // string -> int -> hex
    raddr.sin_port = htons(atol(argv[4]));
    // 异常处理
    if (!raddr.sin_port)
    {
        fprintf(stderr, "invalid listener port\n");
        return 25;
    }
    if (raddr.sin_addr.s_addr == INADDR_NONE)
    {
        struct hostent *n;
        // 默认为 INADDR_NONE，如果填写的 hostname，那么前面的 inet_add 就无法转换，就需要利用这个来处理。
        if ((n = gethostbyname(argv[1])) == NULL)
        {
            perror("gethostbyname");
            return 25;
        }
        bcopy(n->h_addr, (char *)&raddr.sin_addr, n->h_length);
    }

    // 监听端口
    SOCKET lsock = socket(AF_INET, SOCK_STREAM, 0);
    if (lsock == -1)
    {
        perror("socket");
        return 20;
    }
    if (bind(lsock, (struct sockaddr *)&laddr, sizeof(laddr)))
    {
        perror("bind");
        return 20;
    }
    if (listen(lsock, 5))
    {
        perror("listen");
        return 20;
    }

    // TODO：否则会重复绑定。
    laddr.sin_port = htons(0);

    while (1)
    {
        // 非阻塞编程
        fd_set set;
        FD_ZERO(&set);

        // socket加入集合
        FD_SET(lsock, &set);
        // Client socket 加入集合
        int maxsock = (int)lsock;
        for (int i = 0; i < MAXCLIENTS; i++)
        {
            if (client[i].isused)
            {
                // 把 Client 的描述符放进去。
                FD_SET(client[i].csock, &set);
                FD_SET(client[i].rsock, &set);
                // 更新最大描述符
                if ((int)client[i].csock > maxsock)
                {
                    maxsock = (int)client[i].csock;
                }
                if ((int)client[i].rsock > maxsock)
                {
                    maxsock = (int)client[i].rsock;
                }
            }
        }

        // 等待并将请求加入 Client。
        struct timeval tv = {1, 0};
        // select筛选处于就绪状态的fd（解决 accept 傻等问题）
        // 自动遍历套接字描述符，将就绪的描述符加入SET中。
        if (select(maxsock + 1, &set, NULL, NULL, &tv) < 0)
        {
            perror("select");
            return 30;
        }
        int i;
        time_t now = time(NULL);
        if (FD_ISSET(lsock, &set))
        {
            struct sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);

            // TODO：找到 Client socket
            SOCKET csock = accept(lsock, (struct sockaddr *)&client_addr, &len);
            // 输出客户端信息
            char client_ip[16] = "";
            int client_port = ntohs(client_addr.sin_port);
            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, 16);
            printf("[+] ip:%s port:%hu\n", client_ip, client_port);

            client_port = ntohs(laddr.sin_port);
            inet_ntop(AF_INET, &laddr.sin_addr.s_addr, client_ip, 16);
            printf("[+] ip:%s port:%hu\n", client_ip, client_port);

            // 找到没有用过的 Client 结构体，存储接受数据和发送数据的fd
            for (i = 0; i < MAXCLIENTS; i++)
                if (!client[i].isused)
                    break;
            if (i < MAXCLIENTS)
            {
                SOCKET rsock = socket(AF_INET, SOCK_STREAM, 0);
                // todo
                if (rsock == -1)
                {
                    perror("socket");
                    close(csock);
                }
                else if (bind(rsock, (struct sockaddr *)&laddr, sizeof(laddr)))
                {
                    perror("bind");
                    close(csock);
                    close(rsock);
                }
                else if (connect(rsock, (struct sockaddr *)&raddr, sizeof(raddr)))
                {
                    perror("connect");
                    close(csock);
                    close(rsock);
                }
                else
                {
                    printf("[+] client id:%d\n", i);
                    client[i].csock = csock;
                    client[i].rsock = rsock;

                    client[i].activity = now;
                    client[i].isused = 1;
                }
            }
            else
            {
                fprintf(stderr, "Too many client\n");
                close(csock);
            }
        }

        // 传输数据
        char buf[4096];
        for (i = 0; i < MAXCLIENTS; i++)
        {
            int rlength, slength, closed = 0;
            // 如果没有使用，跳过
            if (!client[i].isused)
            {
                continue;
            }
            // 如果客户端发送来消息
            else if (FD_ISSET(client[i].csock, &set))
            {
                if ((rlength = recv(client[i].csock, buf, sizeof(buf), 0)) <= 0 ||
                    (slength = send(client[i].rsock, buf, rlength, 0)) <= 0)
                    closed = 1;
                else
                    client[i].activity = now;
                printf("> Client send %d bytes, actually send %d bytes.\n", rlength, slength);
            }
            // 如果服务端发送来消息
            else if (FD_ISSET(client[i].rsock, &set))
            {
                if ((rlength = recv(client[i].rsock, buf, sizeof(buf), 0)) <= 0 ||
                    (slength = send(client[i].csock, buf, rlength, 0)) <= 0)
                    closed = 1;
                else
                    client[i].activity = now;
                printf("< Server send %d bytes, actually send %d bytes.\n", rlength, slength);
            }
            else if (now - client[i].activity > TIMEOUT)
            {
                closed = 1;
            }

            if (closed)
            {
                close(client[i].csock);
                close(client[i].rsock);
                client[i].isused = 0;
            }
        }
    }
    printf("Hello world");
}

// gcc -o tunnels tunnels.c && ./tunnels 127.0.0.1 8888 192.168.1.1 8888