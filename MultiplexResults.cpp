#include "MultiplexResults.h"

#include <QObject>
#include <QFile>
#include <QTextStream>

#include "MarkerModel.h"
#include "DyeModel.h"

#include <cassert>
#include <float.h>
#include <math.h>
#include "utils.h"

#include "DomTemplateHelper.h"
#include "ComplementarityCheck.h"

//------------------------------------------------------------------------------    
bool SortBySize( const MarkerInfo* m1, const MarkerInfo* m2 )
{
    return m1->m_MinimumSize < m2->m_MinimumSize;
}

float StandardDeviation( const QList<float> & list )
{
	qint32 n = list.size();
	if( n == 0)
	{
        return 0.0f;
	}

    float mean = 0;
    for ( qint32 i = 0; i < n; ++i )
    {
        mean += list[i];
    }

    mean /= n;

    float sumSqr = 0;
    for ( qint32 i = 0; i < n; ++i )
    {
        float diff = mean - list[i];
        float diffSqr = diff * diff;
        sumSqr += diffSqr;
    }
        
    float variance = sumSqr /= n;
    return sqrt( variance );

	
}
// each of these functions calculats a
// rating between 0-1 inclusive for the reaction
// we then combine the ratings in user specified order.
float CalculateAnnealTempRating( const MultiplexResults & results, const GlobalData & globalData  )
{
    
    if ( globalData.m_AnnealTempRange == 0 ) // all the markers have the same anneal temp ( I assume the default )
    {
        return 0.0f;
    }

	float avStandardDev = 0;

    quint32 size = results.m_Reactions.size();
    
    for ( quint32 r = 0; r < size; ++r )
    {
        const MultiplexResults::Reaction & reaction = results.m_Reactions[r];
        
		QList<float> tempList;
 
		quint32 dSize = reaction.m_Dyes.size();
        assert( dSize > 0 ); // what's the point if there's no dyes
        for ( quint32 d = 0; d < dSize; ++d )
        {
            const MultiplexResults::Dye & dye = reaction.m_Dyes[d];
            quint32 mSize = dye.m_Markers.size();
        
            for ( quint32 m = 0; m < mSize; ++m )
            {
                const MarkerInfo* marker = dye.m_Markers[m];
                float temp = marker->m_AnnealingTemp;
				// normalise temp
				temp = ( temp -  globalData.m_MinAnnealTemp ) / globalData.m_AnnealTempRange;
				tempList << temp;
            }       
        }

		float standardDev = StandardDeviation( tempList );
		avStandardDev += standardDev;
    }

	avStandardDev /= size;

    assert( ( avStandardDev >=0 ) && ( avStandardDev <= 1.0f ) );
    return avStandardDev;
}


float CalculateSpaceRating( const MultiplexResults & results, const GlobalData & )
{
    quint32 size = results.m_Reactions.size();
    
    quint32 numItems = 0;
    float totalPercentage = 0;

    for ( quint32 r = 0; r < size; ++r )
    {
        const MultiplexResults::Reaction & reaction = results.m_Reactions[r];

        quint32 dSize = reaction.m_Dyes.size();
        assert( dSize > 0 ); // what's the point if there's no dyes
        for ( quint32 d = 0; d < dSize; ++d )
        {
            const MultiplexResults::Dye & dye = reaction.m_Dyes[d];
            quint32 mSize = dye.m_Markers.size();

            if ( mSize == 0 )
            {
                continue;
            }

            const QList<MarkerInfo*> & sortedMarkers = dye.m_Markers;

            int begin = sortedMarkers.first()->m_MinimumSize;
            int end = sortedMarkers.last()->m_MaximumSize;
            int totalDistance = ( end - begin ) + 1;  // make sure the total distance is never 0
            
            //qDebug( "Total Distance %i ", totalDistance );
            int unusedDistance = 0;
            
            for ( quint32 m = 0; m < mSize - 1; ++m )
            {
                const MarkerInfo* markerA = sortedMarkers[m];
                const MarkerInfo* markerB = sortedMarkers[m + 1];

                assert( markerB->m_MinimumSize >= markerA->m_MaximumSize );
                unusedDistance += markerB->m_MinimumSize - markerA->m_MaximumSize;                    
            }

            //qDebug( "Unused Distance %i ", unusedDistance );
            
            int usedDistance = totalDistance - unusedDistance;
            assert( usedDistance >= 0 );
            float percentage = static_cast<float>( usedDistance ) /
                static_cast<float>( totalDistance );
            //qDebug( "Percentage %f\n", percentage );

            
            totalPercentage += percentage;
            ++numItems;
        }
    }

    //qDebug( "Num Items: %u\n", numItems );
    float spaceRating = 1.0f - ( totalPercentage / numItems );
    //qDebug( "Space Rating: %f\n", spaceRating );
    
    assert( ( spaceRating >= 0.0f )  && ( spaceRating <= 1.0f ) );  

    return spaceRating;

}


