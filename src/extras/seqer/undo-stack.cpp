
#include <QtWidgets>
#include "sequence.h"
#include "undo-stack.h"

UndoStack::UndoStack(Sequence* sequence): QUndoStack()
{
	this->sequence = sequence;
	this->current_snapshot = Sequence::saveMidiFileToBuffer(sequence->midi_file);
}

UndoStack::~UndoStack()
{
}

void UndoStack::createUndoCommand()
{
	this->push(new UndoCommand(this));
}

UndoCommand::UndoCommand(UndoStack* undo_stack)
{
	this->undo_stack = undo_stack;
	this->undo_snapshot = this->undo_stack->current_snapshot;
	this->redo_snapshot = Sequence::saveMidiFileToBuffer(this->undo_stack->sequence->midi_file);
	this->undo_stack->current_snapshot = this->redo_snapshot;
}

void UndoCommand::undo()
{
	if (this->undo_stack->current_snapshot != this->undo_snapshot)
	{
		this->undo_stack->sequence->midi_file = Sequence::loadMidiFileFromBuffer(this->undo_snapshot);
		this->undo_stack->current_snapshot = this->undo_snapshot;
		this->undo_stack->sequence->update(false);
	}
}

void UndoCommand::redo()
{
	// The conditional is because Qt calls redo as soon as a command is added to the undo stack, which isn't relevant for a snapshot-based undo system.

	if (this->undo_stack->current_snapshot != this->redo_snapshot)
	{
		this->undo_stack->sequence->midi_file = Sequence::loadMidiFileFromBuffer(this->redo_snapshot);
		this->undo_stack->current_snapshot = this->redo_snapshot;
		this->undo_stack->sequence->update(false);
	}
}

