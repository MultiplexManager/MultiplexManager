
#include <QtGui>

#include <DyeModel.h>

#include "DyeModelDelegate.h"
#include <QColorDialog>

#include <QUndoStack>
#include "ModelUndoCommand.h"

DyeModelDelegate::DyeModelDelegate(QUndoStack* undoStack, QObject *parent)
    : QItemDelegate(parent),
      m_UndoStack( undoStack )
{
}

QWidget *DyeModelDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem & option ,
    const QModelIndex &  index ) const
{
    
    if ( index.column() == DyeInfo::COLOR )
    {
        QColor col = index.model()->data( index, Qt::DecorationRole ).value<QColor>();

        QColor newCol = QColorDialog::getColor( col, parent );
        
        if ( newCol.isValid() )
        {
            QAbstractItemModel * model = const_cast<QAbstractItemModel*>( index.model() );
            QVariant before = col;
            QVariant after = newCol;
            
            ModelUndoCommand* undoCommand = new ModelUndoCommand( model, index, before, after );
            m_UndoStack->push( undoCommand );
            // don't need to actually call setData() because
            // pushing the undo command actually calls redo() straight away
            //model->setData( index, newCol );

            
        }
        return NULL;
                    
    }
    else
    {
        return QItemDelegate::createEditor( parent, option, index );
    }


    return NULL;
}


void DyeModelDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QVariant before = model->data( index ) ;
    QItemDelegate::setModelData( editor, model, index );

    QVariant after = model->data( index ) ;

    ModelUndoCommand* undoCommand = new ModelUndoCommand( model, index, before, after );

    m_UndoStack->push( undoCommand ); // this has the unforunate effect of calling setData() again, 
                                      // because that's how the stack works

    if ( index.column() == DyeInfo::PREFERENCE )
    {
        DyeModel* dyeModel = reinterpret_cast<DyeModel*>( model );

        dyeModel->SortByPreference();

    }
}

                      
