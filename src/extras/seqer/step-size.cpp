
#include <wx/wx.h>
#include <midifile.h>
#include "seqer.h"
#include "sequence-editor.h"
#include "step-size.h"

int GetFinerMeasureDivision(int existing_measure_division, int time_signature_numerator)
{
	if (existing_measure_division < time_signature_numerator)
	{
		for (int finer_measure_division = existing_measure_division + 1; true; finer_measure_division++)
		{
			if (time_signature_numerator % finer_measure_division == 0) return finer_measure_division;
		}
	}
	else
	{
		for (int power_of_two = 1; true; power_of_two++)
		{
			int finer_measure_division = time_signature_numerator << power_of_two;
			if (finer_measure_division > existing_measure_division) return finer_measure_division;
		}
	}
}

int GetCoarserMeasureDivision(int existing_measure_division, int time_signature_numerator)
{
	if (existing_measure_division <= time_signature_numerator)
	{
		for (int coarser_measure_division = existing_measure_division - 1; true; coarser_measure_division--)
		{
			if (time_signature_numerator % coarser_measure_division == 0) return coarser_measure_division;
		}
	}
	else
	{
		for (int power_of_two = 1; true; power_of_two++)
		{
			int coarser_measure_division = time_signature_numerator << power_of_two;
			if (coarser_measure_division >= existing_measure_division) return coarser_measure_division >> 1;
		}
	}
}

int GetFinerOrEqualMeasureDivision(int existing_measure_division, int time_signature_numerator)
{
	int finer_measure_division = GetFinerMeasureDivision(existing_measure_division, time_signature_numerator);
	int coarser_measure_division = GetCoarserMeasureDivision(finer_measure_division, time_signature_numerator);
	return (existing_measure_division == coarser_measure_division) ? existing_measure_division : finer_measure_division;
}

int GetCoarserOrEqualMeasureDivision(int existing_measure_division, int time_signature_numerator)
{
	int coarser_measure_division = GetCoarserMeasureDivision(existing_measure_division, time_signature_numerator);
	int finer_measure_division = GetFinerMeasureDivision(coarser_measure_division, time_signature_numerator);
	return (existing_measure_division == finer_measure_division) ? existing_measure_division : coarser_measure_division;
}

int GetEquivalentMeasureDivision(int source_measure_division, int source_time_signature_numerator, int source_time_signature_denominator, int target_time_signature_numerator, int target_time_signature_denominator)
{
	if ((source_time_signature_numerator == target_time_signature_numerator) && (source_time_signature_denominator == target_time_signature_denominator)) return source_measure_division;
	double raw_target_measure_division = (double)(source_measure_division * target_time_signature_numerator * source_time_signature_denominator) / (double)(source_time_signature_numerator * target_time_signature_denominator);
	int coarser_target_measure_division = GetCoarserOrEqualMeasureDivision((int)(raw_target_measure_division), target_time_signature_numerator);
	int finer_target_measure_division = GetFinerMeasureDivision(coarser_target_measure_division, target_time_signature_numerator);
	return ((double)(finer_target_measure_division) - raw_target_measure_division < raw_target_measure_division - (double)(coarser_target_measure_division)) ? finer_target_measure_division : coarser_target_measure_division;
}

int GetFinerMeasureMultiple(int existing_measure_multiple)
{
	for (int power_of_two = 1; true; power_of_two++)
	{
		int finer_measure_multiple = 1 << power_of_two;
		if (finer_measure_multiple >= existing_measure_multiple) return finer_measure_multiple >> 1;
	}
}

int GetCoarserMeasureMultiple(int existing_measure_multiple)
{
	for (int power_of_two = 1; true; power_of_two++)
	{
		int coarser_measure_multiple = 1 << power_of_two;
		if (coarser_measure_multiple > existing_measure_multiple) return coarser_measure_multiple;
	}
}

StepSize::~StepSize()
{
}

StepsPerMeasureSize::StepsPerMeasureSize(SequenceEditor* sequence_editor, int amount, int numerator, int denominator)
{
	this->sequence_editor = sequence_editor;
	this->amount = amount;
	this->numerator = numerator;
	this->denominator = denominator;
}

StepsPerMeasureSize::StepsPerMeasureSize(SequenceEditor* sequence_editor)
{
	this->sequence_editor = sequence_editor;
	MidiFileEvent_t time_signature_event = MidiFile_getLatestTimeSignatureEventForTick(this->sequence_editor->sequence->midi_file, 0);
	this->amount = MidiFileTimeSignatureEvent_getNumerator(time_signature_event);
	this->numerator = MidiFileTimeSignatureEvent_getNumerator(time_signature_event);
	this->denominator = MidiFileTimeSignatureEvent_getDenominator(time_signature_event);
}

