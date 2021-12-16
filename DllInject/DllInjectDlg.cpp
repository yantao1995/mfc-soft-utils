 
// DllInjectDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "DllInject.h"
#include "DllInjectDlg.h"
#include "afxdialogex.h"
#include <TlHelp32.h>
#include <process.h>
#include "Psapi.h"
#include "CPROCESS.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <cstring>
#define WM_PROCESS_ID WM_USER + 1;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDllInjectDlg 对话框



CDllInjectDlg::CDllInjectDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLLINJECT_DIALOG, pParent)
	, m_Pid(0)
	, m_DllCS(_T(""))
	, m_edit_module_checked(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDllInjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_DllPath);
	DDX_Text(pDX, IDC_EDIT2, m_Pid);
	DDX_Control(pDX, IDC_LIST1, m_ModuleList);
	DDX_Text(pDX, IDC_EDIT1, m_DllCS);
	DDX_Control(pDX, IDC_EDIT3, m_IDC_EDIT_Process);
	DDX_Text(pDX, IDC_EDIT4, m_edit_module_checked);
}

BEGIN_MESSAGE_MAP(CDllInjectDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CDllInjectDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDllInjectDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON4, &CDllInjectDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CDllInjectDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON3, &CDllInjectDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON6, &CDllInjectDlg::OnBnClickedButton6)
	ON_EN_CHANGE(IDC_EDIT1, &CDllInjectDlg::OnEnChangeEdit1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CDllInjectDlg::OnDblclkList1)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CDllInjectDlg::OnClickList1)
END_MESSAGE_MAP()


// CDllInjectDlg 消息处理程序

BOOL CDllInjectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_ModuleList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT| LVS_EX_SINGLEROW);
	m_ModuleList.InsertColumn(0, L"模块名称",0, 100);
	m_ModuleList.InsertColumn(1, L"模块基址",0, 100);
	m_ModuleList.InsertColumn(2, L"模块大小",0, 100);
	m_ModuleList.InsertColumn(3, L"模块路径",0, 100);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CDllInjectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDllInjectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDllInjectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CDllInjectDlg::OnBnClickedButton1()
{
	CFileDialog fileDlg(true,L"dll", L"*.dll",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,L"dll类型文件(*.dll)||",NULL);
	if (fileDlg.DoModal()==IDOK) 
	{
		m_DllPath.SetWindowTextW(fileDlg.GetPathName());
	}
}


void CDllInjectDlg::OnBnClickedButton2()
{
	CPROCESS m_ProcessDlg;
	m_ProcessDlg.DoModal();
}


BOOL CDllInjectDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
		case WM_USER+1:
		{
			UpdateData(TRUE);
			m_Pid = pMsg->wParam;
			UpdateData(FALSE);
			if (m_Pid)
			{
				InitModuleList();
			}
			break;
		}
		case WM_USER + 2:
		{
			UpdateData(TRUE);
			BSTR b = (BSTR)pMsg->wParam;
			CString s(b);
			SysFreeString(b);
			m_IDC_EDIT_Process.SetWindowText(s);
			UpdateData(FALSE);
			break;
		}
		default:
			break;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CDllInjectDlg::InitModuleList()
{
	UpdateData(TRUE);
	m_ModuleList.DeleteAllItems();
	WCHAR modBaseAddr[260];
	WCHAR modBaseSize[260];
	ULONG uIndex = 0;
	MODULEENTRY32 me32;
	me32.dwSize = sizeof(MODULEENTRY32);
	HANDLE hTool = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_Pid);
	if (hTool!=NULL) 
	{
		if (Module32First(hTool, &me32))
		{
			do
			{
				wsprintf(modBaseAddr, L"%X", me32.modBaseAddr);
				wsprintf(modBaseSize, L"%d字节", me32.modBaseSize);
				m_ModuleList.InsertItem(uIndex,me32.szModule);
				m_ModuleList.SetItemText(uIndex, 1, modBaseAddr);
				m_ModuleList.SetItemText(uIndex, 2, modBaseSize);
				m_ModuleList.SetItemText(uIndex, 3, me32.szExePath);
				uIndex++;
			} while (Module32Next(hTool, &me32));
		}else {
			MessageBoxW(L"受保护的进程,无法查看模块列表", L"打开进程失败");
		}
	}
	wcModuleName = {};
	m_edit_module_checked = "";
	CloseHandle(hTool);
	UpdateData(FALSE);
}


