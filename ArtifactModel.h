#ifndef ARTIFACT_MODEL_H
#define ARTIFACT_MODEL_H

#include "VectorModel.h"


class QDomNode;
class QDomDocument;


struct ArtifactInfo
{
    ArtifactInfo()
    {
        m_Size = 0;
        m_Width = 1;
    }
    
    qint32 m_Size;
    qint32 m_Width;;
    
    QVariant data( quint32 col, int role ) const;
    void setData( quint32 col, const QVariant & value );
    
    enum Columns
    {
        SIZE,
        WIDTH,
    };
    
    static int FieldCount() 
    {
        return 2;
    }

    static QVariant HeaderData ( int section, Qt::Orientation orientation, int role );

    
    void Save( QDomDocument* doc, QDomNode* parent );
    void Load( QDomNode* node );
};


class ArtifactModel : public VectorModel<ArtifactInfo>
{
public:
    ArtifactModel( QVector<ArtifactInfo> & source )
        : VectorModel<ArtifactInfo>( source )
    {
    }
};

#endif
