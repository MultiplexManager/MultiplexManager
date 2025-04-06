#ifndef MODEL_UNDO_ACTION_H
#define MODEL_UNDO_ACTION_H

#include <QUndoCommand>

#include <QAbstractItemModel>
#include <QModelIndex>

#include <MarkerModel.h>


class ModelUndoCommand : public QUndoCommand
{
public:
    ModelUndoCommand( QAbstractItemModel* model, QModelIndex index, QVariant before, QVariant after );

    virtual void undo();
    virtual void redo();

private:
    
    QAbstractItemModel* m_Model;
    QModelIndex m_Index;
    QVariant m_Before;
    QVariant m_After;

};


class RecalculateAnnealTempCommand : public QUndoCommand
{
public:
	RecalculateAnnealTempCommand( MarkerModel & markers, 
								  QList<float> & newValues );
	
	virtual void undo();
    virtual void redo();
private:

	MarkerModel & m_MarkerInfo;
	
	QList<float> m_Before;
	QList<float> m_After;
};
#endif
