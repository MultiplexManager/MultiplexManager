#include "MultiplexSolver.h"
#include "MultiplexResults.h"

#include "utils.h"

#include <float.h>
#include <QTime>



#define CHECK_CANCEL() if ( m_Cancelled ) return Cancelled;

static const qint32 NUM_RESULTS_TO_KEEP = 5;


MultiplexSolver::MultiplexSolver( QVector<MarkerInfo> & markerData,
                                  QVector<DyeInfo> & dyeData, OptionData* options,
                                  QObject* parent  )
    : QThread( parent), 
      m_MarkerData( markerData ),
      m_DyeData( dyeData ),
      m_Options( *options ),
      m_Cancelled( false )
{
 
    // OK, so the basic algorithm is

    // 1. work out which markers can never be a certain dye color
    //    because they lie across spikes
    // 2  Work out which markers can never be the same dye in a reaction
    //    because they are too close together
    // 3  Generate a list of indicies which indicate the order to insert the markers in the reaction
    // 4  Insert them all
    // 5  Calculate a rating for this reaction
    // 6  If this is better than the best, store it
    // 7  Either make this the current state, or not, based on a random value
    //    dependent on how close we are to the end
    // 8  Randomly swap the order around
    // 9  goto 4 ( a large number of times )


    GetRatingFunctions( m_Options.m_Criteria, !m_Options.m_UseAllMarkers, m_RatingFunctions );

	m_GlobalData.m_NumMarkers = m_MarkerData.size();
	m_GlobalData.m_NumDyes = m_DyeData.size();
	m_GlobalData.m_NumSelecting = m_Options.m_UseAllMarkers ? m_MarkerData.size() : m_Options.m_SelectBest;
	m_GlobalData.m_MaxNumAlleles = 0;
	m_GlobalData.m_MaxAnnealTemp = 0.0f;
	m_GlobalData.m_MinAnnealTemp = FLT_MAX;
	m_GlobalData.m_MaxGeneticLocation = 0.0f;
	m_GlobalData.m_MinGeneticLocation = FLT_MAX;

    m_GlobalData.m_ComplementarityThreshold = m_Options.m_ComplementarityThreshold;

	for ( qint32 i = 0; i < m_MarkerData.size(); ++i )
	{
		m_GlobalData.m_MaxNumAlleles = max( m_GlobalData.m_MaxNumAlleles, 
											m_MarkerData[i].m_NumberOfAlleles );
		m_GlobalData.m_MaxGeneticLocation = max( m_GlobalData.m_MaxGeneticLocation, 
												 m_MarkerData[i].m_GeneticLocation );

		m_GlobalData.m_MinGeneticLocation = min( m_GlobalData.m_MinGeneticLocation, 
												 m_MarkerData[i].m_GeneticLocation );
		
		m_GlobalData.m_MaxAnnealTemp = max( m_GlobalData.m_MaxAnnealTemp, 
												 m_MarkerData[i].m_AnnealingTemp );

		m_GlobalData.m_MinAnnealTemp = min( m_GlobalData.m_MinAnnealTemp, 
											m_MarkerData[i].m_AnnealingTemp );
											
	}

    m_GlobalData.m_GeneticLocationRange = 
        m_GlobalData.m_MaxGeneticLocation - m_GlobalData.m_MinGeneticLocation;


    m_GlobalData.m_AnnealTempRange = 
        m_GlobalData.m_MaxAnnealTemp - m_GlobalData.m_MinAnnealTemp;

}


void MultiplexSolver::run()
{
   
    setPriority( QThread::IdlePriority );
	
    SolveResult result = Solve();
	
	switch (result )
    {
		case OK:
			emit( complete( true ) );
			break;
		case NoSolution:
			emit( complete( false ) );
			break;
		case Cancelled:        
			emit( cancelled() );    
    };

	
}

