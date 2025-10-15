
// AppChat_2.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CAppChat2App:
// See AppChat_2.cpp for the implementation of this class
//

class CAppChat2App : public CWinApp
{
public:
	CAppChat2App();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CAppChat2App theApp;
