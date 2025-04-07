
#include <QtGui>
#include <QRegExp>

#include "MarkerModel.h"
#include "MarkerModelDelegate.h"
#include <QColorDialog>

#include "AnnealTempWidget.h"

#include "ModelUndoCommand.h"
#include <QUndoStack>
#include <QComboBox>

MarkerModelDelegate::MarkerModelDelegate(QUndoStack* undoStack, QSortFilterProxyModel* proxy, 
					 const QVector<DyeInfo> & dyeData,
					 QObject *parent)
    : QItemDelegate(parent),
      m_UndoStack( undoStack ),
      m_Proxy( proxy ),
      m_DyeData( dyeData ),
	  m_EditingSequence( false )
{
}

QWidget *MarkerModelDelegate::createEditor(QWidget *parent,
                                           const QStyleOptionViewItem & /*option*/ ,
                                           const QModelIndex &  index ) const
{
    QModelIndex source = m_Proxy->mapToSource( index );

    if ( source.column() == MarkerInfo::ANNEALING_TEMP )
    {
        const MarkerModel* markerModel = static_cast<const MarkerModel*>( source.model() );
        int row = source.row();
        
        AnnealTempWidget* editor = new AnnealTempWidget( markerModel->GetData()[row].m_Forward,
                                                         markerModel->GetData()[row].m_Reverse,
                                                         const_cast<MarkerModelDelegate*>(this),     
                                                         parent );

        // this makes it crash! need to find a way to correctly handle
        // focus events but still deal with button clicking
        //editor->GetLineEdit()->installEventFilter();
        editor->setFocusPolicy( Qt::StrongFocus );

        return editor;
    }
    else if ( source.column() == MarkerInfo::FORCE_DYE )
    {        	
        QPixmap map( 32, 32 );
        map.fill( Qt::white );

        QComboBox* cb = new QComboBox( parent );
        
        cb->addItem( QIcon( map ), tr("Any"), -1 );

        for ( qint32 i = 0; i < m_DyeData.size(); ++i )
        {
            const DyeInfo & dye = m_DyeData[i];
            map.fill( dye.m_Color );
            QIcon icon( map );
            cb->addItem( icon, dye.m_Name, dye.m_Id );
        }
        //cb->installEventFilter();
        return cb;
    }
    else
    {
        return new QLineEdit( parent );
    }
    
}


void MarkerModelDelegate::setEditorData( QWidget *editor,
                                         const QModelIndex & index) const
{
    QModelIndex source = m_Proxy->mapToSource( index );
        
    if ( source.column() == MarkerInfo::FORCE_DYE )
    {
	    QComboBox* cb = static_cast<QComboBox*>( editor );
	    qint32 forceId = source.model()->data( source, Qt::EditRole).toInt();
            qint32 index = cb->findData( forceId );
            cb->setCurrentIndex( index );

    }
    else if ( ( source.column() == MarkerInfo::NAME ) || ( source.column() == MarkerInfo::FORWARD ) 
         || ( source.column() == MarkerInfo::REVERSE ) )
    {
		QLineEdit* edit = static_cast<QLineEdit*>( editor );
        edit->setText(  source.model()->data(source, Qt::DisplayRole).toString() );
		if  (( source.column() == MarkerInfo::FORWARD ) || ( source.column() == MarkerInfo::REVERSE ) )
        {
			m_EditingSequence = true;
			edit->installEventFilter( const_cast<MarkerModelDelegate*>( this ) );
		}   
 }



}


void MarkerModelDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const
{
    QModelIndex source = m_Proxy->mapToSource( index );
    
    model = m_Proxy->sourceModel();

    QVariant before = model->data( source, Qt::EditRole ) ;

    QLineEdit* edit = NULL;
    if ( source.column() == MarkerInfo::ANNEALING_TEMP )
    {
        AnnealTempWidget* widget = static_cast<AnnealTempWidget*>( editor );
        edit = widget->GetLineEdit();

    }
    else if ( source.column() == MarkerInfo::FORCE_DYE )
    {
	    QComboBox* cb = static_cast<QComboBox*>( editor );	    
	    qint32 index = cb->currentIndex();
	    if ( index != 0 )
	    {
                model->setData(source, QVariant( m_DyeData[ index - 1 ].m_Id ) );
	    }
		else
		{
			model->setData(source, QVariant( -1 ) );;
		}
    }
    else
    {
        edit = static_cast<QLineEdit*>( editor );
    }

    if ( edit )
    {
        QString s = edit->text();            
        if ( ( source.column() == MarkerInfo::FORWARD ) || ( source.column() == MarkerInfo::REVERSE ) )
        {
            s = FilterOutNonSequenceCharacters( s );
        }
        model->setData(source, s );
    }

    
    QVariant after = model->data( source, Qt::EditRole ) ;

    ModelUndoCommand* undoCommand = new ModelUndoCommand( model, source, before, after );

    m_UndoStack->push( undoCommand );

	m_EditingSequence = false;

}


bool MarkerModelDelegate::eventFilter ( QObject * editor, QEvent * event )
{
	
	if ( m_EditingSequence )
	{
		if (event->type() == QEvent::KeyPress) 
        {
			QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            // let the user, copy paste, undo and redo as normal
            if ( keyEvent->matches( QKeySequence::Copy ) || 
                 keyEvent->matches( QKeySequence::Paste ) ||
                 keyEvent->matches( QKeySequence::Undo ) || 
                 keyEvent->matches( QKeySequence::Redo ) )
            {
                return QItemDelegate::eventFilter( editor, event );
            }


			switch ( keyEvent->key() )
			{
                // we don't want people typing
                // letters other than sequences in the sequences box
                // so have a white list of keys
				case Qt::Key_Escape:
				case Qt::Key_Tab:
				case Qt::Key_Backtab:
				case Qt::Key_Backspace:
				case Qt::Key_Return:
				case Qt::Key_Enter:
				case Qt::Key_Home:
				case Qt::Key_End:
				case Qt::Key_Left:
				case Qt::Key_Right:
				case Qt::Key_A:
				case Qt::Key_G:
				case Qt::Key_T:
				case Qt::Key_C:				
				case Qt::Key_Space:				
					return QItemDelegate::eventFilter( editor, event );
			};
			
			return true;
		} 
	}
	
    return QItemDelegate::eventFilter( editor, event );
	
}

QString MarkerModelDelegate::FilterOutNonSequenceCharacters( const QString & string )
{

    QString result = string;
    result.remove( QRegExp("[^atgcATGC ]") );
    return result;
}
