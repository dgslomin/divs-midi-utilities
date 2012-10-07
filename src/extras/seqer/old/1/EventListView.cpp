// EventListView.cpp : implementation of the CEventListView class
//

#include "stdafx.h"
#include "Sequencer.h"

#include "SequenceDoc.h"
#include "EventListView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEventListView

IMPLEMENT_DYNCREATE(CEventListView, CScrollView)

BEGIN_MESSAGE_MAP(CEventListView, CScrollView)
	//{{AFX_MSG_MAP(CEventListView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEventListView construction/destruction

CEventListView::CEventListView()
{
	// TODO: add construction code here
	this->m_showNoteDuration = true;
}

CEventListView::~CEventListView()
{
}

BOOL CEventListView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CEventListView drawing

void CEventListView::OnDraw(CDC* pDC)
{
	CSequenceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here

	CRect clientRect;
	this->GetClientRect(&clientRect);

	CRect rectClipBox;
	pDC->GetClipBox(&rectClipBox);

	pDC->SelectStockObject(DEFAULT_GUI_FONT);

	TEXTMETRIC textMetrics;
	pDC->GetTextMetrics(&textMetrics);

	this->m_arrRowEvents.RemoveAll();
	this->m_arrColumnWidths.RemoveAll();

	MidiFileMeasureBeat_t pMeasureBeat = MidiFileMeasureBeat_new(1, 1.0);

	for (MidiFileEvent_t pEvent = MidiFile_getFirstEvent(pDoc->m_pMidiFile); pEvent != NULL; pEvent = MidiFileEvent_getNextEventInFile(pEvent))
	{
		if (MidiFileEvent_isNoteStartEvent(pEvent))
		{
			CString s;
			CSize extent;

			this->m_arrRowEvents.Add(pEvent);

			MidiFile_setMeasureBeatFromTick(pDoc->m_pMidiFile, MidiFileEvent_getTick(pEvent), pMeasureBeat);
			s.Format("%d:%06.3f", MidiFileMeasureBeat_getMeasure(pMeasureBeat), MidiFileMeasureBeat_getBeat(pMeasureBeat));
			extent = pDC->GetTextExtent(s);
			if ((this->m_arrColumnWidths.GetUpperBound() <= 0) || ((DWORD)(extent.cx) > this->m_arrColumnWidths[0])) this->m_arrColumnWidths.SetAtGrow(0, extent.cx);

			s.Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(pEvent)));
			extent = pDC->GetTextExtent(s);
			if ((this->m_arrColumnWidths.GetUpperBound() <= 1) || ((DWORD)(extent.cx) > this->m_arrColumnWidths[1])) this->m_arrColumnWidths.SetAtGrow(1, extent.cx);

			s.Format("%d", MidiFileNoteStartEvent_getChannel(pEvent) + 1);
			extent = pDC->GetTextExtent(s);
			if ((this->m_arrColumnWidths.GetUpperBound() <= 2) || ((DWORD)(extent.cx) > this->m_arrColumnWidths[2])) this->m_arrColumnWidths.SetAtGrow(2, extent.cx);

			extent = pDC->GetTextExtent(_T("Note"));
			if ((this->m_arrColumnWidths.GetUpperBound() <= 3) || ((DWORD)(extent.cx) > this->m_arrColumnWidths[3])) this->m_arrColumnWidths.SetAtGrow(3, extent.cx);

			s.Format("%d", MidiFileNoteStartEvent_getNote(pEvent));
			extent = pDC->GetTextExtent(s);
			if ((this->m_arrColumnWidths.GetUpperBound() <= 4) || ((DWORD)(extent.cx) > this->m_arrColumnWidths[4])) this->m_arrColumnWidths.SetAtGrow(4, extent.cx);

			s.Format("%d", MidiFileNoteStartEvent_getVelocity(pEvent));
			extent = pDC->GetTextExtent(s);
			if ((this->m_arrColumnWidths.GetUpperBound() <= 5) || ((DWORD)(extent.cx) > this->m_arrColumnWidths[5])) this->m_arrColumnWidths.SetAtGrow(5, extent.cx);

			if (this->m_showNoteDuration)
			{
				s.Format("%0.3f", MidiFile_getBeatFromTick(pDoc->m_pMidiFile, MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(pEvent))) - MidiFile_getBeatFromTick(pDoc->m_pMidiFile, MidiFileEvent_getTick(pEvent)));
			}
			else
			{
				MidiFile_setMeasureBeatFromTick(pDoc->m_pMidiFile, MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(pEvent)), pMeasureBeat);
				s.Format("%d:%06.3f", MidiFileMeasureBeat_getMeasure(pMeasureBeat), MidiFileMeasureBeat_getBeat(pMeasureBeat));
			}

			extent = pDC->GetTextExtent(s);
			if ((this->m_arrColumnWidths.GetUpperBound() <= 6) || ((DWORD)(extent.cx) > this->m_arrColumnWidths[6])) this->m_arrColumnWidths.SetAtGrow(6, extent.cx);

			s.Format("%d", MidiFileNoteEndEvent_getVelocity(MidiFileNoteStartEvent_getNoteEndEvent(pEvent)));
			extent = pDC->GetTextExtent(s);
			if ((this->m_arrColumnWidths.GetUpperBound() <= 7) || ((DWORD)(extent.cx) > this->m_arrColumnWidths[7])) this->m_arrColumnWidths.SetAtGrow(7, extent.cx);
		}
	}

	long lColumnGap = pDC->GetTextExtent(_T("   ")).cx;

	for (int row_number = 0; row_number < this->m_arrRowEvents.GetUpperBound(); row_number++)
	{
		CRect rect;
		rect.top = clientRect.top + (row_number * (textMetrics.tmHeight + 1));
		rect.bottom = rect.top + textMetrics.tmHeight;

		if ((rect.bottom >= rectClipBox.top) && (rect.top <= rectClipBox.bottom))
		{
			MidiFileEvent_t pEvent = (MidiFileEvent_t)(this->m_arrRowEvents[row_number]);
			CString s;

			MidiFile_setMeasureBeatFromTick(pDoc->m_pMidiFile, MidiFileEvent_getTick(pEvent), pMeasureBeat);
			s.Format("%d:%06.3f", MidiFileMeasureBeat_getMeasure(pMeasureBeat), MidiFileMeasureBeat_getBeat(pMeasureBeat));
			rect.left = 0;
			rect.right = rect.left + this->m_arrColumnWidths[0];
			pDC->DrawText(s, &rect, DT_NOCLIP | DT_RIGHT);

			s.Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(pEvent)));
			rect.left = rect.right + lColumnGap;
			rect.right = rect.left + this->m_arrColumnWidths[1];
			pDC->DrawText(s, &rect, DT_NOCLIP | DT_RIGHT);

			s.Format("%d", MidiFileNoteStartEvent_getChannel(pEvent) + 1);
			rect.left = rect.right + lColumnGap;
			rect.right = rect.left + this->m_arrColumnWidths[2];
			pDC->DrawText(s, &rect, DT_NOCLIP | DT_RIGHT);

			rect.left = rect.right + lColumnGap;
			rect.right = rect.left + this->m_arrColumnWidths[3];
			pDC->DrawText(_T("Note"), &rect, DT_NOCLIP | DT_LEFT);

			s.Format("%d", MidiFileNoteStartEvent_getNote(pEvent));
			rect.left = rect.right + lColumnGap;
			rect.right = rect.left + this->m_arrColumnWidths[4];
			pDC->DrawText(s, &rect, DT_NOCLIP | DT_RIGHT);

			s.Format("%d", MidiFileNoteStartEvent_getVelocity(pEvent));
			rect.left = rect.right + lColumnGap;
			rect.right = rect.left + this->m_arrColumnWidths[5];
			pDC->DrawText(s, &rect, DT_NOCLIP | DT_RIGHT);

			if (this->m_showNoteDuration)
			{
				s.Format("%0.3f", MidiFile_getBeatFromTick(pDoc->m_pMidiFile, MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(pEvent))) - MidiFile_getBeatFromTick(pDoc->m_pMidiFile, MidiFileEvent_getTick(pEvent)));
			}
			else
			{
				MidiFile_setMeasureBeatFromTick(pDoc->m_pMidiFile, MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(pEvent)), pMeasureBeat);
				s.Format("%d:%06.3f", MidiFileMeasureBeat_getMeasure(pMeasureBeat), MidiFileMeasureBeat_getBeat(pMeasureBeat));
			}

			rect.left = rect.right + lColumnGap;
			rect.right = rect.left + this->m_arrColumnWidths[6];
			pDC->DrawText(s, &rect, DT_NOCLIP | DT_RIGHT);

			s.Format("%d", MidiFileNoteEndEvent_getVelocity(MidiFileNoteStartEvent_getNoteEndEvent(pEvent)));
			rect.left = rect.right + lColumnGap;
			rect.right = rect.left + this->m_arrColumnWidths[7];
			pDC->DrawText(s, &rect, DT_NOCLIP | DT_RIGHT);
		}
	}

	MidiFileMeasureBeat_free(pMeasureBeat);

	CSize scrollSizeTotal;
	scrollSizeTotal.cx = 0;
	scrollSizeTotal.cy = row_number * textMetrics.tmHeight;

	CSize scrollSizeRow;
	scrollSizeRow.cx = 0;
	scrollSizeRow.cy = textMetrics.tmHeight;

	this->SetScrollSizes(MM_TEXT, scrollSizeTotal, sizeDefault, scrollSizeRow);
}

void CEventListView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view

	sizeTotal.cx = sizeTotal.cy = 0;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

/////////////////////////////////////////////////////////////////////////////
// CEventListView diagnostics

#ifdef _DEBUG
void CEventListView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CEventListView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CSequenceDoc* CEventListView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSequenceDoc)));
	return (CSequenceDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEventListView message handlers
