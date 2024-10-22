/*
-------------------------------------------------------------------------------
 This source file is part of NeoCrypt (a file encryption software).

 Copyright (C) 2002-2003, Arijit De <arijit1985@yahoo.co.in>

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 http://www.gnu.org/copyleft/gpl.html.
-------------------------------------------------------------------------------
*/

// NeoCryptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NeoCrypt.h"
#include "NeoCryptDlg.h"
#include <list>
#include <string>
#include "shlobj.h"
#include "PwdDialog.h"
#include "Algorithm.h"
#include "Encryptor.h"
#include "Decryptor.h"
#include "ProgressListener.h"
#include "ProgUpdater.h"
#include "Util.h"
#include "RegSetting.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
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

///////////////////////////////////////////////////////////////////////////////
// Array of Modes
struct Mode
{
	char* name;
	Algorithm::Type alg;
	char* desc;
};

Mode gModes[3] = {
	{"AES (Default)",Algorithm::AES,"AES is a 128-bit block cipher with a 128, 192, or 256-bit key."},
	{"DES",Algorithm::DES,"DES is a 64-bit block cipher with a 56-bit key.\nWarning: This is weak!"},
	{"Triple-DES",Algorithm::TRIPLE_DES,"Triple DES is a 64-bit block cipher with a 112 or 168-bit key."}
};

///////////////////////////////////////////////////////////////////////////////
// CNeoCryptDlg dialog

BOOL showConfirmation = TRUE; // Public, default true

CNeoCryptDlg::CNeoCryptDlg(LPCTSTR buf)
	: CDialog(CNeoCryptDlg::IDD, 0), p_enc(0), p_dec(0), p_list(0), p_progTotal(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_ImageList.Create(16, 16, ILC_COLOR8, 2, 2);
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ICON1));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ICON2));

	ParseNames(buf, m_InitFiles);
}

CNeoCryptDlg::~CNeoCryptDlg()
{
	delete p_list;
	delete p_progTotal;
	delete p_enc;
	delete p_dec;
}

void CNeoCryptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNeoCryptDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(NCM_AddFiles, OnExternalAddFiles)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_HELP_ABOUTNEOCRYPT, OnHelpAbout)
	ON_COMMAND(ID_OPTIONS_HIDEPWD, OnOptionsHidePwd)
	ON_COMMAND(ID_OPTIONS_SHOWCONFIRMATION,OnOptionsShowConf)
	ON_BN_CLICKED(IDC_BUT_ADDDIR, OnBnClickedButAdddir)
	ON_BN_CLICKED(IDC_BUT_ADDFILE, OnBnClickedButAddfile)
	ON_BN_CLICKED(IDC_BUT_CLEAR, OnBnClickedButClear)
	ON_BN_CLICKED(IDC_BUT_REMOVE, OnBnClickedButRemove)
	ON_BN_CLICKED(IDC_PWD_ADD, OnBnClickedPwdAdd)
	ON_BN_CLICKED(IDC_PWD_SUB, OnBnClickedPwdSub)
	ON_BN_CLICKED(IDC_PWD_UPLOAD, OnBnClickedPwdUpload)
	ON_BN_CLICKED(IDC_PWD_CLEAR, OnBnClickedPwdClear)
	ON_CBN_SELCHANGE(IDC_COMBO_MODE, OnCbnSelchangeComboMode)
	ON_BN_CLICKED(IDC_BUT_GO, OnBnClickedButGo)
	ON_BN_CLICKED(IDC_RADIO_DECRYPT, OnBnClickedRadioDecrypt)
	ON_BN_CLICKED(IDC_RADIO_ENCRYPT, OnBnClickedRadioEncrypt)
	ON_LBN_DBLCLK(IDC_LIST_PWD, OnLbnDblclkListPwd)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

// CNeoCryptDlg message handlers

