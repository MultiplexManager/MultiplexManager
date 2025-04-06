
#include <QtGui>

#include "ArtifactModelDelegate.h"
#include <QColorDialog>

#include <QUndoStack>
#include "ModelUndoCommand.h"

ArtifactModelDelegate::ArtifactModelDelegate(QUndoStack* undoStack, QObject *parent)
    : QItemDelegate(parent),
      m_UndoStack( undoStack )
{
}

QWidget *ArtifactModelDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & /*option*/,
                                             const QModelIndex &/*index*/) const
{
    return new QLineEdit( parent );
}

void ArtifactModelDelegate::setEditorData( QWidget */*editor*/,
                                           const QModelIndex &/*index*/) const
{
    
}

void ArtifactModelDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QVariant before = model->data( index ) ;

    QLineEdit* edit = static_cast<QLineEdit*>( editor );

    if ( edit->text().trimmed().size() != 0 )
    {
        QVariant v = edit->text();            
        model->setData(index, v );
    }

    QVariant after = model->data( index ) ;

    ModelUndoCommand* undoCommand = new ModelUndoCommand( model, index, before, after );

    m_UndoStack->push( undoCommand );
}

                      
