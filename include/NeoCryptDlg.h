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
class Encryptor;
class Decryptor;
class ProgressListener;
#include <list>
#include <string>
#include "Algorithm.h"

extern UINT NCM_AddFiles;
extern LPVOID gShared;

// CNeoCryptDlg dialog
class CNeoCryptDlg : public CDialog
{
	// Construction
public:
	CNeoCryptDlg(LPCTSTR files);	// standard constructor
	~CNeoCryptDlg();

	// Dialog Data
	enum { IDD = IDD_NEOCRYPT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Implementation
protected:
	HICON m_hIcon;
	CImageList m_ImageList;
	Encryptor* p_enc;
	Decryptor* p_dec;
	ProgressListener* p_list;
	ProgressListener* p_progTotal;
	std::list<std::string> m_InitFiles;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnExternalAddFiles(WPARAM wParam, LPARAM lParam);
	void OnDropFiles(HDROP hDropInfo);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnFileExit();
	afx_msg void OnHelpAbout();
	afx_msg void OnOptionsHidePwd();
	afx_msg void OnOptionsShowConf();
	afx_msg void OnBnClickedButAdddir();
	void AddToFileList(const char* name, bool dir);
	afx_msg void OnBnClickedButAddfile();
	afx_msg void OnBnClickedButClear();
	afx_msg void OnBnClickedButRemove();
	afx_msg void OnBnClickedPwdAdd();
	afx_msg void OnBnClickedPwdSub();
	afx_msg void OnCbnSelchangeComboMode();
	afx_msg void OnBnClickedButGo();
	afx_msg void OnBnClickedRadioDecrypt();
	afx_msg void OnBnClickedRadioEncrypt();
	afx_msg void OnBnClickedPwdUpload();
	afx_msg void OnBnClickedPwdClear();
	afx_msg void OnLbnDblclkListPwd();

private:
	static DWORD WINAPI Go(CNeoCryptDlg* me);
	bool ProcessFile(const char* name, bool encrypt, std::list<std::string>& pwd, Algorithm::Type alg);
	void ProcessDir(const char* name, std::list<std::string>& fileList);
	void ParseNames(const char* src, std::list<std::string>& dest);
	void SetStaticPwdMsg(int num);
	bool IsDuplicatePassword(CListBox* pwdList, const CString& newPassword);
	void UpdatePasswordListSelection();
	void GetApplyRegSettings();
	void SaveRegSettings();
};