MultiplexSolver::SolveResult MultiplexSolver::Solve()
{
    float markerProgress = 1.0f / static_cast<float>( m_MarkerData.size() );
    float progressValue = 0.0f;

    Log( tr("Checking Markers Against Artifacts"), Header );
    QTime timer;
    
    timer.start();
    
    for ( qint32 m = 0; m < m_MarkerData.size(); ++m )
    {
        emit( progress( tr("Checking Markers Against Artifacts"), progressValue ) );

        MarkerInfo* marker = &m_MarkerData[m];
        quint32 markerMin = marker->m_MinimumSize;
        quint32 markerMax = marker->m_MaximumSize;

        for ( qint32 d = 0; d < m_DyeData.size(); ++d )
        {
            DyeInfo * dye = &m_DyeData[d];
            
            // if we are forcing this marker to a particular dye, and
            // this isn't it, skip to the next dye
            if ( ( marker->m_ForceDye != -1 ) && ( (qint32) dye->m_Id != marker->m_ForceDye ) )
            {
                continue;
            }
            
            if ( dye->m_Artifacts.size() )
            {
                bool bad = false;
                for ( qint32 a = 0; a < dye->m_Artifacts.size(); ++a )
                {
                    ArtifactInfo* artifact = &dye->m_Artifacts[a];
                    qint32 delta = ( artifact->m_Width - 1 ) >> 1;
                    quint32 spikeMin = artifact->m_Size - delta;
                    quint32 spikeMax = artifact->m_Size + delta;
                    
                    // they dont intersect, record the fact
                    if ( ! ( ( spikeMin > markerMax) || (markerMin > spikeMax) ) )
                    {                        
                        Log( tr("Marker %1 overlaps Dye %2 artifact at %3:%4").arg( marker->m_Name ).
                             arg( dye->m_Name ).arg( spikeMin ).arg(spikeMax ) );
                        bad = true;
                    }
                    
                    CHECK_CANCEL(); // call this inside loops so we have a chance to get out
                }

                if ( !bad )
                {
                    // store off the fact that this dye can be used with this marker
                    // and vice versa in some hash maps for easy lookup later
                    m_DyeMarkerCombos[dye].append( marker );
                    m_MarkerDyeCombos[marker].append( dye );
                }
            }
            else // no artifacts for that dye, so it's all good
            {
                m_DyeMarkerCombos[dye].append( marker );
                m_MarkerDyeCombos[marker].append( dye );
            }

        }

        
        if ( m_MarkerDyeCombos.value(marker).size() == 0 )
        {
            if  ( marker->m_ForceDye == -1 )
            {
                Log( tr( "The Marker %1 intersected artifacts in all the dyes and hence can't be used." ).
                     arg( marker->m_Name ), Warning );
            }
            else
            {
                Log( tr( "The Marker %1 intersected artifacts in the dye it was forced to,  and hence can't be used." ).
                     arg( marker->m_Name ), Warning );
            }
            return NoSolution;
        }


        progressValue += markerProgress;                      
    }

    emit( progress( tr("Checking Markers Against Artifacts"), 1.0f ) );         

    Log( tr( "Time taken to Check Markers Against artifacts: %1 sec").
		 arg( static_cast<float>( timer.restart() ) / 1000.0f ) );
 
	

    progressValue = 0.0f;

    Log( tr("Checking Markers Against Other Markers"), Header );
    
    // create a list of checkers up front.
    // this saves running the regex and string to latin1 conversion
    // every time we check a marker against another marker
    // the pair is for forward and reverse sequences
    QList< QPair< Checker, Checker > > complementarityCheckers;

    for ( qint32 i = 0; i < m_MarkerData.size(); ++i )
    {
        MarkerInfo* marker = &m_MarkerData[i];
        
        Checker markerForward( marker->m_Name + " Forward ", marker->m_Forward );
        Checker markerReverse( marker->m_Name + " Reverse ", marker->m_Reverse );

        complementarityCheckers << qMakePair( markerForward, markerReverse );
        
    }


    for ( qint32 m1 = 0; m1 < m_MarkerData.size(); ++m1 )
    {
        emit( progress( tr("Checking Markers Against Other Markers"), progressValue ) );
        
        MarkerInfo* marker1 = &m_MarkerData[m1];
        for ( qint32 m2 = m1 + 1 ; m2 < m_MarkerData.size(); ++m2 )
        {
            MarkerInfo* marker2 = &m_MarkerData[m2];
            if ( !marker1->TooCloseTo( marker2, m_Options.m_MinDistanceBetweenDyesOfSameColor ) )
            {
                // record the fact that these markers can be used together in the same reaction
                m_MarkerMarkerInSameDyeCombos[marker1].append( marker2 );
                m_MarkerMarkerInSameDyeCombos[marker2].append( marker1 );
            }
            else
            {
                Log( tr("%1 is too close to %2").arg( marker1->m_Name ).arg( marker2->m_Name ) );
            }

            // this value is the maximum complementarity of 
            // marker1 forward vs marker2 forward and marker1 forward vs marker2 reverse etc
            //
            bool goodTogether =  CalculateComplementarity( complementarityCheckers[m1], 
                                                           complementarityCheckers[m2] );
            if ( goodTogether )
            {
                m_MarkerMarkerInSameReactionCombos[marker1].append( marker2 );
                m_MarkerMarkerInSameReactionCombos[marker2].append( marker1 );
            }
 
        }

        // the [] here is deliberate, to ensure there is an entry for 
        // marker1
        if ( m_MarkerMarkerInSameDyeCombos[ marker1 ].size() == 0 )
        {
            Log(tr( "The Marker %1 was too close to all the other markers "
                    "and hence must be in a dye by itself." ).arg( marker1->m_Name ), Warning );

        }

        if ( m_MarkerMarkerInSameReactionCombos[ marker1 ].size() == 0 )
        {
            Log( tr( "The Marker %1 was complementary to all the other markers "
                     "and hence must be in a reaction by itself." ).arg( marker1->m_Name ),Warning );

        }

        CHECK_CANCEL();

        progressValue += markerProgress;            
    }

    emit( progress( tr("Checking Markers Against Other Markers"), 1.0f ) );

    Log( tr( "Time taken to Check Markers Against Other Markers: %1 sec").
		 arg( static_cast<float>( timer.restart() ) / 1000.0f ) );
 
    
    MultiplexSolver::SolveResult result;
	if (  m_MarkerData.size() <= 6 )
	{
		QList<quint32> indicies;
		result = RecurseBrute( m_Results, indicies, 
							   m_Options.m_UseAllMarkers ? m_MarkerData.size() : m_Options.m_SelectBest, 
							   0, 0.0f );

	}
	else
	{
		result = SimulateAnnealing( m_Results );
	}
    switch ( result )
    {
    case MultiplexSolver::OK:
	    emit( progress( tr("Complete!"), 0.0f ) );

		Log( tr( "Time taken to Generate Results: %1 sec").
			 arg( static_cast<float>( timer.restart() ) / 1000.0f ) );
 
	    break;
    case MultiplexSolver::Cancelled:
	    emit( progress( tr("Cancelled!"), 0.0f ) );
	    break;
    case MultiplexSolver::NoSolution:
	    emit( progress( tr("No Solution!"), 0.0f ) );
	    break;

    }

        
    return result;

}

