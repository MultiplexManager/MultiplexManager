#ifndef MULTIPLEX_RESULTS_H 
#define MULTIPLEX_RESULTS_H 

#include <QList>
#include <QString>

struct DyeInfo;
struct MarkerInfo;
struct ArtifactInfo;

class QDomDocument;
class QDomNode;
class QTextStream;

struct MultiplexResults;

struct GlobalData
{
	
	qint32 m_NumMarkers;
	qint32 m_NumDyes;	
	qint32 m_NumSelecting;
	qint32 m_MaxNumAlleles;

	float m_MinAnnealTemp;
	float m_MaxAnnealTemp;
    float m_AnnealTempRange;

	float m_MinGeneticLocation;
	float m_MaxGeneticLocation;
    float m_GeneticLocationRange;

    quint32 m_ComplementarityThreshold;

};

typedef float (*RatingFunction)( const MultiplexResults & a_Results, const GlobalData & a_GlobalData );

struct AvailableCriteria
{
	AvailableCriteria( qint32 id, const QString & name, 
					   const QString & toolTip, 
                       bool onlyValidWhenPickingSubset, 
                       RatingFunction function )
		: m_Id( id ), m_Name( name), m_ToolTip( toolTip )
        , m_OnlyValidWhenPickingSubset( onlyValidWhenPickingSubset )
        , m_Function( function )
	{
	}
	
	qint32 m_Id;
	QString m_Name;
	QString m_ToolTip;
    bool m_OnlyValidWhenPickingSubset;
	RatingFunction m_Function;
};


void GetAvailableCriteria( QList<AvailableCriteria> & outList );


void GetRatingFunctions( const QList<qint32> & criteria, 
                         bool selectBest,
                         QList<RatingFunction> & outFunctions );
 

struct MultiplexResults
{

    MultiplexResults() :
        m_Rating( 0.0f ), 
        m_ComplementarityThreshold( 7 )
    {
    }
    
    struct Dye
    {
        DyeInfo* m_Dye;
        QList<MarkerInfo*> m_Markers;        
        
        bool operator==( const Dye & other ) const;


        void Save( QDomDocument* doc, QDomNode* parent );
        void Load( QDomNode* node, 
                   QList<DyeInfo*> & dyes, QList<MarkerInfo*> & markers );
    };

    struct Reaction
    {
        Reaction() : m_TotalMarkers( 0 ) { }
        qint32 m_TotalMarkers;
        QList<Dye> m_Dyes;

        bool operator==( const Reaction & other ) const;

        void Save( QDomDocument* doc, QDomNode* parent );
        void Load( QDomNode* node, 
                   QList<DyeInfo*> & dyes, QList<MarkerInfo*> & markers );
    };


    bool OkToDropMarker( quint32 a_ReactionIndex, quint32 a_DyeIndex, 
                         const MarkerInfo* marker, QString & reason ) const;

    QList<Reaction> m_Reactions;

    float m_Rating;
    
    qint32 m_ComplementarityThreshold; // used to validate when dragging

    void CalculateRating( const QList<RatingFunction> & functions, const GlobalData & globalData );

    // in this case < == better
    bool operator<( const MultiplexResults & other ) const
    {
        return m_Rating < other.m_Rating;
    }

    bool operator==( const MultiplexResults & other ) const;

    void Save( QDomDocument* doc, QDomNode* parent );
    void Load( QDomNode* node, 
               QList<DyeInfo*> & dyes, QList<MarkerInfo*> & markers );


	void SaveResultsAsText( const QString & fileName ) const;

	void SaveResultsAsText( QTextStream & stream, qint32 index = -1 ) const;

};

#endif
