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

#pragma once


// CPwdDialog dialog

class CPwdDialog : public CDialog
{
	DECLARE_DYNAMIC(CPwdDialog)

public:
	// standard constructor (new password)
	CPwdDialog(bool hide,CWnd* pParent = NULL);

	// additional constructor (view existing password)
	CPwdDialog(bool hide, CWnd* pParent, const char* initialPassword);

	virtual ~CPwdDialog();

// Dialog Data
	enum { IDD = IDD_PWD_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

private:
	char mPassword[257];								// 256 characters plus null terminator

public:
	bool mHidePwd;
	const char* GetPassword();
	virtual BOOL OnInitDialog();
};