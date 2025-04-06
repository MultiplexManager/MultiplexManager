
#include "MarkerModel.h"
#include "utils.h"
#include <cassert>

#include "DomTemplateHelper.h"
#include <QColor>
#include <QBrush>


quint32 MarkerInfo::NextID = 0;

QVariant MarkerInfo::data( quint32 col, int role ) const
{

	if ( role == Qt::ToolTipRole )
	{
		return GetToolTip( col );

	}
	

    if ( (role != Qt::DisplayRole) &&
         (role != Qt::EditRole) )
    {
        return QVariant();
    }


    switch( col )
    {
        case NAME:
            return m_Name;
        case CHROMOSOME:
            return m_Chromosome;
        case GENETIC_LOCATION:
            return m_GeneticLocation;
        case HETEROZYGOSITY:
            return QString::number( m_Heterozygosity, 'g', 2 );
        case NUMBER_OF_ALLELES:
            return m_NumberOfAlleles;
        case ANNEALING_TEMP:
            return QString::number( m_AnnealingTemp, 'g', 4 );
        case MINIMUM_SIZE:
            return m_MinimumSize;
        case MAXIMUM_SIZE:
            return m_MaximumSize;
        case FORWARD:
            return m_Forward;
        case REVERSE:
            return m_Reverse;
        case FORCE_DYE:
            return m_ForceDye;

        default:
            assert( 0 && "Invalid column index!");

    }
}

void MarkerInfo::setData( quint32 col, const QVariant & value )
{
    switch ( col )
    {
        case NAME:
            m_Name = value.toString();
            break;
        case CHROMOSOME:
            m_Chromosome = value.toString();
            break;
        case GENETIC_LOCATION:
            m_GeneticLocation = max( 0.0, value.toDouble() );
            break;
        case HETEROZYGOSITY:
            m_Heterozygosity = value.toDouble();
            m_Heterozygosity = max( min( m_Heterozygosity, 1.0f ), 0.0f );  
            break;
        case NUMBER_OF_ALLELES:
            m_NumberOfAlleles = max( 1, value.toInt() );
            break;
        case ANNEALING_TEMP:
            m_AnnealingTemp = max( 0.0, value.toDouble() );
            break;
        case MINIMUM_SIZE:
            m_MinimumSize = max( 1, value.toInt() );
            break;
        case MAXIMUM_SIZE:
            m_MaximumSize = max( 1, value.toInt() );
            break;
        case FORWARD:
            m_Forward = value.toString();
            break;
        case REVERSE:
            m_Reverse = value.toString();
            break;
        case FORCE_DYE:
            m_ForceDye = value.toInt();
            break;
	    
	    
    }
}

QVariant MarkerInfo::HeaderData ( int section, Qt::Orientation orientation, int role )
{
    if ( section == -1 )
    {
        return QVariant();
    }

	if ( role == Qt::ToolTipRole && orientation == Qt::Horizontal )
    {
        return GetToolTip( section );
    }

    if ( role != Qt::DisplayRole )
    {
        return QVariant();
    }

    if ( orientation == Qt::Horizontal )
    {
        switch ( section )
        {
            case NAME:
                return QObject::tr("Name");
            case CHROMOSOME:
                return QObject::tr("Chromosome");
            case GENETIC_LOCATION:
                return QObject::tr("Genetic Location");
            case HETEROZYGOSITY:
                return QObject::tr("Heterozygosity");
            case NUMBER_OF_ALLELES:
                return QObject::tr("Number of Alleles");
            case ANNEALING_TEMP:
                return QObject::tr("Annealing Temperature");
            case MINIMUM_SIZE:
                return QObject::tr("Minimum Allele Size");
            case MAXIMUM_SIZE:
                return QObject::tr("Maximum Allele Size");
            case FORWARD:
                return QObject::tr("Forward Sequence");
            case REVERSE:
                return QObject::tr("Reverse Sequence");
            case FORCE_DYE:
                return QObject::tr("Enforce Marker->Dye ");
            default:
                assert( 0 && "Invalid column index!");
	    
        }
    }
    else // orientation vertical, show row number
    {
        return section + 1;
    }
}


