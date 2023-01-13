// CSTesterDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CSTester.h"
#include "CSTesterDlg.h"
#include "GSFile.h"
#include "DlgHarder.h"
#include "TradeFileConvert.h"
#include "DlgConfirm.h"
#include "DlgNfs.h"
#include "DlgRoot.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

LARGE_INTEGER g_liCpuFreq;
LARGE_INTEGER g_liStart;
LARGE_INTEGER g_liEnd;
Serial g_Serial;
int g_nPortOpen;
WORD g_wDeviceID;
WORD g_wStation;
long g_lBalance;
BYTE g_bDeviceType;
BYTE g_nBeep;
map<BYTE, LINEINF> g_PrmStation;

BOOL CCSTesterDlg::m_bEnableDebuger = FALSE;


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CCSTesterDlg �Ի���

CCSTesterDlg::CCSTesterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCSTesterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	LoadErrorString("TicketInfoDes.ini");
}

void CCSTesterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REDIT_OUT, m_rEditOut);
}

void CCSTesterDlg::ChildWindowAnchor()
{
	RECT mainRect, pageRect, editRect, statusRect, operRect;
	DWORD rebarSize = 0;

	GetClientRect(&mainRect);
	
	rebarSize = m_wndToolBar.GetToolBarCtrl().GetButtonSize();
	rebarSize = HIWORD(rebarSize) + 1;
	m_wndReBar.SetWindowPos(NULL, mainRect.left, 0, mainRect.right, rebarSize, SWP_SHOWWINDOW);


	m_WndStatus.GetClientRect(&statusRect);
	statusRect.top = mainRect.bottom - (statusRect.bottom - statusRect.top);
	statusRect.bottom = mainRect.bottom;
	statusRect.left = mainRect.left;
	statusRect.right = mainRect.right;
	m_WndStatus.MoveWindow(&statusRect);

	pageRect.left = mainRect.left;
	pageRect.right = 165;
	pageRect.top = rebarSize;
	pageRect.bottom = statusRect.top;
	m_pBom->MoveWindow(&pageRect);
	m_pAgm->MoveWindow(&pageRect);
	m_pTvm->MoveWindow(&pageRect);
	m_pTcm->MoveWindow(&pageRect);
	
	operRect = mainRect;
	operRect.bottom = statusRect.top;
	operRect.left = pageRect.right;
	operRect.top = operRect.bottom - 0x40;
	m_pOper->MoveWindow(&operRect);
	m_pDec->MoveWindow(&operRect);
	m_pInc->MoveWindow(&operRect);
	m_pSjtSale->MoveWindow(&operRect);
	m_pRefund->MoveWindow(&operRect);
	m_pUpdate->MoveWindow(&operRect);
	m_pParam->MoveWindow(&operRect);
	m_pPrice->MoveWindow(&operRect);
	m_pSamAct->MoveWindow(&operRect);
	m_pDescind->MoveWindow(&operRect);
	m_pDebug->MoveWindow(&operRect);
	m_pDegrade->MoveWindow(&operRect);
	m_pSaleEx->MoveWindow(&operRect);
	
	editRect = mainRect;
	editRect.top = pageRect.top;
	editRect.bottom = operRect.top;
	editRect.left = pageRect.right;
	m_rEditOut.MoveWindow(&editRect);

}

BOOL CCSTesterDlg::AddBars()
{
	CRect rectToolBar;
	CRect rectAddress;
	REBARBANDINFO rbbi;

	AddStatusBar();

	// ���������ʼ��
	if (!m_wndReBar.Create(this))
	{
		TRACE0("Failed to create rebar\n");
		return FALSE;
	}
	if (!AddToolBar(rectToolBar))
	{
		TRACE0("Failed to create toolbar\n");
		return FALSE;
	}

	if (!m_wndDlgBar.Create(IDD_DIALOGBAR, this))
	{
			TRACE0("Failed to create dialogbar\n");
			return -1;      // fail to create
	}

	m_wndReBar.AddBar(&m_wndDlgBar, NULL, NULL, RBBS_FIXEDBMP | RBBS_BREAK);
	m_wndReBar.AddBar(&m_wndToolBar,NULL, NULL, RBBS_GRIPPERALWAYS | RBBS_FIXEDBMP | RBBS_USECHEVRON);

	// set up min/max sizes and ideal sizes for pieces of the rebar


	rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE;
	m_wndDlgBar.GetWindowRect(&rectAddress);
	rbbi.cyMinChild = rectAddress.Height();
	rbbi.cxMinChild = 80;
	rbbi.cxIdeal = 200;
	m_wndReBar.GetReBarCtrl().SetBandInfo(0, &rbbi);

	rbbi.cbSize = sizeof(rbbi);
	rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_SIZE;
	rbbi.cxMinChild = rectToolBar.Width();
	rbbi.cyMinChild = rectToolBar.Height();
	rbbi.cx = rbbi.cxIdeal = rectToolBar.Width() * 9;
	m_wndReBar.GetReBarCtrl().SetBandInfo(1, &rbbi);
	rbbi.cxMinChild = 0;

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);


	return TRUE;
}

BOOL CCSTesterDlg::AddToolBar(CRect& rcToolBar)
{
	CImageList img;
	CBitmap bmp;
	const int nNumBtn = 6;
	TBBUTTON tb[nNumBtn];
	UINT bmpID[] = {IDB_BMP_BOM, IDB_BMP_AGM, IDB_BMP_TVM, IDB_BMP_TCM/*, IDB_BMP_AVM*/, IDB_BMP_ABOUT};

	if (!m_wndToolBar.CreateEx(this))
	{
		TRACE0("Failed to create toolbar\n");
		return FALSE;
	}

	CToolBarCtrl& tbCtrl = m_wndToolBar.GetToolBarCtrl();

	tbCtrl.SetButtonWidth(70, 150);
	tbCtrl.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

	img.Create(32, 32, ILC_COLOR32 | ILC_MASK, 1, 1);

	for (size_t i=0;i<sizeof(bmpID)/sizeof(bmpID[0]);i++)
	{
		if (bmp.LoadBitmap(bmpID[i]) == NULL)
			return FALSE;

		img.Add(&bmp, RGB(255, 0, 0));
		bmp.Detach();
	}

	m_wndToolBar.GetToolBarCtrl().SetImageList(&img);
	img.Detach();

	m_wndToolBar.ModifyStyle(0, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT);

	AddTbBtn(tb[0], 0, IDM_DEV_BOM, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECKGROUP, "bom");
	AddTbBtn(tb[1], 1, IDM_DEV_AGM, TBSTATE_ENABLED                  , TBSTYLE_CHECKGROUP, "agm");
	AddTbBtn(tb[2], 2, IDM_DEV_TVM, TBSTATE_ENABLED                  , TBSTYLE_CHECKGROUP, "tvm");
	AddTbBtn(tb[3], 3, IDM_DEV_TCM, TBSTATE_ENABLED                  , TBSTYLE_CHECKGROUP, "tcm");
	//AddTbBtn(tb[4], 4, IDM_DEV_AVM, TBSTATE_ENABLED                  , TBSTYLE_CHECKGROUP, "avm");
	AddTbBtn(tb[4], 0, 0          , TBSTATE_ENABLED                  , TBSTYLE_SEP       , NULL);
	AddTbBtn(tb[5], 4, 0x10005    , TBSTATE_ENABLED                  , TBSTYLE_BUTTON    , "����");
	
	if (!tbCtrl.AddButtons(nNumBtn, tb))
		return FALSE;
	// set up toolbar button sizes
	m_wndToolBar.GetItemRect(0, &rcToolBar);
	m_wndToolBar.SetSizes(CSize(80, 55), CSize(32, 32));

	return TRUE;
}

void CCSTesterDlg::AddTbBtn(TBBUTTON& tb, int iBitmap, int iCmd, BYTE fsState, BYTE fsStyle, LPCTSTR pText)
{
	tb.iBitmap = iBitmap; 
	tb.idCommand = iCmd; 
	tb.fsState = fsState; 
	tb.fsStyle = fsStyle; 
	tb.dwData = 0; 
	tb.iString = (INT_PTR)pText; 
}

BOOL CCSTesterDlg::AddStatusBar()
{
	if (!m_WndStatus.Create(WS_CHILD | WS_VISIBLE | SBT_OWNERDRAW, CRect(0,0,0,0), this, 0))
	{
		TRACE0("Failed to create statusbar\n");
		return FALSE;
	}
	int strPartDim[3] = {200, 400, -1}; //�ָ�����
	m_WndStatus.SetParts(3, strPartDim);

	//����״̬���ı�
	m_WndStatus.SetText("����", 0, 0);
	m_WndStatus.SetText("����", 1, 0);
	m_WndStatus.SetText("����", 2, 0);

	return TRUE;
}

void CCSTesterDlg::AddBitmap(CImageList& imgList, UINT uID)
{
	CBitmap bmp;
	if (bmp.LoadBitmap(uID))
	{
		imgList.Add(&bmp, RGB(255, 255, 255));
	}
}

void CCSTesterDlg::DispResult(CString& strOut, CString strOper, RETINFO Error, LARGE_INTEGER liStart, LARGE_INTEGER liEnd)
{
	CString strOutAll;
	FormHeader(strOutAll, strOper);
	strOutAll += strOut;
	AppendText(strOut);
	SetStatusBar(strOper, Error, liStart, liEnd);
}