BOOL CNeoCryptDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// set image list for ListView
	HWND hlv = GetDlgItem(IDC_LV_FILE)->m_hWnd;
	ListView_SetImageList(hlv, m_ImageList.GetSafeHandle(), LVSIL_SMALL);

	// add ListView columns
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.cx = 280;
	lvc.pszText = "Name";
	lvc.iSubItem = 0;
	ListView_InsertColumn(hlv, 0, &lvc);
	lvc.cx = 100;
	lvc.pszText = "Type";
	lvc.iSubItem = 1;
	ListView_InsertColumn(hlv, 1, &lvc);

	// add init files
	if (m_InitFiles.size())
	{
		std::list<std::string>::iterator it = m_InitFiles.begin();
		std::list<std::string>::iterator end = m_InitFiles.end();

		for (; it != end; ++it)
		{
			DWORD attrib = GetFileAttributes((*it).c_str());
			if (attrib != -1)
			{
				if (attrib & FILE_ATTRIBUTE_DIRECTORY) {
					AddToFileList((*it).c_str(), true);
				}
				else {
					AddToFileList((*it).c_str(), false);
				}
			}
		}
	}

	// check "Encrypt" radio button by default
	CheckDlgButton(IDC_RADIO_ENCRYPT, BST_CHECKED);

	// add modes to dropdown-list
	CWnd* pCombo = GetDlgItem(IDC_COMBO_MODE);
	for (int i = 0; i < 3; ++i)
	{
		pCombo->SendMessage(CB_ADDSTRING, 0, (LPARAM)gModes[i].name);

		// set AES as default
		if (gModes[i].alg == Algorithm::AES)
		{
			pCombo->SendMessage(CB_SETCURSEL, i, 0);
			SendDlgItemMessage(IDC_DESC, WM_SETTEXT, 0, (LPARAM)gModes[i].desc);
		}
	}

	// create progress updater
	p_list = new CProgUpdater(GetDlgItem(IDC_PROG_FILE), GetDlgItem(IDC_PERCENT_FILE));
	p_progTotal = new CProgUpdater(GetDlgItem(IDC_PROG_TOTAL), GetDlgItem(IDC_PERCENT_TOTAL));

	// create encryptor, decryptor
	p_enc = new Encryptor(p_list);
	p_dec = new Decryptor(p_list);

	// get and apply settings from Windows registry
	GetApplyRegSettings();

	// set default password message
	SetStaticPwdMsg(0);

	// allow drag and drop files
	DragAcceptFiles(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CNeoCryptDlg::OnDestroy()
{
	// save settings to Windows registry
	SaveRegSettings();

	// disable drag-and-drop
	DragAcceptFiles(FALSE);

	// kill and cleanp window
	CDialog::OnDestroy();
}

void CNeoCryptDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else {
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNeoCryptDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else {
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNeoCryptDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CNeoCryptDlg::OnFileExit()
{
	DestroyWindow();
}

void CNeoCryptDlg::OnHelpAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

// get/ set options
void CNeoCryptDlg::GetApplyRegSettings()
{
	// get settings from registry
	bool hidePasswords = RegSetting::GetHidePasswords();
	showConfirmation = RegSetting::GetShowConfirmation();

	// update menu UI based on registry settings
	// hiding passwords
	if (hidePasswords)
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_HIDEPWD, MF_CHECKED);
		GetDlgItem(IDC_LIST_PWD)->ShowWindow(FALSE);
		GetDlgItem(IDC_STATIC_PWDMSG)->ShowWindow(TRUE);
	}
	else
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_HIDEPWD, MF_UNCHECKED);
		GetDlgItem(IDC_LIST_PWD)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC_PWDMSG)->ShowWindow(FALSE);
	}

	// showing confirmations
	if (showConfirmation)
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_SHOWCONFIRMATION, MF_CHECKED);
	}
	else
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_SHOWCONFIRMATION, MF_UNCHECKED);
	}
}

void CNeoCryptDlg::SaveRegSettings()
{
	bool hidePW = (GetMenu()->GetMenuState(ID_OPTIONS_HIDEPWD, MF_BYCOMMAND) & MF_CHECKED) != 0;
	bool showConf = (GetMenu()->GetMenuState(ID_OPTIONS_SHOWCONFIRMATION, MF_BYCOMMAND) & MF_CHECKED) != 0;

	RegSetting::SetHidePasswords(hidePW);
	RegSetting::SetShowConfirmation(showConf);
}