const QList<MultiplexResults> & MultiplexSolver::GetResults() const
{
    ValidateResult( m_Results[0] );
    return m_Results;
}


MultiplexSolver::SolveResult MultiplexSolver::RecurseBrute(  QList<MultiplexResults> & results, 
                                                             QList<quint32> & indicies,
                                                             qint32 totalMarkers, 
                                                             int depth, float progressValue ) const
{
    qint32 size = indicies.size();

    if ( size == totalMarkers )
    {
        quint32 index;
        MultiplexResults currentResult;
        foreach ( index, indicies )
        {
            AddMarkerToResult( currentResult, &m_MarkerData[index] );
        }

		PadOutReactionWithAllDyes( &currentResult.m_Reactions.last() );

        //ValidateResult( currentResult );

        currentResult.CalculateRating( m_RatingFunctions, m_GlobalData );
        for ( qint32 i = 0; i < results.size(); ++i )
        {
            // it already is in the list, we don't care.
            if ( results[i] == currentResult )
            {
                return OK;
            }
        }

        
        // we have a complete reaction, check if it's in the best 
        if ( results.size() < NUM_RESULTS_TO_KEEP )
        {
            results << currentResult; // shove the current result in there
            qSort( results );
        }
        else
        {
            // if it's better than the last element in the list
            if ( currentResult < results.last() )
            {
                results.last() = currentResult;
                qSort( results );
            }
        }

        return OK;
    }
  
    CHECK_CANCEL();
    
    
    //qint32 numToAdd = totalMarkers - size;
    for ( qint32 i = 0; i < totalMarkers; ++i )
    {
        if ( indicies.contains( i ) )
        {
            continue;
        }
        
        indicies << i;
        
        SolveResult res = RecurseBrute( results, indicies, totalMarkers, depth + 1, progressValue );

        indicies.pop_back();
        
        if ( res == Cancelled ) return Cancelled; // bail all the way out
		
        if ( depth == 0 )
        {
            progressValue += 1.0f / ( (float) totalMarkers );
            emit( progress( tr("Bruting problem"), progressValue ) );
        } else  if ( depth == 1 )
        {
            progressValue += 1.0f / ( ( float) ( totalMarkers * ( totalMarkers - 1 ) ) );
            emit( progress( tr("Bruting problem"), progressValue ) );
        }
       
		
    }

    return OK;
}

