#pragma once

#include <map>
#include <vector>

#define WM_PIPE_MESSAGE (WM_USER + 1)
#define PIPE_NAME_SELF L"\\\\.\\pipe\\AppChat2Pipe"
#define PIPE_NAME_TARGET L"\\\\.\\pipe\\AppChat1Pipe"

class CAppChat2Dlg : public CDialogEx
{
public:
    CAppChat2Dlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_APPCHAT_2_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();


protected:
    HICON m_hIcon;

    // Controls
    CListCtrl list_user;
    CListCtrl list_mess;
    CEdit    edit_text;
    CButton  btn_send;

    // Chat state
    CString m_selectedUser;
    std::map<CString, std::vector<CString>> m_chatHistory;

    // Pipe listener
    static UINT PipeServerThread(LPVOID pParam);
    void StartPipeServer();

    // Helpers
    void SendViaWindowsMessage(const CString& message);
    void SendViaNamedPipe(const CString& message);

    // Message handling
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnLvnItemchangedListuser(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedButtonsend();
    afx_msg LRESULT OnPipeMessage(WPARAM wParam, LPARAM lParam);
    afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);

    DECLARE_MESSAGE_MAP()
};
