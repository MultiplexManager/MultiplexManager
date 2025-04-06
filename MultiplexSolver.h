#ifndef MULTIPLEX_SOLVER_H
#define MULTIPLEX_SOLVER_H

#include <QObject>

#include "MarkerModel.h"
#include "DyeModel.h"
#include "MultiplexResults.h"

#include "ComplementarityCheck.h"

#include "OptionData.h"

class MultiplexResults;
class Checker;

class MultiplexSolver : public QThread
{
    Q_OBJECT;

public:
    MultiplexSolver( QVector<MarkerInfo> & markerData,
                     QVector<DyeInfo> & dyeData, OptionData* options, QObject* parent = NULL );
    

    // hand ownership of the results back to the app
    const QList<MultiplexResults> & GetResults() const;

    void Cancel();

protected:
    void run();


signals:
    void log( QString displayName ) const;
    void progress( QString displayName, float value ) const;
    void complete( bool );
    void cancelled();

private:

    enum SolveResult
    {
        OK,
        NoSolution,
        Cancelled
    };

    enum LogType
    {
        Message,
        Warning,
        Header,
        Preformat,
    };

    SolveResult Solve();

    bool CalculateComplementarity( const QPair< Checker, Checker > & a_MarkerA,
                                   const QPair< Checker, Checker > & a_MarkerB  ) const;


    void Log( const QString & txt, LogType type = Message ) const;

    SolveResult RecurseBrute( QList<MultiplexResults> & results, 
                              QList<quint32> & indicies,
                              qint32 totalMarkers, 
                              int depth, 
                              float progress ) const;


    SolveResult SimulateAnnealing( QList<MultiplexResults> & results ) const;


 
    void AddMarkerToResult( MultiplexResults & newResult, MarkerInfo* currentMarker ) const;

    void ValidateResult( const MultiplexResults & result ) const ;  

	void PadOutReactionWithAllDyes( MultiplexResults::Reaction * reaction ) const;

    QVector<MarkerInfo> & m_MarkerData;
    QVector<DyeInfo> & m_DyeData;
    OptionData m_Options;

    // make some lookup structures for this
    // we know we will need

    // mapping from marker to dyes it can be used with
    QHash<MarkerInfo*, QList<DyeInfo*> > m_MarkerDyeCombos;
    // mapping from dye to markers it can be used with
    QHash<DyeInfo*, QList<MarkerInfo*> > m_DyeMarkerCombos;

    // mapping from marker list of markers that can be used in the same reaction
    // ( ie. passes the complementarity check )
    QHash<MarkerInfo*, QList<MarkerInfo*> > m_MarkerMarkerInSameReactionCombos;
    // this is markers -> list of markers OK to have in the same dye colour
    // in the same reaction
    QHash<MarkerInfo*, QList<MarkerInfo*> > m_MarkerMarkerInSameDyeCombos;

    QList<MultiplexResults> m_Results;

    QList<RatingFunction> m_RatingFunctions;
	
	GlobalData m_GlobalData;

    volatile bool m_Cancelled;

};

#endif
