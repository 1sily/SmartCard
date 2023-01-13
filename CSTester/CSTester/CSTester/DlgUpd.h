#pragma once
#include "afxwin.h"
#include <vector>
#include <map>

using namespace std;

#define WM_UPDATE_TRICE		(WM_USER + 200)


// CDlgUpd �Ի���

class CDlgUpd : public CDialog
{
	DECLARE_DYNAMIC(CDlgUpd)

public:
	CDlgUpd(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgUpd();

// �Ի�������
	enum { IDD = IDD_DLG_UPDATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	void SetUpdInfo(int nTimes);

protected:
	CComboBox m_cbUpdType;
	CComboBox m_cbPayType;
	CComboBox m_cbEntryLine;
	CComboBox m_cbEntryStation;

	BYTE m_bUpdateType;
	WORD m_wEntryStation;
	RETINFO m_RetUptate;

	// ���ø�������
	void SetUpdateType(WORD wErr, BYTE& bUpdateType);

	// ����֧����ʽ
	void SetPayType(BYTE bUpdateType);

	// ���÷���
	void SetTradeAmount(BYTE bUpdateType, WORD wEntryStation);

	// ������վվ��������
	void SetEntryStation(BYTE bLineNo);

	// ���۷���
	void Decrease(long lDecVal);

public:
	// ������վ��·������
	void SetEntryLine();

	afx_msg void OnBnClickedBnUpdate();
	afx_msg void OnCbnSelchangeCombEntryStation();
	afx_msg void OnBnClickedBnCancel();
	afx_msg void OnCbnSelchangeCombEntryLine();
};
