#ifndef RESULTS_DISPLAY_H
#define RESULTS_DISPLAY_H

#include <QList>
#include <QString>
#include <QColor>

#include <QGraphicsScene>

#include "MultiplexResults.h"

#include <QGraphicsTextItem>
#include <QGraphicsRectItem>

class LocusDisplay;
class QGraphicsTextItem;

class QUndoStack;
class MarkerInfo;

struct RowInfo
{
    MultiplexResults* m_Results;
    quint32 m_ReactionIndex;
    quint32 m_DyeIndex;        
    float m_YValue;
    bool operator<( float y )
    {
	return m_YValue < y;
    }
    
    bool operator==( float y )
    {
	return m_YValue == y;
    }
    
    QColor GetColor() const;

    bool ContainsMarker( MarkerInfo * marker );

};

class ResultsDisplay : public QGraphicsScene
{
	Q_OBJECT;
public:

    ResultsDisplay(  MultiplexResults* results, float horizScale, QUndoStack* undoStack  ); 

    //void GetColorForPosition( float x, float y, float * outputY, QColor * color );

    //bool DropIntoReaction( float * y, QColor* color );
    void UpdateLocusOnDrag( LocusDisplay* locus, float mouseY );

    // old mouse y = 
    void DropLocusIntoReaction( LocusDisplay* locus, float mouseY, bool addUndoCommand = true );

	const MultiplexResults & GetResults() 
	{
		return m_Results;
	}

	void SetNewScale( float horizScale );

private:

    void AddResultsToScene();
    LocusDisplay* AddLocus( float x, float y, int size, 
                            const QColor & color, const QString & label, 
							const QString & toolTip, 
                            const QFont & font, bool interactive );
    
    void AddDyeSpike( int x, int width, float height,
                      const QColor & color );

    void AddScale( int minx, int maxx, 
                   float top,  const QFont & font );



    MultiplexResults & m_Results;

    float m_HorizScale;
    QList<RowInfo> m_Rows;

    QGraphicsSimpleTextItem m_CantDropHereHint;
    QGraphicsRectItem m_CantDropHereHintBox;

    QList<QGraphicsTextItem*> m_Labels;
    QUndoStack* m_UndoStack;

};


 
#endif
