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

// PwdDialog.cpp : implementation file
//

#include "stdafx.h"
#include "NeoCrypt.h"
#include "PwdDialog.h"

// CPwdDialog dialog

IMPLEMENT_DYNAMIC(CPwdDialog, CDialog)
CPwdDialog::CPwdDialog(bool hide,CWnd* pParent /*=NULL*/)
	: CDialog(CPwdDialog::IDD, pParent), mHidePwd(hide)
{
	mPassword[0] = '\0';
}

CPwdDialog::CPwdDialog(bool hide, CWnd* pParent, const char* initialPassword)
	: CDialog(CPwdDialog::IDD, pParent)
	, mHidePwd(hide)
{
	strncpy_s(mPassword, initialPassword, 256);
	mPassword[256] = '\0';
}

CPwdDialog::~CPwdDialog()
{
}

void CPwdDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPwdDialog, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()

// CPwdDialog message handlers

void CPwdDialog::OnBnClickedOk()
{
	CWnd* pEdit = GetDlgItem(IDC_EDIT1);

	// edit mode
	if (pEdit)
	{
		pEdit->SendMessage(WM_GETTEXT, 257, (LPARAM)mPassword);

		// debug: check password length
		// int rawLength = strlen(mPassword);
		// CString debugMsg;
		// debugMsg.Format(_T("Raw password length: %d"), rawLength);
		// AfxMessageBox(debugMsg);

		CString str(mPassword);
		str.TrimLeft();
		str.TrimRight();

		if (str.GetLength() > 0)
		{
			strncpy_s(mPassword, str.GetString(), 256);
			mPassword[256] = '\0';  // ensure null-termination
			OnOK();
		}
		else
		{
			AfxMessageBox(_T("Password cannot be empty."), MB_ICONWARNING);
			pEdit->SetFocus();
		}
	}
	else
	{
		OnCancel();
	}
}

void CPwdDialog::OnBnClickedCancel()
{
	OnCancel();
}

const char* CPwdDialog::GetPassword()
{
	// debug: get password length
	// int returnedLength = strlen(mPassword);
	// CString debugMsg;
	// debugMsg.Format(_T("Returned password length: %d"), returnedLength);
	// AfxMessageBox(debugMsg);

	return mPassword;
}

BOOL CPwdDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CWnd* hEdit = GetDlgItem(IDC_EDIT1);

	// if edit mode
	if (strlen(mPassword) > 0)
	{
		// set title accordingly
		SetWindowText(_T("View/Edit Password"));
	}

	// input maximum of 256 characters
	hEdit->SendMessage(EM_LIMITTEXT, (WPARAM)256, 0);

	// set the initial password text
	hEdit->SetWindowText(CString(mPassword));

	if (mHidePwd)
	{
		hEdit->SendMessage(EM_SETPASSWORDCHAR, (WPARAM)'*', 0);
	}

	hEdit->SetFocus();

	// place cursor at the end of the text
	int textLength = hEdit->SendMessage(WM_GETTEXTLENGTH, 0, 0);
	hEdit->SendMessage(EM_SETSEL, textLength, textLength);

	// set return to false for manual control
	return FALSE;
}