float CalculateDyeRating( const MultiplexResults & results, const GlobalData &  )
{
    QList<DyeInfo*> usedDyes;
    QList<DyeInfo*> totalDyes;

    quint32 size = results.m_Reactions.size();

    for ( quint32 r = 0; r < size; ++r )
    {
        const MultiplexResults::Reaction & reaction = results.m_Reactions[r];
        
        quint32 dSize = reaction.m_Dyes.size();
        assert( dSize > 0 ); // what's the point if there's no dyes
        for ( quint32 d = 0; d < dSize; ++d )
        {
            const MultiplexResults::Dye & dye = reaction.m_Dyes[d];
            totalDyes << dye.m_Dye;

            quint32 mSize = dye.m_Markers.size();
            if ( mSize == 0 ) 
            {
                continue;
            }

            if ( usedDyes.indexOf( dye.m_Dye ) == -1 )
            {
                usedDyes << dye.m_Dye;
            }
        }
    }

    float res =  static_cast<float>(usedDyes.size() ) /
        static_cast<float>(totalDyes.size() );
    assert( ( res >=0 ) && ( res <= 1.0f ) );
    return res;


}

float CalculateReactionRating( const MultiplexResults & results, const GlobalData & globalData  )
{
    // at worst, there could be one reaction per
    // marker. 
   
    float res =  static_cast<float>( results.m_Reactions.size() ) /
        ( static_cast<float>( globalData.m_NumSelecting  ) );
    assert( res >=0  );
    res = min( 1.0f, res );
	return res;

}


float CalculateHeterozygosityRating( const MultiplexResults & results, const GlobalData &  )
{
    quint32 size = results.m_Reactions.size();
    
    qint32 numMarkers = 0;
    float heterozygosity = 0;
    for ( quint32 r = 0; r < size; ++r )
    {
        const MultiplexResults::Reaction & reaction = results.m_Reactions[r];
        
        quint32 dSize = reaction.m_Dyes.size();
        assert( dSize > 0 ); // what's the point if there's no dyes
        for ( quint32 d = 0; d < dSize; ++d )
        {
            const MultiplexResults::Dye & dye = reaction.m_Dyes[d];
            quint32 mSize = dye.m_Markers.size();
        
            for ( quint32 m = 0; m < mSize; ++m )
            {
                const MarkerInfo* marker = dye.m_Markers[m];
                heterozygosity += marker->m_Heterozygosity;
                ++numMarkers;
            }
        }
    }

    heterozygosity /= numMarkers;
    float res = 1.0f - heterozygosity;
    assert( ( res >=0 ) && ( res <= 1.0f ) );
    return res;

}