void CCSTesterDlg::DispResult(_Struct_Type StructType, LPVOID lpStruct, WORD lenStruct, CString strOper, LARGE_INTEGER liStart, LARGE_INTEGER liEnd)
{
	char szTemp1[64] = {0};
	char szTemp2[64] = {0};
	char szTemp3[64] = {0};
	char szTemp4[128] = {0};
	char szTemp5[64] = {0};
	CString strOut, strTemp;

	if (lenStruct > 0)
	{
		switch (StructType)
		{
		case struct_init_arr:
			{
				uint8_t * p_sam_status = (uint8_t *)lpStruct;
				strTemp.Format("%s%s%02X\n%s%s%02X\n%s%s%02X\n%s%s%02X\n%s%s%02X\n%s%s%02X\n%s%s%02X\n%s%s%02X\n",
					"����[1]sam��״̬", SPACE_MIDDLE, p_sam_status[0],
					"����[2]sam��״̬", SPACE_MIDDLE, p_sam_status[1],
					"����[3]sam��״̬", SPACE_MIDDLE, p_sam_status[2],
					"����[4]sam��״̬", SPACE_MIDDLE, p_sam_status[3],
					"����[5]sam��״̬", SPACE_MIDDLE, p_sam_status[4],
					"����[6]sam��״̬", SPACE_MIDDLE, p_sam_status[5],
					"����[7]sam��״̬", SPACE_MIDDLE, p_sam_status[6],
					"����[8]sam��״̬", SPACE_MIDDLE, p_sam_status[7]);
				strOut += strTemp;
			}
			break;
		case struct_Login:
			{
				POPERLOGIN pStruct = (POPERLOGIN)lpStruct;
				strTemp.Format("%s%s%s\n%s%s%d",
					"����Ա��", SPACE_MIDDLE, pStruct->szOperID,
					"BOM��κ�", SPACE_MIDDLE, pStruct->nBomShiftID);
				strOut += strTemp;

				break;
			}
		case struct_ES_Analyse:
			{
				PESANALYZE pStruct = (PESANALYZE)lpStruct;
				memcpy(szTemp1, pStruct->cPhysicalID, sizeof(pStruct->cPhysicalID));
				memcpy(szTemp2, pStruct->cLogicalID, sizeof(pStruct->cLogicalID));
				memcpy(szTemp3, pStruct->cIssueDate, sizeof(pStruct->cIssueDate));
				memcpy(szTemp4, pStruct->cTicketType, sizeof(pStruct->cTicketType));
				//BCDTimetoString(pStruct->cIssueDate, 7, szTemp3);
				//BCDTimetoString(pStruct->bExpiry, 4, szTemp4);

				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%d\n%s%s%02X\n%s%s%02X\n",
					"������",		SPACE_MIDDLE, szTemp1, 
					"�߼�����",		SPACE_MIDDLE, szTemp2, 
					"�ƿ�ʱ��",		SPACE_MIDDLE, szTemp3,
					"Ʊ������",		SPACE_MIDDLE, szTemp4,
					"Ʊ�����",		SPACE_MIDDLE, pStruct->lBalance, 
					"Ѻ��",			SPACE_MIDDLE, pStruct->lDeposite, 
					"����״̬",		SPACE_MIDDLE, pStruct->bIssueStatus, 
					"״̬",			SPACE_MIDDLE, pStruct->bStatus);
				strOut += strTemp;

				/*
				BCDTimetoString(pStruct->bStartDate, 7, szTemp1);
				BCDTimetoString(pStruct->bEndDate, 7, szTemp2);
				sprintf(szTemp3, "%02x%02x/%02x/%x%02x", 
					pStruct->bLastStationID[0], pStruct->bLastStationID[1], 
					pStruct->bLastDeviceType,
					pStruct->bLastDeviceID[0], pStruct->bLastDeviceID[1]);
				BCDTimetoString(pStruct->dtLastDate, 7, szTemp4);
				BCDTimetoString(pStruct->dtEntryDate, 7, szTemp5);*/

				memcpy(szTemp1, pStruct->cExpire, sizeof(pStruct->cExpire));
				memcpy(szTemp2, pStruct->RFU, sizeof(pStruct->RFU));
				//memcpy(szTemp3, pStruct->cLine, sizeof(pStruct->cLine));
				//memcpy(szTemp4, pStruct->cStationNo, sizeof(pStruct->cStationNo));
				memcpy(szTemp3, pStruct->cDateStart, sizeof(pStruct->cDateStart));
				memcpy(szTemp4, pStruct->cDateEnd, sizeof(pStruct->cDateEnd));
				BCDTimetoString(pStruct->dtDaliyActive, 7, szTemp5);


				strTemp.Format("%s%s%02X\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n",
					"Ʊ����������",	SPACE_MIDDLE, pStruct->bCharacter, 
					"������Ч��",	SPACE_MIDDLE, szTemp1, 
					"E/S SAM�߼�����",			SPACE_MIDDLE, szTemp2,
					"�߼���Ч�ڿ�ʼ",	SPACE_MIDDLE, szTemp3,
					"�߼���Ч�ڽ���",	SPACE_MIDDLE, szTemp4, 
					"����Ʊ����ʱ��",		SPACE_MIDDLE, szTemp5);

				strOut += strTemp;

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cardfactory, 4);

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->certificate_code, 32);
				//memcpy(szTemp3, pStruct->cCertificateCode, sizeof(pStruct->cCertificateCode));

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->certificate_name, sizeof(pStruct->certificate_name));


				strTemp.Format("%s%s%02X\n%s%s%02X\n%s%s%02X\n%s%s%s\n%s%s%s\n%s%s%02X\n%s%s%s\n",
					"���˿�",		SPACE_MIDDLE, pStruct->certificate_iscompany, 
					"����ͨ��",		SPACE_MIDDLE, pStruct->certificate_ismetro, 
					"֤������",		SPACE_MIDDLE, pStruct->certificate_type, 
					"֤������",		SPACE_MIDDLE, szTemp3, 
					"֤������",		SPACE_MIDDLE, szTemp4, 
					"�Ա�",	SPACE_MIDDLE, pStruct->certificate_sex, 
					"���̴���",	SPACE_MIDDLE, szTemp2);
				strOut += strTemp;

				break;
			}
		case struct_Analyse:
			{
				PBOMANALYZE pStruct = (PBOMANALYZE)lpStruct;
				memcpy(szTemp1, pStruct->cPhysicalID, sizeof(pStruct->cPhysicalID));
				memcpy(szTemp2, pStruct->cLogicalID, sizeof(pStruct->cLogicalID));
				BCDTimetoString(pStruct->bIssueData, 4, szTemp3);
				BCDTimetoString(pStruct->bExpiry, 4, szTemp4);
		
				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%02X%02X\n%s%s%d\n%s%s%d\n%s%s%d\n%s%s%d\n%s%s%s\n%s%s%s\n",
					"������",		SPACE_MIDDLE, szTemp1, 
					"�߼�����",		SPACE_MIDDLE, szTemp2, 
					"Ʊ������",		SPACE_MIDDLE, pStruct->bTicketType[0], pStruct->bTicketType[1], 
					"Ʊ�����",		SPACE_MIDDLE, pStruct->lBalance, 
					"Ѻ��",			SPACE_MIDDLE, pStruct->lDepositorCost, 
					"Ǯ������",		SPACE_MIDDLE, pStruct->lLimitedBalance, 
					"���·���",		SPACE_MIDDLE, pStruct->lPenalty, 
					"��������",		SPACE_MIDDLE, szTemp3, 
					"������Ч��",	SPACE_MIDDLE, szTemp4);
				strOut += strTemp;

				BCDTimetoString(pStruct->bStartDate, 7, szTemp1);
				BCDTimetoString(pStruct->bEndDate, 7, szTemp2);
				sprintf(szTemp3, "%02x%02x/%02x/%x%02x", 
					pStruct->bLastStationID[0], pStruct->bLastStationID[1], 
					pStruct->bLastDeviceType,
					pStruct->bLastDeviceID[0], pStruct->bLastDeviceID[1]);
				BCDTimetoString(pStruct->dtLastDate, 7, szTemp4);
				BCDTimetoString(pStruct->dtEntryDate, 7, szTemp5);

				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%d\n%s%s%s\n%s%s%s\n%s%s%02x%02x\n%s%s%s\n%s%s%d\n%s%s%02x%02x\n%s%s%02x%02x\n",
					"Ʊ����������",	SPACE_MIDDLE, szTemp1, 
					"Ʊ��ͣ������",	SPACE_MIDDLE, szTemp2, 
					"״̬",			SPACE_MIDDLE, pStruct->bStatus,
					"�ϴν����豸",	SPACE_MIDDLE, szTemp3,
					"�ϴν���ʱ��",	SPACE_MIDDLE, szTemp4, 
					"��վվ��",		SPACE_MIDDLE, pStruct->bEntrySationID[0], pStruct->bEntrySationID[1],
					"��վʱ��",		SPACE_MIDDLE, szTemp5,
					"����ģʽ",		SPACE_MIDDLE, pStruct->bLimitMode,
					"��վվ������",	SPACE_MIDDLE, pStruct->bLimitEntryID[0], pStruct->bLimitEntryID[1],
					"��վվ������",	SPACE_MIDDLE, pStruct->bLimitExitID[0], pStruct->bLimitExitID[1]);
				strOut += strTemp;

				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cCityCode, sizeof(pStruct->cCityCode));

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cSellerCode, sizeof(pStruct->cSellerCode));

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->cCertificateCode, 20);
				//memcpy(szTemp3, pStruct->cCertificateCode, sizeof(pStruct->cCertificateCode));

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cCertificateName, sizeof(pStruct->cCertificateName));

				memset(szTemp5, 0, sizeof(szTemp5));
				BCDTimetoString(pStruct->bCertExpire, 4, szTemp5);

				strTemp.Format("%s%s%d\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%d\n%s%s%x\n%s%s%s\n%s%s%s\n%s%s%c\n%s%s%d\n%s%s%d\n%s%s%d\n%s%s%c\n%s%s%s\n",
					"����״̬",		SPACE_MIDDLE, pStruct->bActiveStatus, 
					"���д���",		SPACE_MIDDLE, szTemp1, 
					"�����̴���",	SPACE_MIDDLE, szTemp2, 
					"��ֵ����",		SPACE_MIDDLE, pStruct->lChargeCount, 
					"���Ѽ���",		SPACE_MIDDLE, pStruct->lTradeCount, 
					"֤������",		SPACE_MIDDLE, pStruct->bCertificateType, 
					"֤������",		SPACE_MIDDLE, szTemp3, 
					"֤������",		SPACE_MIDDLE, szTemp4, 
					"Ӧ�ñ�ʶ",		SPACE_MIDDLE, pStruct->cTestFlag,
					"��ֵ����",		SPACE_MIDDLE, pStruct->lChargeUpper,
					"����Ѻ��",		SPACE_MIDDLE, pStruct->lDepositeSale,
					"������",		SPACE_MIDDLE, pStruct->lBrokerage,
					"Ӧ��ģʽ",		SPACE_MIDDLE, pStruct->cTkAppMode,
					"֤����Ч��",	SPACE_MIDDLE, szTemp5);
				strOut += strTemp;

				break;
			}

		case struct_Sam:
			{
				PSAMSTATUS pStruct = (PSAMSTATUS)lpStruct;
				for (int i=0;i<8;i++)
				{
					sprintf(szTemp2, "����%dSAM��״̬", i + 1);
					sprintf(szTemp3, "����%dSAM����", i + 1);

					memcpy(szTemp1, pStruct[i].cSAMID, sizeof(pStruct[i].cSAMID));
					strTemp.Format("%s%s%02x\n%s%s%s\n",
						szTemp2,		SPACE_MIDDLE, pStruct[i].bSAMStatus,
						szTemp3,		SPACE_MIDDLE, szTemp1);
						strOut += strTemp;
				}
				break;
			}

		case struct_Purse:
			{
				PPURSETRADE pStruct = (PPURSETRADE)lpStruct;

				sprintf(szTemp1, "%C%C%C%C/%C%C/%C%C%C", 
					pStruct->cStationID[0], pStruct->cStationID[1], pStruct->cStationID[2],pStruct->cStationID[3],
					pStruct->cDevType[0], pStruct->cDevType[1],
					pStruct->cDeviceID[0], pStruct->cDeviceID[1], pStruct->cDeviceID[2]);
				memcpy(szTemp2, pStruct->cSAMID, sizeof(pStruct->cSAMID));
				BCDTimetoString(pStruct->dtDate, sizeof(pStruct->dtDate), szTemp3);
				memcpy(szTemp4, pStruct->cLogicalID, sizeof(pStruct->cLogicalID));
				memcpy(szTemp5, pStruct->cPhysicalID, sizeof(pStruct->cPhysicalID));

				strTemp.Format("%s%s%C%C\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%s\n%s%s%02x%02x\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%d\n%s%s%d\n%s%s%d\n%s%s%d\n%s%s%C%C\n",
					"��������",	SPACE_MIDDLE, pStruct->cTradeType[0], pStruct->cTradeType[1], 
					"�豸��Ϣ",	SPACE_MIDDLE, szTemp1, 
					"SAM����",	SPACE_MIDDLE, szTemp2, 
					"SAM����ˮ",SPACE_MIDDLE, pStruct->lSAMTrSeqNo, 
					"����ʱ��",	SPACE_MIDDLE, szTemp3, 
					"Ʊ������",	SPACE_MIDDLE, pStruct->bTicketType[0], pStruct->bTicketType[1],
					"�߼�����",	SPACE_MIDDLE, szTemp4, 
					"������",	SPACE_MIDDLE, szTemp5,
					"����״̬",	SPACE_MIDDLE, pStruct->bStatus, 
					"���׽��",	SPACE_MIDDLE, pStruct->lTradeAmount, 
					"���",		SPACE_MIDDLE, pStruct->lBalance, 
					"��ֵ�ۼ�",	SPACE_MIDDLE, pStruct->lChargeCount, 
					"�����ۼ�",	SPACE_MIDDLE, pStruct->lTradeCount, 
					"֧������",	SPACE_MIDDLE, pStruct->cPaymentType[0], pStruct->cPaymentType[1]);
				strOut += strTemp;

				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cMACorTAC, sizeof(pStruct->cMACorTAC));
				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cEntrySAMID, sizeof(pStruct->cEntrySAMID));
				BCDTimetoString(pStruct->dtEntryDate, sizeof(pStruct->dtEntryDate), szTemp3);
				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cOperatorID, sizeof(pStruct->cOperatorID));

				strTemp.Format("%s%s%C%C%C%C\n%s%s%s\n%s%s%02x%02x\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%d\n",
					"֧��ƾ֤",		SPACE_MIDDLE, pStruct->cReceiptID[0], pStruct->cReceiptID[1], pStruct->cReceiptID[2], pStruct->cReceiptID[3], 
					"������֤��",	SPACE_MIDDLE, szTemp1, 
					"��վվ��",		SPACE_MIDDLE, pStruct->bEntryStationID[0], pStruct->bEntryStationID[1], 
					"��վSAM����",	SPACE_MIDDLE, szTemp2, 
					"��վʱ��",		SPACE_MIDDLE, szTemp3, 
					"����Ա��",		SPACE_MIDDLE, szTemp4, 
					"BOM��κ�",	SPACE_MIDDLE, pStruct->bBOMShiftID);
				strOut += strTemp;

				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cSamLast, sizeof(pStruct->cSamLast));
				BCDTimetoString(pStruct->dtLast, sizeof(pStruct->dtLast), szTemp2);
				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%d\n%s%s%c\n",
					"�ϴν���SAM����",	SPACE_MIDDLE, szTemp1, 
					"�ϴν���ʱ��",		SPACE_MIDDLE, szTemp2, 
					"����ֵ",			SPACE_MIDDLE, pStruct->lTradeWallet, 
					"��Ӧ�ñ�ʶ",		SPACE_MIDDLE, pStruct->cTestFlag);
				strOut += strTemp;

				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cCityCode, 4);

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cIndustryCode, 4);

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->cClassicType, 2);

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cSamPosId, sizeof(pStruct->cSamPosId));

				strTemp.Format("%s%s%C%C%C\n%s%s%02x%02x\n%s%s%02x%02x\n%s%s%c\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%C\n",
					"����ģʽ",		SPACE_MIDDLE, pStruct->cLimitMode[0], pStruct->cLimitMode[1], pStruct->cLimitMode[2],
					"���ƽ�վվ��", SPACE_MIDDLE, pStruct->bLimitEntryID[0], pStruct->bLimitEntryID[1],
					"���Ƴ�վվ��", SPACE_MIDDLE, pStruct->bLimitExitID[0], pStruct->bLimitExitID[1],
					"��վ����ģʽ", SPACE_MIDDLE, pStruct->cExitMode,
					"���д���",		SPACE_MIDDLE, szTemp1,
					"��ҵ����",		SPACE_MIDDLE, szTemp2,
					"TAC���׷���",	SPACE_MIDDLE, szTemp3,
					"SAM���ն˺�",	SPACE_MIDDLE, szTemp4,
					"��Ӧ��ģʽ",	SPACE_MIDDLE, pStruct->cTkAppMode);
				strOut += strTemp;
				
				//������ͨ�����ؽṹ
				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n",
					"��¼�汾",		SPACE_MIDDLE, pStruct->cRecordVer,
					"������������", SPACE_MIDDLE, pStruct->cIssueCardCode,
					"��Կ�汾��",   SPACE_MIDDLE, pStruct->cKeyVer,
					"��Կ����",     SPACE_MIDDLE, pStruct->cKeyIndex,
					"α�����",		SPACE_MIDDLE, pStruct->cRandom,
					"�㷨��ʶ",		SPACE_MIDDLE, pStruct->cAlgFlag,
					"�ֿ�������",	SPACE_MIDDLE, pStruct->cCardHolderName,
					"֤������",	    SPACE_MIDDLE, pStruct->cCertificateType,
					"֤������",	    SPACE_MIDDLE, pStruct->cCertificateCode,
				    "ԭ��Ʊ������",	SPACE_MIDDLE, pStruct->cOriginalCardType,
					"������",	    SPACE_MIDDLE, pStruct->cCardLocation);
				strOut += strTemp;

				break;
			}

		case struct_SvtSale:
			{
				POTHERSALE pStruct = (POTHERSALE)lpStruct;

				sprintf(szTemp1, "%C%C%C%C/%C%C/%C%C%C", 
					pStruct->cStationID[0], pStruct->cStationID[1], pStruct->cStationID[2],pStruct->cStationID[3],
					pStruct->cDevType[0], pStruct->cDevType[1],
					pStruct->cDeviceID[0], pStruct->cDeviceID[1], pStruct->cDeviceID[2]);
				memcpy(szTemp2, pStruct->cLogicalID, sizeof(pStruct->cLogicalID));
				memcpy(szTemp3, pStruct->cPhysicalID, sizeof(pStruct->cPhysicalID));
				strTemp.Format("%s%s%C%C\n%s%s%s\n%s%s%02x%02x\n%s%s%s\n%s%s%s\n%s%s%d\n",
					"��������",	SPACE_MIDDLE, pStruct->cTradeType[0], pStruct->cTradeType[1], 
					"�豸��Ϣ",	SPACE_MIDDLE, szTemp1, 
					"Ʊ������",	SPACE_MIDDLE, pStruct->bTicketType[0], pStruct->bTicketType[1], 
					"�߼�����",	SPACE_MIDDLE, szTemp2, 
					"������",	SPACE_MIDDLE, szTemp3, 
					"����״̬",	SPACE_MIDDLE, pStruct->bStatus);
				strOut += strTemp;

				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cBusinessseqno, sizeof(pStruct->cBusinessseqno));
				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cSAMID, sizeof(pStruct->cSAMID));
				BCDTimetoString(pStruct->dtDate, sizeof(pStruct->dtDate), szTemp3);
				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cOperatorID, sizeof(pStruct->cOperatorID));
				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%d\n%s%s%d\n%s%s%d\n%s%s%s\n%s%s%C%C%C%C\n%s%s%s\n%s%s%d\n%s%s%d\n%s%s%c\n",
					"�ѻ�ҵ����ˮ",	SPACE_MIDDLE, szTemp1, 
					"SAM����",		SPACE_MIDDLE, szTemp2, 
					"SAM����ˮ",	SPACE_MIDDLE, pStruct->lSAMTrSeqNo, 
					"�������",		SPACE_MIDDLE, pStruct->bAmountType, 
					"Ѻ����",		SPACE_MIDDLE, pStruct->nAmount, 
					"����ʱ��",		SPACE_MIDDLE, szTemp3, 
					"֧��ƾ֤",		SPACE_MIDDLE, pStruct->cReceiptID[0], pStruct->cReceiptID[1], pStruct->cReceiptID[2], pStruct->cReceiptID[3], 
					"����Ա��",		SPACE_MIDDLE, szTemp4, 
					"BOM��κ�",	SPACE_MIDDLE, pStruct->bBOMShfitID,
					"�ۿ�����",		SPACE_MIDDLE, pStruct->lBrokerage,
					"Ӧ�ñ�ʶ",		SPACE_MIDDLE, pStruct->cTestFlag);
				strOut += strTemp;
				break;
			}

		case struct_SjtSale:
			{
				PSJTSALE pStruct = (PSJTSALE)lpStruct;

				sprintf(szTemp1, "%C%C%C%C/%C%C/%C%C%C", 
					pStruct->cStationID[0], pStruct->cStationID[1], pStruct->cStationID[2],pStruct->cStationID[3],
					pStruct->cDevType[0], pStruct->cDevType[1],
					pStruct->cDeviceID[0], pStruct->cDeviceID[1], pStruct->cDeviceID[2]);
				memcpy(szTemp2, pStruct->cSAMID, sizeof(pStruct->cSAMID));
				BCDTimetoString(pStruct->dtDate, sizeof(pStruct->dtDate), szTemp3);
				memcpy(szTemp4, pStruct->cPaymentTKLogicalID, sizeof(pStruct->cPaymentTKLogicalID));

				strTemp.Format("%s%s%C%C\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%s\n%s%s%d\n%s%s%s\n%s%s%d\n",
					"��������",			SPACE_MIDDLE, pStruct->cTradeType[0], pStruct->cTradeType[1], 
					"�豸��Ϣ",			SPACE_MIDDLE, szTemp1, 
					"SAM����",			SPACE_MIDDLE, szTemp2,
					"SAM����ˮ",		SPACE_MIDDLE, pStruct->lSAMTrSeqNo, 
					"����ʱ��",			SPACE_MIDDLE, szTemp3, 
					"֧����ʽ",			SPACE_MIDDLE, pStruct->bPaymentMeans, 
					"֧�����߼�����",	SPACE_MIDDLE, szTemp4, 
					"�ۿ��ۼ�",			SPACE_MIDDLE, pStruct->lTradeCount);
				strOut += strTemp;

				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cLogicalID, sizeof(pStruct->cLogicalID));

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cPhysicalID, sizeof(pStruct->cPhysicalID));

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->cMACorTAC, sizeof(pStruct->cMACorTAC));

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cOperatorID, sizeof(pStruct->cOperatorID));
				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%d\n%s%s%d\n%s%s%02x%02x\n%s%s%d\n%s%s%s\n%s%s%d\n%s%s%d\n%s%s%s\n%s%s%d\n%s%s%d\n%s%s%c\n",
					"�߼�����",		SPACE_MIDDLE, szTemp1, 
					"������",		SPACE_MIDDLE, szTemp2, 
					"����״̬",		SPACE_MIDDLE, pStruct->bStatus, 
					"���۽��",		SPACE_MIDDLE, pStruct->nChargeValue,
					"Ʊ������",		SPACE_MIDDLE, pStruct->bTicketType[0], pStruct->bTicketType[1], 
					"���α���",		SPACE_MIDDLE, pStruct->bZoneID, 
					"������֤��",	SPACE_MIDDLE, szTemp3, 
					"Ѻ���ʶ",		SPACE_MIDDLE, pStruct->bDepositorCost,
					"Ѻ��",			SPACE_MIDDLE, pStruct->nAmountCost, 
					"����Ա��",		SPACE_MIDDLE, szTemp4, 
					"BOM��κ�",	SPACE_MIDDLE, pStruct->bBOMShiftID,
					"�ۿ�����",		SPACE_MIDDLE, pStruct->lBrokerage,
					"Ӧ�ñ�ʶ",		SPACE_MIDDLE, pStruct->cTestFlag);
				strOut += strTemp;

				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cClassicType, sizeof(pStruct->cClassicType));

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cSamPosId, sizeof(pStruct->cSamPosId));

				strTemp.Format("%s%s%s\n%s%s%s\n",
					"TAC���׷���",	SPACE_MIDDLE, szTemp1,
					"SAM���ն˺�",	SPACE_MIDDLE, szTemp2);
				strOut += strTemp;

				break;
			}

		case struct_Deffer:
			{
				PTICKETDEFER pStruct = (PTICKETDEFER)lpStruct;

				sprintf(szTemp1, "%C%C%C%C/%C%C/%C%C%C", 
					pStruct->cStationID[0], pStruct->cStationID[1], pStruct->cStationID[2],pStruct->cStationID[3],
					pStruct->cDevType[0], pStruct->cDevType[1],
					pStruct->cDeviceID[0], pStruct->cDeviceID[1], pStruct->cDeviceID[2]);
				memcpy(szTemp2, pStruct->cSAMID, sizeof(pStruct->cSAMID));
				memcpy(szTemp3, pStruct->cLogicalID, sizeof(pStruct->cLogicalID));
				memcpy(szTemp4, pStruct->cPhysicalID, sizeof(pStruct->cPhysicalID));

				strTemp.Format("%s%s%C%C\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%02x%02x\n%s%s%s\n%s%s%s\n",
					"��������",		SPACE_MIDDLE, pStruct->cTradeType[0], pStruct->cTradeType[1], 
					"�豸��Ϣ",		SPACE_MIDDLE, szTemp1, 
					"SAM����",		SPACE_MIDDLE, szTemp2,
					"SAM����ˮ",	SPACE_MIDDLE, pStruct->lSAMTrSeqNo, 
					"Ʊ������",		SPACE_MIDDLE, pStruct->bTicketType[0], pStruct->bTicketType[1], 
					"�߼�����",		SPACE_MIDDLE, szTemp3, 
					"������",		SPACE_MIDDLE, szTemp4);
				strOut += strTemp;

				BCDTimetoString(pStruct->dtOldExpiryDate, sizeof(pStruct->dtOldExpiryDate), szTemp1);
				BCDTimetoString(pStruct->dtNewExpiryDate, sizeof(pStruct->dtNewExpiryDate), szTemp2);
				BCDTimetoString(pStruct->dtOperateDate, sizeof(pStruct->dtOperateDate), szTemp3);
				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cOperatorID, sizeof(pStruct->cOperatorID));

				strTemp.Format("%s%s%d\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%c\n",
					"����״̬",		SPACE_MIDDLE, pStruct->bStatus, 
					"ԭ��Ч��",		SPACE_MIDDLE, szTemp1, 
					"����Ч��",		SPACE_MIDDLE, szTemp2, 
					"����ʱ��",		SPACE_MIDDLE, szTemp3, 
					"����Ա��",		SPACE_MIDDLE, szTemp4,
					"BOM��κ�",	SPACE_MIDDLE, pStruct->bBOMShfitID,
					"Ӧ�ñ�ʶ",		SPACE_MIDDLE, pStruct->cTestFlag);
				strOut += strTemp;
				break;
			}

		case struct_Lock:
			{
				PTICKETLOCK pStruct = (PTICKETLOCK)lpStruct;

				sprintf(szTemp1, "%C%C%C%C/%C%C/%C%C%C", 
					pStruct->cStationID[0], pStruct->cStationID[1], pStruct->cStationID[2],pStruct->cStationID[3],
					pStruct->cDevType[0], pStruct->cDevType[1],
					pStruct->cDeviceID[0], pStruct->cDeviceID[1], pStruct->cDeviceID[2]);
				memcpy(szTemp2, pStruct->cSAMID, sizeof(pStruct->cSAMID));

				strTemp.Format("%s%s%C%C\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%02x%02x\n",
					"��������",		SPACE_MIDDLE, pStruct->cTradeType[0], pStruct->cTradeType[1], 
					"�豸��Ϣ",		SPACE_MIDDLE, szTemp1, 
					"SAM����",		SPACE_MIDDLE, szTemp2,
					"SAM����ˮ",	SPACE_MIDDLE, pStruct->lSAMTrSeqNo, 
					"Ʊ������",		SPACE_MIDDLE, pStruct->bTicketType[0], pStruct->bTicketType[1]);
				strOut += strTemp;


				memcpy(szTemp1, pStruct->cLogicalID, sizeof(pStruct->cLogicalID));
				memcpy(szTemp2, pStruct->cPhysicalID, sizeof(pStruct->cPhysicalID));
				BCDTimetoString(pStruct->dtDate, sizeof(pStruct->dtDate), szTemp3);
				memcpy(szTemp4, pStruct->cOperatorID, sizeof(pStruct->cOperatorID));
				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%d\n%s%s%C\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%c\n%s%s%C\n",
					"�߼�����",		SPACE_MIDDLE, szTemp1, 
					"������",		SPACE_MIDDLE, szTemp2, 
					"����״̬",		SPACE_MIDDLE, pStruct->bStatus, 
					"�������",		SPACE_MIDDLE, pStruct->cLockFlag, 
					"����ʱ��",		SPACE_MIDDLE, szTemp3, 
					"����Ա��",		SPACE_MIDDLE, szTemp4,
					"BOM��κ�",	SPACE_MIDDLE, pStruct->bBOMShfitID,
					"Ӧ�ñ�ʶ",		SPACE_MIDDLE, pStruct->cTestFlag,
					"��Ӧ��ģʽ",	SPACE_MIDDLE, pStruct->cTkAppMode);
				strOut += strTemp;
				break;
			}

		case struct_Refund:
			{
				PDIRECTREFUND pStruct = (PDIRECTREFUND)lpStruct;

				sprintf(szTemp1, "%C%C%C%C/%C%C/%C%C%C", 
					pStruct->cStationID[0], pStruct->cStationID[1], pStruct->cStationID[2],pStruct->cStationID[3],
					pStruct->cDevType[0], pStruct->cDevType[1],
					pStruct->cDeviceID[0], pStruct->cDeviceID[1], pStruct->cDeviceID[2]);
				memcpy(szTemp2, pStruct->cSAMID, sizeof(pStruct->cSAMID));
				memcpy(szTemp3, pStruct->cLogicalID, sizeof(pStruct->cLogicalID));
				memcpy(szTemp4, pStruct->cPhysicalID, sizeof(pStruct->cPhysicalID));
				strTemp.Format("%s%s%C%C\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%02x%02x\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%d\n%s%s%d\n%s%s%d\n%s%s%d\n",
					"��������",		SPACE_MIDDLE, pStruct->cTradeType[0], pStruct->cTradeType[1], 
					"�豸��Ϣ",		SPACE_MIDDLE, szTemp1, 
					"SAM����",		SPACE_MIDDLE, szTemp2, 
					"SAM����ˮ",	SPACE_MIDDLE, pStruct->lSAMTrSeqNo,
					"Ʊ������",		SPACE_MIDDLE, pStruct->bTicketType[0], pStruct->bTicketType[1], 
					"�߼�����",		SPACE_MIDDLE, szTemp3,
					"������",		SPACE_MIDDLE, szTemp4,
					"����״̬",		SPACE_MIDDLE, pStruct->bStatus,
					"�˻����",		SPACE_MIDDLE, pStruct->lBalanceReturned, 
					"�˻�Ѻ��",		SPACE_MIDDLE, pStruct->nDepositReturned,
					"����",			SPACE_MIDDLE, pStruct->nForfeiture, 
					"����ԭ��",		SPACE_MIDDLE, pStruct->bForfeitReason);
				strOut += strTemp;

				BCDTimetoString(pStruct->dtDate, sizeof(pStruct->dtDate), szTemp1);

				BCDTimetoString(pStruct->dtApplyDate, sizeof(pStruct->dtApplyDate), szTemp2);

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->cMACOrTAC, sizeof(pStruct->cMACOrTAC));

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cOperatorID, sizeof(pStruct->cOperatorID));

				strTemp.Format("%s%s%d\n%s%s%C\n%s%s%s\n%s%s%C%C%C%C\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%d\n%s%s%c\n",
					"�����ۼ�",		SPACE_MIDDLE, pStruct->lTradeCount, 
					"�˿�����",		SPACE_MIDDLE, pStruct->bReturnTypeCode,
					"�˿�ʱ��",		SPACE_MIDDLE, szTemp1, 
					"����ƾ֤",		SPACE_MIDDLE, pStruct->cReceiptID[0], pStruct->cReceiptID[1], pStruct->cReceiptID[2], pStruct->cReceiptID[3],
					"����ʱ��",		SPACE_MIDDLE, szTemp2,
					"������֤��",	SPACE_MIDDLE, szTemp3,
					"����Ա��",		SPACE_MIDDLE, szTemp4,
					"BOM��κ�",	SPACE_MIDDLE, pStruct->bBOMShfitID,
					"�˿�����",		SPACE_MIDDLE, pStruct->lBrokerage,
					"Ӧ�ñ�ʶ",		SPACE_MIDDLE, pStruct->cTestFlag);
				strOut += strTemp;

				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cClassicType, sizeof(pStruct->cClassicType));

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cSamPosId, sizeof(pStruct->cSamPosId));

				strTemp.Format("%s%s%s\n%s%s%s\n",
					"TAC���׷���",	SPACE_MIDDLE, szTemp1,
					"SAM���ն˺�",	SPACE_MIDDLE, szTemp2);
				strOut += strTemp;

				break;
			}

		case struct_TkInfo:
			{
				PTICKETINFO pStruct = (PTICKETINFO)lpStruct;

				memcpy(szTemp1, pStruct->cPhysicalID, sizeof(pStruct->cPhysicalID));
				memcpy(szTemp2, pStruct->cLogicalID, sizeof(pStruct->cLogicalID));
				BCDTimetoString(pStruct->bIssueData, 4, szTemp3);
				BCDTimetoString(pStruct->bExpiry, 4, szTemp4);

				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%02X%02X\n%s%s%d\n%s%s%d\n%s%s%d\n%s%s%s\n%s%s%s\n",
					"������",		SPACE_MIDDLE, szTemp1, 
					"�߼�����",		SPACE_MIDDLE, szTemp2, 
					"Ʊ������",		SPACE_MIDDLE, pStruct->bTicketType[0], pStruct->bTicketType[1], 
					"Ʊ�����",		SPACE_MIDDLE, pStruct->lBalance, 
					"Ѻ��",			SPACE_MIDDLE, pStruct->lDepositorCost, 
					"Ǯ������",		SPACE_MIDDLE, pStruct->lLimitedBalance, 
					"��������",		SPACE_MIDDLE, szTemp3, 
					"������Ч��",	SPACE_MIDDLE, szTemp4);
				strOut += strTemp;

				BCDTimetoString(pStruct->bStartDate, 7, szTemp1);
				BCDTimetoString(pStruct->bEndDate, 7, szTemp2);
				sprintf(szTemp3, "%02x%02x/%02x/%x%02x", 
					pStruct->bLastStationID[0], pStruct->bLastStationID[1], 
					pStruct->bLastDeviceType,
					pStruct->bLastDeviceID[0], pStruct->bLastDeviceID[1]);
				BCDTimetoString(pStruct->dtLastDate, 7, szTemp4);
				BCDTimetoString(pStruct->dtEntryDate, 7, szTemp5);

				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%d\n%s%s%s\n%s%s%s\n%s%s%02x%02x\n%s%s%s\n%s%s%d\n%s%s%02x%02x\n%s%s%02x%02x\n",
					"Ʊ����������",	SPACE_MIDDLE, szTemp1, 
					"Ʊ��ͣ������",	SPACE_MIDDLE, szTemp2, 
					"״̬",			SPACE_MIDDLE, pStruct->bStatus,
					"�ϴν����豸",	SPACE_MIDDLE, szTemp3,
					"�ϴν���ʱ��",	SPACE_MIDDLE, szTemp4, 
					"��վվ��",		SPACE_MIDDLE, pStruct->bEntrySationID[0], pStruct->bEntrySationID[1],
					"��վʱ��",		SPACE_MIDDLE, szTemp5,
					"����ģʽ",		SPACE_MIDDLE, pStruct->bLimitMode,
					"��վվ������",	SPACE_MIDDLE, pStruct->bLimitEntryID[0], pStruct->bLimitEntryID[1],
					"��վվ������",	SPACE_MIDDLE, pStruct->bLimitExitID[0], pStruct->bLimitExitID[1]);
				strOut += strTemp;

				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cCityCode, sizeof(pStruct->cCityCode));

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cSellerCode, sizeof(pStruct->cSellerCode));

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->cCertificateCode, 20);

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cCertificateName, sizeof(pStruct->cCertificateName));

				memset(szTemp5, 0, sizeof(szTemp5));
				BCDTimetoString(pStruct->bCertExpire, 4, szTemp5);

				strTemp.Format("%s%s%d\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%d\n%s%s%x\n%s%s%s\n%s%s%s\n%s%s%c\n%s%s%c\n%s%s%s\n",
					"����״̬",		SPACE_MIDDLE, pStruct->bActiveStatus, 
					"���д���",		SPACE_MIDDLE, szTemp1, 
					"�����̴���",	SPACE_MIDDLE, szTemp2, 
					"��ֵ����",		SPACE_MIDDLE, pStruct->lChargeCount, 
					"���Ѽ���",		SPACE_MIDDLE, pStruct->lTradeCount, 
					"֤������",		SPACE_MIDDLE, pStruct->bCertificateType, 
					"֤������",		SPACE_MIDDLE, szTemp3, 
					"֤������",		SPACE_MIDDLE, szTemp4, 
					"Ӧ�ñ�ʶ",		SPACE_MIDDLE, pStruct->cTestFlag,
					"Ӧ��ģʽ",		SPACE_MIDDLE, pStruct->cTkAppMode,
					"֤����Ч��",		SPACE_MIDDLE, szTemp5);
				strOut += strTemp;

				FormHeader(strOut, strOper);

				strTemp.Format("\t     %-21s%-6s%-6s%-10s%-10s%-10s%-16s\n",
					"����ʱ��", "վ��", "״̬", "���", "�豸����", "�豸��", "SAM����");
				strOut += strTemp;

				for (BYTE i=0;i<pStruct->bUsefulCount;i++)
				{
					BCDTimetoString(pStruct->bhs[i].dtDate, sizeof(pStruct->bhs[i].dtDate), szTemp1);
					memset(szTemp2, 0, sizeof(szTemp2));
					memcpy(szTemp2, pStruct->bhs[i].cSAMID, 16);

					strTemp.Format("\t     %-21s%02X%02X  %-6d%-10d%-10d%02X%02X      %-16s\n",
						szTemp1, 
						pStruct->bhs[i].bStationID[0], pStruct->bhs[i].bStationID[1], 
						pStruct->bhs[i].bStatus,
						pStruct->bhs[i].lTradeAmount, 
						pStruct->bhs[i].bDeviceType, 
						pStruct->bhs[i].bDeviceID[0], pStruct->bhs[i].bDeviceID[1], 
						szTemp2);
					strOut += strTemp;
				}
				strOut += "\n";

				break;				   
			}
		case struct_Update:
			{
				PTICKETUPDATE pStruct = (PTICKETUPDATE)lpStruct;

				sprintf(szTemp1, "%C%C%C%C/%C%C/%C%C%C", 
					pStruct->cStationID[0], pStruct->cStationID[1], pStruct->cStationID[2],pStruct->cStationID[3],
					pStruct->cDevType[0], pStruct->cDevType[1],
					pStruct->cDeviceID[0], pStruct->cDeviceID[1], pStruct->cDeviceID[2]);

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cSAMID, sizeof(pStruct->cSAMID));

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->cLogicalID, sizeof(pStruct->cLogicalID));

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cPhysicalID, sizeof(pStruct->cPhysicalID));

				strTemp.Format("%s%s%C%C\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%02x%02x\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%d\n%s%s%C\n%s%s%d\n",
					"��������",		SPACE_MIDDLE, pStruct->cTradeType[0], pStruct->cTradeType[1],
					"�����豸",		SPACE_MIDDLE, szTemp1,
					"SAM����",		SPACE_MIDDLE, szTemp2,
					"SAM��ˮ",		SPACE_MIDDLE, pStruct->lSAMTrSeqNo,
					"Ʊ������",		SPACE_MIDDLE, pStruct->bTicketType[0], pStruct->bTicketType[1], 
					"�߼�����",		SPACE_MIDDLE, szTemp3,
					"������",		SPACE_MIDDLE, szTemp4,
					"���Ѽ���",		SPACE_MIDDLE, pStruct->lTradeCount,
					"����״̬",		SPACE_MIDDLE, pStruct->bStatus,
					"��������",		SPACE_MIDDLE, pStruct->cUpdateZone,
					"����ԭ��",		SPACE_MIDDLE, pStruct->bUpdateReasonCode);
				strOut += strTemp;

				BCDTimetoString(pStruct->dtUpdateDate, sizeof(pStruct->dtUpdateDate), szTemp1);

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cReceiptID, sizeof(pStruct->cReceiptID));

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->cOperatorID, sizeof(pStruct->cOperatorID));

				strTemp.Format("%s%s%s\n%s%s%d\n%s%s%d\n%s%s%s\n%s%s%s\n%s%s%02x%02x\n%s%s%d\n%s%s%c\n",
					"����ʱ��",		SPACE_MIDDLE, szTemp1, 
					"֧����ʽ",		SPACE_MIDDLE, pStruct->bPaymentMode,
					"���·���",		SPACE_MIDDLE, pStruct->nForfeiture,
					"֧��ƾ֤",		SPACE_MIDDLE, szTemp2,
					"����Ա��",		SPACE_MIDDLE, szTemp3,
					"��վվ��",		SPACE_MIDDLE, pStruct->bEntryStationID[0], pStruct->bEntryStationID[1], 
					"BOM��κ�",	SPACE_MIDDLE, pStruct->bBOMShfitID,
					"Ӧ�ñ�ʶ",		SPACE_MIDDLE, pStruct->cTestFlag);
				strOut += strTemp;

				strTemp.Format("%s%s%C%C%C\n%s%s%02x%02x\n%s%s%02x%02x\n%s%s%C\n",
					"����ģʽ",		SPACE_MIDDLE, pStruct->cLimitMode[0], pStruct->cLimitMode[1], pStruct->cLimitMode[2],
					"���ƽ�վվ��", SPACE_MIDDLE, pStruct->bLimitEntryID[0], pStruct->bLimitEntryID[1],
					"���Ƴ�վվ��", SPACE_MIDDLE, pStruct->bLimitExitID[0], pStruct->bLimitExitID[1],
					"��Ӧ��ģʽ",	SPACE_MIDDLE, pStruct->cTkAppMode);
				strOut += strTemp;

				break;				   
			}
		case struct_Entry:
			{
				PENTRYGATE pStruct = (PENTRYGATE)lpStruct;

				sprintf(szTemp1, "%C%C%C%C/%C%C/%C%C%C", 
					pStruct->cStationID[0], pStruct->cStationID[1], pStruct->cStationID[2],pStruct->cStationID[3],
					pStruct->cDevType[0], pStruct->cDevType[1],
					pStruct->cDeviceID[0], pStruct->cDeviceID[1], pStruct->cDeviceID[2]);

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cSAMID, sizeof(pStruct->cSAMID));

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->cLogicalID, sizeof(pStruct->cLogicalID));

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cPhysicalID, sizeof(pStruct->cPhysicalID));

				BCDTimetoString(pStruct->dtDate, sizeof(pStruct->dtDate), szTemp5);

				strTemp.Format("%s%s%C%C\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%02x%02x\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%d\n%s%s%d\n%s%s%c\n",
					"��������",		SPACE_MIDDLE, pStruct->cTradeType[0], pStruct->cTradeType[1], 
					"��վ�豸",		SPACE_MIDDLE, szTemp1, 
					"SAM����",		SPACE_MIDDLE, szTemp2,
					"SAM����ˮ",	SPACE_MIDDLE, pStruct->lSAMTrSeqNo,
					"Ʊ������",		SPACE_MIDDLE, pStruct->bTicketType[0], pStruct->bTicketType[1], 
					"�߼�����",		SPACE_MIDDLE, szTemp3,
					"������",		SPACE_MIDDLE, szTemp4,
					"����ʱ��",		SPACE_MIDDLE, szTemp5,
					"����״̬",		SPACE_MIDDLE, pStruct->bStatus,
					"���",			SPACE_MIDDLE, pStruct->lBalance, 
					"Ӧ�ñ�ʶ",		SPACE_MIDDLE, pStruct->cTestFlag);
				strOut += strTemp;

				strTemp.Format("%s%s%C%C%C\n%s%s%02x%02x\n%s%s%02x%02x\n%s%s%c\n%s%s%d\n%s%s%C\n",
					"����ģʽ",		SPACE_MIDDLE, pStruct->cLimitMode[0], pStruct->cLimitMode[1], pStruct->cLimitMode[2],
					"���ƽ�վվ��", SPACE_MIDDLE, pStruct->bLimitEntryID[0], pStruct->bLimitEntryID[1],
					"���Ƴ�վվ��", SPACE_MIDDLE, pStruct->bLimitExitID[0], pStruct->bLimitExitID[1],
					"��վ����ģʽ", SPACE_MIDDLE, pStruct->cEntryMode,
					"���Ѽ���",		SPACE_MIDDLE, pStruct->lTradeCount,
					"��Ӧ��ģʽ",	SPACE_MIDDLE, pStruct->cTkAppMode);
				strOut += strTemp;

				break;
			}
		case struct_Ver:
			{
				PREADERVERSION pStruct = (PREADERVERSION)lpStruct;

				BCDtoString(pStruct->verApiFile, sizeof(pStruct->verApiFile), szTemp1);
				BCDtoString(pStruct->verRfFile, sizeof(pStruct->verRfFile), szTemp2);
				BCDtoString(pStruct->verSamFile, sizeof(pStruct->verSamFile), szTemp3);


				strTemp.Format("%s%s%x.%02X\n%s%s%s\n%s%s%X\n%s%s%s\n%s%s%x\n%s%s%s\n",
					"APIʶ��汾",		SPACE_MIDDLE, pStruct->verApi[0], pStruct->verApi[1], 
					"API�ļ��汾",		SPACE_MIDDLE, szTemp1, 
					"RF����ʶ��汾",	SPACE_MIDDLE, pStruct->verRfDev,
					"RF�����ļ��汾",	SPACE_MIDDLE, szTemp2,
					"SAM����ʶ��汾",	SPACE_MIDDLE, pStruct->verSamDev, 
					"SAM�����ļ��汾",	SPACE_MIDDLE, szTemp3);
				strOut += strTemp;

				break;
			}
		case struct_ETicket:
			{
				PETICKETDEALINFO pStruct = (PETICKETDEALINFO)lpStruct;

				//BCDTimetoString(pStruct->bBussinessDate, sizeof(pStruct->bBussinessDate), szTemp1);

				//sprintf(szTemp2,"%02x%02x%02x%02x%02x%02x%02x%02x",pStruct->bLogicalID[0],pStruct->bLogicalID[1],pStruct->bLogicalID[2],
				//	pStruct->bLogicalID[3],pStruct->bLogicalID[4],pStruct->bLogicalID[5],pStruct->bLogicalID[6],pStruct->bLogicalID[7]);

				//sprintf(szTemp3,"%02x%02x%02x%02x%02x%02x%02x%02x",pStruct->bPhysicalID[0],pStruct->bPhysicalID[1],pStruct->bPhysicalID[2],
					//pStruct->bPhysicalID[3],pStruct->bPhysicalID[4],pStruct->bPhysicalID[5],pStruct->bPhysicalID[6],pStruct->bPhysicalID[7]);
				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cTxnDate, sizeof(pStruct->cTxnDate));

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cTransCode, sizeof(pStruct->cTransCode));

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->cTicketMainType, sizeof(pStruct->cTicketMainType));

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cTicketSubType, sizeof(pStruct->cTicketSubType));

				memset(szTemp5, 0, sizeof(szTemp5));
				memcpy(szTemp5, pStruct->cChipType, sizeof(pStruct->cChipType));


				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n",
					"��Ӫ����",		SPACE_MIDDLE, szTemp1, 
					"����״̬",		SPACE_MIDDLE, szTemp2, 
					"��Ʊ������",	SPACE_MIDDLE, szTemp3,
					"��Ʊ������",	SPACE_MIDDLE, szTemp4,
					"Ʊ��оƬ����",	SPACE_MIDDLE, szTemp5); 
					//"�߼�����",		SPACE_MIDDLE, pStruct->cTicketLogicalNo,
					//"Ʊ��CSN",		SPACE_MIDDLE, pStruct->cTicketCSN);
				strOut += strTemp;

				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cTicketLogicalNo, sizeof(pStruct->cTicketLogicalNo));

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cTicketCSN, sizeof(pStruct->cTicketCSN));

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->cTicketCount, sizeof(pStruct->cTicketCount));

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cPsamNo, sizeof(pStruct->cPsamNo));

				memset(szTemp5, 0, sizeof(szTemp5));
				memcpy(szTemp5, pStruct->cPsamSeq, sizeof(pStruct->cPsamSeq));


				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n",
					"�߼�����",		SPACE_MIDDLE, szTemp1, 
					"Ʊ��CSN",		SPACE_MIDDLE, szTemp2, 
					"Ʊ������",		SPACE_MIDDLE, szTemp3,
					"PSAM����",		SPACE_MIDDLE, szTemp4,
					"PSAM����ˮ��",	SPACE_MIDDLE, szTemp5); 
				strOut += strTemp;

				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cTxnTms, sizeof(pStruct->cTxnTms));

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cPayType, sizeof(pStruct->cPayType));

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->cBeforeTxnBalance, sizeof(pStruct->cBeforeTxnBalance));

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cTxnAmountNo, sizeof(pStruct->cTxnAmountNo));

				memset(szTemp5, 0, sizeof(szTemp5));
				memcpy(szTemp5, pStruct->cTxnAmount, sizeof(pStruct->cTxnAmount));


				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n",
					"��������ʱ��",		SPACE_MIDDLE, szTemp1, 
					"֧����ʽ",		SPACE_MIDDLE, szTemp2, 
					"����ǰƱֵ",		SPACE_MIDDLE, szTemp3,
					"���״���",		SPACE_MIDDLE, szTemp4,
					"���׽��",	SPACE_MIDDLE, szTemp5); 
				strOut += strTemp;

				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cTxnStation, sizeof(pStruct->cTxnStation));

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cLastStation, sizeof(pStruct->cLastStation));

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->cLastTxnTms, sizeof(pStruct->cLastTxnTms));

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cDevNodeId, sizeof(pStruct->cDevNodeId));

				memset(szTemp5, 0, sizeof(szTemp5));
				memcpy(szTemp5, pStruct->cDevSeqNo, sizeof(pStruct->cDevSeqNo));

				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n",
					"��ǰ��վ",			SPACE_MIDDLE, pStruct->cTxnStation,
					"�ϴ�ʹ�ó�վ",		SPACE_MIDDLE, pStruct->cLastStation,
					"�ϴ�ʹ��ʱ��",		SPACE_MIDDLE, pStruct->cLastTxnTms,
					"�ն��豸��ʶ",		SPACE_MIDDLE, pStruct->cDevNodeId,
					"�ն��豸��ˮ��",	SPACE_MIDDLE, pStruct->cDevSeqNo);
				strOut += strTemp;

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cCenterCode, sizeof(pStruct->cCenterCode));

				strTemp.Format("%s%s%s\n",
					"����Ʊ��",			SPACE_MIDDLE, szTemp4);
				strOut += strTemp;
				break;
			}
		case struct_GetETicket:
			{
				PGETETICKETRESPINFO pStruct = (PGETETICKETRESPINFO)lpStruct;

				memset(szTemp1, 0, sizeof(szTemp1));
				memcpy(szTemp1, pStruct->cAmount, sizeof(pStruct->cAmount));

				memset(szTemp2, 0, sizeof(szTemp2));
				memcpy(szTemp2, pStruct->cSum, sizeof(pStruct->cSum));

				memset(szTemp4, 0, sizeof(szTemp4));
				memcpy(szTemp4, pStruct->cCenterCode, sizeof(pStruct->cCenterCode));

				memset(szTemp3, 0, sizeof(szTemp3));
				memcpy(szTemp3, pStruct->cStartStation, sizeof(pStruct->cStartStation));

				memset(szTemp5, 0, sizeof(szTemp5));
				memcpy(szTemp5, pStruct->cTerminalStation, sizeof(pStruct->cTerminalStation));

				strTemp.Format("%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n%s%s%s\n",
					"����Ʊ����",			SPACE_MIDDLE, szTemp1,
					"�ܽ��",				SPACE_MIDDLE, szTemp2,
					"Ʊ��",					SPACE_MIDDLE, szTemp4,
					"��ʼվ�����",			SPACE_MIDDLE, szTemp3,
					"����վ�����",			SPACE_MIDDLE, szTemp5);
				strOut += strTemp;

				memset(szTemp5, 0, sizeof(szTemp5));
				memcpy(szTemp5, pStruct->cTicketLogicalNo, sizeof(pStruct->cTicketLogicalNo));

				strTemp.Format("%s%s%s\n",
					"�߼�����",			SPACE_MIDDLE, szTemp5);
				strOut += strTemp;
				break;
			}
		}
	}
	if (StructType != struct_History && StructType != struct_TkInfo)
		FormHeader(strOut, strOper);
	AppendText(strOut);
	SetStatusBar(strOper, g_retInfo, liStart, liEnd);
}

