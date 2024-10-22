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

#include "stdafx.h"
#include "RegSetting.h"
#include <winreg.h>
#include <stdexcept>
#include <string>

LONG RegSetting::CreateRoot(ErrorHandling errorHandling)
{
    HKEY hKey = NULL;
    LONG result = ERROR_SUCCESS;
    DWORD disposition;

    try
    {
        // first, try to open the key
        result = RegOpenKeyEx(HKEY_CURRENT_USER, GetRegistryPath(), 0, KEY_READ, &hKey);

        if (result == ERROR_SUCCESS)
        {
            // key already exists, no need to create
            RegCloseKey(hKey);
            return ERROR_SUCCESS;
        }

        // key doesn't exist, create it
        result = RegCreateKeyEx(HKEY_CURRENT_USER, GetRegistryPath(), 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
            &hKey, &disposition);

        if (result != ERROR_SUCCESS) {
            throw std::runtime_error("Failed to create HKCU\Software\NeoCrypt registry key");
        }

        RegCloseKey(hKey);
    }
    catch (const std::exception& e)
    {
        if (hKey != NULL) {
            RegCloseKey(hKey);
        }

        switch (errorHandling)
        {
        case ErrorHandling::Silent:
            // Do nothing, silently fail
            break;
        case ErrorHandling::ThrowException:
            throw; // Re-throw the caught exception
        case ErrorHandling::ReturnErrorCode:
            // Return the Windows error code
            return result;
        }
    }

    return ERROR_SUCCESS;
}

bool RegSetting::GetHidePasswords()
{
    const DWORD defaultValue = 1;  // default true

    DWORD value = ReadRegistry("HidePasswords", defaultValue);
    return value != 0;
}

void RegSetting::SetHidePasswords(bool value)
{
    WriteRegistry("HidePasswords", value ? 1 : 0);
}

bool RegSetting::GetShowConfirmation()
{
    const DWORD defaultValue = 1;  // default true

    DWORD value = ReadRegistry("ShowConfirmation", defaultValue);
    return value != 0;
}

void RegSetting::SetShowConfirmation(bool value)
{
    WriteRegistry("ShowConfirmation", value ? 1 : 0);
}

DWORD RegSetting::ReadRegistry(const char* name, DWORD defaultValue)
{
	DWORD value = defaultValue;
	HKEY hKey = NULL;

	LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, GetRegistryPath(), 0, KEY_QUERY_VALUE, &hKey);
	if (result == ERROR_SUCCESS)
	{
		DWORD valType;
		DWORD valSize = sizeof(DWORD);

		result = RegQueryValueEx(hKey, name, 0, &valType, (BYTE*)&value, &valSize);
		if (result != ERROR_SUCCESS)
		{
			// if the value doesn't exist or there's an error, use the default
			value = defaultValue;
		}
		else if (valType != REG_DWORD)
		{
			// if the value is not a DWORD, use the default
			value = defaultValue;
		}

		RegCloseKey(hKey);
	}

	return value;
}

void RegSetting::WriteRegistry(const char* name, DWORD value)
{
	HKEY hKey = NULL;

	LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, GetRegistryPath(), 0, 0, 0, KEY_SET_VALUE, 0, &hKey, 0);
	if (result == ERROR_SUCCESS)
	{
		RegSetValueEx(hKey, name, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
		RegCloseKey(hKey);
	}
}