float CalculateNumAllelesRating( const MultiplexResults & results, const GlobalData & globalData )
{
    quint32 size = results.m_Reactions.size();

    qint32 numMarkers = 0;

    qint32 totalNumAlleles = 0;

    for ( quint32 r = 0; r < size; ++r )
    {
        const MultiplexResults::Reaction & reaction = results.m_Reactions[r];
        
        quint32 dSize = reaction.m_Dyes.size();
        assert( dSize > 0 ); // what's the point if there's no dyes
        for ( quint32 d = 0; d < dSize; ++d )
        {
            const MultiplexResults::Dye & dye = reaction.m_Dyes[d];
            quint32 mSize = dye.m_Markers.size();
        
            for ( quint32 m = 0; m < mSize; ++m )
            {
                const MarkerInfo* marker = dye.m_Markers[m];
                totalNumAlleles += marker->m_NumberOfAlleles;
                ++numMarkers;
            }
        }
    }


    float averageAlleles = (float) totalNumAlleles;
    averageAlleles /= ( numMarkers * globalData.m_MaxNumAlleles );

    float res = 1.0f - averageAlleles;
    assert( ( res >=0 ) && ( res <= 1.0f ) );
    return res;
}



float CalculateGeneicLocationRating( const MultiplexResults & results, const GlobalData & globalData )
{
    
    if ( globalData.m_GeneticLocationRange == 0 ) // no difference in genetic location, just return 0.
    {
        return 0.0f;
    }

    quint32 size = results.m_Reactions.size();


    QMap< QString, QList<float> > geneticLocations;

    for ( quint32 r = 0; r < size; ++r )
    {
        const MultiplexResults::Reaction & reaction = results.m_Reactions[r];
        
        quint32 dSize = reaction.m_Dyes.size();
        assert( dSize > 0 ); // what's the point if there's no dyes
        for ( quint32 d = 0; d < dSize; ++d )
        {
            const MultiplexResults::Dye & dye = reaction.m_Dyes[d];
            quint32 mSize = dye.m_Markers.size();
        
            for ( quint32 m = 0; m < mSize; ++m )
            {
                const MarkerInfo* marker = dye.m_Markers[m];
                float rescaledGeneticLoc = ( marker->m_GeneticLocation - globalData.m_MinGeneticLocation ) / 
                    globalData.m_GeneticLocationRange;

                geneticLocations[marker->m_Chromosome]<< rescaledGeneticLoc;
            }
        }
    }
    
    
    float avergageStandardDev = 0;
    for (  QMap< QString, QList<float> >::const_iterator it = geneticLocations.begin();
           it != geneticLocations.end(); ++it )
    {
        
        float standardDev = StandardDeviation( *it );
        assert( standardDev >= 0.0f && standardDev <= 1.0f );
        avergageStandardDev += standardDev;

    }

    avergageStandardDev /= geneticLocations.size();

 

    float res =  1.0f - avergageStandardDev; 
    assert( ( res >=0 ) && ( res <= 1.0f ) );
    return res;

    
}


void GetAvailableCriteria( QList<AvailableCriteria> & outList )
{
	outList << AvailableCriteria( 0,
								  QObject::tr( "Minimize Total Number of Reactions"),
								  QObject::tr( "Minimize the total number of reactions." ),
                                  false,
								  CalculateReactionRating );
	
	/*
	// We've decided this isn't really a sensible critiea.
	// We can easily turn this on later if we want.
	outList << AvailableCriteria( 1,
								  QObject::tr( "Minimize Total Dyes used" ),
								  QObject::tr( "Minimize the total number of used dyes." ),
                                  false,
								  CalculateDyeRating );
	*/
	outList << AvailableCriteria( 2, 
								  QObject::tr( "Maximize Spacing Between Markers in the Same Dye" ), 
								  QObject::tr( "Maximize Spacing between markers in the same dye." ), 
                                  false,
								  CalculateSpaceRating );

	outList << AvailableCriteria( 3, 
								  QObject::tr("Minimize Difference in Annealing Temperature" ), 
								  QObject::tr("Minimize difference in annealing temperature." ), 
                                  false,
								  CalculateAnnealTempRating );

    outList << AvailableCriteria( 4, 
								  QObject::tr("Maximize Average Heterozygosity" ), 
								  QObject::tr("Maximize Average Heterozygosity of all selected markers. \n"
                                              "Only enabled if the Select Best N option is used." ), 
                                  true,
								  CalculateHeterozygosityRating );

    outList << AvailableCriteria( 5, 
								  QObject::tr("Maximize Number of Alleles" ), 
								  QObject::tr("Maximize number of alleles all selected markers. \n"
                                              "Only enabled if the Select Best N option is used." ), 
                                  true,
								  CalculateNumAllelesRating );

    outList << AvailableCriteria( 6, 
								  QObject::tr("Maximize Standard Deviation of Genetic Location" ), 
								  QObject::tr("Maximize Standard Deviation of Genetic Location. \n"
                                              "Only enabled if the Select Best N option is used." ), 
                                  true,
								  CalculateGeneicLocationRating );
   



	
	
}