BOOL CCSTesterDlg::GetSetting(int& nPort, WORD& wStation, WORD& wDevice)
{
	return m_wndDlgBar.GetSetting(nPort, wStation, wDevice);
}

void CCSTesterDlg::LoadErrorString(CString strPath)
{
	CStdioFile sfile;
	CString strRead;
	if (sfile.Open(strPath, CFile::modeRead))
	{
		while (sfile.ReadString(strRead))
		{
			m_ErrorDiscribe.AddTail(strRead);
		}
	}
}

void CCSTesterDlg::GetErrorString(DWORD nError, CString& strError)
{
	CString str, strElement;
	str.Format("%u", nError);
	int nLen;

	POSITION pos = m_ErrorDiscribe.GetHeadPosition();
	for (int i=0;i < m_ErrorDiscribe.GetCount();i++)
	{
		strElement = m_ErrorDiscribe.GetNext(pos);
		nLen = str.GetLength();
		if (str == strElement.Left(nLen))
		{
			nLen = strElement.GetLength() - nLen - 1;
			strError = strElement.Right(nLen);
			break;
		}
	}
}

void CCSTesterDlg::ShowDialogs(DWORD flag)
{
	m_pOper->ShowWindow((flag >> DLG_OPER) & 0x01);
	m_pInc->ShowWindow((flag >> DLG_INC) & 0x01);
	m_pDec->ShowWindow((flag >> DLG_DEC) & 0x01);
	m_pSjtSale->ShowWindow((flag >> DLG_SSALE) & 0x01);
	m_pUpdate->ShowWindow((flag >> DLG_UPD) & 0x01);
	m_pPrice->ShowWindow((flag >> DLG_PRICE) & 0x01);
	m_pParam->ShowWindow((flag >> DLG_PARAM) & 0x01);	
	m_pRefund->ShowWindow((flag >> DLG_RFD) & 0x01);	
	m_pSamAct->ShowWindow((flag >> DLG_SAM_ACT) & 0x01);
	m_pDescind->ShowWindow((flag >> DLG_DESCIND) & 0x01);
	m_pDebug->ShowWindow((flag >> DLG_DEBUG) & 0x01);
	m_pDegrade->ShowWindow((flag >> DLG_DEGRADE) & 0x01);
	m_pSaleEx->ShowWindow((flag >> DLG_SALEEX) & 0x01);
}