void CNeoCryptDlg::OnOptionsHidePwd()
{
	CMenu* menu = GetMenu();

	if (menu->GetMenuState(ID_OPTIONS_HIDEPWD, MF_BYCOMMAND) & MF_CHECKED)
	{
		menu->CheckMenuItem(ID_OPTIONS_HIDEPWD, MF_UNCHECKED);
		GetDlgItem(IDC_LIST_PWD)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC_PWDMSG)->ShowWindow(FALSE);
	}
	else
	{
		menu->CheckMenuItem(ID_OPTIONS_HIDEPWD, MF_CHECKED);
		GetDlgItem(IDC_LIST_PWD)->ShowWindow(FALSE);
		GetDlgItem(IDC_STATIC_PWDMSG)->ShowWindow(TRUE);
	}
}

void CNeoCryptDlg::OnOptionsShowConf()
{
	CMenu* menu = GetMenu();

	if (menu->GetMenuState(ID_OPTIONS_SHOWCONFIRMATION, MF_BYCOMMAND) & MF_CHECKED)
	{
		menu->CheckMenuItem(ID_OPTIONS_SHOWCONFIRMATION, MF_UNCHECKED);
		showConfirmation = FALSE;
	}
	else
	{
		menu->CheckMenuItem(ID_OPTIONS_SHOWCONFIRMATION, MF_CHECKED);
		showConfirmation = TRUE;
	}
}

void CNeoCryptDlg::ParseNames(const char* buf, std::list<std::string>& files)
{
	CString arg(buf);
	if (!arg.IsEmpty())
	{
		arg.TrimLeft();
		arg.TrimRight();
		arg.Insert(arg.GetLength(), ' ');
		int i;

		while ((i = arg.FindOneOf(" \"")) != -1)
		{
			CString file;

			if (arg[i] == '\"')
			{
				if (i != 0)
				{
					file = arg.Left(i);
					file.TrimLeft();
					file.TrimRight();
					arg.Delete(0, i + 1);
				}
				else
				{
					arg.Delete(0, 1);

					int j = arg.FindOneOf("\"");
					if (j == -1)
						break;

					file = arg.Left(j);
					file.TrimLeft();
					file.TrimRight();
					arg.Delete(0, j + 1);
				}
			}
			else
			{
				file = arg.Left(i);
				arg.Delete(0, i + 1);
			}
			arg.TrimLeft();

			if (file.IsEmpty()) {
				continue;
			}
			else if (GetFileAttributes(file) == -1)
			{
				std::stringstream s;
				s << "Cannot access file \"" << file.operator LPCTSTR() << "\". This file will be ignored.";
				MessageBox(s.str().c_str(), "NeoCrypt", MB_ICONWARNING);
			}
			else
			{
				files.push_front(std::string(file.operator LPCTSTR()));
			}
		}
	}
}

LRESULT CNeoCryptDlg::OnExternalAddFiles(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0 && lParam != 0) {
		return FALSE;
	}

	if (strcmp((char*)gShared, "OK") != 0 && GetDlgItem(IDC_BUT_GO)->IsWindowEnabled())
	{
		std::list<std::string> files;
		ParseNames((const char*)gShared, files);

		if (files.size() == 0) {
			return TRUE;
		}

		std::list<std::string>::iterator it = files.begin();
		std::list<std::string>::iterator end = files.end();

		for (; it != end; ++it)
		{
			DWORD attrib = GetFileAttributes((*it).c_str());
			if (attrib != -1)
			{
				if (attrib & FILE_ATTRIBUTE_DIRECTORY) {
					AddToFileList((*it).c_str(), true);
				}
				else {
					AddToFileList((*it).c_str(), false);
				}
			}
		}

		strcpy((char*)gShared, "OK");
		memcpy(((char*)gShared) + 3, (char*)&m_hWnd, 4);
	}
	return FALSE;
}

void CNeoCryptDlg::OnBnClickedButAdddir()
{
	IMalloc* pim;
	SHGetMalloc(&pim);

	ITEMIDLIST* pidl;
	SHGetSpecialFolderLocation(m_hWnd, CSIDL_DRIVES, &pidl);

	char buf[MAX_PATH];
	BROWSEINFO bi;
	bi.pidlRoot = pidl;
	bi.hwndOwner = m_hWnd;
	bi.pszDisplayName = buf;
	bi.lpszTitle = "Select Folder";
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = 0;
	bi.iImage = 0;

	LPITEMIDLIST folder = SHBrowseForFolder(&bi);
	if (!folder) {
		return;
	}

	SHGetPathFromIDListA(folder, buf);

	pim->Free(pidl);
	AddToFileList(buf, true);
}

