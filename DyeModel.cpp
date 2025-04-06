
#include "DyeModel.h"
#include "utils.h"

#include "DomTemplateHelper.h"

#include <cassert>

quint32 DyeInfo::NextID = 0;

void DyeModel::PopulateDefaultDyes()
{
    
    static const QColor yellow( 255, 255, 0 );
    static const QColor red( 255, 30, 30 );
    static const QColor blue( 127, 127, 255 );
    static const QColor green( 30, 255, 30 );

    DyeInfo d1;
    d1.m_Name = "6-FAM";
    d1.m_Color = blue;    
    d1.m_Preference = 5;
    m_Vector << d1;

    DyeInfo d2;
    d2.m_Name = "VIC";
    d2.m_Color = green;    
    d2.m_Preference = 5;
    m_Vector << d2;

    DyeInfo d3;
    d3.m_Name = "NED";
    d3.m_Color = yellow;
    d3.m_Preference = 5;
    m_Vector << d3;

    DyeInfo d4;
    d4.m_Name = "PET";
    d4.m_Color = red;
    d4.m_Preference = 5;
    m_Vector << d4;

    
}

QVariant DyeInfo::data( quint32 col, int role ) const
{
    switch ( role )
    {
		case Qt::DisplayRole:
		{
			switch ( col )
			{
				case NAME:
					return m_Name;
				case COLOR:
					return QVariant();
				case PREFERENCE:
					return m_Preference;
				default:
					assert( 0 && "Invalid column index" );
			};
		}
		break;
		case Qt::EditRole:
		{
			switch ( col )
			{
				case NAME:
					return m_Name;
				case COLOR:
					return m_Color;
				case PREFERENCE:
					return m_Preference;
				default:
					assert( 0 && "Invalid column index" );
			};
		};
		break;
		case Qt::DecorationRole:
		{
			switch( col )
			{
				case NAME:
					return QVariant();
				case COLOR:
					return m_Color;
				case PREFERENCE:
					return QVariant();
				default:
					assert( 0 && "Invalid column index" );
			};
		}
		break;
		case Qt::ToolTipRole:
		{
			switch( col )
			{
				case NAME:
					return QVariant();
				case COLOR:
					return QVariant();
				case PREFERENCE:
					return QObject::tr("Some dyes maybe more desirable than others (eg, cheaper).\n" 
									   "If there is an otherwise equal choice, the dye with the lowest value will be selected.\n"
									   "Rate them from 1 to 10");
				default:
					assert( 0 && "Invalid column index" );
			};
		}
		break;
		default:
			return QVariant();
			break;
    }
}

void DyeInfo::setData( quint32 col, const QVariant & value )
{
    switch ( col )
    {
		case NAME:
			m_Name = value.toString();
			break;
		case COLOR:
			m_Color = value.value<QColor>();
			break;
		case PREFERENCE:
			m_Preference = clamp( value.toUInt(), 1, 10 );
			break;
			
    }
}

QVariant DyeInfo::HeaderData ( int section, Qt::Orientation orientation, int role )
{
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
				break;
			case COLOR:
				return QObject::tr("Color");
				break;
			case PREFERENCE:
				return QObject::tr("Preference");
				break;
			default:
				assert( 0 && "Invalid column index!");
	    
		}

    }
    else
    {
		return section + 1;
    }
}



void DyeInfo::Save( QDomDocument* doc, QDomNode* parent )
{
    QDomElement dye = doc->createElement("dye");
    
    AddElement( doc, &dye, "id", &m_Id );
    AddElement( doc, &dye, "name", &m_Name );
    AddElement( doc, &dye, "color", &m_Color );
    AddElement( doc, &dye, "preference", &m_Preference );

    if ( m_Artifacts.size() > 0 )
    {
        QDomElement artifacts = doc->createElement("artifacts");        

        ArtifactInfo a;
        foreach( a, m_Artifacts )
        {
            a.Save( doc, &artifacts );
        }

        dye.appendChild( artifacts );
    }

    parent->appendChild( dye );

}
void DyeInfo::Load( QDomNode* node )
{
    ReadElement( node, "id", &m_Id );
    // ensure the next generated ID is 
    // bigger than any we have read in
    NextID = max( m_Id + 1, NextID );

    ReadElement( node, "name", &m_Name );
    ReadElement( node, "color", &m_Color );
    ReadElement( node, "preference", &m_Preference );

    assert( m_Artifacts.size() == 0 );

    const QDomElement & artifacts = node->firstChildElement( "artifacts" );
    if ( !artifacts.isNull() )
    {
        QDomNode artifactNode = artifacts.firstChild();
        while ( !artifactNode.isNull() )
        {
            ArtifactInfo a;
            a.Load( &artifactNode );
            m_Artifacts << a; // insert into list
            artifactNode = artifactNode.nextSibling();
            
        }

    }
}


void DyeModel::SortByPreference()
{
    qSort( m_Vector.begin(), m_Vector.end() );
    QModelIndex startIndex = index( 0, 0 );
    QModelIndex endIndex = index( m_Vector.size() - 1, DyeInfo::FieldCount() - 1 );
    emitDataChanged( startIndex, endIndex );

}

