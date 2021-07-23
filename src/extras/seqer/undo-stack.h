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

public slots:
	void sequenceUpdated(bool create_undo_command);
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