void CNeoCryptDlg::AddToFileList(const char* name, bool dir)
{
	HWND hlv;
	GetDlgItem(IDC_LV_FILE, &hlv);

	// check if the file/dir is already in the list
	LVFINDINFOA lvfi;
	lvfi.flags = LVFI_STRING;
	lvfi.psz = name;
	if (::SendMessage(hlv, LVM_FINDITEM, (WPARAM)(int)-1, (LPARAM)&lvfi) != -1) {
		return;
	}

	// add name to list
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.pszText = const_cast<LPSTR>(name);
	lvi.iImage = (dir ? 0 : 1);
	ListView_InsertItem(hlv, &lvi);

	lvi.mask = LVIF_TEXT;
	lvi.iItem = 0;
	lvi.iSubItem = 1;

	if (dir) {
		lvi.pszText = "Directory";
	}
	else {
		lvi.pszText = "File";
	}
	
	ListView_SetItem(hlv, &lvi);
}

void CNeoCryptDlg::OnBnClickedButAddfile()
{
	char* filter = "All files (*.*)|*.*||";
	CFileDialog cfd(TRUE, 0, 0, OFN_FILEMUSTEXIST | OFN_ENABLESIZING | OFN_EXPLORER |
		/*OFN_FORCESHOWHIDDEN*/ 0x10000000 | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ALLOWMULTISELECT,
		filter, this);

	char* buf = new char[10 * 1024];
	buf[0] = 0;

	cfd.m_ofn.lpstrFile = buf;
	cfd.m_ofn.nMaxFile = 10 * 1024;

	if (cfd.DoModal() == IDOK)
	{
		POSITION pos = cfd.GetStartPosition();
		CString path;

		do
		{
			path = cfd.GetNextPathName(pos);
			AddToFileList(path, false);
		} while (pos);
	}

	delete[] buf;
}

void CNeoCryptDlg::OnBnClickedButClear()
{
	ListView_DeleteAllItems(GetDlgItem(IDC_LV_FILE)->m_hWnd);
}

void CNeoCryptDlg::OnBnClickedButRemove()
{
	HWND hlv;
	GetDlgItem(IDC_LV_FILE, &hlv);

	int selCount = ListView_GetSelectedCount(hlv);

	if (selCount == 0) {
		return;
	}

	while (selCount > 0)
	{
		int sel = ListView_GetNextItem(hlv, -1, LVNI_SELECTED);
		ListView_DeleteItem(hlv, sel);
		--selCount;
	}
}

bool CNeoCryptDlg::IsDuplicatePassword(CListBox* pwdList, const CString& newPassword)
{
	int count = pwdList->GetCount();
	for (int i = 0; i < count; ++i)
	{
		CString existingPassword;
		pwdList->GetText(i, existingPassword);
		if (existingPassword == newPassword) {
			return true;
		}
	}
	return false;
}

void CNeoCryptDlg::UpdatePasswordListSelection()
{
	// update password list and selection
	CWnd* pwdList = GetDlgItem(IDC_LIST_PWD);
	SetStaticPwdMsg(pwdList->SendMessage(LB_GETCOUNT));
	pwdList->SendMessage(LB_SETSEL, FALSE, -1);
	pwdList->SendMessage(LB_SETSEL, TRUE, 0);
}

void CNeoCryptDlg::OnBnClickedPwdAdd()
{
	bool hide = (GetMenu()->GetMenuState(ID_OPTIONS_HIDEPWD, MF_BYCOMMAND) & MF_CHECKED) != 0;

	CPwdDialog pd(hide, this);
	CListBox* pwdList = (CListBox*)GetDlgItem(IDC_LIST_PWD);

	if (pd.DoModal() == IDOK)
	{
		// debug: add password
		// CString password = pd.GetPassword();
		// CString debugMsg;
		// debugMsg.Format(_T("Password length: %d"), password.GetLength());
		// AfxMessageBox(debugMsg, MB_ICONINFORMATION);

		const char* newPassword = pd.GetPassword();
		CString password(newPassword);

		// check for duplicate password
		if (IsDuplicatePassword(pwdList, password))
		{
			CString msg;
			msg.Format(_T("The password \"%s\" already exists in the list."), password);
			MessageBox(msg, _T("NeoCrypt"), MB_ICONINFORMATION);
			return;
		}

		int result = (int)pwdList->SendMessage(LB_INSERTSTRING, 0, (LPARAM)pd.GetPassword());

		if (result < 0) {
			MessageBox("Error occured while adding password!", "Runtime Error", MB_ICONERROR);
		}
		else
		{
			result = (int)pwdList->SendMessage(LB_GETCOUNT, 0, 0);
			UpdatePasswordListSelection();

			// debug: password added
			// CString addedPassword;
			// pwdList->SendMessage(LB_GETTEXT, 0, (LPARAM)addedPassword.GetBufferSetLength(256));
			// addedPassword.ReleaseBuffer();
			// debugMsg.Format(_T("Added password length: %d"), addedPassword.GetLength());
			// AfxMessageBox(debugMsg, MB_ICONINFORMATION);
		}
	}
}

