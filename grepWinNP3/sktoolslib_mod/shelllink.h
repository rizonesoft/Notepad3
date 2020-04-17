// sktoolslib - common files for SK tools

// Copyright (C) 2012 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#pragma once

#include <Shlobj.h>
#include <IntShCut.h>
#include <string>


//Class which contains all the parameters related to shortcut
class CShellLinkInfo
{
public:
//Constructors / Destructors
  CShellLinkInfo();
  CShellLinkInfo(const CShellLinkInfo& sli);
  ~CShellLinkInfo();

//Methods
  CShellLinkInfo& operator=(const CShellLinkInfo& sli);

//Variables
  std::wstring  m_sTarget;
  LPITEMIDLIST  m_pidl;
  std::wstring  m_sArguments;
  std::wstring  m_sDescription;
  WORD          m_wHotkey;
  std::wstring  m_sIconLocation;
  int           m_nIconIndex;
  int           m_nShowCmd;
  std::wstring  m_sWorkingDirectory;
};


//Class which wraps standard shortcuts i.e. IShellLink
class CShellLink
{
public:
//Constructors / Destructors
  CShellLink();
  virtual ~CShellLink();

//Methods
  BOOL Create(const CShellLinkInfo& sli);
  BOOL Load(const std::wstring& sFilename);
  BOOL Save(const std::wstring& sFilename);
  BOOL Resolve(HWND hParentWnd, DWORD dwFlags);

//Accessors
  std::wstring  GetPath() const;
  LPITEMIDLIST  GetPathIDList() const;
  std::wstring  GetArguments() const;
  std::wstring  GetDescription() const;
  WORD          GetHotKey() const;
  std::wstring  GetIconLocation() const;
  int           GetIconLocationIndex() const;
  int           GetShowCommand() const;
  std::wstring  GetWorkingDirectory() const;

//Mutators
  void SetPath(const std::wstring& sPath);
  void SetPathIDList(LPITEMIDLIST pidl);
  void SetArguments(const std::wstring& sArguments);
  void SetDescription(const std::wstring& sDescription);
  void SetHotKey(WORD wHotkey);
  void SetIconLocation(const std::wstring& sIconLocation);
  void SetIconLocationIndex(int nIconIndex);
  void SetShowCommand(int nShowCmd);
  void SetWorkingDirectory(const std::wstring& sWorkingDirectory);

protected:
  BOOL Initialise();
  CShellLinkInfo m_sli;
  IShellLink*    m_psl;
  IPersistFile*  m_ppf;
  BOOL           m_bAttemptedInitialise;
};


//Class which wraps internet shortcuts i.e. IUniformResourceLocator
class CUrlShellLink : public CShellLink
{
public:
//Constructors / Destructors
  CUrlShellLink();
  virtual ~CUrlShellLink();

//Methods
  BOOL Create(const CShellLinkInfo& sli);
  BOOL Load(const std::wstring& sFilename);
  BOOL Save(const std::wstring& sFilename);
  BOOL Invoke(HWND hParentWnd, DWORD dwFlags, const std::wstring& sVerb);

//Following 4 functions just ASSERT if called
  std::wstring GetArguments() const;
  LPITEMIDLIST GetPathIDList() const;
  void SetArguments(const std::wstring& sArguments);
  void SetPathIDList(LPITEMIDLIST pidl);

protected:
  BOOL Initialise();
  IUniformResourceLocator* m_pURL;
};
