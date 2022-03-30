# SOCKS 用握手协议通知代理软件进行SOCKS连接，尽可能透明的转发。

# VPN 是正向代理（翻墙）。正向代理需要手动设置代理的 IP 和端口。
# Nginx 是反向代理，对用户没有感知。

# 建立连接的步骤如下
"""
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    一、客户端认证请求
        +----+----------+----------+
        |VER | NMETHODS | METHODS  |
        +----+----------+----------+
        | 1  |    1     |  1~255   |
        +----+----------+----------+
    二、服务端回应认证
        +----+--------+
        |VER | METHOD |
        +----+--------+
        | 1  |   1    |
        +----+--------+
    三、客户端连接请求(连接目的网络)
        +----+-----+-------+------+----------+----------+
        |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
        +----+-----+-------+------+----------+----------+
        | 1  |  1  |   1   |  1   | Variable |    2     |
        +----+-----+-------+------+----------+----------+
    四、服务端回应连接
        +----+-----+-------+------+----------+----------+
        |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
        +----+-----+-------+------+----------+----------+
        | 1  |  1  |   1   |  1   | Variable |    2     |
        +----+-----+-------+------+----------+----------+

*数字代表字节数
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
"""
import socketserver
import socket
from time import ctime
import struct
import select


HOST = ""
PORT = 2333
ADDR = (HOST, PORT)

SOCK_VERSION = 5

class Proxy(socketserver.StreamRequestHandler):
    
    
    # TCPServer 在接收到请求后，会将数据封装到类中，然后调用 handle 方法。
    def handle(self):
        # print("[*] connected: ", self.client_address)
        
        # # 接收数据：处理文件描述符
        # data = self.rfile.readline().strip()
        # print(data)
        
        # # 返回数据：处理远程连接的文件描述符
        # self.wfile.write(bytes('[%s] %s' % (ctime(), data.decode("UTF-8")), "UTF-8"))
        
        """
            一、客户端认证请求
        +----+----------+----------+
        |VER | NMETHODS | METHODS  |
        +----+----------+----------+
        | 1  |    1     |  1~255   |
        +----+----------+----------+
        """
        # 从客户端读取两个字节的数据，并解包
        header = self.connection.recv(2)
        # !指网络字节顺序，B指unsigned char
        VER, NMETHODS = struct.unpack("!BB", header)
        # VER 表示 SOCK 版本
        assert VER == SOCK_VERSION
        # NMETHODS 表示方法个数，然后遍历后面 n 个字节。判断是否为支持的方法。
        # 0x00 不需要认证
        # 0x01 GSSAPI
        # 0x02 用户名、密码认证
        methods = self.getAvaliableMethods(NMETHODS)    
        if 0 not in set(methods):
            self.server.close_request(self.request)
        
        """
        二、服务端回应认证
            +----+--------+
            |VER | METHOD |
            +----+--------+
            | 1  |   1    |
            +----+--------+
        """
        # 发送协商的数据包
        self.connection.sendall(struct.pack("!BB", SOCK_VERSION, 0))
        
        """
        三、客户端连接请求(处理连接请求)
            +----+-----+-------+------+----------+----------+
            |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
            +----+-----+-------+------+----------+----------+
            | 1  |  1  |   1   |  1   | Variable |    2     |
            +----+-----+-------+------+----------+----------+
        """
        version, cmd, _, address_type = struct.unpack("!BBBB", self.connection.recv(4))
        assert version == SOCK_VERSION
        # DST.ADDR 转换
        if address_type == 1:
            # inet_ntoa 将域名转换为 ip 地址。
            address = socket.inet_ntoa(self.connection.recv(4))
        elif address_type == 3:
            domain_len = ord(self.connection.recv(1)[0])
            address = self.connection.recv(domain_len)
        # DST.PORT 转换
        port = struct.unpack("!H", self.connection.recv(2))[0]
        print("[*] target: {} port: {}".format(address, port))
        
        
        """
        四、服务端回应连接
            +----+-----+-------+------+----------+----------+
            |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
            +----+-----+-------+------+----------+----------+
            | 1  |  1  |   1   |  1   | Variable |    2     |
            +----+-----+-------+------+----------+----------+
        """
        try:
            # CMD 是 SOCK 的命令码
            # 0x01 表示
            if cmd == 1:
                # 连接远程
                remote = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                remote.connect((address, port))
                # 获取 sock 名称
                bind_address = remote.getsockname()
            else:
                self.closesock()
            addr = struct.unpack("!I", socket.inet_aton(bind_address[0]))[0]
            port = bind_address[1]
            
            reply = struct.pack("!BBBBIH", SOCK_VERSION, 0, 0, address_type, addr, port)
            print("[*] connected addr: {} port: {}".format(addr, port))
        except Exception as err:
            print(err)
            error_number = 5
            reply = struct.pack("!BBBBIH", SOCK_VERSION, 0, error_number, 0, address_type, 0, 0)
        #  返回服务器 sock 回复信息 
        self.connection.sendall(reply)
        
        flag = False
        if reply[1] == 0 and cmd == 1:
            flag = True
        
        """
        五、交换数据
        
        Client -> current machine -> Remote
        Client <- current machine <- Remote
        """
        while True:
            if flag == False:
                break
            client = self.connection
            rs, ws, es = select.select([client, remote], [], [])
            # 如果 Client 发送数据，则将数据发送给 Remote
            if client in rs:
                data = client.recv(4096)
                print("client data: {}".format(data))
                
                l = remote.send(data)
                if l<=0:
                    break
            # 如果 Remote 返回数据，则将数据发送给 Client
            if remote in rs:
                data = remote.recv(4096)
                l = client.send(data)
                if l<=0:
                    break
        
        self.closesock()
                
        
        
    def closesock(self):
        self.server.close_request(self.request)
        
    def getAvaliableMethods(self, n):
        methods = []
        for i in range(n):
            methods.append(ord(self.connection.recv(1)))        
        return methods
        

if __name__ == "__main__":
    # 
    sockServer = socketserver.TCPServer(ADDR, Proxy)
    print("[*] waiting")   
    sockServer.serve_forever()
