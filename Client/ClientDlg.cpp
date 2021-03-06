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
	// 대용량 데이터가 전송 또는 수신될 때, 필요한 기본 코드를 수행
	// message_id 251 : 서버에 대용량의 데이터를 전송할때 사용하는 예약번호
	// message_id 252 : 대용량의 데이터를 수신할 때 사용하는 예약번호 (아직 추가로 수신할 데이터가 있다)
	// message_id 253 : 대용량의 데이터를 수신할 때 사용하는 예약번호 (더이상 전송할 데이터가 없다)
	ClientSocket::ProcessRecvData(ah_socket, a_msg_id, ap_recv_data, a_body_size);
	
	if (a_msg_id == NM_CHAT_DATA) // 수신된 데이터가 채팅 데이터인 경우
	{
		mp_parent->AddEventString((wchar_t*)ap_recv_data); // 수신된 데이터가 유니코드 형태의 문자열로 저장되어 있기 때문에 형변환 필요
	}

	return 1;
}



// CClientDlg 대화 상자

CClientDlg::CClientDlg(UserData a_user, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent), m_client(this) // m_client(this): 객체를 생성할 때 대화상자의 주소를 넘겨준다
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	user_data = a_user;
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
	ON_BN_CLICKED(IDC_CONNECT_BTN, &CClientDlg::OnBnClickedConnectBtn)
	ON_BN_CLICKED(IDC_DISCONNECT_BTN, &CClientDlg::OnBnClickedDisconnectBtn)
	ON_BN_CLICKED(IDC_DELETECHAT_BTN, &CClientDlg::OnBnClickedDeletechatBtn)
END_MESSAGE_MAP()



// 리스트 박스 메시지 추가
void CClientDlg::AddEventString(CString parm_string)
{
	int index = m_event_list.InsertString(-1, parm_string); // 리스트 목록 끝에(-1) 문자열(parm_string) 추가. 반환값(index): 추가되는 위치
	m_event_list.SetCurSel(index);                          // 추가한 곳(index) 커서 활성화
}


// CClientDlg 메시지 처리기

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	
	
	GetDlgItem(IDC_DISCONNECT_BTN)->EnableWindow(FALSE); 
	GetDlgItem(IDC_SEND_BTN)->EnableWindow(FALSE);
	
	SetDlgItemText(IDC_SHOWID_EDIT, user_data.GetID());


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





// 서버 접속에 대한 결과를 알려주는 메시지를 처리한다 
// FD_CONNECT : 26001 메시지
afx_msg LRESULT CClientDlg::OnConnected(WPARAM wParam, LPARAM lParam)
{
	// 서버 접속에 대한 결과
	if (m_client.ResultOfConnection(lParam) == 1) // 접속 성공
	{
		AddEventString(L"서버에 접속했습니다!");

		GetDlgItem(IDC_CONNECT_BTN)->EnableWindow(FALSE);       
		GetDlgItem(IDC_DISCONNECT_BTN)->EnableWindow(TRUE);

		GetDlgItem(IDC_IP_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_PORT_EDIT)->EnableWindow(FALSE);

		GetDlgItem(IDC_SEND_BTN)->EnableWindow(TRUE);
	}
	else // 접속 실패
	{
		AddEventString(L"서버에 접속할수 없습니다!");
	}

	return 0;
}


// 접속한 서버에서 데이터를 전송하거나 접속을 해제할 때 발생하는 메시지를 처리한다
// FD_READ, FD_CLOSE : 26002 메시지
afx_msg LRESULT CClientDlg::OnReadAndClose(WPARAM wParam, LPARAM lParam)
{
	// ProcessServerEvent: FD_READ는 1, FD_CLOSE는 0 반환
	// FD_CLOSE ->  SocketAPI에서 소켓 해제에 관한 처리를 다 해줌
	// FD_READ  ->  ProcessServerEvent - ProcessRecvEvent - ProcessRecvData (ProcessRecvData 함수만 재정의 해주면 된다)
	
	if (m_client.ProcessServerEvent(wParam, lParam) == 0)  // 서버에서 연결을 끊은 경우
	{
		AddEventString(L"서버에서 연결을 해제했습니다.");

		GetDlgItem(IDC_DISCONNECT_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_CONNECT_BTN)->EnableWindow(TRUE);

		GetDlgItem(IDC_IP_EDIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_PORT_EDIT)->EnableWindow(TRUE);

		GetDlgItem(IDC_SEND_BTN)->EnableWindow(FALSE);
	}
	else
	{
		// FD_READ
	}
	
	return 0;
}




// '전송' 버튼 클릭 이벤트
void CClientDlg::OnBnClickedSendBtn()
{
	if (m_client.IsConnected() == 1) // 서버에 접속중
	{
		CString str;
		GetDlgItemText(IDC_CHAT_EDIT, str);
		SetDlgItemText(IDC_CHAT_EDIT, L"");

		m_client.SendFrameData(NM_CHAT_DATA, (char*)(const wchar_t*)str, (str.GetLength() + 1) * 2);
		// 문자열을 전송할 때는 NULL 문자를 포함해서 전송하기 때문에 문자열 길이에 1을 더하고. 유니코드에서 문자 1개는 2byte를 차지하기 때문에 문자열 길이에 2를 곱한다
	}
}



// '서버 접속' 버튼 클릭
void CClientDlg::OnBnClickedConnectBtn()
{
	// 서버와 접속 상태인지 확인하여 서버와 접속중이라면 처리하지 않는다
	if (!m_client.IsConnected()) // 서버에 접속중이 아니라면
	{
		CString ip, port;
		GetDlgItemText(IDC_IP_EDIT, ip); 
		GetDlgItemText(IDC_PORT_EDIT, port);

		if (ip.IsEmpty() || port.IsEmpty())
		{
			MessageBox(L"IP 주소, 포트번호를 입력해 주세요!", NULL, MB_OK);
			return;
		}
		else
		{
			AddEventString(L"서버에 접속을 시도합니다. : " + ip);			
			m_client.ConnectToServer(ip, _ttoi(port), m_hWnd); // _ttoi : cstring -> int


			// 로그인 정보(ID) 서버에게 보내기...
			CString id = user_data.GetID();
			CString pw = user_data.GetPW();
			CString name = user_data.GetName();
			CString str;
			str.Format(L"%s/%s/%s", id, pw, name);
			m_client.SendFrameData(NM_LOGIN_DATA, (char*)(const wchar_t*)str, (str.GetLength() + 1) * 2);
		}
	}
	else
	{
		AddEventString(L"이미 서버에 접속중 입니다.");
	}
}

// '접속 해제' 버튼 클릭 이벤트
void CClientDlg::OnBnClickedDisconnectBtn()
{
	if (m_client.IsConnected()) // 서버에 접속중 이면
	{
		m_client.DisconnectSocket(m_client.GetHandle(), 0);
		AddEventString(L"서버와 연결이 해제되었습니다.");

		GetDlgItem(IDC_DISCONNECT_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_CONNECT_BTN)->EnableWindow(TRUE);

		GetDlgItem(IDC_IP_EDIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_PORT_EDIT)->EnableWindow(TRUE);

		GetDlgItem(IDC_SEND_BTN)->EnableWindow(FALSE);
	}
}


// '지우개' 버튼 클릭 이벤트
void CClientDlg::OnBnClickedDeletechatBtn()
{
	m_event_list.ResetContent();
}