inline quint32 RandIndex( quint32 num )
{
    return my_rand() % num;
}


inline float RandFloat( )
{
    return ( (float) my_rand() ) / ( ( float ) RAND_MAX );
}

MultiplexSolver::SolveResult MultiplexSolver::SimulateAnnealing( QList<MultiplexResults> & results ) const
{
    
    emit( progress( tr("Running Simulated Annealing"), 1.0f ) );

    my_srand(0); // init random number generator to a known seed for consitent results

    bool justAddResult = true;
    
    qint32 numMarkers =  m_MarkerData.size();

    QList<quint32> indicies;
    for ( qint32 i = 0; i < numMarkers; ++i )
    {
        indicies <<  i;
    }
    quint32 numToInsert = m_Options.m_UseAllMarkers ? numMarkers : m_Options.m_SelectBest;

    float lastResultRating = FLT_MAX;

    bool allDone = false;

    quint32 totalIterations = m_Options.m_IterationsToRun * 1000000;


    quint32 iteration = 0;

    float chanceOfChoosingWorseResult = 1.0f;

    do
    {

		quint32 numToPickFrom = chanceOfChoosingWorseResult < 0.05f ? numMarkers : numToInsert;  // number we intend to insert
        QList<quint32> neighbour = indicies;
        quint32 indexA, indexB;
        do
        {
            indexA = RandIndex( numToInsert ); // make sure one of the things we swap is actaully in the 
            // towrards the very end, make sure we just try and optimise the order of the markers
			// we have
			indexB = RandIndex( numToPickFrom );
		
        }
        while( indexA == indexB );

        std::swap( neighbour[indexA], neighbour[indexB] );

        quint32 index;
        MultiplexResults currentResult;
        
        for ( quint32 i = 0; i < numToInsert; ++i )
        {
            index = neighbour[i];
            AddMarkerToResult( currentResult, &m_MarkerData[index] );
            
        } 
        //ValidateResult( currentResult );
		PadOutReactionWithAllDyes( &currentResult.m_Reactions.last() );

        currentResult.CalculateRating( m_RatingFunctions, m_GlobalData );

        bool alreadyInBestList = false;
        for ( qint32 i = 0; i < results.size(); ++i )
        {
            // it already is in the list, we don't care.
            if ( results[i] == currentResult )
            {
                alreadyInBestList = true;
                break;
            }

        }

        if ( !alreadyInBestList )
        {
            // we have a complete reaction, check if it's in the best 
            if ( justAddResult )
            {
                results << currentResult; // shove the current result in there
                qSort( results );
                if ( results.size() ==  NUM_RESULTS_TO_KEEP )
                {
                    justAddResult = false;
                }
            }
            else
            {
                // if it's better than the last element in the list
                if ( currentResult < results.last() )
                {
                    results.last() = currentResult;
                    qSort( results );
                }
            }
        }

        // this is the crux of simulated annealing: to start of with, 
        // we basically sample at random, then as we proceed the probability
        // of chooosing a worse result than our current one falls to zero
        // (i.e. a simple greedy search )
        // This means we're less likely to get stuck at a local minima
        
        float randValue = RandFloat();

        if ( ( randValue < chanceOfChoosingWorseResult  ) 
          || (  currentResult.m_Rating < lastResultRating ) )         
        /*if (  currentResult.m_Rating < lastResultRating ) */
        {
            indicies = neighbour;
            lastResultRating = currentResult.m_Rating;
        }
            
            
        if ( iteration % 100 == 0 )
        {
            CHECK_CANCEL();
            
            float progressValue = ( (float) iteration )/ ( (float) totalIterations  );
            emit( progress( tr("Running Simulated Annealing"), progressValue ) );   

            // as we progress, the chance of choosing a worse result
            // during our random walk falls to zero.
            chanceOfChoosingWorseResult = 1.0f - progressValue;
            if ( chanceOfChoosingWorseResult < 0.05f )
            {
                chanceOfChoosingWorseResult = 0.0f; // make sure we switch to full greedy mode close ot the end 
                // ( we'd get close, but we want to make sure it gets there )
            }
            if ( iteration == totalIterations )
            {
                allDone = true;
            }

        }
        

        ++iteration;
        
    } 
    while ( !allDone );

    
    emit( progress( tr("Running Simulated Annealing"), 1.0f ) );

    return OK;
}

