# Microsoft Developer Studio Project File - Name="NeoCrypt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=NeoCrypt - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Neocrypt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Neocrypt.mak" CFG="NeoCrypt - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NeoCrypt - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "NeoCrypt - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NeoCrypt - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "bin\release"
# PROP BASE Intermediate_Dir "obj\release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bin\release"
# PROP Intermediate_Dir "obj\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\include" /I "c:\cryptlib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 cl32.lib /nologo /subsystem:windows /machine:I386 /libpath:"C:\projects\cryptlib\binaries32_vs"

!ELSEIF  "$(CFG)" == "NeoCrypt - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bin\debug"
# PROP BASE Intermediate_Dir "obj\debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bin\debug"
# PROP Intermediate_Dir "obj\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\include" /I "c:\cryptlib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 cl32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"C:\projects\cryptlib\binaries32_vs"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "NeoCrypt - Win32 Release"
# Name "NeoCrypt - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\Decryptor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Encryptor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\File.cpp
# End Source File
# Begin Source File

SOURCE=.\src\NeoCrypt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\NeoCryptDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PwdDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Stdafx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\Algorithm.h
# End Source File
# Begin Source File

SOURCE=.\include\Decryptor.h
# End Source File
# Begin Source File

SOURCE=.\include\Encryptor.h
# End Source File
# Begin Source File

SOURCE=.\include\File.h
# End Source File
# Begin Source File

SOURCE=.\include\NeoCrypt.h
# End Source File
# Begin Source File

SOURCE=.\include\NeoCryptDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\ProgressListener.h
# End Source File
# Begin Source File

SOURCE=.\include\PwdDialog.h
# End Source File
# Begin Source File

SOURCE=.\include\Resource.h
# End Source File
# Begin Source File

SOURCE=.\include\Stdafx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Res\Clip04.ico
# End Source File
# Begin Source File

SOURCE=.\Res\Clsdfold.ico
# End Source File
# Begin Source File

SOURCE=.\Res\Logo.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\NeoCrypt.ico
# End Source File
# Begin Source File

SOURCE=.\NeoCrypt.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\Res\NeoCrypt.manifest
# End Source File
# End Target
# End Project
