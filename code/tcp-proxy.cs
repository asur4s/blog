using System;  
using System.Net;  
using System.Net.Sockets;  
using System.Threading;  
  
class Program  
{  
    static void ProxyStream(string stream1name, NetworkStream stream1, string stream2name, NetworkStream stream2)  
    {  
        var buffer = new byte[65536];  
  
        try  
        {  
            while (true)  
            {  
				// Stream1 输入，Stream2 输出
                var len = stream1.Read(buffer, 0, 65536);  
                stream2.Write(buffer, 0, len);  
            }  
        }  
        catch (Exception)  
        {  
            Console.WriteLine("Stream from " + stream1name + " to " + stream2name + " closed");  
        }  
    }  
  
    static void Main(string[] args)  
    {  
        if (args.Length != 3)  
        {  
            Console.WriteLine("Usage: program.exe localport remoteServerHost remoteServerPort");  
            Console.WriteLine("Example: program.exe 13389 10.1.2.3 3389");  
            return;  
        }  
  
        var localPort = int.Parse(args[0]);  
        var remoteServerHost = args[1];  
        var remoteServerPort = int.Parse(args[2]);  
		// 创建监听程序
        var l = new TcpListener(IPAddress.Any, localPort);  
        l.Start();  
        while (true)  
        {  
			// 阻塞等待
            var client1 = l.AcceptTcpClient();
			// 输出 Remote 和 Client 信息
            var remoteAddress = (client1.Client.RemoteEndPoint as IPEndPoint).Address.ToString();  
            Console.WriteLine("Accepted session from " + remoteAddress);  
            var client2 = new TcpClient(remoteServerHost, remoteServerPort);  
            Console.WriteLine("Created connection to " + remoteServerHost + ":" + remoteServerPort);  
			// 流量转发（client->remote remote->client）
            new Thread(() => { ProxyStream(remoteAddress, client1.GetStream(), remoteServerHost, client2.GetStream()); }).Start();  
            new Thread(() => { ProxyStream(remoteServerHost, client2.GetStream(), remoteAddress, client1.GetStream()); }).Start();  
        }  
    }  
}  

/*
C:\Windows\Microsoft.NET\Framework64\v3.5\csc.exe SharpDCSync.cs
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe SharpDCSync.cs
*/