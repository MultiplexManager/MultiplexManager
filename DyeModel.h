#ifndef DYE_MODEL_H
#define DYE_MODEL_H

#include "VectorModel.h"
#include "ArtifactModel.h"

#include <QColor>

class QDomNode;
class QDomDocument;

struct DyeInfo
{
    DyeInfo() 
    {
        m_Id = NextID++;
        m_Preference = 5;
    }


    quint32 m_Id;

    QString  m_Name;
    QColor m_Color;
    quint32 m_Preference;

    QVariant data( quint32 col, int role ) const;
    void setData( quint32 col, const QVariant & value );
    
    static int FieldCount() 
    {
        return 3;
    }

    enum Columns
    {
        NAME = 0,
        COLOR,
        PREFERENCE,
    };

    static QVariant HeaderData ( int section, Qt::Orientation orientation, int role );

    QVector<ArtifactInfo> m_Artifacts;

    bool operator<( const DyeInfo & other ) const
    {
        return m_Preference < other.m_Preference;
    }

    void Save( QDomDocument* doc, QDomNode* parent );
    void Load( QDomNode* node );

    static quint32 NextID;


};

class DyeModel : public VectorModel<DyeInfo>
{
public:
    DyeModel( QVector<DyeInfo> & source )
        : VectorModel<DyeInfo>( source )
    {
    }

    void PopulateDefaultDyes();
    void SortByPreference();

    
};

#endif