void GetRatingFunctions( const QList<qint32> & criteria, 
                         bool selectBest,
                         QList<RatingFunction> & outFunctions )
{
    QList<AvailableCriteria> list;
    GetAvailableCriteria( list );
    for ( qint32 i = 0; i < criteria.size(); ++i )
    {
        qint32 crit = criteria[i];
        for ( qint32 j = 0; j < list.size(); ++j )
        {
            if ( list[j].m_Id == crit )
            {
                // don't add the function if it doesn't make sense
                // to, because we're not select best
                if ( !selectBest && list[j].m_OnlyValidWhenPickingSubset )
                {
                    continue;
                }
                outFunctions << list[j].m_Function;
                break;
            }
        }
    }

}
 

//------------------------------------------------------------------------------    
void MultiplexResults::CalculateRating( const QList<RatingFunction> & functions, const GlobalData & globalData )
{
    // sort the markers by size
    // the space rating wants them sorted,
    // and it means we can compare to results to see if they are the same
    
    quint32 size = m_Reactions.size();
    for ( quint32 r = 0; r < size; ++r )
    {
        MultiplexResults::Reaction & reaction = m_Reactions[r];
        quint32 dSize = reaction.m_Dyes.size();
        for ( quint32 d = 0; d < dSize; ++d )
        {
            MultiplexResults::Dye & dye = reaction.m_Dyes[d];            
            qSort( dye.m_Markers.begin(), dye.m_Markers.end(), SortBySize );
        }
    }

    size = functions.size();

    m_Rating = 0.0f;
    float scale = 1.0f;
    for ( qint32 i = size - 1; i >= 0; --i )
    {
        float rating = (*functions[i])( *this, globalData );
        assert( ( rating >=0 ) && ( rating <= 1.0f ) );
        m_Rating += ( rating * scale );
        scale *= 10.0f;
    }

    m_ComplementarityThreshold = globalData.m_ComplementarityThreshold;

}

//------------------------------------------------------------------------------    
void MultiplexResults::Save( QDomDocument* doc, QDomNode* parent )
{
    QDomElement result = doc->createElement("result");
    
    for ( qint32 i = 0; i < m_Reactions.size(); ++i )
    {
        m_Reactions[i].Save( doc, &result );
    }

    parent->appendChild( result );

    AddElement( doc, parent, "Rating", &m_Rating );
    AddElement( doc, parent, "CompCheckThreshold", &m_ComplementarityThreshold );
}

//------------------------------------------------------------------------------    
void MultiplexResults::Load( QDomNode* node, QList<DyeInfo*> & dyes, QList<MarkerInfo*> & markers )
{

    QDomElement reaction = node->firstChildElement("reaction" );
    while( !reaction.isNull() )
    {
        Reaction r;
        r.Load( &reaction, dyes, markers );
        m_Reactions << r;

        reaction = reaction.nextSiblingElement("reaction");
    }

    ReadElement( node, "Rating", &m_Rating );
    ReadElement( node, "CompCheckThreshold", &m_ComplementarityThreshold );
}

//------------------------------------------------------------------------------    
void MultiplexResults::Reaction::Save( QDomDocument* doc, QDomNode* parent )
{

    QDomElement reaction = doc->createElement("reaction");
    
    for ( qint32 i = 0; i < m_Dyes.size(); ++i )
    {
        m_Dyes[i].Save( doc, &reaction );
    }

    parent->appendChild( reaction );
}