void MultiplexSolver::AddMarkerToResult( MultiplexResults & result, MarkerInfo* currentMarker ) const
{
    if  ( result.m_Reactions.size() == 0 ) 
    {
        result.m_Reactions << MultiplexResults::Reaction();
        result.m_Reactions.last().m_TotalMarkers = 0;
    }
    
    MultiplexResults::Reaction * currentReaction = &result.m_Reactions.last();
    if ( currentReaction->m_Dyes.size() == 0 )
    {
        currentReaction->m_Dyes << MultiplexResults::Dye();
        currentReaction->m_Dyes.last().m_Dye =  &m_DyeData[0];
    }
    

    


    bool badDye = false;
    bool badReaction = false;

    MultiplexResults::Dye* currentDye;
    do
    {
        badDye = false;
        badReaction = false;
        currentDye = &currentReaction->m_Dyes.last();
        DyeInfo* dyeInfo = currentDye->m_Dye;
        
        assert( m_MarkerDyeCombos.contains( currentMarker ) );
        badDye = !m_MarkerDyeCombos.value(currentMarker).contains( dyeInfo );
        

        if ( !badDye ) // no point checking if we already are bad
        {
            assert (  m_MarkerMarkerInSameDyeCombos.contains( currentMarker ) );
            const QList<MarkerInfo*> & niceMarkers = m_MarkerMarkerInSameDyeCombos.value(currentMarker);
            for ( qint32 i = 0; i < currentDye->m_Markers.size(); ++i )
            {
                if ( niceMarkers.indexOf( currentDye->m_Markers[i] ) == - 1)
                {
                    badDye = true;
                    break;
                }
            }
        }

        
        const QList<MarkerInfo*> & niceMarkersForReaction = m_MarkerMarkerInSameReactionCombos.value( currentMarker );

        badReaction |= currentReaction->m_TotalMarkers >= m_Options.m_MaxLociPerReaction;

        // now check for markers that can't exist in the same reaction!
        if ( !badReaction )
        {
            qint32 numDyes = currentReaction->m_Dyes.size();
            for ( int i = 0; i < numDyes; ++i )
            {
                MultiplexResults::Dye * dye = &currentReaction->m_Dyes[i];
                for ( qint32 j = 0; j < dye->m_Markers.size(); ++j )
                {
                    if ( !niceMarkersForReaction.contains( dye->m_Markers[j] ) )
                    {
                        badReaction = true;
                        break;
                    }
                }            
            }
        }
          

        if ( badReaction )
        {
			PadOutReactionWithAllDyes( currentReaction ); 
            result.m_Reactions << MultiplexResults::Reaction();
            currentReaction = &result.m_Reactions.last();
            currentReaction->m_Dyes << MultiplexResults::Dye();
            currentDye = &currentReaction->m_Dyes.last();
            currentDye->m_Dye =  &m_DyeData[0];
        }
        else  if ( badDye ) // we can't put the current marker in the current dye
        {             // either because of a spike, or because
                      // there's a marker already there that is too close

            // if the reaction is full already, start a new one
            int numDyesInReaction = currentReaction->m_Dyes.size();
            if ( numDyesInReaction == m_DyeData.size() ) 
            {
               
                
                result.m_Reactions << MultiplexResults::Reaction();
                currentReaction = &result.m_Reactions.last();
                currentReaction->m_Dyes << MultiplexResults::Dye();
                currentDye = &currentReaction->m_Dyes.last();
                currentDye->m_Dye =  &m_DyeData[0];

            }
            else
            {
                // add a new dye            
                currentReaction->m_Dyes << MultiplexResults::Dye();
                currentDye = &currentReaction->m_Dyes.last();
                currentDye->m_Dye =  &m_DyeData[numDyesInReaction];
                
            }
            
        }

    } while ( badDye || badReaction );

    assert( m_DyeMarkerCombos.value( currentDye->m_Dye ).contains( currentMarker ) );
    assert( m_MarkerDyeCombos.value( currentMarker ).contains( currentDye->m_Dye ) );
    currentDye->m_Markers << currentMarker;
    ++currentReaction->m_TotalMarkers;

    

}

