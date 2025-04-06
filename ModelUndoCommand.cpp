#include "ModelUndoCommand.h"

#include <cassert>

ModelUndoCommand::ModelUndoCommand( QAbstractItemModel* model, QModelIndex index, QVariant before, QVariant after )
    : m_Model( model) ,
      m_Index( index ),
      m_Before( before ),
      m_After( after )
{
    QVariant headerData = m_Model->headerData( m_Index.column(), Qt::Horizontal );
    setText("Edit " + headerData.toString() );
}

void ModelUndoCommand::undo()
{
    m_Model->setData( m_Index, m_Before );

}

void ModelUndoCommand::redo()
{
    m_Model->setData( m_Index, m_After );
}


RecalculateAnnealTempCommand::RecalculateAnnealTempCommand( 
	MarkerModel & markers, 
	QList<float> & newValues )
	: m_MarkerInfo( markers )
	, m_After( newValues )
{
	assert( m_MarkerInfo.GetData().size() == m_After.size() );
	for ( qint32 i = 0; i < m_MarkerInfo.GetData().size(); ++i )
	{
		m_Before << m_MarkerInfo.GetData()[i].m_AnnealingTemp;
	}

	setText("Recalculate All Annealing Temperatures");
}

void RecalculateAnnealTempCommand::undo()
{
    assert( m_MarkerInfo.GetData().size() == m_Before.size() );
	for ( qint32 i = 0; i < m_MarkerInfo.GetData().size(); ++i )
	{
		m_MarkerInfo.GetData()[i].m_AnnealingTemp = m_Before[i];
	}
	m_MarkerInfo.RefreshData();
}

void RecalculateAnnealTempCommand::redo()
{

    assert( m_MarkerInfo.GetData().size() == m_After.size() );
	for ( qint32 i = 0; i < m_MarkerInfo.GetData().size(); ++i )
	{
		m_MarkerInfo.GetData()[i].m_AnnealingTemp = m_After[i];
	}    

	m_MarkerInfo.RefreshData();
}

