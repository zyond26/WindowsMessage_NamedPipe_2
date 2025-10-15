#include "pch.h"
#include "framework.h"
#include "AppChat_2.h"
#include "AppChat_2Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define WM_PIPE_MESSAGE (WM_USER + 1)
#define PIPE_NAME_SELF L"\\\\.\\pipe\\AppChat2Pipe"
#define PIPE_NAME_TARGET L"\\\\.\\pipe\\AppChat1Pipe"

BEGIN_MESSAGE_MAP(CAppChat2Dlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_user, &CAppChat2Dlg::OnLvnItemchangedListuser)
    ON_BN_CLICKED(IDC_BUTTON_send, &CAppChat2Dlg::OnBnClickedButtonsend)
    ON_WM_COPYDATA()
    ON_MESSAGE(WM_PIPE_MESSAGE, &CAppChat2Dlg::OnPipeMessage)
END_MESSAGE_MAP()

CAppChat2Dlg::CAppChat2Dlg(CWnd* pParent)
    : CDialogEx(IDD_APPCHAT_2_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAppChat2Dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_user, list_user);
    DDX_Control(pDX, IDC_LIST_mess, list_mess);
    DDX_Control(pDX, IDC_EDIT_text, edit_text);
    DDX_Control(pDX, IDC_BUTTON_send, btn_send);
}

BOOL CAppChat2Dlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetWindowText(L"AppChat_2");

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    list_user.InsertColumn(0, L"Name", LVCFMT_LEFT, 150);
    list_user.InsertItem(0, L"Mơ");
    list_user.InsertItem(1, L"Tiến");
    list_user.InsertItem(2, L"Hùng");

    list_mess.InsertColumn(0, L"Messages", LVCFMT_LEFT, 400);

    StartPipeServer();
    return TRUE;
}

void CAppChat2Dlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
        int cx = GetSystemMetrics(SM_CXICON), cy = GetSystemMetrics(SM_CYICON);
        CRect rect; GetClientRect(&rect);
        int x = (rect.Width() - cx) / 2, y = (rect.Height() - cy) / 2;
        dc.DrawIcon(x, y, m_hIcon);
    }
    else CDialogEx::OnPaint();
}

HCURSOR CAppChat2Dlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CAppChat2Dlg::OnLvnItemchangedListuser(NMHDR* pNMHDR, LRESULT* pResult)
{
    int select = list_user.GetNextItem(-1, LVNI_SELECTED);
    if (select != -1)
    {
        m_selectedUser = list_user.GetItemText(select, 0);

        list_mess.DeleteAllItems();
        auto it = m_chatHistory.find(m_selectedUser);
        if (it != m_chatHistory.end()) {
            const std::vector<CString>& messages = it->second;
            for (size_t i = 0; i < messages.size(); ++i) {
                list_mess.InsertItem((int)i, messages[i]);
            }
        }
    }
      *pResult = 0;
}

void CAppChat2Dlg::SendViaWindowsMessage(const CString& message)
{
    HWND hWnd = ::FindWindow(NULL, L"AppChat_1");
    if (!hWnd) {
        AfxMessageBox(L"Không tìm thấy AppChat_1!");
        return;
    }

    CString msg = L"gửi tới [" + m_selectedUser + L"] " + message;
    COPYDATASTRUCT cds{
        1, 
        (msg.GetLength() + 1) * sizeof(TCHAR),
        (PVOID)(LPCTSTR)msg  // con trỏ tới vùng chứa dữ liệu tin nhắn
    };
    ::SendMessage(
        hWnd, 
        WM_COPYDATA,  // mã message chuẩn của wndows để báo app biết đây là dữ liệu copydatastruct
        (WPARAM)m_hWnd, // địa chỉ biết ai gửi - ở đây là appchat2
        (LPARAM)&cds // con trỏ đến copydatastruct chứa dữ liệu cần gửi
    );

    CString display = L"[Tôi → " + m_selectedUser + L"] " + message;
    m_chatHistory[m_selectedUser].push_back(display);
    list_mess.InsertItem(list_mess.GetItemCount(), display);
}