CString CCSTesterDlg::GetLineName(BYTE lineID)
{
	CString strRet;

	switch (lineID)
	{
	case 0x01:
		strRet = "һ����";
		break;
	case 0x02:
		strRet = "������";
		break;
	}

	return strRet;
}

CString CCSTesterDlg::GetStationPrmPath()
{
	CString strRet;
	char szPath[MAX_PATH] = {0};

	::GetModuleFileName(NULL, szPath, MAX_PATH);

	for (size_t i=strlen(szPath);i>0;i--)
	{
		if (szPath[i-1] == '\\')
		{
			szPath[i] = 0;
			break;
		}
	}

	strRet = CString(szPath) + "PRM.0201";

	return strRet;
}

// ���س�վ��
void CCSTesterDlg::LoadStationTable(CString strPath)
{
	char szRead[256]		= {0};
	char szTemp[32]			= {0};
	BYTE bLineID			= 0;
	WORD wStationID			= 0;
	CString strStationName;
	LINEINF lineinf;
	map<BYTE, LINEINF>::iterator itor;


	UnLoadStationTable();

	GSFile file(strPath.GetBuffer(), ModeRead);
	if (file.IsOpen())
	{
		while (file.ReadLine(szRead, sizeof(szRead)))
		{
			if (memcmp(szRead, "0201", 4) == 0)
			{
				memcpy(szTemp, szRead + 5, 24);

				bLineID = TwoChar2Hex(szRead + 5);

				itor = g_PrmStation.find(bLineID);

				wStationID = (bLineID << 8) + TwoChar2Hex(szRead + 7);
				memcpy(szTemp, szRead + 9, 20);
				strStationName = szTemp;

				if (itor == g_PrmStation.end())		// ��Ҫ�½�һ����·
				{
					lineinf.vsta.clear();
					lineinf.lineName = GetLineName(bLineID);

					lineinf.vsta.push_back(make_pair(wStationID, strStationName));
					g_PrmStation.insert(make_pair(bLineID, lineinf));
				}
				else		// ��·�Ѵ���
				{
					itor->second.vsta.push_back(make_pair(wStationID, strStationName));
				}
			}
		}

		file.Close();
	}

}

