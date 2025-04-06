#ifndef VECTOR_MODEL_UNDO_COMMANDS_H
#define VECTOR_MODEL_UNDO_COMMANDS_H

#include "VectorModel.h"
#include <QUndoCommand>
#include <QTableView>
#include <cassert>

template <typename T>
class UndoAddRowCommand : public QUndoCommand
{

public:
    UndoAddRowCommand(  VectorModel<T> * model, QTableView* view )
        : m_Model( model ),
          m_View( view ),
          m_Row( -1 )
    {
        setText( "Add Row");
    }

    virtual void undo()
    {
        assert( m_Row != -1 );
        m_Model->RemoveRow( m_Row );
    }
    
    virtual void redo()
    {
        m_Row = m_Model->AddRow();
        m_View->selectRow( m_Row );
    }
private:
    VectorModel<T> * m_Model;
    QTableView* m_View;
    int m_Row;



};

template <typename T>
class UndoRemoveRowCommand : public QUndoCommand
{

public:
    UndoRemoveRowCommand(  VectorModel<T> * model, QTableView* view, int row )
        : m_Model( model ),
          m_View( view ),
          m_Row( row )
    {
        setText("Remove Row");
    }

    virtual void undo()
    {
        m_Model->insertRows( m_Row, 1 );
        m_Model->SetRowContents( m_Row, m_Data );
        m_View->selectRow( m_Row );
    }
    
    virtual void redo()
    {
        // take a copy of the data
        m_Data = m_Model->GetData()[m_Row];
        m_Model->RemoveRow( m_Row );
    }
private:
    VectorModel<T> * m_Model;
    QTableView* m_View;
    int m_Row;
    T m_Data;


};


#endif
