// TabletInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TabletInfo.h"
#include "TabletInfoDlg.h"
#define _WIN32_DCOM
#include <comdef.h>
#include <atlbase.h>
#include <rpcsal.h>
// headers needed to use Mobile Broadband APIs 
#pragma comment(lib, "mbnapi_uuid.lib")
#include "mbnapi.h"
#pragma comment(lib, "IPHLPAPI.lib")
#include <iphlpapi.h>


#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTabletInfoDlg dialog

CComPtr<IMbnInterfaceManager>  g_InterfaceMgr = NULL;



CTabletInfoDlg::CTabletInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTabletInfoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTabletInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTabletInfoDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_QUERY, &CTabletInfoDlg::OnBnClickedQuery)
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CTabletInfoDlg message handlers

BOOL CTabletInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CoCreateInstance(CLSID_MbnInterfaceManager,NULL,CLSCTX_ALL,IID_IMbnInterfaceManager,(void**)&g_InterfaceMgr);
	for (int i=0;i<7;i++)
	{
		m_Bitmap[i].LoadBitmapA(IDB_SIGNAL0+i);
	}
	m_pSignal = (CStatic*)GetDlgItem(IDC_SIGNAL);
	BITMAP bm;
	m_Bitmap[0].GetBitmap(&bm);
	CRect rc;
	GetWindowRect(&rc);
	m_pSignal->SetWindowPos(NULL,rc.Width()-GetSystemMetrics(SM_CXFRAME)-bm.bmWidth,rc.Height()-GetSystemMetrics(SM_CYFRAME)-GetSystemMetrics(SM_CYCAPTION)-bm.bmHeight,bm.bmWidth,bm.bmHeight,0);
	SetTimer(1,1000,NULL);
	//m_pSignal->SetBitmap((HBITMAP)m_Bitmap[0].Detach());
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTabletInfoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTabletInfoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTabletInfoDlg::OnBnClickedQuery()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_QUERY)->EnableWindow(0);
	m_hQueryThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)QueryThread,this,0,NULL);
}