// ж�س�վ��
void CCSTesterDlg::UnLoadStationTable()
{
	map<BYTE, LINEINF>::iterator itor;
	for(itor=g_PrmStation.begin();itor!=g_PrmStation.end();++itor)
	{
		itor->second.vsta.clear();
	}
	g_PrmStation.clear();
}

// ��ʾ��ͨѶ��ʽ���˵�ǰ��С��
void CCSTesterDlg::SetSubMenuCheck(int nSubPos, UINT uTargID)
{
	UINT nID, nCount;

	CMenu * pMenu = GetMenu()->GetSubMenu(nSubPos);
	nCount = pMenu->GetMenuItemCount();
	for (UINT i=0;i<nCount;i++)
	{
		nID = pMenu->GetMenuItemID(i);

		if (nID == uTargID)
			pMenu->CheckMenuItem(nID, MF_CHECKED);
		else
			pMenu->CheckMenuItem(nID, MF_UNCHECKED);

	}
}

int CCSTesterDlg::EnumSerials(CComboBox& comb)
{
	HKEY			hKey;
	int				nIndex		= 0;
	TCHAR			szName[64]	= {0};
	TCHAR			szValue[64]	= {0};
	DWORD			dwSize		= 64;
	DWORD			dwType		= REG_SZ;
	if(ERROR_SUCCESS == ::RegOpenKey(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", &hKey))
	{
		comb.ResetContent();
		while(ERROR_NO_MORE_ITEMS != RegEnumValue(hKey, nIndex++, szName, &dwSize, NULL, &dwType, (LPBYTE)szValue, &dwSize))
		{
			comb.AddString(szValue);
			ZeroMemory(szName, 64);
			ZeroMemory(szValue, 64);
			dwSize		= 64;
			dwType		= REG_SZ;
		}
	}

	int ret = comb.GetCount();
	if (ret > 0)
		comb.SetCurSel(0);

	return ret;

}

BEGIN_MESSAGE_MAP(CCSTesterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DEVICECHANGE()
	ON_COMMAND(IDM_DEV_BOM, &CCSTesterDlg::OnDevBom)
	ON_COMMAND(IDM_DEV_AGM, &CCSTesterDlg::OnDevAgm)
	ON_COMMAND(IDM_DEV_TVM, &CCSTesterDlg::OnDevTvm)
	ON_COMMAND(IDM_DEV_TCM, &CCSTesterDlg::OnDevTcm)
	ON_COMMAND(IDM_SAVE_DISP, &CCSTesterDlg::OnSaveDisp)
	ON_COMMAND(IDM_CLEAR_DISP, &CCSTesterDlg::OnClearDisp)
	ON_COMMAND(IDM_EXIT, &CCSTesterDlg::OnExit)
	//}}AFX_MSG_MAP
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_ANALYSE, &CCSTesterDlg::OnAnalyse)
	ON_MESSAGE(WM_INCREASE, &CCSTesterDlg::OnIncrease)
	ON_MESSAGE(WM_DECREASE, &CCSTesterDlg::OnDecrease)
	ON_MESSAGE(WM_UPDATE, &CCSTesterDlg::OnUpdate)
	ON_MESSAGE(WM_REFUND, &CCSTesterDlg::OnRefund)
	ON_MESSAGE(WM_SJTSALE, &CCSTesterDlg::OnSjtsale)
	ON_MESSAGE(WM_SALEEX, &CCSTesterDlg::OnSaleEx)
	ON_MESSAGE(WM_INITREADER, &CCSTesterDlg::OnInitReader)
	ON_MESSAGE(WM_UNINITREADER, &CCSTesterDlg::OnUnInitReader)
	ON_MESSAGE(WM_CHANGE_ANT, &CCSTesterDlg::OnChangeAnt)
	ON_MESSAGE(WM_SAMACT, &CCSTesterDlg::OnSamAct)
	ON_MESSAGE(WM_DESCIND, &CCSTesterDlg::OnChargeDescind)

	ON_COMMAND(IDM_QUERY_FARE, &CCSTesterDlg::OnQueryFare)
	ON_COMMAND(IDM_PARAM, &CCSTesterDlg::OnParam)
	ON_COMMAND(ID_SAM_INFO, &CCSTesterDlg::OnSamInfo)
	ON_COMMAND(IDM_CMD, &CCSTesterDlg::OnCmd)
	ON_COMMAND(IDM_TEST_HARD, &CCSTesterDlg::OnTestHard)
	ON_COMMAND(IDM_READER_VERSION, &CCSTesterDlg::OnReaderVersion)
	ON_COMMAND(IDM_TRADE_FILE_EXCH, &CCSTesterDlg::OnTradeFileExch)
	ON_COMMAND(IDM_DEBUG, &CCSTesterDlg::OnDebug)
	ON_COMMAND(IDM_GETFILE, &CCSTesterDlg::OnGetFileFromReader)
	ON_COMMAND(IDM_UPGRADE, &CCSTesterDlg::OnUpgrade)
	ON_COMMAND(IDM_ENALBE_DEBUGER, &CCSTesterDlg::OnEnalbeDebuger)
	ON_COMMAND(IDM_SET_DEGRADE, &CCSTesterDlg::OnSetDegrade)
END_MESSAGE_MAP()


// CCSTesterDlg ��Ϣ�������
BOOL CCSTesterDlg::OnDeviceChange(UINT nEventType,DWORD dwData)
{
	m_wndDlgBar.SendMessage(WM_DEVICECHANGE);
	return TRUE;
}

BOOL CCSTesterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	QueryPerformanceFrequency(&g_liCpuFreq);

	CString strPath = GetStationPrmPath();
	LoadStationTable(strPath);

	m_bEnableDebuger = HasRootAuth();
	EnableRootMenus(m_bEnableDebuger);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CCSTesterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CCSTesterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
//
HCURSOR CCSTesterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CCSTesterDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: �ڴ˴������Ϣ����������
	AddBars();

	m_pOper = new CDlgOper;
	m_pOper->Create(IDD_DLG_OPER, this);

	m_pBom = new CDlgBom;
	m_pBom->Create(IDD_DLG_BOM, this);

	m_pAgm = new CDlgAgm;
	m_pAgm->Create(IDD_DLG_AGM, this);

	m_pTvm = new CDlgTvm;
	m_pTvm->Create(IDD_DLG_TVM, this);

	m_pTcm = new CDlgTcm;
	m_pTcm->Create(IDD_DLG_TCM, this);

	m_pInc = new CDlgInc;
	m_pInc->Create(IDD_DLG_INCR, this);

	m_pDec = new CDlgDec;
	m_pDec->Create(IDD_DLG_DECR, this);

	m_pSjtSale = new CDlgSjtSale;
	m_pSjtSale->Create(IDD_DLG_SJTSALE, this);

	m_pRefund = new CDlgRefund;
	m_pRefund->Create(IDD_DLG_REFUND, this);

	m_pUpdate = new CDlgUpd;
	m_pUpdate->Create(IDD_DLG_UPDATE, this);

	m_pPrice = new CDlgPrice;
	m_pPrice->Create(IDD_DLG_PRICE, this);

	m_pParam = new CDlgParam;
	m_pParam->Create(IDD_DLG_PARAM, this);

	m_pSamAct = new CDlgSamAct;
	m_pSamAct->Create(IDD_DLG_SAM_ACT, this);

	m_pDescind = new CDlgDescind;
	m_pDescind->Create(IDD_DLG_DESCIND, this);

	m_pDebug = new CDlgDebug;
	m_pDebug->Create(IDD_DLG_DEBUG, this);

	m_pDegrade = new CDlgDegrade;
	m_pDegrade->Create(IDD_DLG_DEGRADE, this);

	m_pSaleEx = new CDlgSaleEx;
	m_pSaleEx->Create(IDD_DLG_SALE_EX, this);

	ChildWindowAnchor();

	m_pBom->SendMessage(WM_PAGECHANGED);
	m_pBom->ShowWindow(SW_SHOW);
	m_pPageLast = m_pBom;

	ShowWindow(SW_MAXIMIZE);

}

void CCSTesterDlg::OnDevBom()
{
	// TODO: �ڴ���������������
	if (m_pPageLast != m_pBom)
	{
		m_pPageLast->ShowWindow(SW_HIDE);

		m_pBom->SendMessage(WM_PAGECHANGED);
		m_pBom->ShowWindow(SW_SHOW);

		// ��������ģʽ��Ϊ������
		SendMessage(WM_CHANGE_ANT, 0x01);
		m_pPageLast = m_pBom;
		ShowDialogs(0);
	}
}

void CCSTesterDlg::OnDevAgm()
{
	// TODO: �ڴ���������������
	if (m_pPageLast != m_pAgm)
	{
		m_pPageLast->ShowWindow(SW_HIDE);

		m_pAgm->SendMessage(WM_PAGECHANGED);
		m_pAgm->ShowWindow(SW_SHOW);
		m_pPageLast = m_pAgm;
		ShowDialogs(0);
	}
}

void CCSTesterDlg::OnDevTvm()
{
	// TODO: �ڴ���������������
	if (m_pPageLast != m_pTvm)
	{
		m_pPageLast->ShowWindow(SW_HIDE);

		m_pTvm->SendMessage(WM_PAGECHANGED);
		m_pTvm->ShowWindow(SW_SHOW);
		m_pPageLast = m_pTvm;
		ShowDialogs(0);
	}
}

void CCSTesterDlg::OnDevTcm()
{
	// TODO: �ڴ���������������
	if (m_pPageLast != m_pTcm)
	{
		m_pPageLast->ShowWindow(SW_HIDE);

		m_pTcm->SendMessage(WM_PAGECHANGED);
		m_pTcm->ShowWindow(SW_SHOW);
		m_pPageLast = m_pTcm;
		ShowDialogs(0);
	}
}

void CCSTesterDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������

	if (m_rEditOut.GetSafeHwnd())
	{
		ChildWindowAnchor();
	}
}

void CCSTesterDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialog::OnGetMinMaxInfo(lpMMI);
	lpMMI-> ptMinTrackSize.x = 800; 
	lpMMI-> ptMinTrackSize.y = 500; 
}


COLORREF corText[3] = {RGB(200, 0, 0), RGB(0, 200, 0), RGB(0, 0, 200),};
int g_nCount = 0;
void CCSTesterDlg::AppendText(CString& strText)
{

	int nLen;
	CHARFORMAT cf = {0};
	g_nCount = (g_nCount + 1) % 3;
	cf.crTextColor = corText[g_nCount];
	cf.dwEffects = ~CFE_AUTOCOLOR; 
	cf.dwMask = CFM_COLOR;	


	nLen = m_rEditOut.GetWindowTextLength(); 
	m_rEditOut.SetFocus (); 
	m_rEditOut.SetSel (nLen, nLen); 
	m_rEditOut.SetWordCharFormat(cf);
	m_rEditOut.ReplaceSel (strText);
}

void CCSTesterDlg::SetStatusBar(CString strOper, RETINFO Error, LARGE_INTEGER liStart, LARGE_INTEGER liEnd)
{
	char szError[128] = {0};
	char szTime[32] = {0};
	char szTemp[32]	= {0};
	long gap = 0;

	sprintf_s(szError, sizeof(szError), "�����룺%04X,%02X  %s", Error.wErrCode, Error.bNoticeCode, szTemp);

	//gap = liEnd.QuadPart - liStart.QuadPart;

	//if(gap > 500)
		//gap = gap - 300;


	sprintf_s(szTime, sizeof(szTime), "��ʱ:%8lf", ((double)(liEnd.QuadPart - liStart.QuadPart) / (double)(g_liCpuFreq.QuadPart)));

	m_WndStatus.SetText(strOper, 0, SBT_POPOUT);
	m_WndStatus.SetText(szError, 1, SBT_POPOUT);
	m_WndStatus.SetText(szTime, 2, SBT_POPOUT);
}