void CNeoCryptDlg::OnBnClickedPwdSub()
{
	CWnd* pList = GetDlgItem(IDC_LIST_PWD);

	int count = (int)pList->SendMessage(LB_GETCOUNT, 0, 0);
	int* items = new int[count];

	count = (int)pList->SendMessage(LB_GETSELITEMS, count, (LPARAM)items);

	if (count)
	{
		--count;
		while (count >= 0)
		{
			pList->SendMessage(LB_DELETESTRING, items[count], 0);
			--count;
		}
	}
	else {
		pList->SendMessage(LB_RESETCONTENT, 0, 0);
	}

	delete[] items;

	UpdatePasswordListSelection();
}

void CNeoCryptDlg::OnBnClickedPwdUpload()
{
	char* filter = "Supported Files (*.txt;*.csv)|*.txt;*.csv|Text Files (*.txt)|*.txt|CSV Files (*.csv)|*.csv||";

	CFileDialog cfd(TRUE, 0, 0, OFN_FILEMUSTEXIST | OFN_ENABLESIZING | OFN_EXPLORER |
		/*OFN_FORCESHOWHIDDEN*/ 0x10000000 | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ALLOWMULTISELECT,
		filter, this);

	// if file selected
	if (cfd.DoModal() == IDOK)
	{
		// get the selected file's path
		CString filePath = cfd.GetPathName();

		// open the file for reading
		CStdioFile file;
		if (file.Open(filePath, CFile::modeRead | CFile::typeBinary))
		{
			CString line;
			CListBox* pwdList = (CListBox*)GetDlgItem(IDC_LIST_PWD);

			// determine if it's a CSV or TXT file based on file extension
			bool isCSV = (filePath.Right(4).CompareNoCase(".csv") == 0);

			// read the file line by line
			while (file.ReadString(line))
			{
				// trim spaces before processing
				line.TrimLeft();
				line.TrimRight();

				// skip empty lines
				if (line.IsEmpty()) {
					continue;
				}

				// array to store listbox values
				CStringArray values;

				if (isCSV)
				{
					// for CSV files, split the line by commas then add to the array
					Util::splitCSVLine(line, values);
				}
				else
				{
					// for text files, just add each line to the array
					values.Add(line);
				}

				for (int i = 0; i < values.GetCount(); ++i)
				{
					CString password = values[i];
					password.TrimLeft();
					password.TrimRight();

					if (password.GetLength() <= 256)
					{
						if (!IsDuplicatePassword(pwdList, password))
						{
							// add each CSV value as a new row in the listbox
							int result = pwdList->InsertString(0, password);
							if (result < 0)
							{
								MessageBox("Error occurred while adding value to the list!", "Runtime Error", MB_ICONERROR);
								break;
							}
						}
						else
						{
							// duplicate password
							CString msg;
							msg.Format(_T("Duplicate password found and skipped: %s"), password);
							MessageBox(msg, "NeoCrypt", MB_ICONINFORMATION);
						}
					}
					else
					{
						// error when the password is too long
						CString msg;
						msg.Format("Password exceeds 256 characters and will not be added. Length: %d", password.GetLength());
						MessageBox(msg, "NeoCrypt", MB_ICONWARNING);
					}
				}
			}
			file.Close();
			UpdatePasswordListSelection();
		}
		else
		{
			MessageBox("Unable to open the file!", "NeoCrypt", MB_ICONERROR);
		}
	}
}

