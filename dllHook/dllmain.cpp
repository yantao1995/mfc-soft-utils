// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <string>
#include <iostream>
#include <winsock.h>
#include "stdafx.h"
#pragma comment(lib,"ws2_32.lib")

using namespace std;

// 模块基址
DWORD dllBaseAddress = (DWORD)GetModuleHandle(L"WeChatWin.dll");

// 接收消息hook位置偏移
DWORD recievedHookOffset = 0x5E2F0E2B - 0x5DF30000;  //0x4112F5;// 0x5C1812F5 - 0x5BD70000;

// 接收消息hook位置的内存地址
DWORD recievedHookAddress = dllBaseAddress + recievedHookOffset;

// 被覆盖的数据的偏移
DWORD coeverDataOffset = 0x5F8F04A0 -0x5DF30000;

// 被覆盖的数据的内存地址
DWORD coeverDataAddress = dllBaseAddress + coeverDataOffset;

// Hook下一个指令的地址
DWORD recievedHookRetAddress = recievedHookAddress + 5;
CHAR originalRecieveCode[5] = { 0 };

//TCP socket ----

SOCKET s_server;
int send_len = 0;
int str_len = 0;
char send_buf[10240];

void DealRecievedMessage(WCHAR* sender, WCHAR* msg)
{
	/*wstring wsSender = sender;
	wstring wsMsg = msg;
	wstring wsShowMsg = L"发送者：" + wsSender + L" 消息： " + wsMsg;
	MessageBox(NULL, wsShowMsg.c_str(), L"收到消息", 0);*/
	if (sender != NULL && msg != NULL && 
		wcslen(sender)>0 && wcslen(msg) > 0)
	{
		wstring wsSender = sender;
		wstring wsMsg = msg;
		wstring wsShowMsg = wsSender + L"|_^*|*^_|" + wsMsg ;
		/*str_len = 0;
		for (size_t i = 0; i < wsShowMsg.size(); i++)
		{
			str_len++;
			send_buf[i] = wsShowMsg[i];
		}
		send_buf[str_len] = '\0';*/
		//
		const wchar_t* src = wsShowMsg.c_str();
		char* des = NULL;
		int len = WideCharToMultiByte(CP_ACP, 0, src, wcslen(src), NULL, 0, NULL, NULL);
		des = (char*)malloc(sizeof(char) * (len + 1));
		WideCharToMultiByte(CP_ACP, 0, src, wcslen(src), des, len, NULL, NULL);
		if (des!= NULL && strlen(des) > 0)
		{
			des[len] = '\0';
			//
			send_len = send(s_server, des, strlen(des) , 0);
			if (send_len < 0) {
				MessageBox(NULL, L"消息传输失败", L"消息提示", 0);
			}
		}
	}
}

__declspec(naked) void ListenRecievedMessage()
{
	__asm
	{
		//保存现场
		pushad;
		pushfd;

		// 发送的消息
		push[ebp - 0x258];
		// 发送的人
		push[ebp - 0x280];


		// 调用处理消息的函数
		call DealRecievedMessage;

		// 平衡堆栈 一个参数占0x4，两个参数就是0x8
		add esp, 0x8;

		//恢复现场
		popfd;
		popad;

		// 重新执行被覆盖的 
		// push WeChatWi.5B4903C0
		push coeverDataAddress;

		// 返回hook的下一条指令
		jmp recievedHookRetAddress
	}
}

void HookRecievedMessage()
{
	// 跳转需要五个字节
	BYTE jmpCode[5] = { 0 };

	// 第一个字节填 E9，对应汇编的jmp
	jmpCode[0] = 0xE9;

	// 后面四个字节，填要跳转的位置，之所以减5，是因为当前的指令占五个字节
	*(DWORD*)&jmpCode[1] = (DWORD)ListenRecievedMessage - recievedHookAddress - 5;

	// 把老的指令读出来存好，方便恢复
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)recievedHookAddress, originalRecieveCode, 5, 0);

	// 把新的执行写到hook的位置
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)recievedHookAddress, jmpCode, 5, 0);
}

void UnHookRecievedMessage()
{
	// 回复被覆盖的指令
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)recievedHookAddress, originalRecieveCode, 5, 0);
}


//socket 
void openSocket() {
	SOCKADDR_IN server_addr;
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		MessageBox(NULL, L"初始化套接字库失败", L"消息提示", 0);
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		MessageBox(NULL, L"套接字库版本号不符", L"消息提示", 0);
		WSACleanup();
	}
	//填充服务端信息
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(38083);
	//创建套接字
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		MessageBox(NULL, L"请先启动消息接收服务器", L"消息提示", 0);
		WSACleanup();
	}
}

void closeSocket()
{
	//关闭套接字
	closesocket(s_server);
	//释放DLL资源
	WSACleanup();
}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			HookRecievedMessage();
			openSocket();
			MessageBox(NULL, L"微信注入", L"注入成功！", NULL);
		}
		break;
		case DLL_PROCESS_DETACH:
		{
			UnHookRecievedMessage();
			closeSocket();
			MessageBox(NULL, L"微信卸载", L"卸载成功！", NULL);
		}
		break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;	
	}
    return TRUE;
}

