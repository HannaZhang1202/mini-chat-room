//ChatCli.cpp : Defines the entry point for the application. use VC 6.0
//=========================================================================
//传输控制协议TCP
//说明：  Chat Client客户端程序
//=========================================================================
//VC 6.0   add  WSOCK32.LIB in Project -> Settings... ->  Link
#include <winsock.h>
#include <stdlib.h>
//--------------------------------------------------------------
// Global Variables:
HINSTANCE hInst;				// 实例句柄，方便各程序访问
HWND hWnd;						// 主窗口句柄，方便各程序访问
char Title[]="Chat-Client";		// 窗口标题 The title bar text
int WinX0=250,WinY0=50,WinW=360,WinH=200;	// 窗口位置、大小

LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
//--------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hI,HINSTANCE hP,LPSTR lp,int nC)
{
	MSG msg;
	hInst = hI; // Store instance handle in our global variable
	WNDCLASS wc;

	memset(&wc,0,sizeof(WNDCLASS));

	wc.lpfnWndProc	= (WNDPROC)WndProc;
	wc.hInstance		= hI;
	wc.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wc.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wc.lpszClassName	= "Win32API";
	RegisterClass(&wc);
//特殊窗口1，始终在顶层，任务栏显示。
	hWnd=CreateWindowEx(WS_EX_TOPMOST,"Win32API",Title,
		WS_DLGFRAME|WS_SYSMENU,
//		WinX0,WinY0,WinW,WinH,			// 窗口位置、大小
		CW_USEDEFAULT,CW_USEDEFAULT,WinW,WinH,	// 窗口位置 Windows selects the default position 、大小
		NULL, NULL, hI, NULL);
   	if(!hWnd)   return FALSE;

   	ShowWindow(hWnd, nC);
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
//--------------------------------------------------------------
//按钮
HWND CreateButton(char *Titel,int x0,int y0,int w,int h,int ID,HWND hW,HINSTANCE hInst)
{
	return CreateWindowEx(WS_EX_PALETTEWINDOW,"BUTTON",Titel,
		WS_VISIBLE | WS_CHILD ,
		x0,y0,w,h, hW,(HMENU)ID,hInst,NULL);
}
//---------------------------------------------------------------------------
//单行编辑框
HWND CreateEdit(char *Titel,int x0,int y0,int w,int h,int ID,HWND hW,HINSTANCE hInst)
{
	return CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT",Titel,
		WS_VISIBLE | WS_CHILD |
		    ES_LEFT | ES_MULTILINE | WS_HSCROLL ,
		x0,y0,w,h, hW,
		(HMENU)ID,hInst,NULL);
}
//---------------------------------------------------------------------------
//多行文本框
HWND CreateMemo(char *Titel,int x0,int y0,int w,int h,int ID,HWND hW,HINSTANCE hInst)
{
	return CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT",Titel,
		WS_VISIBLE | WS_CHILD |
		    ES_LEFT | ES_MULTILINE |ES_READONLY|
		    WS_HSCROLL | WS_VSCROLL,
		x0,y0,w,h,
		hW,(HMENU)ID,hInst,NULL);
}
//---------------------------------------------------------------------------
//向多行文本框中添加文本
void Puts(HWND hW,int ID_EDIT,char *str)
{
	int nLen=GetWindowTextLength(GetDlgItem(hW,ID_EDIT));
	SendMessage(GetDlgItem(hW,ID_EDIT),EM_SETSEL,nLen,nLen);
	SendMessage(GetDlgItem(hW,ID_EDIT),EM_REPLACESEL,TRUE,(long)(LPCTSTR)str);
	nLen=GetWindowTextLength(GetDlgItem(hW,ID_EDIT));
	SendMessage(GetDlgItem(hW,ID_EDIT),EM_SETSEL,nLen,nLen);
	SendMessage(GetDlgItem(hW,ID_EDIT),EM_REPLACESEL,TRUE,(long)(LPCTSTR)"\r\n");
}
//---------------------------------------------------------------------------
//按钮
#define BUTTON1 501
#define BUTTON2 502
//---------------------------------------------------------------------------
//单行编辑框
#define EDIT1 601
#define EDIT2 602
#define EDIT3 603
#define EDIT4 604
//---------------------------------------------------------------------------
//多行编辑框
#define MEMO1 701
//---------------------------------------------------------------------------
WSADATA ws;
SOCKET Ss,Cs;
struct sockaddr_in SA,CA;
char aa[200];
char bb[200];
int d;
//---------------------------------------------------------------------------
//消息处理
LRESULT CALLBACK WndProc(HWND hW, UINT msg, WPARAM wP, LPARAM lP)
{
int i;
	switch (msg)
	{
	case WM_DESTROY:	//关闭程序触发事件
		WSAAsyncSelect(Ss, hW, 0, 0);	//向Windows注销Socket触发事件
		closesocket(Cs);				//释放连接
		WSACleanup( );					//卸载动态连接库WinSock DLL
		PostQuitMessage(0);
		break;
	case WM_CREATE:		//程序初始化触发事件
		WSAStartup(0x0101,&ws);								//装载动态连接库WinSock DLL
		CreateButton("Send",10,22,50,20,BUTTON1,hW,hInst);	//发送信息按钮
		CreateButton("Conn",10,2,50,20,BUTTON2,hW,hInst);	//连接Chat Server按钮
		CreateEdit("127.0.0.1",70,2,120,20,EDIT2,hW,hInst);	//Chat Server的IP地址输入框
		CreateEdit("7777",200,2,60,20,EDIT3,hW,hInst);		//Chat Server的Port地址输入框
		CreateEdit("Name",270,2,80,20,EDIT4,hW,hInst);		//用户登录名输入框
		CreateEdit("Hello!",70,22,280,20,EDIT1,hW,hInst);	//发送信息输入框
		CreateMemo("Info.",	2,45,350,120,MEMO1,hW,hInst);	//接受信息显示窗口
		SetFocus(GetDlgItem(hW,BUTTON2));
		EnableWindow(GetDlgItem(hW,BUTTON1),FALSE);
		break;
	case WM_USER+1:				//Socket触发事件
		switch(LOWORD(lP))
		{
		case FD_CONNECT:		//Chat Server的建立连接应答
			//			Puts(hW,MEMO1,"FD_CONNECT->Ok!");
			break;
		case FD_READ:			//收到Chat Server信息
			d=recv(wP,bb,sizeof(bb),0);//wP=Cs	//读接收缓冲区
			bb[d]=0;
			wsprintf(aa,"FD_READ->:%s",bb);
			Puts(hW,MEMO1,bb);
			break;
		case FD_CLOSE:			//Chat Server的释放连接请求
			Puts(hW,MEMO1,"Server Stop!");
			break;
		}
		break;
	case WM_COMMAND:		//控件触发事件
		switch(wP)
		{
		case BUTTON1:		//按下发送信息按钮
			GetDlgItemText(hW,EDIT1,bb,sizeof(bb));
			send(Cs,bb,strlen(bb),0);				//向Chat Server发送信息
			break;
		case BUTTON2:		//按下连接Chat Server按钮
			GetDlgItemText(hW,EDIT2,aa,sizeof(aa));
			GetDlgItemText(hW,EDIT3,bb,sizeof(bb));
			Cs = socket(PF_INET, SOCK_STREAM, 0);	//创建TCP套接字Socket
			SA.sin_family = PF_INET;				//远程Chat Server的属性：地址类型 AF_INET for Internet
			SA.sin_port = htons(atoi(bb));			//远程Chat Server的属性：端口
			SA.sin_addr.s_addr =inet_addr(aa);		//远程Chat Server的属性：IP地址
			d=sizeof(SA);
			i=connect(Cs,(struct sockaddr *)&SA,d);	//连接Chat Server
			if(!i)
			{									//连接Chat Server成功
				EnableWindow(GetDlgItem(hW,BUTTON2),FALSE);
				EnableWindow(GetDlgItem(hW,BUTTON1),TRUE);
				EnableWindow(GetDlgItem(hW,EDIT2),FALSE);
				EnableWindow(GetDlgItem(hW,EDIT3),FALSE);
				EnableWindow(GetDlgItem(hW,EDIT4),FALSE);
				WSAAsyncSelect(Cs,hW,WM_USER+1,FD_CONNECT|FD_READ|FD_CLOSE);//向Windows注册Socket触发事件
			   GetDlgItemText(hW,EDIT4,bb,sizeof(bb));
			   send(Cs,bb,strlen(bb),0);		//连接成功后，向Chat Server发送用户登录名
			}
			else
				Puts(hW,MEMO1,"connect fail!");	//连接Chat Server失败
			Beep(1000,100);
			break;
		}
		break;
	}
	return DefWindowProc(hW,msg,wP,lP);
}
//--------------------------------------------------------------


