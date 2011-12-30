
// SequenceDoc.h : interface of the CSequenceDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEQUENCEDOC_H__B880163B_7665_40F7_B3B6_15C746F22417__INCLUDED_)
#define AFX_SEQUENCEDOC_H__B880163B_7665_40F7_B3B6_15C746F22417__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <midifile.h>

class CSequenceDoc : public CDocument
{
protected: // create from serialization only
	CSequenceDoc();
	DECLARE_DYNCREATE(CSequenceDoc)

// Attributes
public:
	MidiFile_t m_pMidiFile;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSequenceDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSequenceDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSequenceDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEQUENCEDOC_H__B880163B_7665_40F7_B3B6_15C746F22417__INCLUDED_)
