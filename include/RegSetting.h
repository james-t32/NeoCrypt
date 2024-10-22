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

// Enum for error handling options
enum class ErrorHandling
{
    Silent,
    ThrowException,
    ReturnErrorCode
};


class RegSetting
{
public:
    static LONG CreateRoot(ErrorHandling errorHandling = ErrorHandling::Silent);
    static bool GetHidePasswords();
    static void SetHidePasswords(bool value);
    static bool GetShowConfirmation();
    static void SetShowConfirmation(bool value);

private:
    static const char* GetRegistryPath() { return "Software\\NeoCrypt\\1.4"; }
    static DWORD ReadRegistry(const char* name, DWORD defaultValue);
    static void WriteRegistry(const char* name, DWORD value);
};