QVariant MarkerInfo::GetToolTip ( quint32 col)
{
	switch( col )
	{
		case NAME:
			return QObject::tr("Name of the marker.  This field is purely so\nyou can identify your marker in the ouput.");
		case CHROMOSOME:
			return QObject::tr("Chromosome the marker is on.\ne.g. 3, X, Y");
		case GENETIC_LOCATION:
			return QObject::tr("Genetic Location of the marker on the chromosome.\nUsually in centimorgans.");
		case HETEROZYGOSITY:
			return QObject::tr("Heterozygosity of the marker. Range is 0 - 1. ");
		case NUMBER_OF_ALLELES:
			return QObject::tr("Number of different alleles for this marker.");
		case ANNEALING_TEMP:
			return QObject::tr("Annealing Temperature, either determined empirically\nor estimated by clicking the Calculate button.");
		case MINIMUM_SIZE:
			return QObject::tr("Minimum Allele Size, in base pairs.");
		case MAXIMUM_SIZE:
			return QObject::tr("Maximum Allele Size, in base pairs.");
		case FORWARD:
			return QObject::tr("Forward Sequence of the primer pair.");
		case REVERSE:
			return QObject::tr("Reverse Sequence of the primer pair");
		case FORCE_DYE:
			return QVariant();
		default:
			assert( 0 && "Invalid column index" );
	};

	return QVariant();
}



void MarkerInfo::Save( QDomDocument* doc, QDomNode* parent )
{
    QDomElement marker = doc->createElement("marker");
    
    AddElement( doc, &marker, "id", &m_Id );
    AddElement( doc, &marker, "name", &m_Name );
    AddElement( doc, &marker, "chromosome", &m_Chromosome );
    AddElement( doc, &marker, "geneticLocation", &m_GeneticLocation );
    AddElement( doc, &marker, "heterozygosity", &m_Heterozygosity );
    AddElement( doc, &marker, "numAlleles", &m_NumberOfAlleles );
    AddElement( doc, &marker, "annealingTemp", &m_AnnealingTemp );
    AddElement( doc, &marker, "minimumSize", &m_MinimumSize );
    AddElement( doc, &marker, "maximumSize", &m_MaximumSize );
    AddElement( doc, &marker, "forwardSeq", &m_Forward );
    AddElement( doc, &marker, "reverseSeq", &m_Reverse );
    AddElement( doc, &marker, "forceDyeToMarker", &m_ForceDye );
    
    parent->appendChild( marker );

}
void MarkerInfo::Load( QDomNode* node )
{    
    ReadElement( node, "id", &m_Id );
    // make sure the next ID is
    // at least as one more than the one we just read in
    NextID = max( m_Id + 1, NextID );
    ReadElement( node, "name", &m_Name );
    ReadElement( node, "chromosome", &m_Chromosome );
    ReadElement( node, "geneticLocation", &m_GeneticLocation );
    ReadElement( node, "heterozygosity", &m_Heterozygosity );
    ReadElement( node, "numAlleles", &m_NumberOfAlleles );
    ReadElement( node, "annealingTemp", &m_AnnealingTemp );
    ReadElement( node, "minimumSize", &m_MinimumSize );
    ReadElement( node, "maximumSize", &m_MaximumSize );
    ReadElement( node, "forwardSeq", &m_Forward );
    ReadElement( node, "reverseSeq", &m_Reverse );
    ReadElement( node, "forceDyeToMarker", &m_ForceDye );
}


bool MarkerInfo::TooCloseTo( const MarkerInfo* other, qint32 minSeparation ) const
{
    // they dont intersect, continue
    bool dontIntersect =  ( m_MinimumSize - minSeparation >= other->m_MaximumSize) || 
        (other->m_MinimumSize - minSeparation >= m_MaximumSize );

    return !dontIntersect;

}


QVariant MarkerModel::data ( const QModelIndex & index, int role ) const
{

    if (!index.isValid())
        return QVariant();

    qint32 col = index.column();
    
    // show the dye colour in the decoration role, and don't 
    // show the text if the dye has been deleted.
    if ( ( col == MarkerInfo::FORCE_DYE )  && 
         ( ( role == Qt::DecorationRole ) || ( role == Qt::DisplayRole  ) ) )
         
    {
        const MarkerInfo & marker = m_Vector[ index.row() ];

        if ( marker.m_ForceDye == -1 )
        {
            if ( role == Qt::DisplayRole )
            {
                return "Any";
            }
            else
            {
                return Qt::white;
            }            
        }
        else
        {
            for ( qint32 i = 0; i < m_Dyes.size(); ++i )
            {
                const DyeInfo & dye = m_Dyes[i];
                if ( (qint32) dye.m_Id == marker.m_ForceDye )
                {
                    if ( role == Qt::DisplayRole )
                    {
                        return dye.m_Name; 
                    }
                    else
                    {
                        return dye.m_Color;
                    }
                }
            }
            
            return QVariant();
        }
    }
    else
    {
        return m_Vector[ index.row() ].data( index.column(), role );
    }

    // special case for the force dye option, 
    
    
}

void MarkerModel::RefreshData()
{
    QModelIndex startIndex = index( 0, 0 );
    QModelIndex endIndex = index( m_Vector.size() - 1, MarkerInfo::FieldCount() - 1 );
    emitDataChanged( startIndex, endIndex );
    
}