StepsPerMeasureSize::~StepsPerMeasureSize()
{
}

StepSize* StepsPerMeasureSize::ZoomIn()
{
	MidiFileEvent_t time_signature_event = this->sequence_editor->GetLatestTimeSignatureEventForRowNumber(this->sequence_editor->current_row_number);
	int current_amount = GetEquivalentMeasureDivision(this->amount, this->numerator, this->denominator, MidiFileTimeSignatureEvent_getNumerator(time_signature_event), MidiFileTimeSignatureEvent_getDenominator(time_signature_event));
	int new_amount = GetFinerMeasureDivision(current_amount, MidiFileTimeSignatureEvent_getNumerator(time_signature_event));
	return new StepsPerMeasureSize(this->sequence_editor, new_amount, MidiFileTimeSignatureEvent_getNumerator(time_signature_event), MidiFileTimeSignatureEvent_getDenominator(time_signature_event));
}

StepSize* StepsPerMeasureSize::ZoomOut()
{
	MidiFileEvent_t time_signature_event = this->sequence_editor->GetLatestTimeSignatureEventForRowNumber(this->sequence_editor->current_row_number);
	int current_amount = GetEquivalentMeasureDivision(this->amount, this->numerator, this->denominator, MidiFileTimeSignatureEvent_getNumerator(time_signature_event), MidiFileTimeSignatureEvent_getDenominator(time_signature_event));

	if (current_amount == 1)
	{
		return new MeasuresPerStepSize(this->sequence_editor, 2);
	}
	else
	{
		int new_amount = GetCoarserMeasureDivision(current_amount, MidiFileTimeSignatureEvent_getNumerator(time_signature_event));
		return new StepsPerMeasureSize(this->sequence_editor, new_amount, MidiFileTimeSignatureEvent_getNumerator(time_signature_event), MidiFileTimeSignatureEvent_getDenominator(time_signature_event));
	}
}

void StepsPerMeasureSize::PopulateDialog(StepSizeDialog* dialog)
{
	dialog->amount_text_box->SetValue(wxString::Format("%d", this->amount));
	dialog->steps_per_measure_radio_button->SetValue(true);
}

double StepsPerMeasureSize::GetStepFromTick(long tick)
{
	MidiFileTrack_t conductor_track = MidiFile_getFirstTrack(this->sequence_editor->sequence->midi_file);
	double time_signature_event_beat = 0.0;
	double step = 0.0;
	int numerator = 4;
	int denominator = 4;

	for (MidiFileEvent_t event = MidiFileTrack_getFirstEvent(conductor_track); (event != NULL) && (MidiFileEvent_getTick(event) < tick); event = MidiFileEvent_getNextEventInTrack(event))
	{
		if (MidiFileEvent_isTimeSignatureEvent(event))
		{
			double next_time_signature_event_beat = MidiFile_getBeatFromTick(this->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event));
			step += ((next_time_signature_event_beat - time_signature_event_beat) * ((double)(GetEquivalentMeasureDivision(this->amount, this->numerator, this->denominator, numerator, denominator)) * denominator / numerator / 4));
			time_signature_event_beat = next_time_signature_event_beat;
			numerator = MidiFileTimeSignatureEvent_getNumerator(event);
			denominator = MidiFileTimeSignatureEvent_getDenominator(event);
		}
	}

	step += ((MidiFile_getBeatFromTick(this->sequence_editor->sequence->midi_file, tick) - time_signature_event_beat) * ((double)(GetEquivalentMeasureDivision(this->amount, this->numerator, this->denominator, numerator, denominator)) * denominator / numerator / 4));
	return step;
}

