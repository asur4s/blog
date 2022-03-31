from socket import *

import socket
import requests
import socks

# socket.set

socks.set_default_proxy(socks.SOCKS5, "127.0.0.1", 2333)
socket.socket = socks.socksocket

url = 'http://www.baidu.com'
html = requests.get(url, timeout=8)
html.encoding = "UTF-8"
print(html.text)