void CCSTesterDlg::FormHeader(CString& strTarg, CString strSrc)
{
	FormOut(strTarg, 2);
	CString strTemp = COMPART_LINE;
	int nPos = (int)(strTemp.GetLength() - strSrc.GetLength()) / 2;

	// 2��ǰ��ո�ĳ���
	memcpy(strTemp.GetBuffer() + nPos + 2, strSrc.GetBuffer(), strSrc.GetLength());
	strTarg = strTemp + "\n" + strTarg + "\n";
}

void CCSTesterDlg::FormOut(CString& strTarg, int nRow)
{
	int nPosEnter;
	int nIndex, nLen;
	CString strTemp, strSrc;

	nIndex = 0;
	strSrc = strTarg;
	strTarg = "";

	while(1) 
	{
		nLen = strSrc.GetLength();
		if (nLen <= 0)	
		{
			if (nIndex % nRow != 0)
				strTarg += "\n";				
			break;
		}

		nPosEnter = strSrc.Find("\n");
		if (nPosEnter > 0)
		{
			strTemp = strSrc.Left(nPosEnter);
			strSrc = strSrc.Right(nLen - nPosEnter - 1);
		}
		else
		{
			strTemp = strSrc;
			strSrc = "";
		}

		AddSpace(strTemp);
		nIndex ++;

		strTarg += strTemp;
		if (nIndex % nRow == 0)
			strTarg += "\n";
		else
			strTarg += "|";
	}
}

#define LEN_NAME	26		// ����ֶ������ո�ĳ���
#define LEN_VALUE	28		// ����ֶ�ֵ���ո�ĳ���
void CCSTesterDlg::AddSpace(CString& str)
{
	int nPosSep;
	int nLen;
	char szlSPace[LEN_NAME] = {0};
	char szrSpace[LEN_VALUE] = {0};

	nPosSep = str.Find(":");
	if (nPosSep > 0)
	{
		nLen = str.GetLength();
		memset(szlSPace, 0x20, LEN_NAME - nPosSep);
		if (LEN_VALUE > nLen - nPosSep - 1)
			memset(szrSpace, 0x20, LEN_VALUE - (nLen - nPosSep - 1));
		//else
		//	memset(szrSpace, 0x20, LEN_VALUE - (nLen - nPosSep - 1));

		str = CString(szlSPace) + str + CString(szrSpace);
	}
}

void CCSTesterDlg::BCDTimetoString(LPBYTE pbSrc, BYTE bLenSrc, char * pszTarg)
{
	char szTemp[16] = {0};

	sprintf_s(szTemp, sizeof(szTemp), "%02X%02X-%02X-%02X", pbSrc[0], pbSrc[1], pbSrc[2], pbSrc[3]);
	strcpy(pszTarg, szTemp);

	if (bLenSrc > 4)
	{
		sprintf_s(szTemp, sizeof(szTemp), " %02X:%02X:%02X", pbSrc[4], pbSrc[5], pbSrc[6]);
		strcat_s(pszTarg, bLenSrc * 2 + 6, szTemp);
	}
}

void CCSTesterDlg::BCDtoString(LPBYTE pbSrc, BYTE bLenSrc, char * pszTarg)
{
	for (BYTE i=0;i<bLenSrc;i++)
	{
		sprintf(pszTarg + 2 * i, "%02X", pbSrc[i]);
	}
}

void CCSTesterDlg::OnSaveDisp()
{
	// TODO: �ڴ���������������
	CFileDialog dlg(FALSE, "txt", NULL, 4 | 2, "�ı��ļ� (*.txt)|*.txt|�����ļ� (*.*)|*.*||");
	if (dlg.DoModal() == IDOK)
	{
		CString str, strPath;

		strPath = dlg.GetPathName();
		m_rEditOut.GetWindowText(str);

		CStdioFile sfile;
		if (sfile.Open(strPath, CFile::modeCreate | CFile::modeWrite))
		{
			sfile.WriteString(str);
			sfile.Close();
		}
	}
}

void CCSTesterDlg::OnClearDisp()
{
	// TODO: �ڴ���������������
	m_rEditOut.SetWindowText("");
}

void CCSTesterDlg::OnExit()
{
	// TODO: �ڴ���������������
	g_Serial.Close();

	CDialog::OnCancel();
}

void CCSTesterDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	g_Serial.Close();

	CDialog::OnClose();
}

LRESULT CCSTesterDlg::OnAnalyse(WPARAM wparam, LPARAM lparam)
{
	m_pOper->SendMessage(WM_ANALYSE);

	if (g_bDeviceType == 3)
		ShowDialogs(1 << DLG_OPER);

	g_lBalance = g_Analyse.lBalance;

	return 0;
}

LRESULT CCSTesterDlg::OnIncrease(WPARAM wparam, LPARAM lparam)
{
	ShowDialogs(1 << DLG_INC);
	m_pInc->SetOldBalance(g_lBalance);

	return 0;
}

LRESULT CCSTesterDlg::OnDecrease(WPARAM wparam, LPARAM lparam)
{
	ShowDialogs(1 << DLG_DEC);
	m_pDec->SetOldBalance(g_lBalance);

	return 0;
}