void CNeoCryptDlg::OnBnClickedPwdClear()
{
	CListBox* pwdList = (CListBox*)GetDlgItem(IDC_LIST_PWD);
	if (pwdList)
	{
		// clear all items in the list box
		pwdList->ResetContent();

		// update the password count and list selection
		SetStaticPwdMsg(0);
		UpdatePasswordListSelection();
	}
	else
	{
		MessageBox("Unable to update password list", "Runtime Error", MB_ICONERROR);
	}
}

void CNeoCryptDlg::SetStaticPwdMsg(int num)
{
	char buf[128];
	sprintf_s(buf, "%d password(s) are hidden.\n(Options -> Hide Passwords)", num);
	GetDlgItem(IDC_STATIC_PWDMSG)->SendMessage(WM_SETTEXT, 0, (LPARAM)buf);
}

void CNeoCryptDlg::OnLbnDblclkListPwd()
{
	// check if passwords hidden
	UINT menuState = GetMenu()->GetMenuState(ID_OPTIONS_HIDEPWD, MF_BYCOMMAND);
	bool passwordsHidden = (menuState & MF_CHECKED) != 0;

	// only allow password editing when passwords not hidden
	if (!passwordsHidden)
	{
		CListBox* pwdList = (CListBox*)GetDlgItem(IDC_LIST_PWD);
		int selectedIndex = pwdList->GetCurSel();
		if (selectedIndex != LB_ERR)
		{
			char selectedPassword[257];					// 256 characters + null terminator
			pwdList->GetText(selectedIndex, selectedPassword);

			bool hide = (GetMenu()->GetMenuState(ID_OPTIONS_HIDEPWD, MF_BYCOMMAND) & MF_CHECKED) != 0;
			CPwdDialog pd(hide, this, selectedPassword);

			if (pd.DoModal() == IDOK)
			{
				const char* newPassword = pd.GetPassword();
				if (strcmp(newPassword, selectedPassword) != 0)
				{
					// check if the new password is a duplicate (excluding the current entry)
					bool isDuplicate = false;
					int count = pwdList->GetCount();

					for (int i = 0; i < count; ++i)
					{
						if (i != selectedIndex)
						{
							CString existingPassword;
							pwdList->GetText(i, existingPassword);
							if (existingPassword == newPassword)
							{
								isDuplicate = true;
								break;
							}
						}
					}

					if (!isDuplicate)
					{
						// password was changed, update the existing entry
						pwdList->DeleteString(selectedIndex);
						int newIndex = pwdList->InsertString(selectedIndex, newPassword);

						if (newIndex != LB_ERR)
						{
							// select list item
							pwdList->SendMessage(LB_SETSEL, FALSE, -1);
							pwdList->SendMessage(LB_SETSEL, TRUE, newIndex);

							// debug: password list udpated
							// CString debugMsg;
							// debugMsg.Format(_T("Updated password at index %d. New length: %d"), newIndex, strlen(newPassword));
							// AfxMessageBox(debugMsg, MB_ICONINFORMATION);
						}
						else
						{
							MessageBox("Error occurred while updating password!", "Runtime Error", MB_ICONERROR);
							
						}
					}
					else
					{
						// dupliate password
						MessageBox("This password already exists in the list.", "NeoCrypt", MB_ICONWARNING);
					}
				}
				else
				{
					// password was not changed
					// AfxMessageBox(_T("Password was not modified."), MB_ICONINFORMATION);
				}
			}
		}
	}
}

void CNeoCryptDlg::OnCbnSelchangeComboMode()
{
	int sel = (int)SendDlgItemMessage(IDC_COMBO_MODE, CB_GETCURSEL, 0, 0);

	if (sel == CB_ERR) {
		return;
	}

	SendDlgItemMessage(IDC_DESC, WM_SETTEXT, 0, (LPARAM)gModes[sel].desc);
}

void CNeoCryptDlg::OnBnClickedButGo()
{
	GetDlgItem(IDC_BUT_GO)->EnableWindow(FALSE);

	DWORD threadId;
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&CNeoCryptDlg::Go, this, 0, &threadId);
}

