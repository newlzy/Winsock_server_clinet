#include <stdio.h> //输入和输出
#include <stdlib.h> //动态内存管理，随机数生成，与环境的通信，整数算数，搜索，排序和转换
#include <WinSock2.h> //传输通信
#include <WS2tcpip.h> //用于检索ip地址的新函数和结构
#include <Windows.h>

#define DEFAULT_PORT "27015"  //默认端口
#define DEFAULT_BUFLEN 512 //默认缓冲区

#pragma comment(lib,"Ws2_32.lib") //引入ws2_32.lib库，不然编译报错
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

int main(int argc, char** argv) {
	printf("客户端\n\n");

#pragma region 客户端创建套接字

	//WSADATA结构包含有关Windows Sockets实现的信息。
	WSADATA wsaData;

	int iResult; //结果

	//Winsock进行初始化
	//调用 WSAStartup 函数以启动使用 WS2 _32.dll
	//WSAStartup的 MAKEWORD (2，2) 参数发出对系统上 Winsock 版本2.2 的请求，并将传递的版本设置为调用方可以使用的最高版本的 Windows 套接字支持。
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup 失败：%d\n", iResult);
		return 1;
	}

	//初始化之后实例套接字对象供客户端使用
	//创建套接字
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	// ZeroMemory 函数，将内存块的内容初始化为零
	ZeroMemory(&hints, sizeof(hints));
	//addrinfo在getaddrinfo()调用中使用的结构
	hints.ai_family = AF_UNSPEC; //地址类型，协议族
	hints.ai_socktype = SOCK_STREAM; //套接字类型
	hints.ai_protocol = IPPROTO_TCP; //协议类型

	//解析服务器地址和端口
	//getaddrinfo函数提供从ANSI主机名到地址的独立于协议的转换。
	//参数1：该字符串包含一个主机(节点)名称或一个数字主机地址字符串。
	//参数2：服务名或端口号。
	// 参数3：指向addrinfo结构的指针，该结构提供有关调用方支持的套接字类型的提示。
	//参数4：指向一个或多个包含主机响应信息的addrinfo结构链表的指针。
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo 失败：%d\n", iResult);
		WSACleanup(); //WSACleanup 用于终止 WS2 _ 32 DLL 的使用。
		return 1;
	}

	SOCKET ConnectSocket = INVALID_SOCKET; //创建套接字对象

	//尝试连接到返回的第一个地址。
	//调用getaddrinfo

	//尝试连接到一个地址，直到一个成功
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		//创建用于连接到服务器的SOCKET
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		//检查是否存在错误，以确保套接字为有效套接字。
		if (ConnectSocket == INVALID_SOCKET) {
			//WSAGetLastError返回与上次发生的错误相关联的错误号。
			printf("套接字错误：%d\n", WSAGetLastError());
			//调用 freeaddrinfo 函数以释放由 getaddrinfo 函数为此地址信息分配的内存。
			freeaddrinfo(result);
			WSACleanup(); //用于终止 WS2 _ 32 DLL 的使用。
			return 1;
		}

#pragma endregion

#pragma region 连接到套接字

		//调用 connect 函数，将创建的套接字和 sockaddr 结构作为参数传递。
		//connect函数建立到指定套接字的连接。
		//参数1：标识未连接套接字的描述符。
		//参数2：一个指向要建立连接的sockaddr结构的指针。
		//参数3：参数所指向的sockaddr结构的长度，以字节为单位
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket); //关闭一个已存在的套接字。
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	//应该尝试getaddrinfo返回的下一个地址,如果连接调用失败。但对于这个简单的例子，我们只是释放资源。由getaddrinfo返回并打印一个错误消息
	freeaddrinfo(result); //释放由 getaddrinfo 函数为此地址信息分配的内存。

	if (ConnectSocket == INVALID_SOCKET) {
		printf("无法连接到服务器！！\n");
		WSACleanup();
		return 1;
	}

#pragma endregion

#pragma region 在客户端上发送和接收数据
	//下面的代码演示建立连接后客户端使用的 发送 和 接收 功能。

	int recvbuflen = DEFAULT_BUFLEN; //缓冲区

	const char* sendbuf = "开始测试0000000";
	char recvbuf[DEFAULT_BUFLEN];

	//发送一个初始缓冲区
	//send函数参数1：标识已连接套接字的描述符。
	//参数2：指向包含要传送的数据的缓冲区的指针。
	//参数3：参数buf所指向的缓冲区中数据的长度(以字节为单位)。strlen获取字符串长度
	//参数4：指定调用方式的一组标志。
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("发送失败： %d\n", WSAGetLastError());
		closesocket(ConnectSocket); //关闭套接字
		WSACleanup();
		return 1;
	}

	printf("字节发送：%d\n", iResult);

	//关闭正在发送的连接，因为不再发送数据
	//客户端仍然可以使用ConnectSocket来接收数据
	//shutdown禁用套接字上的发送或接收功能。
	//参数1：套接字描述符
	//参数2：关闭类型描述符。1代表关闭发送操作
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("关闭失败！%d\n", WSAGetLastError());
		closesocket(ConnectSocket); //关闭套接字
		WSACleanup();
		return 1;
	}

	//接收数据，直到服务器关闭连接
	do {
		//recv函数从已连接的套接字或已绑定的无连接套接字接收数据。
		//参数1：套接字描述符
		//参数2：一个指向缓冲区的指针，用来接收传入的数据。
		//参数3：参数buf所指向的缓冲区的长度，以字节为单位。
		//参数4：一组影响此函数行为的标志
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("接收的字节数：%d\n", iResult);
		else if (iResult == 0)
			printf("连接关闭了！");
		else
			printf("接收失败！！%d \n", WSAGetLastError());
	} while (iResult > 0);

#pragma endregion

#pragma region 断开客户端连接

	//两种方法断开客户端连接

	// 1.
	//shutdown禁用套接字上的发送或接收功能。
	//参数1：套接字描述符
	//参数2：关闭类型描述符。1代表关闭发送操作
	//注意：这时客户端应用程序仍可以在套接字上接收数据。
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("关闭失败: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// 2.  用WSACleanup函数来释放资源
	/*closesocket(ConnectSocket);
	WSACleanup();*/

#pragma endregion

	return 0;
}