//------------------------------------------------------------------------------    
void MultiplexResults::Reaction::Load( QDomNode* node, QList<DyeInfo*> & dyes, QList<MarkerInfo*> & markers )
{

    m_TotalMarkers = 0;
    QDomElement dye = node->firstChildElement("dye" );
    while( !dye.isNull() )
    {
        Dye d;
        d.Load( &dye, dyes, markers );
        m_Dyes << d;
        m_TotalMarkers += d.m_Markers.size();

        dye = dye.nextSiblingElement("dye");
    }
    
    
}
//------------------------------------------------------------------------------    
bool MultiplexResults::Reaction::operator==( const Reaction & other ) const
{
    return ( m_TotalMarkers == other.m_TotalMarkers ) && ( m_Dyes == other.m_Dyes );
}
//------------------------------------------------------------------------------    
void MultiplexResults::Dye::Save( QDomDocument* doc, QDomNode* parent )
{
    QDomElement dye = doc->createElement("dye");
    
    AddElement( doc, &dye, "dye", &m_Dye->m_Id );

    QDomElement markers = doc->createElement("markers");

    for ( int i = 0; i < m_Markers.size(); ++i )
    {
        AddElement( doc, &markers, "marker", &m_Markers[i]->m_Id);
    }

    dye.appendChild( markers );

    parent->appendChild( dye );

}

void MultiplexResults::Dye::Load( QDomNode* node, QList<DyeInfo*> & dyes, QList<MarkerInfo*> & markers )
{
    quint32 dyeId;
    ReadElement( node, "dye", &dyeId );

    
    DyeInfo* dye = FindDeref( dyes, dyeId );
    if ( ! dye )
    {
        return;
    }
    
    m_Dye = dye;

    
    QDomElement markersNode = node->firstChildElement( "markers" );
    if ( markersNode.isNull() )
    {
        return;
    }


    QDomElement markerNode = markersNode.firstChildElement( "marker" );
    
    while ( !markerNode.isNull() )
    {
        
        QDomCharacterData textNode =  markerNode.firstChild().toCharacterData();        
        quint32 markerId = textNode.data().toUInt();


        MarkerInfo* marker = FindDeref( markers, markerId );
        if ( marker )
        {
            assert( !m_Markers.contains( marker ) );
            m_Markers << marker;
        }            

        markerNode = markerNode.nextSiblingElement("marker");
    } 

    
}

//------------------------------------------------------------------------------    
bool MultiplexResults::Dye::operator==( const Dye & other ) const
{
    return ( m_Dye == other.m_Dye ) && ( m_Markers == other.m_Markers );
}

