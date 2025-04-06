#ifndef MARKER_MODEL_H
#define MARKER_MODEL_H

#include <VectorModel.h>
#include <DyeModel.h>


class QDomNode;
class QDomDocument;

struct MarkerInfo
{
    MarkerInfo() 
    {
        m_Id = NextID++;
        m_Chromosome = "1";
        m_GeneticLocation = 0;
        m_Heterozygosity = 0.0f;
        m_NumberOfAlleles = 1;
        m_AnnealingTemp = 0.0f;
        m_MinimumSize = 0;
        m_MaximumSize = 0;	
        m_ForceDye = -1;
    }
    
    quint32 m_Id;

    QString  m_Name;
    QString m_Chromosome; 
    float m_GeneticLocation; // in Centimorgans
    float m_Heterozygosity;
    qint32 m_NumberOfAlleles;
    float   m_AnnealingTemp;
    qint32 m_MinimumSize;
    qint32 m_MaximumSize;
    QString m_Forward;
    QString m_Reverse;

    qint32 m_ForceDye;

    enum Columns
    {
        NAME = 0,
        CHROMOSOME,
        GENETIC_LOCATION,
        HETEROZYGOSITY,
        NUMBER_OF_ALLELES,
        ANNEALING_TEMP,
        MINIMUM_SIZE,
        MAXIMUM_SIZE,
        FORWARD,
        REVERSE,
        FORCE_DYE,
    };

    QVariant data( quint32 col, int role ) const;
    void setData( quint32 col, const QVariant & value );
   

    static int FieldCount() 
    {
        return 11;
    }

    static QVariant HeaderData ( int section, Qt::Orientation orientation, int role );

    static QVariant GetToolTip ( quint32 col);
    
    void Save( QDomDocument* doc, QDomNode* parent );
    void Load( QDomNode* node );

    bool TooCloseTo( const MarkerInfo* other, qint32 minSeparation ) const;

    static quint32 NextID;

    
};


class MarkerModel : public VectorModel<MarkerInfo>
{
    Q_OBJECT;

public:
    MarkerModel( QVector<MarkerInfo> & source, const QVector<DyeInfo> & dyes  )
        : VectorModel<MarkerInfo>( source )
        , m_Dyes( dyes )
    {
    }

    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

public slots:
    virtual void RefreshData();


private:
    const QVector<DyeInfo> & m_Dyes;
    
};

#endif
