#include "Criteria.h"

#include  "MultiplexResults.h"

CriteriaDialog::CriteriaDialog( const QList<qint32> & criteria, 
                                bool selectBest,
								QWidget * parent )
    : QDialog( parent )
	, m_SelectBest( selectBest )
{
    setupUi( this );
    
	QList<AvailableCriteria> list;

	GetAvailableCriteria( list );
	
	for ( qint32 i = 0; i < criteria.size(); ++i )
	{
		for ( qint32 j = 0; j < list.size(); ++j )
		{
			if ( list[j].m_Id  == criteria[i] )
			{
				QListWidgetItem* item = new QListWidgetItem( list[j].m_Name, NULL, list[j].m_Id ); 
				item->setToolTip( list[j].m_ToolTip );
                if ( !selectBest && list[j].m_OnlyValidWhenPickingSubset )
                {
                      item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
                      item->setCheckState( Qt::Unchecked );
                }
                else
                {
                    item->setCheckState( Qt::Checked );
                }
				listWidget->addItem( item );
				list[j].m_Id =  -1;
			}
		}
	}


	for ( qint32 j = 0; j < list.size(); ++j )
	{
		if ( list[j].m_Id  !=  -1 )
		{
			QListWidgetItem* item = new QListWidgetItem( list[j].m_Name, NULL, list[j].m_Id ); 
			item->setToolTip( list[j].m_ToolTip );
			item->setCheckState( Qt::Unchecked );
            if ( !selectBest && list[j].m_OnlyValidWhenPickingSubset )
            {
                item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
            }
            listWidget->addItem( item );	
                        
		}
	}

    listWidget->setCurrentRow ( 0 );
    
}

void CriteriaDialog::GetCriteria( QList<qint32> & criteria )
{
	qint32 count = listWidget->count();
	for ( qint32 i = 0 ; i < count; ++i )
	{
		QListWidgetItem* item = listWidget->item( i );
		if ( item->checkState() == Qt::Checked )
		{
			criteria << item->type();
		}
	} 
	
}

void CriteriaDialog::on_btnMoveUp_clicked()
{
    int row = listWidget->currentRow();
    if (row <= 0)
	{
        return; // nothing to do
	}
	
    listWidget->insertItem(row - 1, listWidget->takeItem(row));
    listWidget->setCurrentRow(row - 1);
}

void CriteriaDialog::on_btnMoveDown_clicked()
{
	int row = listWidget->currentRow();
	if (row == -1 || row == listWidget->count() - 1)
	{
		return; // nothing to do
	}

	listWidget->insertItem(row + 1, listWidget->takeItem(row));
	listWidget->setCurrentRow(row + 1);    
    
}

void CriteriaDialog::on_listWidget_currentRowChanged( int currentRow )
{
	btnMoveUp->setEnabled( true );
	btnMoveDown->setEnabled( true );
		
	if ( currentRow == -1 )
	{
		btnMoveUp->setEnabled( false );
		btnMoveDown->setEnabled( false );
	} 
	else if ( currentRow == 0 )
	{
		btnMoveUp->setEnabled( false );
	}
	else if ( currentRow ==  listWidget->count() - 1 )
	{
		btnMoveDown->setEnabled( false );
	} 
}

void CriteriaDialog::on_btnDefaults_clicked() 
{

	QList<AvailableCriteria> list;

	GetAvailableCriteria( list );

	listWidget->clear();

	for ( qint32 j = 0; j < list.size(); ++j )
	{
		QListWidgetItem* item = new QListWidgetItem( list[j].m_Name, NULL, list[j].m_Id ); 
		item->setToolTip( list[j].m_ToolTip );
		if ( !m_SelectBest && list[j].m_OnlyValidWhenPickingSubset )
		{
			item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
			item->setCheckState( Qt::Unchecked );
		}
		else
		{
			item->setCheckState( Qt::Checked );
		}
		listWidget->addItem( item );
		list[j].m_Id =  -1;
	}

}




