//ChatSer.cpp : Defines the entry point for the application. use VC 6.0
//=========================================================================
//传输层协议－传输控制协议TCP
//说明：  Chat Server服务器端程序
//=========================================================================
//VC 6.0   add  WSOCK32.LIB in Project -> Settings... ->  Link
#include <winsock.h>
#include <stdlib.h>
//--------------------------------------------------------------
// Global Variables:
HINSTANCE hInst;				// 实例句柄，方便各程序访问
HWND hWnd;						// 主窗口句柄，方便各程序访问
char Title[]="Chat-Server";		// 窗口标题 The title bar text
int WinX0=500,WinY0=100,WinW=360,WinH=200;	// 窗口位置、大小

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
		WinX0,WinY0,WinW,WinH,			// 窗口位置、大小
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
//---------------------------------------------------------------------------
//多行编辑框
#define MEMO1 701
//---------------------------------------------------------------------------
#define UN 100		//100个用户登录信息
WSADATA ws;
SOCKET Ss,Cs,Csi[UN];
struct sockaddr_in SA,CA,CAi[11];
char aa[200];
char bb[200];
char User[UN][20];
int d;
int GetUserBySocket(SOCKET s)
{
	int i;
	for(i=0;i<UN;i++)
		if(Csi[i]==s)
			break;
	return i;
}
void Broadcast(SOCKET s,char *str)
{
	char a1[200];
	int i;
	int ui;
	ui = GetUserBySocket(s);
	for(i=0;i<UN;i++)
	{
		if(Csi[i]==-1) continue;
		wsprintf(a1,"%s:%s",User[ui],str);
		send(Csi[i],a1,strlen(a1),0);
	}
}
int NameCmp(char *str)
{
int i;
	for(i=0;i<UN;i++)
 	   if(!strcmp(User[i],str)) return 1;
	return 0;
}
void BroadcastToClients(HWND hW,SOCKET s,char *str)	//向其它客户转发信息或用户登录名注册
{
	int i,j=1;
	for(i=0;i<UN;i++)
		if((Csi[i]==s) && (User[i][0]==0))
		{
			strcpy(aa,str);
			while(NameCmp(aa))				//查找用户登录名
				wsprintf(aa,"%s-%d",str,j++);
			if(j>1)
			{
				wsprintf(bb,"Your Name=%s!",aa);
				send(s,bb,strlen(bb),0);
			}
			strcpy(User[i],aa);
			wsprintf(aa,"Come in!");
			Broadcast(s,aa);				//广播“新用户 Come in!”
			wsprintf(aa,"%s:Come in![%s:%d]",User[i],
			inet_ntoa(CAi[i].sin_addr),ntohs(CAi[i].sin_port));
			Puts(hW,MEMO1,aa);
			return;
		}
	Broadcast(s,str);			//广播客户发来信息
}
void DelUserName(HWND hW,SOCKET s,char *str)	//删除释放连接的用户登录信息
{
	int i;
	int ui;
	ui = GetUserBySocket(s);
	for(i=0;i<UN;i++)
		if(Csi[i]==s)
		{
			Broadcast(s,str);
			wsprintf(aa,"%s:%s[%s:%d]",User[ui],str,
			inet_ntoa(CAi[i].sin_addr),ntohs(CAi[i].sin_port));
			Puts(hW,MEMO1,aa);
			Csi[i]=-1;
			User[i][0]=0;
			return;
		}
}
void AddUser()	//添加建立连接的用户登录信息
{
	int i;
	for(i=0;i<UN;i++)
		if(Csi[i]==-1)
		{
			Csi[i]=Cs;
			CAi[i]=CA;
			User[i][0]=0;
			break;
		}
}
//---------------------------------------------------------------------------
//消息处理
LRESULT CALLBACK WndProc(HWND hW, UINT msg, WPARAM wP, LPARAM lP)
{
int i;
	switch (msg)
	{
	case WM_DESTROY:	//关闭程序触发事件
		WSAAsyncSelect(Ss, hW, 0, 0);	//向Windows注销Socket触发事件
		closesocket(Ss);				//释放连接
		WSACleanup( );					//卸载动态连接库WinSock DLL
		PostQuitMessage(0);
		break;
	case WM_CREATE:		//程序初始化触发事件
		WSAStartup(0x0101,&ws);								//装载动态连接库WinSock DLL
		CreateButton("Send",10,22,50,20,BUTTON1,hW,hInst);	//Chat Server的推送信息按钮
		CreateButton("Start",10,2,50,20,BUTTON2,hW,hInst);	//启动服务按钮
		CreateEdit("192.168.40.106",70,2,180,20,EDIT2,hW,hInst);	//Chat Server的IP地址输入框
		CreateEdit("7777",260,2,90,20,EDIT3,hW,hInst);		//Chat Server的Port端口输入框
		CreateEdit("Hello client!",70,22,280,20,EDIT1,hW,hInst);	//Chat Server的推送信息输入框
		CreateMemo("Info.",	2,45,350,120,MEMO1,hW,hInst);	//客户登录信息显示窗口
		SetFocus(GetDlgItem(hW,BUTTON1));
		EnableWindow(GetDlgItem(hW,BUTTON1),FALSE);
		EnableWindow(GetDlgItem(hW,BUTTON2),TRUE);
		for(i=0;i<UN;i++)	//初始化客户登录信息变量
		{
			Csi[i]=-1;
			User[i][0]=0;
		}
		break;
	case WM_USER+1:				//Socket触发事件
		switch(LOWORD(lP))
		{
		case FD_ACCEPT:			//Chat Client的建立连接请求
			d=sizeof(CA);
			Cs=accept(Ss,(struct sockaddr *) &CA,&d);	//接受Chat Client的连接请求
			AddUser();									//添加建立连接的用户登录信息
			break;
		case FD_READ:			//收到Chat Client信息
			d=recv(wP,bb,sizeof(bb),0);//wP=Cs			//读接收缓冲区

			bb[d]=0;
			BroadcastToClients(hW,wP,bb);				//向其它客户转发信息或用户登录名注册
			break;
		case FD_CLOSE:			//Chat Client的释放连接请求
			wsprintf(bb,"Leave!");
			DelUserName(hW,wP,bb);						//删除释放连接的用户登录信息
			break;
		}
		break;
	case WM_COMMAND:			//控件触发事件
		switch(wP)
		{
		case BUTTON1:			//按下推送信息按钮
			GetDlgItemText(hW,EDIT1,bb,sizeof(bb));
			for(i=0;i<UN;i++)	//向登录的客户推送信息
			{
				if(Csi[i]==-1) continue;
				wsprintf(aa,"Server:%s",bb);
				send(Csi[i],aa,strlen(aa),0);
			}
			break;
		case BUTTON2:			//按下启动服务按钮
			EnableWindow(GetDlgItem(hW,BUTTON2),FALSE);
			EnableWindow(GetDlgItem(hW,BUTTON1),TRUE);
			EnableWindow(GetDlgItem(hW,EDIT2),FALSE);
			EnableWindow(GetDlgItem(hW,EDIT3),FALSE);
			GetDlgItemText(hW,EDIT2,aa,sizeof(aa));
			GetDlgItemText(hW,EDIT3,bb,sizeof(bb));
			Ss=socket(AF_INET, SOCK_STREAM,0);	//创建TCP套接字Socket
			SA.sin_family=AF_INET;				//Chat Server的属性：地址类型 AF_INET for Internet
			SA.sin_port = htons(atoi(bb));		//Chat Server的属性：端口
			SA.sin_addr.s_addr =inet_addr(aa);	//Chat Server的属性：IP地址

			bind(Ss,(struct sockaddr *) &SA,sizeof(SA));				//设置socket属性
			WSAAsyncSelect(Ss,hW,WM_USER+1,FD_ACCEPT|FD_READ|FD_CLOSE);	//向Windows注册Socket触发事件
			listen(Ss,100);												//建立连接缓冲区，100个连接
			Beep(2000,100);
			break;
		}
		break;
	}
	return DefWindowProc(hW,msg,wP,lP);
}
//--------------------------------------------------------------

