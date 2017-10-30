#ifndef MUSIC_MATH_INCLUDED
#define MUSIC_MATH_INCLUDED

class StepSize;
class StepsPerMeasureSize;
class MeasuresPerStepSize;
class SecondsPerStepSize;

#include <wx/wx.h>
#include "seqer.h"
#include "sequence-editor.h"

wxString GetNoteNameFromNumber(int note_number);
int GetNoteNumberFromName(wxString note_name);
int GetNoteOctave(int note_number);
int SetNoteOctave(int note_number, int octave);
int MatchNoteOctave(int note_number, int existing_note_number);
int GetNoteChromatic(int note_number);
int SetNoteChromatic(int note_number, int chromatic);
wxString GetKeyNameFromNumber(int key_number, bool is_minor);
int GetChromaticFromDiatonicInKey(int diatonic, int key_number);

/*
 * Zooming in steps per measure mode moves through a series of logical
 * divisions of a measure -- first the factors of the time signature numerator,
 * then powers of two times the time signature numerator.
 *
 * Examples:
 *
 * 4/4: 1, 2, 4, 8, 16, ...
 * 3/4: 1, 3, 6, 12, 24, ...
 * 3/8: 1, 3, 6, 12, 24, ...
 * 6/8: 1, 2, 3, 6, 12, 24, ...
 * 12/8: 1, 2, 3, 4, 6, 12, 24, ...
 * 5/8: 1, 5, 10, 20, ...
 */

int GetFinerMeasureDivision(int existing_measure_division, int time_signature_numerator);
int GetCoarserMeasureDivision(int existing_measure_division, int time_signature_numerator);
int GetFinerOrEqualMeasureDivision(int existing_measure_division, int time_signature_numerator);
int GetCoarserOrEqualMeasureDivision(int existing_measure_division, int time_signature_numerator);
int GetEquivalentMeasureDivision(int source_measure_division, int source_time_signature_numerator, int source_time_signature_denominator, int target_time_signature_numerator, int target_time_signature_denominator);
int GetFinerMeasureMultiple(int existing_measure_multiple);
int GetCoarserMeasureMultiple(int existing_measure_multiple);

class StepSize
{
public:
	virtual ~StepSize() = 0;
	virtual StepSize* ZoomIn() = 0;
	virtual StepSize* ZoomOut() = 0;
	virtual void PopulateDialog(StepSizeDialog* dialog) = 0;
	virtual double GetStepFromTick(long tick) = 0;
	virtual long GetTickFromStep(double step) = 0;
	virtual wxString GetTimeStringFromTick(long tick) = 0;
};

class StepsPerMeasureSize: public StepSize
{
public:
	SequenceEditor* sequence_editor;
	int amount;
	int numerator;
	int denominator;

	StepsPerMeasureSize(SequenceEditor* sequence_editor, int amount, int numerator, int denominator);
	StepsPerMeasureSize(SequenceEditor* sequence_editor);
	~StepsPerMeasureSize();
	StepSize* ZoomIn();
	StepSize* ZoomOut();
	void PopulateDialog(StepSizeDialog* dialog);
	double GetStepFromTick(long tick);
	long GetTickFromStep(double step);
	wxString GetTimeStringFromTick(long tick);
};

class MeasuresPerStepSize: public StepSize
{
public:
	SequenceEditor* sequence_editor;
	int amount;

	MeasuresPerStepSize(SequenceEditor* sequence_editor, int amount);
	~MeasuresPerStepSize();
	StepSize* ZoomIn();
	StepSize* ZoomOut();
	void PopulateDialog(StepSizeDialog* dialog);
	double GetStepFromTick(long tick);
	long GetTickFromStep(double step);
	wxString GetTimeStringFromTick(long tick);
};

class SecondsPerStepSize: public StepSize
{
public:
	SequenceEditor* sequence_editor;
	double amount;

	SecondsPerStepSize(SequenceEditor* sequence_editor, double amount);
	~SecondsPerStepSize();
	StepSize* ZoomIn();
	StepSize* ZoomOut();
	void PopulateDialog(StepSizeDialog* dialog);
	double GetStepFromTick(long tick);
	long GetTickFromStep(double step);
	wxString GetTimeStringFromTick(long tick);
};

#endif
