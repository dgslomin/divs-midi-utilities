// SequenceDoc.cpp : implementation of the CSequenceDoc class
//

#include "stdafx.h"
#include "Sequencer.h"

#include "SequenceDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSequenceDoc

IMPLEMENT_DYNCREATE(CSequenceDoc, CDocument)

BEGIN_MESSAGE_MAP(CSequenceDoc, CDocument)
	//{{AFX_MSG_MAP(CSequenceDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSequenceDoc construction/destruction

CSequenceDoc::CSequenceDoc()
{
	// TODO: add one-time construction code here
	this->m_pMidiFile = NULL;
}

CSequenceDoc::~CSequenceDoc()
{
	if (this->m_pMidiFile != NULL) MidiFile_free(this->m_pMidiFile);
}

BOOL CSequenceDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	if (this->m_pMidiFile != NULL)
	{
		MidiFile_free(this->m_pMidiFile);
		this->m_pMidiFile = NULL;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSequenceDoc diagnostics

#ifdef _DEBUG
void CSequenceDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSequenceDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSequenceDoc commands



BOOL CSequenceDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	this->m_pMidiFile = MidiFile_load((char *)(lpszPathName));
	return (this->m_pMidiFile != NULL);
}

BOOL CSequenceDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	return (MidiFile_save(this->m_pMidiFile, lpszPathName) == 0);
}
