#include<iostream>
#include<windows.h>
#include<tlhelp32.h>
#define BUFF 99 // Common Buffer Size
using namespace std;

char _p_name[BUFF]; // Name of Process
char _dll_name[BUFF]; // Name of DLL for injection
DWORD pid = NULL;  // PID of process for injection

char *buff = (char*) malloc(BUFF);  // Common Buffer
char appname[BUFF];  // Application name

HWND hWnd;   // Main Window
HWND button; // Child "Button"
HWND edit_proc;   // Child "Edit" for processname
HWND edit_dll;   // Child "Edit" for DLL
HWND label;  // Child "Label"


// Debugging function
void ShowMessage(LPCSTR lpText)
{
  MessageBox(hWnd,lpText,"Message",1);
}

// Injection
bool Inject()
{
	if(pid==NULL) return false;

	// Получение дескриптора процесса
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS,false,pid);
	if(process==NULL) return false;

	// "Вытягивание" функции из системной библиотеки для динамической  
	// подгрузки DLL в адресное пространство открытого процесса
	LPVOID fp = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"),"LoadLibraryA");
	if(fp==NULL) return false;

	// Выделение участка памяти размером strlen(_dll_name) для последующей 
	// записи имени библеотеки в память процесса.
	LPVOID alloc = (LPVOID)VirtualAllocEx(process,0,strlen(_dll_name), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if(alloc==NULL) return false;

	// Запись имени инжектируемой DLL в память
	BOOL w = WriteProcessMemory(process,(LPVOID)alloc,_dll_name,strlen(_dll_name),0);
	if(w==NULL) return false;

	// Создание "удаленного" потока в адресном пространстве
	// открытого процесса и последующая подгрузка нашей DLL
	HANDLE thread = CreateRemoteThread(process,0,0,(LPTHREAD_START_ROUTINE)fp,(LPVOID)alloc,0,0);
	if(thread==NULL) return false;

	CloseHandle(process);
	return true;

}

int StartInjection()
{
	// Searching of Process
	PROCESSENTRY32 pe;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(snapshot==INVALID_HANDLE_VALUE) 
	{
		ShowMessage("SnapShot Failed.");
		return 0;
	}
	pe.dwSize = sizeof(PROCESSENTRY32);
	int curr = Process32First(snapshot,&pe);
	while(curr)
	{		
		CharLowerBuff(pe.szExeFile,lstrlen(pe.szExeFile));
		if(strstr(pe.szExeFile,_p_name)) {pid = pe.th32ProcessID; break;}
	    curr = Process32Next(snapshot,&pe);
	}
	if(pid==NULL) 
	{
		ShowMessage("Searching of process failed.");
		return 0;
	}

	bool result = Inject();
	if(result==false) 
	{
		ShowMessage("Injection failed.");
		return 0;
	}

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE prev, LPTSTR cmdline, int nMode)
{
	
	WNDCLASSEX wcex;

	WNDCLASSEX wcl;
	wcl.cbSize = sizeof(WNDCLASSEX);
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hbrBackground = (HBRUSH)(20);
	wcl.hCursor = LoadCursor(hInstance,IDC_HAND);
	wcl.hIcon = LoadIcon(hInstance,IDI_WINLOGO);
	wcl.hIconSm = LoadIcon(hInstance,IDI_WINLOGO);
	wcl.hInstance = hInstance;
	wcl.lpfnWndProc = WndProc;
	wcl.lpszClassName = "SimpleInjection";
	wcl.lpszMenuName = NULL;
	wcl.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClassEx(&wcl);


	// Getting Application Name
	ZeroMemory(appname,BUFF);
	strncpy(buff,strrchr(GetCommandLine(),'\\')+1,BUFF);
	strncpy(appname,buff,strlen(buff)-2);
	CharLowerBuff(appname,sizeof(appname));

	// Creation of main window
	hWnd = CreateWindow("SimpleInjection", "C++ Injector", WS_OVERLAPPEDWINDOW,
      0, 0,300,200, NULL, NULL, hInstance, NULL);

	// Creation of controls
	edit_proc = CreateWindowEx(NULL,"Edit",appname, WS_CHILD | WS_VISIBLE | WS_BORDER, 0,0,100,20,hWnd,NULL,hInstance,NULL);
	edit_dll = CreateWindowEx(NULL,"Edit","inj.dll", WS_CHILD | WS_VISIBLE | WS_BORDER, 0,20,100,20,hWnd,NULL,hInstance,NULL);
	button = CreateWindowEx(NULL,"Button","Inject!", WS_CHILD | WS_VISIBLE, 0,40,100,20,hWnd,NULL,hInstance,NULL);
	label = CreateWindowEx(NULL,"Static","Simple DLL Injector", WS_CHILD | WS_VISIBLE, 0,140,300,15,hWnd,NULL,hInstance,NULL);

	ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);


	MSG msg;
    while (GetMessage(&msg,NULL,0,0))
	{

		// Button Click
		if(msg.hwnd==button && msg.message==WM_LBUTTONUP) 
		{
			// Getting Process Name
			GetWindowText(edit_proc,buff,sizeof(buff));
			strncpy(_p_name,buff,BUFF);
			// Getting DLL Name
			ZeroMemory(buff,sizeof(buff));
			GetWindowText(edit_dll,buff,BUFF);
			strncpy(_dll_name,buff,BUFF);
			// Start Injection
			StartInjection();
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}