LRESULT CCSTesterDlg::OnUpdate(WPARAM wparam, LPARAM lparam)
{
	ShowDialogs(1 << DLG_UPD);
	m_pUpdate->SetUpdInfo(0);

	return 0;
}

LRESULT CCSTesterDlg::OnRefund(WPARAM wparam, LPARAM lparam)
{
	m_pRefund->SetChecker((BOOL)(g_Analyse.dwOperationStauts & (0x01<<5)), (BOOL)(g_Analyse.dwOperationStauts & (0x01<<7)));
	ShowDialogs(1 << DLG_RFD);
	return 0;
}

LRESULT CCSTesterDlg::OnSjtsale(WPARAM wparam, LPARAM lparam)
{
	ShowDialogs(1 << DLG_SSALE);
	return 0;
}

LRESULT CCSTesterDlg::OnSaleEx(WPARAM wparam, LPARAM lparam)
{
	ShowDialogs(1 << DLG_SALEEX);
	return 0;
}

LRESULT CCSTesterDlg::OnSamAct(WPARAM wparam, LPARAM lparam)
{
	ShowDialogs(1 << DLG_SAM_ACT);
	return 0;
}

LRESULT CCSTesterDlg::OnChargeDescind(WPARAM wparam, LPARAM lparam)
{
	ShowDialogs(1 << DLG_DESCIND);
	return 0;
}

LRESULT CCSTesterDlg::OnInitReader(WPARAM wparam, LPARAM lparam)
{
	m_pUpdate->	SetEntryLine();
	m_pPrice->SetStation();

	CMenu * pMenu = GetMenu()->GetSubMenu(2);
	pMenu->EnableMenuItem(IDM_QUERY_FARE, MF_ENABLED);
	pMenu->EnableMenuItem(IDM_PARAM, MF_ENABLED);

	pMenu = GetMenu()->GetSubMenu(5);
	pMenu->EnableMenuItem(IDM_READER_VERSION, MF_ENABLED);

	pMenu = GetMenu()->GetSubMenu(4);
	pMenu->EnableMenuItem(IDM_DEBUG, MF_ENABLED);

	return 0;
}

LRESULT CCSTesterDlg::OnUnInitReader(WPARAM wparam, LPARAM lparam)
{
	CMenu * pMenu = GetMenu()->GetSubMenu(2);
	pMenu->EnableMenuItem(IDM_QUERY_FARE, MF_DISABLED);
	pMenu->EnableMenuItem(IDM_PARAM, MF_DISABLED);

	pMenu = GetMenu()->GetSubMenu(5);
	pMenu->EnableMenuItem(IDM_READER_VERSION, MF_DISABLED);

	pMenu = GetMenu()->GetSubMenu(4);
	pMenu->EnableMenuItem(IDM_DEBUG, MF_DISABLED);

	return 0;
}

LRESULT CCSTesterDlg::OnChangeAnt(WPARAM wparam, LPARAM lparam)
{
	int nAnt = (int)wparam;
	m_pSjtSale->SetAntiMode(nAnt);
	m_pDec->SetAntiMode(nAnt);
	m_pInc->SetAntiMode(nAnt);

	return 0;
}
BOOL CCSTesterDlg::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// TODO: �ڴ����ר�ô����/����û���
	//if (m_wndDlgBar.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	//	return TRUE;
	return CDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CCSTesterDlg::OnCoreUpdt()
{
	// TODO: �ڴ���������������
	ShowDialogs(1 << DLG_CORE);

}

void CCSTesterDlg::OnQueryFare()
{
	// TODO: �ڴ���������������
	ShowDialogs(1 << DLG_PRICE);
}

void CCSTesterDlg::OnParam()
{
	// TODO: �ڴ���������������
	ShowDialogs(1 << DLG_PARAM);
}

void CCSTesterDlg::OnSamInfo()
{
	// TODO: �ڴ���������������
	BYTE bRecv[1024]	= {0};
	SAMSTATUS ss[8]		= {0};
	WORD lenss			= 0;

	CString strDevType;

	QueryPerformanceCounter(&g_liStart);
	g_retInfo = g_Serial.Communicate(0, g_nBeep, 1, 2, NULL, 0, bRecv, sizeof(bRecv));
	QueryPerformanceCounter(&g_liEnd);

	if (g_retInfo.wErrCode == 0)
	{
		memcpy(&lenss, bRecv + 7 + sizeof(RETINFO), 2);

		if (lenss > 0)
		{
			memcpy(&ss, bRecv + 9 + sizeof(RETINFO), lenss);
		}

	}

	((CCSTesterDlg * )AfxGetMainWnd())->DispResult(struct_Sam, &ss, lenss, "��ȡSAM����Ϣ", g_liStart, g_liEnd);
}

void CCSTesterDlg::OnCmd()
{
	// TODO: �ڴ���������������

	m_pPageLast->SendMessage(WM_PAGECHANGED);
	
	CDlgCmd dlg;
	dlg.DoModal();
}

void CCSTesterDlg::OnTestHard()
{
	// TODO: �ڴ���������������


	m_pPageLast->SendMessage(WM_PAGECHANGED);

	CDlgHarder dlg;
	dlg.DoModal();
}

void CCSTesterDlg::OnReaderVersion()
{
	// TODO: �ڴ���������������

	// TODO: �ڴ���������������
	BYTE bRecv[1024]	= {0};
	READERVERSION rv[8]	= {0};
	WORD lenrv			= 0;

	CString strDevType;

	QueryPerformanceCounter(&g_liStart);
	g_retInfo = g_Serial.Communicate(0, g_nBeep, 1, 1, NULL, 0, bRecv, sizeof(bRecv));
	QueryPerformanceCounter(&g_liEnd);

	if (g_retInfo.wErrCode == 0)
	{
		memcpy(&lenrv, bRecv + 7 + sizeof(RETINFO), 2);

		if (lenrv > 0)
		{
			memcpy(&rv, bRecv + 9 + sizeof(RETINFO), lenrv);
		}

	}

	((CCSTesterDlg * )AfxGetMainWnd())->DispResult(struct_Ver, &rv, lenrv, "��ȡ�汾��Ϣ", g_liStart, g_liEnd);

}

void CCSTesterDlg::OnTradeFileExch()
{
	// TODO: �ڴ���������������

	char szFilter[1024] = {0};
	strcat(szFilter, "����Ʊ���� (*.sjt)|*.sjt|");
	strcat(szFilter, "��ֵƱ���� (*.ots)|*.ots|");
	strcat(szFilter, "��բ (*.eng)|*.eng|");
	strcat(szFilter, "Ǯ������ (*.pur)|*.pur|");
	strcat(szFilter, "���� (*.dfr)|*.dfr|");
	strcat(szFilter, "���� (*.upd)|*.upd|");
	strcat(szFilter, "�˿� (*.ref)|*.ref|");
	strcat(szFilter, "�ӽ��� (*.loc)|*.loc|");
	strcat(szFilter, "�����ļ� (*.*)|*.*|");

	CString strFileName, strFolder, strPathName;

	CFileDialog dlg(TRUE, "PRM", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	if (dlg.DoModal() == IDOK)
	{
		strFileName = dlg.GetFileName();
		strPathName = dlg.GetPathName();
		strFolder = strPathName.Left(strPathName.GetLength() - strFileName.GetLength());

		if (CTradeFileConvert::FileConvert(strFileName.GetBuffer(), strPathName.GetBuffer(), strFolder.GetBuffer()))
		{
			AfxMessageBox("ת����ɣ�");
		}
	}
}

void CCSTesterDlg::OnDebug()
{
	// TODO: �ڴ���������������
	ShowDialogs(1 << DLG_DEBUG);
}

void CCSTesterDlg::OnGetFileFromReader()
{
	m_pPageLast->SendMessage(WM_PAGECHANGED);

	CDlgNfs dlg;
	dlg.DoModal();
}

void CCSTesterDlg::OnUpgrade()
{
	// TODO: �ڴ���������������
	BYTE bRecv[1024]	= {0};
	int pos				= 7;

	do 
	{
		QueryPerformanceCounter(&g_liStart);
		g_retInfo = g_Serial.Communicate(0, g_nBeep, 1, 255, NULL, 0, bRecv, sizeof(bRecv));
		QueryPerformanceCounter(&g_liEnd);

		pos += sizeof(RETINFO);

	} while (0);

	((CCSTesterDlg * )AfxGetMainWnd())->DispResult(struct_Null, NULL, 0, "Ʊ����ʽ����", g_liStart, g_liEnd);

}

// ����ȷ��
void CCSTesterDlg::TransConfirm(RETINFO ret)
{
	BYTE bRecv[1024]	= {0};
	int pos				= 7;

	if (ret.bNoticeCode == 0x11)
	{
		CDlgConfirm dlg;
		dlg.DoModal();
	}
}

void CCSTesterDlg::OnEnalbeDebuger()
{
	// TODO: �ڴ���������������
	CMenu * pMenuTools = GetMenu()->GetSubMenu(4);
	CMenu * pMenuHelp = GetMenu()->GetSubMenu(5);

	if (!m_bEnableDebuger)
	{
		CDlgRoot dlg;
		dlg.DoModal();

		EnableRootMenus(m_bEnableDebuger);
	}
}

void CCSTesterDlg::OnSetDegrade()
{
	// TODO: �ڴ���������������
	ShowDialogs(1 << DLG_DEGRADE);
}

void CCSTesterDlg::PostNcDestroy()
{
	// TODO: �ڴ����ר�ô����/����û���

	delete m_pBom;
	delete m_pAgm;
	delete m_pTvm;
	delete m_pTcm;
	delete m_pOper;
	delete m_pInc;
	delete m_pDec;
	delete m_pSjtSale;
	delete m_pRefund;
	delete m_pUpdate;
	delete m_pPrice;
	delete m_pParam;
	delete m_pDebug;
	delete m_pDegrade;
	delete m_pSaleEx;

	CDialog::PostNcDestroy();
}

BOOL CCSTesterDlg::HasRootAuth()
{
	BOOL bRet = FALSE;

	CString strCpuID = CDlgRoot::GetCpuID();

	const CString strPermitID[2] = 
	{
		"BFEBFBFF000306D400C3000076036301",
		"BFEBFBFF000206A700CA000076035A01"
	};

	for (int i=0;i<2;i++)
	{
		if (strCpuID == strPermitID[i])
			bRet = TRUE;
	}

	return bRet;
}

void CCSTesterDlg::EnableRootMenus(BOOL bRoot)
{
	CMenu * pMenuTools = GetMenu()->GetSubMenu(4);
	CMenu * pMenuHelp = GetMenu()->GetSubMenu(5);

	if (bRoot)
	{
		pMenuHelp->CheckMenuItem(IDM_ENALBE_DEBUGER, MF_CHECKED);

		pMenuTools->EnableMenuItem(IDM_CMD, MF_ENABLED);
		pMenuTools->EnableMenuItem(IDM_TEST_HARD, MF_ENABLED);
		pMenuTools->EnableMenuItem(IDM_TRADE_FILE_EXCH, MF_ENABLED);
		pMenuTools->EnableMenuItem(IDM_DEBUG, MF_ENABLED);
		pMenuTools->EnableMenuItem(IDM_UPGRADE, MF_ENABLED);
		pMenuTools->EnableMenuItem(IDM_GETFILE, MF_ENABLED);
		pMenuTools->EnableMenuItem(IDM_SET_DEGRADE, MF_ENABLED);
	}
	else
	{
		pMenuHelp->CheckMenuItem(IDM_ENALBE_DEBUGER, MF_UNCHECKED);
		pMenuTools->EnableMenuItem(IDM_CMD, MF_DISABLED);
		pMenuTools->EnableMenuItem(IDM_TEST_HARD, MF_DISABLED);
		pMenuTools->EnableMenuItem(IDM_TRADE_FILE_EXCH, MF_DISABLED);
		pMenuTools->EnableMenuItem(IDM_DEBUG, MF_DISABLED);
		pMenuTools->EnableMenuItem(IDM_UPGRADE, MF_DISABLED);
		pMenuTools->EnableMenuItem(IDM_GETFILE, MF_DISABLED);
		pMenuTools->EnableMenuItem(IDM_SET_DEGRADE, MF_DISABLED);
	}
}