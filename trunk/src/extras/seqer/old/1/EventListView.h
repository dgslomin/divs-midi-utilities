// EventListView.h : interface of the CEventListView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENTLISTVIEW_H__79013FC6_C7CA_48D2_AFCF_042143187A57__INCLUDED_)
#define AFX_EVENTLISTVIEW_H__79013FC6_C7CA_48D2_AFCF_042143187A57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <midifile.h>

class CEventListView : public CScrollView
{
protected: // create from serialization only
	CEventListView();
	DECLARE_DYNCREATE(CEventListView)

// Attributes
public:
	CSequenceDoc* GetDocument();
	CPtrArray m_arrRowEvents;
	CDWordArray m_arrColumnWidths;
	bool m_showNoteDuration;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEventListView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEventListView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CEventListView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in EventListView.cpp
inline CSequenceDoc* CEventListView::GetDocument()
   { return (CSequenceDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EVENTLISTVIEW_H__79013FC6_C7CA_48D2_AFCF_042143187A57__INCLUDED_)