void CDllInjectDlg::OnBnClickedButton4()
{
	UpdateData(TRUE);
	if (m_Pid)
	{
		InitModuleList();
	}else {
		AfxMessageBox(L"请先选择进程");
	}
	UpdateData(FALSE);
}


void CDllInjectDlg::OnBnClickedButton5()
{
	/*UpdateData(TRUE);
	DWORD dwPos = (DWORD)m_ModuleList.GetFirstSelectedItemPosition();
	dwPos--;
	CString ModuleName = m_ModuleList.GetItemText(dwPos, 1);
	wcModuleName = ModuleName.AllocSysString();
	AfxMessageBox(L"选择成功");
	UpdateData(FALSE);*/
}

void CDllInjectDlg::Inject(DWORD dwPid, TCHAR* szPath)
{
	DWORD dwWriteSize = 0;
	HANDLE  hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,dwPid);
	LPVOID pAddress =  VirtualAllocEx(hProcess,NULL,0x100,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
	WriteProcessMemory(hProcess, pAddress, szPath, ((wcslen(szPath)+1)*2),&dwWriteSize);
	HANDLE hRemoteThread = CreateRemoteThread(hProcess,NULL,NULL,(LPTHREAD_START_ROUTINE)LoadLibraryW, pAddress,NULL,NULL);
	WaitForSingleObject(hRemoteThread, -1);
	VirtualFreeEx(hProcess,pAddress,((wcslen(szPath)+1)*2),MEM_RELEASE);
	CloseHandle(hProcess);
	CloseHandle(hRemoteThread);
	AfxMessageBox(L"完成注入");
}

void CDllInjectDlg::UnInject(DWORD dwPid, TCHAR* szDllName)
{
	ULONG uIndex = 0;
	MODULEENTRY32 me32;
	me32.dwSize = sizeof(MODULEENTRY32);
	BOOL bFlag = false;
	HANDLE hTool = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_Pid);
	if (hTool != NULL)
	{
		if (Module32First(hTool, &me32))
		{
			do
			{
				if (bFlag = (0== _tcsicmp(me32.szModule, szDllName) ||
					0 == _tcsicmp(me32.szExePath,szDllName)))
				{
					break;
				}
				uIndex++;
			} while (Module32Next(hTool, &me32));
		}
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
		if (hProcess == NULL) 
		{
			return;
		}
		LPTHREAD_START_ROUTINE lpThreadFunc = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"FreeLibrary");
		if (lpThreadFunc == NULL)
		{
			return;
		}
		HANDLE hThread = CreateRemoteThread(hProcess,NULL,0,lpThreadFunc,me32.modBaseAddr,0,NULL);
		WaitForSingleObject(hThread, -1);
		CloseHandle(hThread);
		CloseHandle(hProcess);
	}
}


void CDllInjectDlg::OnBnClickedButton3()
{
	UpdateData(TRUE);
	if (m_DllCS == "")
	{
		AfxMessageBox(L"请先选择模块");
		return;
	}
	if (m_Pid == 0)
	{
		AfxMessageBox(L"请先选择进程");
		return;
	}
	WCHAR* wcModulePath = m_DllCS.AllocSysString();
	
	Inject(m_Pid, wcModulePath);
	UpdateData(FALSE);
	InitModuleList();
}


void CDllInjectDlg::OnBnClickedButton6()
{
	UpdateData(TRUE);
	if (wcModuleName==NULL||wcslen(wcModuleName) == 0)
	{
		AfxMessageBox(L"请先选择模块");
	}else {
		UnInject(m_Pid, wcModuleName);
	}
	UpdateData(FALSE);
	InitModuleList();
}


void CDllInjectDlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CDllInjectDlg::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	GetModuleFromList();
	*pResult = 0;
}


void CDllInjectDlg::OnClickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	GetModuleFromList();
	*pResult = 0;
}

void CDllInjectDlg::GetModuleFromList()
{
	UpdateData(TRUE);
	DWORD dwPos = (DWORD)m_ModuleList.GetFirstSelectedItemPosition();
	dwPos--;
	CString ModuleName = m_ModuleList.GetItemText(dwPos, 0);
	wcModuleName = ModuleName.AllocSysString();
	m_edit_module_checked = wcModuleName;
	UpdateData(FALSE);
}



BOOL CDllInjectDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此添加专用代码和/或调用基类
	return CDialogEx::PreCreateWindow(cs);
}