//------------------------------------------------------------------------------    
bool MultiplexResults::OkToDropMarker( quint32 a_ReactionIndex, quint32 a_DyeIndex, 
                                       const MarkerInfo* marker, QString & reason ) const
{

    const Reaction & reaction = m_Reactions[a_ReactionIndex];
    
    const Dye & dye = reaction.m_Dyes[ a_DyeIndex];

    quint32 markerMin = marker->m_MinimumSize;
    quint32 markerMax = marker->m_MaximumSize;

    // check artifacts
    const QVector<ArtifactInfo> & artifacts = dye.m_Dye->m_Artifacts;
    for ( qint32 i = 0; i < artifacts.size(); ++i )
    {
        const ArtifactInfo* artifact = &artifacts[i];
        qint32 delta = ( artifact->m_Width - 1 ) >> 1;
        quint32 spikeMin = artifact->m_Size - delta;
        quint32 spikeMax = artifact->m_Size + delta;
                    
        // they dont intersect, record the fact
        if ( ! ( ( spikeMin > markerMax) || (markerMin > spikeMax) ) )
        {                        
            reason = QObject::tr("Marker %1 overlaps Dye %2 artifact at %3:%4").arg( marker->m_Name ).
                arg( dye.m_Dye->m_Name ).arg( spikeMin ).arg(spikeMax ) ;
            return false;
        }
    }
    
    Checker markerForward( marker->m_Name + " Forward ", marker->m_Forward );
    Checker markerReverse( marker->m_Name + " Reverse ", marker->m_Reverse );


    for ( qint32 i = 0; i < dye.m_Markers.size(); ++i )
    {
        if ( dye.m_Markers[i] == marker )
        {
            continue;
        }
        
        const MarkerInfo* otherMarker = dye.m_Markers[i];

        bool tooClose = marker->TooCloseTo( otherMarker, 0 );
        
        if ( tooClose )
        {
            
            quint32 otherMarkerMin = otherMarker->m_MinimumSize;
            quint32 otherMarkerMax = otherMarker->m_MaximumSize;
            reason = QObject::tr("Marker %1 overlaps Marker %2 at %3:%4").arg( marker->m_Name ).
                arg( otherMarker->m_Name ).arg( otherMarkerMin ).arg( otherMarkerMax ) ;
            return false;
        }
     

        // check complementariy 
        Checker otherMarkerForward( otherMarker->m_Name + " Forward ", otherMarker->m_Forward );
        Checker otherMarkerReverse( otherMarker->m_Name + " Reverse ", otherMarker->m_Reverse );

        
    }
    
    
    for ( qint32 d = 0; d < reaction.m_Dyes.size(); ++d )
    {
        
        const Dye & dye = reaction.m_Dyes[ d ];
        for ( qint32 m = 0; m < dye.m_Markers.size(); ++m )
        {
            if ( dye.m_Markers[m] == marker )
            {
                continue;
            }
            
            const MarkerInfo* otherMarker = dye.m_Markers[m];
            
            // check complementariy 
            Checker otherMarkerForward( otherMarker->m_Name + " Forward ", otherMarker->m_Forward );
            Checker otherMarkerReverse( otherMarker->m_Name + " Reverse ", otherMarker->m_Reverse );
            
            
            Checker::CheckerResults res[4];
            res[0] = markerForward.CheckAgainst( otherMarkerForward );
            res[1] = markerForward.CheckAgainst( otherMarkerReverse );
            res[2] = markerReverse.CheckAgainst( otherMarkerForward );
            res[3] = markerReverse.CheckAgainst( otherMarkerReverse );
            
            for ( quint32 r = 0; r < 4; ++r )
            {
                if ( res[r].score >= m_ComplementarityThreshold ) 
                {
                    reason = QObject::tr("Marker %1 complementary with Marker %2").arg( marker->m_Name ).
                        arg( otherMarker->m_Name ) ;
                    return false;
                }
            }              
        }
    }
    
    return true;
}


bool MultiplexResults::operator==( const MultiplexResults & other ) const
{
    return  ( m_Rating == other.m_Rating ) && ( m_Reactions == other.m_Reactions );
}


void MultiplexResults::SaveResultsAsText( const QString & fileName ) const
{

	QFile file( fileName );
    file.open( QIODevice::WriteOnly | QIODevice::Text );
    QTextStream out( &file );

    SaveResultsAsText( out );

}

void MultiplexResults::SaveResultsAsText( QTextStream & out, qint32 index  ) const
{
 
    if ( index != -1 ) // we need a results number header
    {        
        out << "===========================================================================" << endl;
        out << "Reaction Set " << index << endl;
        out << "===========================================================================" << endl;
        
        out << endl << endl;
    }

    
    quint32 size = m_Reactions.size();

    for ( quint32 r = 0; r < size; ++r )
    {
		out << "Reaction " << ( r + 1 ) << endl;
		
		const MultiplexResults::Reaction & reaction = m_Reactions[r];
        
        quint32 dSize = reaction.m_Dyes.size();
        for ( quint32 d = 0; d < dSize; ++d )
        {
            const MultiplexResults::Dye & dye = reaction.m_Dyes[d];

            quint32 mSize = dye.m_Markers.size();
            if ( mSize == 0 ) 
            {
                continue;
            }

			out << "    Dye  " << dye.m_Dye->m_Name  << endl;
			
			for ( quint32 i = 0; i  < mSize; ++i )
			{
				out << "        " << dye.m_Markers[i]->m_Name;
				out << "\t" << dye.m_Markers[i]->m_Forward;
				out << "\t" << dye.m_Markers[i]->m_Reverse;
				out << endl;

			}
			
        }
    }

    if ( index != -1 )
    {
         out << endl << endl;
    }

}
