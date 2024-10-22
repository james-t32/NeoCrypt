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

// NeoCrypt.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "NeoCrypt.h"
#include "NeoCryptDlg.h"
#include "cryptlib.h"
#include <io.h>
#include <list>
#include <string>
#include <sstream>
#include <RegSetting.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT NCM_AddFiles;
LPVOID gShared;

// CNeoCryptApp

BEGIN_MESSAGE_MAP(CNeoCryptApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// CNeoCryptApp construction

CNeoCryptApp::CNeoCryptApp()
{
	// Initialize COM
	CoInitialize(0);

	cryptInit();
}

// The one and only CNeoCryptApp object

CNeoCryptApp theApp;

// CNeoCryptApp initialization

BOOL CNeoCryptApp::InitInstance()
{
	// enable modern (Win 10/11) visual styles for all dialogs and windows
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	CMFCButton::EnableWindowsTheming();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// create shared memory for transfering filenames
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, 10*1024, "neocrypt_shared");
	if (!hMapFile)
	{
		MessageBox(0, "Could not allocate shared memory! Please increase your virtual memory.\n\
			NeoCrypt will now exit.", "Error", MB_ICONERROR);
		return FALSE;
	}

	gShared = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (!gShared)
	{
		MessageBox(0, "Could not map shared memory! Please increase your virtual memory.\n\
			NeoCrypt will now exit.", "Error", MB_ICONERROR);
		return FALSE;
	}

	// create registry tree, if failure, will use defaults
	LONG result = RegSetting::CreateRoot(ErrorHandling::ReturnErrorCode);
	if (result != ERROR_SUCCESS) {
		MessageBox(0, "Could not access the Windows registry. Your settings will not be saved between sessions.", "NeoCrypt", MB_ICONWARNING);	
	}

	NCM_AddFiles = RegisterWindowMessage("NCM_AddFiles");

	CNeoCryptDlg dlg(m_lpCmdLine);
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	UnmapViewOfFile(gShared);
	CloseHandle(hMapFile);

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

CNeoCryptApp::~CNeoCryptApp()
{
	cryptEnd();
}