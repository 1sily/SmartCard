#pragma once


// CDlgTvm �Ի���

class CDlgTvm : public CDialog
{
	DECLARE_DYNAMIC(CDlgTvm)

public:
	CDlgTvm(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgTvm();

// �Ի�������
	enum { IDD = IDD_DLG_TVM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedBnInit();
	afx_msg void OnBnClickedBnAnalyse();
	afx_msg void OnBnClickedBnSaleSjt();
	afx_msg void OnBnClickedBnClearSjt();
	afx_msg void OnBnClickedBnSvtInc();
	afx_msg void OnBnClickedBnSvtDec();
	afx_msg void OnBnClickedBnSaleOn();

	afx_msg LRESULT OnPanalChanged(WPARAM, LPARAM);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBnOpen();

	CComboBox m_cmbAnt;

	void SjtSale(WORD wValue);
	void SjtAnalyse();
public:
//	afx_msg void OnBnClickedBnSaleOn2();
public:
	afx_msg void OnBnClickedBnSaleOn2();
public:
	afx_msg void OnBnClickedBnSaleSjt2();
public:
//	afx_msg void OnBnClickedBnSaleOn3();
public:
	afx_msg void OnBnClickedBnSaleOn3();
public:
	afx_msg void OnBnClickedBnSaleOn4();
public:
	afx_msg void OnBnClickedBnSaleOn5();
public:
	afx_msg void OnBnClickedBnSaleOn6();
};