BOOL CAppChat2Dlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) // hàm xử lý khi có tin nhắn đến của WM
{
    CString mess = (LPCTSTR)pCopyDataStruct->lpData;
    CString user = L"AppChat_1";
    int s = mess.Find(L"["), e = mess.Find(L"]");
    if (s != -1 && e != -1) user = mess.Mid(s + 1, e - s - 1);

    CString display = L"[AppChat_1] " + mess;
    m_chatHistory[user].push_back(display);
    if (user == m_selectedUser) list_mess.InsertItem(list_mess.GetItemCount(), display);
    return TRUE;
}

void CAppChat2Dlg::SendViaNamedPipe(const CString& message)
{
    CString fullMsg = L"To[" + m_selectedUser + L"]: " + message;
    HANDLE hPipe = CreateFile(
        PIPE_NAME_TARGET,
        GENERIC_WRITE,
        0, 
        NULL, 
        OPEN_EXISTING, 
        0, 
        NULL
    );
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        AfxMessageBox(L"Không kết nối được tới pipe của AppChat_1!");
        return;
    }  
    DWORD bytesWritten ;
    WriteFile(
        hPipe,
        fullMsg.GetString(),
        (fullMsg.GetLength() + 1) * sizeof(wchar_t),
        &bytesWritten,
        NULL
    );
    CloseHandle(hPipe);

    CString display = L"[Tôi → " + m_selectedUser + L"] " + message;
    m_chatHistory[m_selectedUser].push_back(display);
    list_mess.InsertItem(list_mess.GetItemCount(), display);
  
}

UINT CAppChat2Dlg::PipeServerThread(LPVOID pParam) // Luồng lắng nghe tin nhắn đến
{
    auto* dlg = (CAppChat2Dlg*)pParam; //dlg con trỏ tới cửa sổ dialog chính - cửa sổ chat
    while (true)
    {
        HANDLE hPipe = CreateNamedPipe(
            PIPE_NAME_SELF,            
            PIPE_ACCESS_INBOUND,           
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 
            PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, NULL  
        ); 
        if (hPipe == INVALID_HANDLE_VALUE) return 0;

        BOOL connect_ok = FALSE;  
        if (ConnectNamedPipe(hPipe, NULL) || GetLastError() == ERROR_PIPE_CONNECTED) {
            connect_ok= TRUE;
        }
        if (connect_ok)
        {
            wchar_t buf[4096]; 
            DWORD read;
            if (ReadFile(hPipe, buf, sizeof(buf) - sizeof(wchar_t), &read, NULL) && read > 0)
            {
                buf[read / sizeof(wchar_t)] = 0; 
                CString* pStr = new CString(L"[AppChat_1] " + CString(buf));
                dlg->PostMessage(WM_PIPE_MESSAGE, (WPARAM)pStr, 0); 
            }
        }
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
    return 0;
}


void CAppChat2Dlg::StartPipeServer() { AfxBeginThread(PipeServerThread, this); }


LRESULT CAppChat2Dlg::OnPipeMessage(WPARAM wParam, LPARAM) // Hàm xử lý khi có tin nhắn đến
{
    CString* pStr = (CString*)wParam;
    CString received = *pStr;
    delete pStr;

    CString sender = L"Unknown", receiver = L"Unknown", content;

    int s1 = received.Find(L"[");
    int d1 = received.Find(L"]");
    if (s1 != -1 && d1 != -1 && d1 > s1)
        sender = received.Mid(s1 + 1, d1 - s1 - 1);

    int s2 = received.Find(L"To[");
    int d2 = received.Find(L"]:", s2);
    if (s2 != -1 && d2 != -1 && d2 > s2)
    {
        receiver = received.Mid(s2 + 3, d2 - s2 - 3);
        content = received.Mid(d2 + 2);
    }
    else
    {
        receiver = L"Unknown";
        content = received;
    }

    CString display = L"[" + sender + L" → " + receiver + L"] " + content;

    m_chatHistory[receiver].push_back(display);
    if (receiver == m_selectedUser)
        list_mess.InsertItem(list_mess.GetItemCount(), display);

    return 0;
}


void CAppChat2Dlg::OnBnClickedButtonsend()
{
    CString mess;
    edit_text.GetWindowText(mess);
    if ( m_selectedUser.IsEmpty()) {
        AfxMessageBox(L"Chưa chọn người nhận !");
        return;
    }
    else if (mess.IsEmpty()) {
        AfxMessageBox(L"chưa nhập nội dung cần gửi !");
    }

    //SendViaWindowsMessage(mess);

    SendViaNamedPipe(mess);

    edit_text.SetWindowText(L"");
}