DWORD WINAPI CNeoCryptDlg::Go(CNeoCryptDlg* me)
{
	// get operation mode
	bool encrypt;

	if (me->IsDlgButtonChecked(IDC_RADIO_ENCRYPT) == BST_CHECKED) {
		encrypt = true;
	}
	else if (me->IsDlgButtonChecked(IDC_RADIO_DECRYPT) == BST_CHECKED) {
		encrypt = false;
	}
	else
	{
		me->MessageBox("None of the Action radio buttons are checked!", "Logic Error", MB_ICONERROR);

		me->GetDlgItem(IDC_BUT_GO)->EnableWindow(TRUE);
		return 1;
	}

	// get algorithm type
	Algorithm::Type algType = gModes[(int)me->SendDlgItemMessage(IDC_COMBO_MODE, CB_GETCURSEL, 0, 0)].alg;

	// check if encrypting with DES
	if (encrypt && algType == Algorithm::DES)
	{
		// display an error message
		me->MessageBox("Encrypting with the DES algorithm is no longer supported.\nPlease use AES or Triple-DES.", "NeoCrypt", MB_ICONWARNING);
		me->GetDlgItem(IDC_BUT_GO)->EnableWindow(TRUE);
		return 1;
	}

	// get the password list
	std::list<std::string> pwd;
	CWnd* pList = me->GetDlgItem(IDC_LIST_PWD);
	int count = pList->SendMessage(LB_GETCOUNT, 0, 0);
	if (!count)
	{
		me->MessageBox("No password(s) given! Please add one or more passwords first.", "NeoCrypt", MB_ICONWARNING);

		me->GetDlgItem(IDC_BUT_GO)->EnableWindow(TRUE);
		return 1;
	}

	int i;
	for (i = 0; i < count; ++i)
	{
		// maximum 256 charater password, plus pointers, initialized with zero value
		char pass[257] = { 0 };
		pList->SendMessage(LB_GETTEXT, (WPARAM)i, (LPARAM)pass);

		pwd.push_front(pass);
	}

	// enumerate file list items
	CWnd* pLv = me->GetDlgItem(IDC_LV_FILE);
	count = pLv->SendMessage(LVM_GETITEMCOUNT, 0, 0);

	if (count == 0)
	{
		me->MessageBox("No files selected! Please add the files/directories you want to process.", "NeoCrypt", MB_ICONWARNING);

		me->GetDlgItem(IDC_BUT_GO)->EnableWindow(TRUE);
		return 1;
	}

	// listview item initilized with zero value
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_TEXT | LVIF_IMAGE;

	char buf[300];
	std::list<std::string> mFiles;

	// generate file list
	for (lvi.iItem = 0; lvi.iItem < count; ++lvi.iItem)
	{
		// name
		lvi.iSubItem = 0;
		lvi.pszText = buf;
		lvi.cchTextMax = 256;			// currently supports only 256 character file name = windows default
		if (!pLv->SendMessage(LVM_GETITEM, 0, (LPARAM)&lvi))
			continue;

		if (lvi.iImage == 0) {
			// item is a directory
			me->ProcessDir(buf, mFiles);
		}
		else {
			// item is a file
			mFiles.push_front(std::string(buf));
		}
	}

	if (mFiles.size() == 0)
	{
		me->MessageBox("The folder(s) selected do not contain any files!", "NeoCrypt", MB_ICONWARNING);

		me->GetDlgItem(IDC_BUT_GO)->EnableWindow(TRUE);
		return 1;
	}

	me->p_progTotal->setRange(mFiles.size());

	// keep track of processed files
	size_t successCount = 0;
	size_t failCount = 0;

	// process each file in list
	std::list<std::string>::iterator it;
	std::list<std::string>::iterator end = mFiles.end();

	for (it = mFiles.begin(); it != end; ++it)
	{
		me->GetDlgItem(IDC_STATIC_FILE)->SendMessage(WM_SETTEXT, 0, (LPARAM)it->c_str());

		// update processed file count
		if (me->ProcessFile(it->c_str(), encrypt, pwd, algType)) {
			successCount++;
		}
		else {
			failCount++;
		}

		me->p_progTotal->increment(1);
	}

	pLv->SendMessage(LVM_DELETEALLITEMS, 0, 0);
	me->p_progTotal->complete();

	if (failCount == 0)
	{
		// show confirmation message pop-up when enabled
		if (showConfirmation)
		{
			sprintf_s(buf, "%zd of %zd file(s) processed successfully!", successCount, mFiles.size());
			me->MessageBox(buf, "NeoCrypt", MB_OK | MB_ICONINFORMATION);
		}
	}
	else
	{
		// always show errors
		sprintf_s(buf, "%zd of %zd file(s) processed successfully.\n%zd file(s) failed.", successCount, mFiles.size(), failCount);
		me->MessageBox(buf, "NeoCrypt", MB_OK | MB_ICONWARNING);
	}

	me->p_list->reset();
	me->p_progTotal->reset();
	me->GetDlgItem(IDC_BUT_GO)->EnableWindow(TRUE);
	me->GetDlgItem(IDC_STATIC_FILE)->SendMessage(WM_SETTEXT, 0, (LPARAM)"Ready.");

	return 0;
}

