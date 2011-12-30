// Sequencer.h : main header file for the SEQUENCER application
//

#if !defined(AFX_SEQUENCER_H__0F63F532_5701_4B1F_8702_C26C555747E4__INCLUDED_)
#define AFX_SEQUENCER_H__0F63F532_5701_4B1F_8702_C26C555747E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CSequencerApp:
// See Sequencer.cpp for the implementation of this class
//

class CSequencerApp : public CWinApp
{
public:
	CSequencerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSequencerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CSequencerApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEQUENCER_H__0F63F532_5701_4B1F_8702_C26C555747E4__INCLUDED_)
