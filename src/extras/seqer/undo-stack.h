#ifndef UNDO_STACK_INCLUDED
#define UNDO_STACK_INCLUDED

class UndoStack;
class UndoCommand;

#include <QtWidgets>
#include "sequence.h"

class UndoStack: public QUndoStack
{
	Q_OBJECT

public:
	Sequence* sequence;
	QByteArray current_snapshot;

	UndoStack(Sequence* sequence);
	~UndoStack();
	void createUndoCommand();
};

class UndoCommand: public QUndoCommand
{
public:
	UndoStack* undo_stack;
	QByteArray undo_snapshot;
	QByteArray redo_snapshot;

	UndoCommand(UndoStack* undo_stack);
	void undo();
	void redo();
};

#endif
