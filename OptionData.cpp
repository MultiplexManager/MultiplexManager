#include "OptionData.h"

#include "DomTemplateHelper.h"

void OptionData::Save( QDomDocument* doc, QDomNode* parent )
{
    AddElement( doc, parent, "useAllMarkers", &m_UseAllMarkers );
    AddElement( doc, parent, "maxLociPerReaction", &m_MaxLociPerReaction );
    AddElement( doc, parent, "selectBest", &m_SelectBest );
    AddElement( doc, parent, "minDistance", &m_MinDistanceBetweenDyesOfSameColor );
    AddElement( doc, parent, "complementarityThreshold", &m_ComplementarityThreshold );
	AddElement( doc, parent, "iterationsToRun", &m_IterationsToRun );

	AddElement( doc, parent, "primerConcentration", &m_PrimerConcentration );
	AddElement( doc, parent, "degreesLowerForAnnealing", &m_DegreesLowerForAnnealing );

	QString criteriaString;
	
	foreach( qint32 item, m_Criteria )
	{
		criteriaString += QString::number( item ) + " ";
	}
		
	AddElement( doc, parent, "criteria", &criteriaString );

}

void OptionData::Load( QDomNode* node )
{
    
    ReadElement( node, "useAllMarkers", &m_UseAllMarkers );
    ReadElement( node, "maxLociPerReaction", &m_MaxLociPerReaction );
    ReadElement( node, "selectBest", &m_SelectBest );
    ReadElement( node, "minDistance", &m_MinDistanceBetweenDyesOfSameColor );
    ReadElement( node, "complementarityThreshold", &m_ComplementarityThreshold );
    ReadElement( node, "iterationsToRun", &m_IterationsToRun );
	
	ReadElement( node, "primerConcentration", &m_PrimerConcentration );
	ReadElement( node, "degreesLowerForAnnealing", &m_DegreesLowerForAnnealing );

	QString criteriaString;
	ReadElement( node, "criteria", &criteriaString );
	if ( !criteriaString.isEmpty() )
	{
		m_Criteria.clear();
		QStringList splits = criteriaString.split( " ", QString::SkipEmptyParts );
		foreach( QString string, splits )
		{
			m_Criteria << string.toInt();
		}
	}
}