void CNeoCryptDlg::ProcessDir(const char* name, std::list<std::string>& fileList)
{
	// We will recurse through the directory and its sub-directories
	// and add all files to our list
	WIN32_FIND_DATA wfd;
	char mask[MAX_PATH];

	sprintf_s(mask, "%s\\*", name);

	HANDLE hFind = FindFirstFile(mask, &wfd);
	bool found;

	if (hFind == INVALID_HANDLE_VALUE) {
		found = false;
	}
	else {
		found = true;
	}

	while (found)
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strcmp(wfd.cFileName, ".") && strcmp(wfd.cFileName, ".."))
			{
				char dirpath[MAX_PATH];
				sprintf_s(dirpath, "%s\\%s", name, wfd.cFileName);

				ProcessDir(dirpath, fileList);
			}
		}
		else
		{
			char filepath[MAX_PATH];
			sprintf_s(filepath, "%s\\%s", name, wfd.cFileName);

			fileList.push_front(std::string(filepath));
		}

		if (FindNextFile(hFind, &wfd)) {
			found = true;
		}
		else {
			found = false;
		}
	}
}

bool CNeoCryptDlg::ProcessFile(const char* name, bool encrypt, std::list<std::string>& pwd, Algorithm::Type alg)
{
	try
	{
		if (encrypt) {
			p_enc->encrypt(name, pwd, alg);
		}
		else {
			p_dec->decrypt(name, pwd);
		}
	}
	catch (std::logic_error& e)
	{
		MessageBox(e.what(), "Logic Error", MB_ICONERROR);
		return false;
	}
	catch (std::runtime_error& e)
	{
		MessageBox(e.what(), "Runtime Error", MB_ICONWARNING);
		return false;
	}

	return true;
}

void CNeoCryptDlg::OnBnClickedRadioDecrypt()
{
	if (IsDlgButtonChecked(IDC_RADIO_DECRYPT)) {
		GetDlgItem(IDC_COMBO_MODE)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_COMBO_MODE)->EnableWindow(TRUE);
	}
}

void CNeoCryptDlg::OnBnClickedRadioEncrypt()
{
	if (IsDlgButtonChecked(IDC_RADIO_DECRYPT)) {
		GetDlgItem(IDC_COMBO_MODE)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_COMBO_MODE)->EnableWindow(TRUE);
	}
}

void CNeoCryptDlg::OnDropFiles(HDROP hDropInfo)
{
	CHAR filePath[MAX_PATH];

	// get number of files dropped
	UINT fileCount = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

	if (fileCount > 0)
	{
		// debug: get the first file's path
		// DragQueryFile(hDropInfo, 0, filePath, MAX_PATH);
		// debug: display the file path in a message box
		// MessageBox(filePath, _T("File Dropped"), MB_OK);

		// loop through all dropped files
		for (UINT i = 0; i < fileCount; ++i)
		{
			// get the file path for each dropped file
			DragQueryFile(hDropInfo, i, filePath, MAX_PATH);

			// check if it's a folder or a file
			DWORD fileAttributes = GetFileAttributes(filePath);
			if (fileAttributes != INVALID_FILE_ATTRIBUTES)
			{
				if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					// it's a folder
					// MessageBox(filePath, _T("Folder Dropped"), MB_OK);
					AddToFileList(filePath, true);
				}
				else
				{
					// it's a file
					// MessageBox(filePath, _T("File Dropped"), MB_OK);
					AddToFileList(filePath, false);
				}
			}
			else
			{
				MessageBox(_T("Error retrieving file attributes"), _T("Error"), MB_OK);
			}
		}
	}

	// release resources for the drop handle
	DragFinish(hDropInfo);
}