int CTabletInfoDlg::GetAddress(void)
{
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
	BOOL bWIFI = FALSE,bBT=FALSE;

    unsigned int i;

    /* variables used for GetIfTable and GetIfEntry */
    MIB_IFTABLE *pIfTable;
    MIB_IFROW *pIfRow;

    // Allocate memory for our pointers.
    pIfTable = (MIB_IFTABLE *) MALLOC(sizeof (MIB_IFTABLE));
    if (pIfTable == NULL) {
		//MessageBox("Error allocating memory needed to call GetIfTable","Error",MB_ICONERROR);
        return FALSE;
    }
    // Make an initial call to GetIfTable to get the
    // necessary size into dwSize
    dwSize = sizeof (MIB_IFTABLE);
    if (GetIfTable(pIfTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
        FREE(pIfTable);
        pIfTable = (MIB_IFTABLE *) MALLOC(dwSize);
        if (pIfTable == NULL) {
			//MessageBox("Error allocating memory needed to call GetIfTable","Error",MB_ICONERROR); 
            return -1;
        }
    }

    // Make a second call to GetIfTable to get the actual
    // data we want.
    if ((dwRetVal = GetIfTable(pIfTable, &dwSize, FALSE)) == NO_ERROR) 
	{
        for (i = 0; i < pIfTable->dwNumEntries; i++) {
            pIfRow = (MIB_IFROW *) & pIfTable->table[i];
            switch (pIfRow->dwType) {
            case IF_TYPE_ETHERNET_CSMACD:
				{
					if (strstr((char*)pIfRow->bDescr,"Bluetooth") && !bBT)
					{
						sprintf(m_szBT,"%02X-%02X-%02X-%02X-%02X-%02X",pIfRow->bPhysAddr[0],
							pIfRow->bPhysAddr[1],
							pIfRow->bPhysAddr[2],
							pIfRow->bPhysAddr[3],
							pIfRow->bPhysAddr[4],
							pIfRow->bPhysAddr[5]);
						bBT = TRUE;
					}
				}
                break;
            case IF_TYPE_IEEE80211:
				{
					if (strstr((char*)pIfRow->bDescr,"802.11") && !bWIFI)
					{
						sprintf(m_szWIFI,"%02X-%02X-%02X-%02X-%02X-%02X",pIfRow->bPhysAddr[0],
							pIfRow->bPhysAddr[1],
							pIfRow->bPhysAddr[2],
							pIfRow->bPhysAddr[3],
							pIfRow->bPhysAddr[4],
							pIfRow->bPhysAddr[5]);
						bWIFI = TRUE;
					}
				}
                break;
            default:
                //printf("Unknown type %ld\n", pIfRow->dwType);
                break;
            }
        }
    }

    if (pIfTable != NULL)
	{
        FREE(pIfTable);
        pIfTable = NULL;
    }

	return 0;
}

int CTabletInfoDlg::GetIMEI(void)
{
	CComPtr<IMbnInterface> pMbnInterface = NULL;
	MBN_INTERFACE_CAPS InterfaceCaps;
	LONG lBound=0;
	HRESULT hr = -1;
	SAFEARRAY *psa = NULL;
	if (g_InterfaceMgr)
	{
		hr = g_InterfaceMgr->GetInterfaces(&psa);
		if (FAILED(hr)) goto END;
		hr = SafeArrayGetElement(psa, &lBound, &pMbnInterface);
		if (FAILED(hr)) goto END;
	}
	else
	{
		goto END;
	}

	hr = pMbnInterface->GetInterfaceCapability(&InterfaceCaps);
	if (FAILED(hr)) goto END;

	wchar_t* pBuf = InterfaceCaps.deviceID;
	wcstombs(m_szIMEI,pBuf,wcslen(pBuf));
    SysFreeString(InterfaceCaps.customDataClass);
    SysFreeString(InterfaceCaps.customBandClass);
    SysFreeString(InterfaceCaps.deviceID);
    SysFreeString(InterfaceCaps.manufacturer);
    SysFreeString(InterfaceCaps.model);
    SysFreeString(InterfaceCaps.firmwareInfo);

    if(psa)
    {
        SafeArrayDestroy(psa);
    }
	pMbnInterface = NULL;
	return 0;
END:
    if(psa)
    {
        SafeArrayDestroy(psa);
    }
	pMbnInterface = NULL;
	return -1;
}

int CTabletInfoDlg::GetProductKeyInfo(void)
{
	BOOL retval;
	PROCESS_INFORMATION pi={0};
	STARTUPINFO si={0};
	SECURITY_ATTRIBUTES sa={0};
	HANDLE hReadPipe,hWritePipe;
	DWORD dwLen,dwRead;

	sa.bInheritHandle=TRUE;
	sa.nLength=sizeof SECURITY_ATTRIBUTES;
	sa.lpSecurityDescriptor=NULL;
	retval=CreatePipe(&hReadPipe,&hWritePipe,&sa,0);
	si.cb=sizeof STARTUPINFO;
	si.wShowWindow=SW_HIDE;
	si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.hStdOutput=si.hStdError=hWritePipe;
	retval=CreateProcess(NULL,"powershell.exe (get-wmiobject softwarelicensingservice).OA3xOriginalProductKey",&sa,&sa,TRUE,0,NULL,NULL,&si,&pi);
	if(retval)
	{
		WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		dwLen=GetFileSize(hReadPipe,NULL);
		retval=ReadFile(hReadPipe,m_szKeyInfo,dwLen-2,&dwRead,NULL);
	}
	else
	{
		goto end;;
	}

	retval=CreateProcess(NULL,"powershell.exe (get-wmiobject win32_operatingsystem).serialnumber",&sa,&sa,TRUE,0,NULL,NULL,&si,&pi);
	if(retval)
	{
		WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		dwLen=GetFileSize(hReadPipe,NULL);
		retval=ReadFile(hReadPipe,m_szKeyID,dwLen-2,&dwRead,NULL);
		CString str(m_szKeyID);
		str.Remove('-');
		str.Delete(2,2);
		str.Delete(str.GetLength()-5,5);
		memset(m_szKeyID,0,sizeof(m_szKeyID));
		strcpy(m_szKeyID,(LPCSTR)str);
		str.ReleaseBuffer();
	}
	else
	{
		goto end;;
	}

	CloseHandle(hWritePipe);
	CloseHandle(hReadPipe);
	return 0;
end:
	CloseHandle(hWritePipe);
	CloseHandle(hReadPipe);
	return -1;
}

UINT CTabletInfoDlg::QueryThread(LPVOID lParam)
{
	CTabletInfoDlg* p = (CTabletInfoDlg*)lParam;
	memset(p->m_szBT,0,sizeof(p->m_szBT));
	memset(p->m_szWIFI,0,sizeof(p->m_szWIFI));
	memset(p->m_szIMEI,0,sizeof(p->m_szIMEI));
	memset(p->m_szKeyInfo,0,sizeof(p->m_szKeyInfo));
	memset(p->m_szKeyID,0,sizeof(p->m_szKeyID));
	p->GetAddress();
	p->GetIMEI();
	p->GetProductKeyInfo();
	p->SetDlgItemText(IDC_KEYID,p->m_szKeyID);
	p->SetDlgItemText(IDC_KEYINFO,p->m_szKeyInfo);
	p->SetDlgItemText(IDC_IMEI,p->m_szIMEI);
	p->SetDlgItemText(IDC_WIFI,p->m_szWIFI);
	p->SetDlgItemText(IDC_BT,p->m_szBT);
	CFile fp;
	BOOL ret = fp.Open("tablet.txt",CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate);
	if (!ret)
	{
		p->GetDlgItem(IDC_QUERY)->EnableWindow();
		return -1;
	}
	int len=(int)fp.GetLength();
	if (len == 0)
	{
		fp.Write("ID\t\tKEY\t\t\t\tWIFI\t\t\tBT\t\t\tIMEI\r\n",(UINT)strlen("ID\t\t\tKEY\t\t\t\tWIFI\t\t\tBT\t\t\tIMEI"));
	}
	fp.SeekToEnd();
	char buff[1024]={0};
	strcat(buff,p->m_szKeyID);
	strcat(buff,"\t");
	if (strlen(p->m_szKeyInfo))
	{
		strcat(buff,p->m_szKeyInfo);
	}
	else
	{
		strcat(buff,"00000-00000-00000-00000-00000");
	}
	strcat(buff,"\t");
	if (strlen(p->m_szWIFI))
	{
		strcat(buff,p->m_szWIFI);
	}
	else
	{
		strcat(buff,"00-00-00-00-00-00");
	}
	strcat(buff,"\t");
	if (strlen(p->m_szBT)) 
	{
		strcat(buff,p->m_szBT);
	}
	else 
	{
		strcat(buff,"00-00-00-00-00-00");
	}
	strcat(buff,"\t");
	if (strlen(p->m_szIMEI))
	{
		strcat(buff,p->m_szIMEI);
	}
	else
	{
		strcat(buff,"None");
	}
	//strcat(buff,"\r\n");
	fp.Write(buff,(UINT)strlen(buff));
	p->GetDlgItem(IDC_QUERY)->EnableWindow();
	return 0;
}

void CTabletInfoDlg::OnDestroy()
{
	KillTimer(1);
	if (WAIT_OBJECT_0 != WaitForSingleObject(m_hQueryThread,3000))
	{
		TerminateThread(m_hQueryThread,-2);
		CloseHandle(m_hQueryThread);
	}

	for (int i=0;i<7;i++)
	{
		m_Bitmap[i].DeleteObject();
	}
	g_InterfaceMgr = NULL;
	CoUninitialize();
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
}

void CTabletInfoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == 1)
	{
		KillTimer(1);
		CComPtr<IMbnInterface> pMbnInterface = NULL;
		CComPtr<IMbnSignal> pMbnSignal = NULL;
		MBN_INTERFACE_CAPS InterfaceCaps;
		SAFEARRAY *psa = NULL;
		LONG lBound=0;
		ULONG lStrength=0;
		CString signal;
		ULONG lBand=0,lBandClass;
		HRESULT hr = -1;
		if (g_InterfaceMgr)
		{
			hr = g_InterfaceMgr->GetInterfaces(&psa);
			if (FAILED(hr)) goto NO_3G;
			hr = SafeArrayGetElement(psa, &lBound, &pMbnInterface);
			if (FAILED(hr)) goto NO_3G;
		}
		else
		{
			goto NO_3G;
		}

		hr = pMbnInterface->QueryInterface(IID_IMbnSignal,(LPVOID*)&pMbnSignal);
		if (FAILED(hr)) goto NO_SIM;
		hr = pMbnSignal->GetSignalStrength(&lStrength);
		if (FAILED(hr)) goto NO_SIM;
		signal.Format("%ddBm",lStrength);
		SetDlgItemText(IDC_STRENGTH,signal);
		if (lStrength == 0 || lStrength == 99)
		{
			SetDlgItemText(IDC_STRENGTH,NULL);
			m_pSignal->SetBitmap((HBITMAP)m_Bitmap[0].m_hObject);
		}
		else if (lStrength < 6)
		{
			m_pSignal->SetBitmap((HBITMAP)m_Bitmap[1].m_hObject);
		}
		else if (lStrength < 12)
		{
			m_pSignal->SetBitmap((HBITMAP)m_Bitmap[2].m_hObject);
		}
		else if (lStrength < 18)
		{
			m_pSignal->SetBitmap((HBITMAP)m_Bitmap[3].m_hObject);
		}
		else if (lStrength < 24)
		{
			m_pSignal->SetBitmap((HBITMAP)m_Bitmap[4].m_hObject);
		}
		else
		{
			m_pSignal->SetBitmap((HBITMAP)m_Bitmap[5].m_hObject);
		}

		hr = pMbnInterface->GetInterfaceCapability(&InterfaceCaps);
		if (FAILED(hr)) goto END;

		lBandClass=InterfaceCaps.gsmBandClass;
		lBand = 0;
		while (lBandClass)
		{
			lBand++;
			lBandClass >>= 1;
		}
		if (lBand < 21)
		{
			signal.Format("Band:%d",lBand-1);
		}
		else
		{
			signal.Format("Band:--");
		}
		SetDlgItemText(IDC_BAND,signal);

		SysFreeString(InterfaceCaps.customDataClass);
		SysFreeString(InterfaceCaps.customBandClass);
		SysFreeString(InterfaceCaps.deviceID);
		SysFreeString(InterfaceCaps.manufacturer);
		SysFreeString(InterfaceCaps.model);
		SysFreeString(InterfaceCaps.firmwareInfo);

		goto END;
NO_3G:
		SetDlgItemText(IDC_STRENGTH,NULL);
		m_pSignal->SetBitmap((HBITMAP)m_Bitmap[6].m_hObject);
		goto END;
NO_SIM:
		SetDlgItemText(IDC_STRENGTH,NULL);
		m_pSignal->SetBitmap((HBITMAP)m_Bitmap[0].m_hObject);
END:
		if(psa)
		{
			SafeArrayDestroy(psa);
		}
		pMbnInterface = NULL;
		pMbnSignal = NULL;
		SetTimer(1,1000,NULL);
	}
	CDialog::OnTimer(nIDEvent);
}