void MultiplexSolver::ValidateResult( const MultiplexResults & result ) const
{
    for( int r = 0; r < result.m_Reactions.size(); ++r )
    {
        const MultiplexResults::Reaction & reaction = result.m_Reactions[r];
        for ( int d = 0; d < reaction.m_Dyes.size(); d++ )
        {
            
            const MultiplexResults::Dye & dye = reaction.m_Dyes[d];
            DyeInfo * dyeInfo = dye.m_Dye;

            for ( int p = 0; p < dye.m_Markers.size(); ++p )
            {
                MarkerInfo* marker = dye.m_Markers[p];
                assert( m_DyeMarkerCombos.value( dyeInfo ).contains( marker ) );
                assert( m_MarkerDyeCombos.value( marker ).contains( dyeInfo ) );

            }
        }
    }
}

void MultiplexSolver::Cancel()
{
    m_Cancelled = true;
}


void MultiplexSolver::PadOutReactionWithAllDyes( MultiplexResults::Reaction * reaction ) const
{
	
        
	int numDyesInReaction = reaction->m_Dyes.size();
	while ( numDyesInReaction < m_DyeData.size() ) 
	{
		
		// add a new dye            
		reaction->m_Dyes << MultiplexResults::Dye();
		MultiplexResults::Dye * currentDye = &reaction->m_Dyes.last();
		currentDye->m_Dye =  &m_DyeData[numDyesInReaction];
		++numDyesInReaction;
	
        
	}
}

void MultiplexSolver::Log( const QString & txt, LogType type ) const
{
    QString coloredText = txt;

    if ( type != Preformat )
    {
        for ( qint32 d = 0; d < m_DyeData.size(); ++d )
        {
            DyeInfo * dye = &m_DyeData[d];
            QString coloredName = "<font color='" + dye->m_Color.name() + "'>" + dye->m_Name  +"</font>";
            coloredText.replace( dye->m_Name, coloredName );        
        }
    }

    switch ( type )
    {
    case Message:
        emit( log( "<br>" + coloredText ) ) ;
        break;
    case Warning:
        emit( log( "<br><font color='#ff0000'>" + coloredText + "</font>" )  ) ;
        break;
    case Header:
        emit( log( "<hr><h2>" + coloredText + "</h2>" ) ) ;
        break;
    case Preformat:        
        emit( log( "<pre>" + txt + "</pre>" ) ) ;
        break;
    
    }

}

bool MultiplexSolver::CalculateComplementarity( const QPair< Checker, Checker > & a_MarkerA,
                                                const QPair< Checker, Checker > & a_MarkerB  ) const
{
    
    const Checker & marker1Forward( a_MarkerA.first );
    const Checker & marker1Reverse( a_MarkerA.second );
    
    const Checker & marker2Forward( a_MarkerB.first );
    const Checker & marker2Reverse( a_MarkerB.second );

    Checker::CheckerResults res1 = marker1Forward.CheckAgainst( marker2Forward );
    Checker::CheckerResults res2 = marker1Forward.CheckAgainst( marker2Reverse );
    Checker::CheckerResults res3 = marker1Reverse.CheckAgainst( marker2Forward );
    Checker::CheckerResults res4 = marker1Reverse.CheckAgainst( marker2Reverse );
    
    bool bad = false;
    QString resultsLog;

    if ( res1.score >= m_Options.m_ComplementarityThreshold )
    {
        resultsLog += marker1Forward.Display( marker2Forward, res1 );
        bad = true;
    }


    if ( res2.score >= m_Options.m_ComplementarityThreshold )
    {
        resultsLog += marker1Forward.Display( marker2Reverse, res2 );
        bad = true;
    }
    

    if ( res3.score >= m_Options.m_ComplementarityThreshold )
    {
        resultsLog += marker1Reverse.Display( marker2Forward, res3 );
        bad = true;
    }
    
    if ( res4.score >= m_Options.m_ComplementarityThreshold )
    {
        resultsLog += marker1Reverse.Display( marker2Reverse, res4 );
        bad = true;
    }
    

    if ( bad )
    {
        Log( resultsLog, Preformat );
    }

    return !bad;
}



