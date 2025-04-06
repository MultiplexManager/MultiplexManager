#include "ArtifactModel.h"

#include "DomTemplateHelper.h"

#include <cassert>

QVariant ArtifactInfo::data( quint32 col, int role ) const
{
    if ( ( role != Qt::DisplayRole ) && ( role != Qt::EditRole ) )
    {
        return QVariant();
    }
    switch ( col )
    {
        case SIZE:
            return m_Size;
        case WIDTH:
            return m_Width;
        default:
            assert( 0 && "Invalid column index" );
    };

}

void ArtifactInfo::setData( quint32 col, const QVariant & value )
{
    switch ( col )
    {
        case SIZE:
            m_Size = value.toUInt();
            break;
        case WIDTH:
            m_Width = value.toUInt();
            break;
    }
}

QVariant ArtifactInfo::HeaderData ( int section, Qt::Orientation orientation, int role )
{
    if ( role != Qt::DisplayRole )
    {
        return QVariant();
    }

    if ( orientation == Qt::Horizontal )
    {
        switch ( section )
        {
            case SIZE:
                return QObject::tr("Size ( Base Pairs )");
            case WIDTH:
                return QObject::tr("Width");
            default:
                assert( 0 && "Invalid column index!");	    
        }

    }
    else
    {
        return section + 1;
    }
}



void ArtifactInfo::Save( QDomDocument* doc, QDomNode* parent )
{
    QDomElement artifact = doc->createElement("artifact");
    
    AddElement( doc, &artifact, "size", &m_Size );
    AddElement( doc, &artifact, "width", &m_Width );
    
    parent->appendChild( artifact );

}
void ArtifactInfo::Load( QDomNode* node )
{
    ReadElement( node, "size", &m_Size );
    ReadElement( node, "width", &m_Width );
}
