// CPROCESS.cpp: 实现文件
//

#include "pch.h"
#include "DllInject.h"
#include "CPROCESS.h"
#include "afxdialogex.h"
#include <TlHelp32.h>
#include <process.h>
#include "Psapi.h"


// CPROCESS 对话框

IMPLEMENT_DYNAMIC(CPROCESS, CDialogEx)

CPROCESS::CPROCESS(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

CPROCESS::~CPROCESS()
{
}

void CPROCESS::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ProcessList);
}


BEGIN_MESSAGE_MAP(CPROCESS, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON2, &CPROCESS::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CPROCESS::OnBnClickedButton3)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CPROCESS::OnDblclkList1)
END_MESSAGE_MAP()


// CPROCESS 消息处理程序


BOOL CPROCESS::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_ProcessList.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
	m_ProcessList.InsertColumn(0,L"进程名", 0, 160);
	m_ProcessList.InsertColumn(1,L"进程ID", 0, 83);
	InitProcessList();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CPROCESS::InitProcessList()
{
	m_ProcessList.DeleteAllItems();
	HANDLE hTool =  CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
	WCHAR wcth32ProcessID[260];
	ULONG uIndex = 0;
	if (Process32First(hTool, &pe))
	{
		do
		{	
			if (pe.th32ProcessID > 0)
			{
				wsprintf(wcth32ProcessID, L"%d", pe.th32ProcessID);
				m_ProcessList.InsertItem(uIndex, pe.szExeFile);
				m_ProcessList.SetItemText(uIndex, 1, wcth32ProcessID);
				uIndex++;
			}			
		} while (Process32Next(hTool,&pe));
	}
}


void CPROCESS::OnBnClickedButton2()
{
	InitProcessList();
}


void CPROCESS::OnBnClickedButton3()
{
	GetProcessFromList();
}



void CPROCESS::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	GetProcessFromList();
	*pResult = 0;
}

void CPROCESS::GetProcessFromList()
{
	DWORD dwPos = (DWORD)m_ProcessList.GetFirstSelectedItemPosition();
	if (dwPos == 0) 
	{
		AfxMessageBox(L"请选择一个进程");
		return;
	}
	dwPos--;
	CString ProcessID = m_ProcessList.GetItemText(dwPos, 1);
	CString ProcessName = m_ProcessList.GetItemText(dwPos, 0);
	int nProcessID = _ttoi(ProcessID);
	//dll注入器
	CWnd* pWnd = FindWindow(NULL, L"dll注入器");
	HWND hwnd = pWnd->GetSafeHwnd();
	::PostMessage(hwnd, WM_USER + 1, nProcessID, LPARAM(0));
	::PostMessage(hwnd, WM_USER + 2, LPARAM(ProcessName.AllocSysString()), LPARAM(0));
	OnOK();
}
