# Microsoft Developer Studio Project File - Name="zenFolders" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=zenFolders - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zenFolders.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zenFolders.mak" CFG="zenFolders - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zenFolders - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "zenFolders - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/sqba/zenFolders/zenFolders", GGEAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zenFolders - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\release\bin"
# PROP Intermediate_Dir "..\..\release\obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZENFOLDERS_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZENFOLDERS_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 advapi32.lib shell32.lib comctl32.lib version.lib /nologo /dll /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=..\..\release\bin\package.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "zenFolders - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\debug\bin"
# PROP Intermediate_Dir "..\..\debug\obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZENFOLDERS_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZENFOLDERS_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 advapi32.lib shell32.lib comctl32.lib version.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=..\..\debug\bin\package.bat
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "zenFolders - Win32 Release"
# Name "zenFolders - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\browser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cfgxml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\clsfact.cpp
# End Source File
# Begin Source File

SOURCE=.\src\contmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\src\dataobj.cpp
# End Source File
# Begin Source File

SOURCE=.\src\dlglist.cpp
# End Source File
# Begin Source File

SOURCE=.\src\util\dragdrop.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DropSrc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\droptrgt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EnumFE.cpp
# End Source File
# Begin Source File

SOURCE=.\src\enumidl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\extricon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\foldprop.cpp
# End Source File
# Begin Source File

SOURCE=.\src\google\GoogleDS.cpp
# End Source File
# Begin Source File

SOURCE=.\src\icons.cpp
# End Source File
# Begin Source File

SOURCE=.\src\infotip.cpp
# End Source File
# Begin Source File

SOURCE=.\src\listview.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pidl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pidlmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\util\registry.cpp
# End Source File
# Begin Source File

SOURCE=.\src\util\settings.cpp
# End Source File
# Begin Source File

SOURCE=.\src\util\ShellNSE.cpp
# End Source File
# Begin Source File

SOURCE=.\src\shexinit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\shlfldr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\shlview.cpp
# End Source File
# Begin Source File

SOURCE=.\src\shpropsh.cpp
# End Source File
# Begin Source File

SOURCE=.\src\statusbar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\util\string.cpp
# End Source File
# Begin Source File

SOURCE=.\src\toolbar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\viewlist.cpp
# End Source File
# Begin Source File

SOURCE=.\src\zenfolders.cpp
# End Source File
# Begin Source File

SOURCE=.\zenFolders.def
# End Source File
# Begin Source File

SOURCE=.\src\zenfolders.rc
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icons\file.ico
# End Source File
# Begin Source File

SOURCE=.\icons\folder.ico
# End Source File
# Begin Source File

SOURCE=.\icons\folderop.ico
# End Source File
# Begin Source File

SOURCE=.\icons\header.bmp
# End Source File
# Begin Source File

SOURCE=.\icons\zenfolders.ico
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\afxres.h
# End Source File
# Begin Source File

SOURCE=.\src\browser.h
# End Source File
# Begin Source File

SOURCE=.\src\cfgxml.h
# End Source File
# Begin Source File

SOURCE=.\src\clsfact.h
# End Source File
# Begin Source File

SOURCE=.\src\commands.h
# End Source File
# Begin Source File

SOURCE=.\src\contmenu.h
# End Source File
# Begin Source File

SOURCE=.\src\dataobj.h
# End Source File
# Begin Source File

SOURCE=.\src\dlglist.h
# End Source File
# Begin Source File

SOURCE=.\src\util\dragdrop.h
# End Source File
# Begin Source File

SOURCE=.\src\DropSrc.h
# End Source File
# Begin Source File

SOURCE=.\src\droptrgt.h
# End Source File
# Begin Source File

SOURCE=.\src\EnumFE.h
# End Source File
# Begin Source File

SOURCE=.\src\enumidl.h
# End Source File
# Begin Source File

SOURCE=.\src\extricon.h
# End Source File
# Begin Source File

SOURCE=.\src\google\GoogleDS.h
# End Source File
# Begin Source File

SOURCE=.\src\guid.h
# End Source File
# Begin Source File

SOURCE=.\src\icons.h
# End Source File
# Begin Source File

SOURCE=.\src\infotip.h
# End Source File
# Begin Source File

SOURCE=.\src\listview.h
# End Source File
# Begin Source File

SOURCE=.\src\pidl.h
# End Source File
# Begin Source File

SOURCE=.\src\pidlmgr.h
# End Source File
# Begin Source File

SOURCE=.\src\util\registry.h
# End Source File
# Begin Source File

SOURCE=.\src\resource.h
# End Source File
# Begin Source File

SOURCE=.\src\util\settings.h
# End Source File
# Begin Source File

SOURCE=.\src\util\ShellNSE.h
# End Source File
# Begin Source File

SOURCE=.\src\shexinit.h
# End Source File
# Begin Source File

SOURCE=.\src\shlfldr.h
# End Source File
# Begin Source File

SOURCE=.\src\shlview.h
# End Source File
# Begin Source File

SOURCE=.\src\shpropsh.h
# End Source File
# Begin Source File

SOURCE=.\src\statusbar.h
# End Source File
# Begin Source File

SOURCE=.\src\util\string.h
# End Source File
# Begin Source File

SOURCE=.\src\sysicons.h
# End Source File
# Begin Source File

SOURCE=.\src\toolbar.h
# End Source File
# Begin Source File

SOURCE=.\src\util\utilities.h
# End Source File
# Begin Source File

SOURCE=.\src\viewlist.h
# End Source File
# Begin Source File

SOURCE=.\src\util\XmlDOM.h
# End Source File
# Begin Source File

SOURCE=.\src\zenfolders.h
# End Source File
# End Group
# End Target
# End Project
