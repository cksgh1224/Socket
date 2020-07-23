﻿
// ClientDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// 수신된 데이터를 처리하는 함수
int MyClient::ProcessRecvData(SOCKET ah_socket, unsigned char a_msg_id, char* ap_recv_data, BS a_body_size)
{
	if (a_msg_id == NM_CHAT_DATA)
	{
		mp_parent->AddEventString((wchar_t*)ap_recv_data);
	}

	return 1;
}



// CClientDlg 대화 상자

CClientDlg::CClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent), m_client(this)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_event_list);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SEND_BTN, &CClientDlg::OnBnClickedSendBtn)
	ON_MESSAGE(26001, &CClientDlg::OnConnected)
	ON_MESSAGE(26002, &CClientDlg::OnReadAndClose)
END_MESSAGE_MAP()


// CClientDlg 메시지 처리기

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	
	// 서버에 접속하기 (192.168.77.100 IP를 가진 컴퓨터에 27100 포트번호를 사용하는 서버에 접속)
	m_client.ConnectToServer(L"192.168.77.100", 27100, m_hWnd);


	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




// 리스트 박스 메시지 추가
void CClientDlg::AddEventString(CString parm_string)
{
	int index = m_event_list.InsertString(-1, parm_string); // 리스트 목록 끝에(-1) 문자열(parm_string) 추가. 반환값(index): 추가되는 위치
	m_event_list.SetCurSel(index); // 추가한 곳(index) 커서 활성화
}
void CClientDlg::AddEventString(const wchar_t* ap_string)
{
	int index = m_event_list.InsertString(-1, ap_string);
	m_event_list.SetCurSel(index);
}


// '전송' 버튼 클릭 이벤트
void CClientDlg::OnBnClickedSendBtn()
{
	CString str;
	GetDlgItemText(IDC_CHAT_EDIT, str); // IDC_CHAT_EDIT 에디트 박스에 적혀있는 문자열을 str에 넣는다
	SetDlgItemText(IDC_CHAT_EDIT, L"");

	if (m_client.IsConnected() == 1) // 서버에 접속중
	{
		m_client.SendFrameData(NM_CHAT_DATA, (char*)(const wchar_t*)str, (str.GetLength() + 1) * 2); // 서버에 데이터 전송
	}

}




// FD_CONNECT : 26001 메시지
afx_msg LRESULT CClientDlg::OnConnected(WPARAM wParam, LPARAM lParam)
{
	// 접속에 대한 결과를 알려주는 함수
	if (m_client.ResultOfConnection(lParam) == 1) // 접속 성공
	{
		AddEventString(L"서버에 접속했습니다!");
	}
	else // 접속 실패
	{
		AddEventString(L"서버에 접속할수 없습니다!");
	}
	
	return 0;
}


// FD_READ, FD_CLOSE : 26002 메시지
afx_msg LRESULT CClientDlg::OnReadAndClose(WPARAM wParam, LPARAM lParam)
{
	// 접속한 서버에서 데이터를 전송하거나 접속을 해제할 때 발생하는 메시지를 처리한다
	m_client.ProcessServerEvent(wParam, lParam);

	return 0;
}