long StepsPerMeasureSize::GetTickFromStep(double step)
{
	MidiFileTrack_t conductor_track = MidiFile_getFirstTrack(this->sequence_editor->sequence->midi_file);
	double time_signature_event_beat = 0.0;
	double time_signature_event_step = 0.0;
	int numerator = 4;
	int denominator = 4;

	for (MidiFileEvent_t event = MidiFileTrack_getFirstEvent(conductor_track); event != NULL; event = MidiFileEvent_getNextEventInTrack(event))
	{
		if (MidiFileEvent_isTimeSignatureEvent(event))
		{
			double next_time_signature_event_beat = MidiFile_getBeatFromTick(this->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event));
			double next_time_signature_event_step = time_signature_event_step + ((next_time_signature_event_beat - time_signature_event_beat) * ((double)(GetEquivalentMeasureDivision(this->amount, this->numerator, this->denominator, numerator, denominator)) * denominator / numerator / 4));
			if (next_time_signature_event_step >= step) break;
			time_signature_event_beat = next_time_signature_event_beat;
			time_signature_event_step = next_time_signature_event_step;
			numerator = MidiFileTimeSignatureEvent_getNumerator(event);
			denominator = MidiFileTimeSignatureEvent_getDenominator(event);
		}
	}

	return MidiFile_getTickFromBeat(this->sequence_editor->sequence->midi_file, time_signature_event_beat + ((step - time_signature_event_step) / ((double)(GetEquivalentMeasureDivision(this->amount, this->numerator, this->denominator, numerator, denominator)) * denominator / numerator / 4)));
}

wxString StepsPerMeasureSize::GetTimeStringFromTick(long tick)
{
	return wxString(MidiFile_getMeasureBeatStringFromTick(this->sequence_editor->sequence->midi_file, tick));
}

MeasuresPerStepSize::MeasuresPerStepSize(SequenceEditor* sequence_editor, int amount)
{
	this->sequence_editor = sequence_editor;
	this->amount = amount;
}

MeasuresPerStepSize::~MeasuresPerStepSize()
{
}

StepSize* MeasuresPerStepSize::ZoomIn()
{
	if (this->amount == 1)
	{
		MidiFileEvent_t time_signature_event = this->sequence_editor->GetLatestTimeSignatureEventForRowNumber(this->sequence_editor->current_row_number);
		int new_amount = GetFinerMeasureDivision(1, MidiFileTimeSignatureEvent_getNumerator(time_signature_event));
		return new StepsPerMeasureSize(this->sequence_editor, new_amount, MidiFileTimeSignatureEvent_getNumerator(time_signature_event), MidiFileTimeSignatureEvent_getDenominator(time_signature_event));
	}
	else
	{
		return new MeasuresPerStepSize(this->sequence_editor, GetFinerMeasureMultiple(this->amount));
	}
}

StepSize* MeasuresPerStepSize::ZoomOut()
{
	return new MeasuresPerStepSize(this->sequence_editor, GetCoarserMeasureMultiple(this->amount));
}

void MeasuresPerStepSize::PopulateDialog(StepSizeDialog* dialog)
{
	dialog->amount_text_box->SetValue(wxString::Format("%d", this->amount));
	dialog->measures_per_step_radio_button->SetValue(true);
}

double MeasuresPerStepSize::GetStepFromTick(long tick)
{
	return MidiFile_getMeasureFromTick(this->sequence_editor->sequence->midi_file, tick) / this->amount;
}

long MeasuresPerStepSize::GetTickFromStep(double step)
{
	return MidiFile_getTickFromMeasure(this->sequence_editor->sequence->midi_file, step * this->amount);
}

wxString MeasuresPerStepSize::GetTimeStringFromTick(long tick)
{
	return wxString(MidiFile_getMeasureBeatStringFromTick(this->sequence_editor->sequence->midi_file, tick));
}

SecondsPerStepSize::SecondsPerStepSize(SequenceEditor* sequence_editor, double amount)
{
	this->sequence_editor = sequence_editor;
	this->amount = amount;
}

SecondsPerStepSize::~SecondsPerStepSize()
{
}

StepSize* SecondsPerStepSize::ZoomIn()
{
	return new SecondsPerStepSize(this->sequence_editor, this->amount / 2);
}

StepSize* SecondsPerStepSize::ZoomOut()
{
	return new SecondsPerStepSize(this->sequence_editor, this->amount * 2);
}

void SecondsPerStepSize::PopulateDialog(StepSizeDialog* dialog)
{
	dialog->amount_text_box->SetValue(wxString::Format("%0.3lf", this->amount));
	dialog->seconds_per_step_radio_button->SetValue(true);
}

double SecondsPerStepSize::GetStepFromTick(long tick)
{
	return MidiFile_getTimeFromTick(this->sequence_editor->sequence->midi_file, tick) / this->amount;
}

long SecondsPerStepSize::GetTickFromStep(double step)
{
	return MidiFile_getTickFromTime(this->sequence_editor->sequence->midi_file, step * this->amount);
}

wxString SecondsPerStepSize::GetTimeStringFromTick(long tick)
{
	return wxString(MidiFile_getHourMinuteSecondStringFromTick(this->sequence_editor->sequence->midi_file, tick));
}

