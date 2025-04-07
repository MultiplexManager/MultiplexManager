
#include "ResultsDisplay.h"

#include "LocusDisplay.h"
#include "DyeSpikeDisplay.h"

#include <QGraphicsScene>
#include <QApplication>

#include "utils.h"

#include "DyeModel.h"
#include "MarkerModel.h"
#include <math.h>

#include <QUndoCommand>
#include <QUndoStack>

#include <QPropertyAnimation>
#include <float.h>
#include <QGraphicsView>

static const float MIDDLE_OFFSET = 15.0f;

static const QString REACTION_LABEL( QObject::tr("<center>%1 Plex %2 <br/>Avg Temp: %3</center>") );
static const QString REACTION_LABEL_TOOLTIP( QObject::tr( "Min Temp %1 - Max Temp %2" ) );
QRectF Expand( const QRectF & rect, float ammount )
{
	QRectF r = rect;
	r.setWidth( rect.width() + ammount );
	r.setHeight( rect.height() + ammount );
	r.moveTopLeft( QPointF( rect.left() - ammount * 0.5f,
							rect.top() - ammount * 0.5f ) );

	return r;
}

class DragDropUndoCommand : public QUndoCommand
{
public:
    DragDropUndoCommand( ResultsDisplay* display, LocusDisplay* locus, 
                         float newY, float oldY )
        : m_Display( display ),
          m_Locus( locus ),
          m_NewY( newY ),
          m_OldY( oldY )
    {
        setText( "Moved " + m_Locus->GetMarkerInfo()->m_Name );
    }
    
    virtual void undo()
    {
        m_Display->DropLocusIntoReaction( m_Locus, m_OldY, false );
    }
    
    virtual void redo()
    {
        m_Display->DropLocusIntoReaction( m_Locus, m_NewY, false );
    }
private:
    ResultsDisplay* m_Display;
    LocusDisplay* m_Locus;
    float m_NewY;
    float m_OldY;
    

};
//------------------------------------------------------------------------------    
ResultsDisplay::ResultsDisplay( MultiplexResults* results, 
                                float horizScale, QUndoStack* stack )
    : m_Results( *results )
    , m_HorizScale( horizScale )
    , m_UndoStack( stack )
{

    AddResultsToScene();    

}

void ResultsDisplay::SetNewScale( float horizScale )
{
	removeItem( &m_CantDropHereHintBox );
	clear();
	m_HorizScale = horizScale;
	AddResultsToScene();
}
//------------------------------------------------------------------------------    
void ResultsDisplay::AddResultsToScene()
{
    static const quint32 PxBetweenDyes = 50;
    static const quint32 PxBetweenReactions = 50;


    QFont labelFont( QApplication::font() );
    labelFont.setPixelSize( 10 );

    float yValue = 0;
    
    QList<DyeInfo*> usedDyes;

    QList<float> topOfReactions;
    QList<int> numberOfMarkersList;

    QList<float> averageAnnealTemps;
    QList<float> minAnnealTemps;
    QList<float> maxAnnealTemps;

    for( int r = 0; r < m_Results.m_Reactions.size(); ++r )
    {
        topOfReactions << yValue;
        
        quint32 numOfMarkers = 0;
        float annealTemp = 0;
        float minAnnealTemp = FLT_MAX;
		float maxAnnealTemp = -FLT_MIN;

        MultiplexResults::Reaction & reaction = m_Results.m_Reactions[r];
        for ( int d = 0; d < reaction.m_Dyes.size(); d++ )
        {                
            const MultiplexResults::Dye & dye = reaction.m_Dyes[d];
            
            RowInfo ri;
            ri.m_Results = &m_Results;
            ri.m_ReactionIndex = r;
            ri.m_DyeIndex = d;
            ri.m_YValue = yValue + MIDDLE_OFFSET;
            m_Rows << ri;

			
			if ( usedDyes.indexOf( dye.m_Dye ) == -1 )
            {
                usedDyes << dye.m_Dye; 
            }

            const QColor & color = dye.m_Dye->m_Color;
            
            for ( int p = 0; p < dye.m_Markers.size(); ++p )
            {
                MarkerInfo & marker = *dye.m_Markers[p];
                
                QString label = marker.m_Name + QString(" - %1°").
                    arg( marker.m_AnnealingTemp, 0, 'f', 1 );
                QString toolTip = label + QString( tr( "\nNumber of Alleles: %1\nChromosome: %2\n"
													   " Genetic Location: %3\nHeterozygosity: %4" ) ).
					arg( marker.m_NumberOfAlleles ).
					arg( marker.m_Chromosome ).
					arg( marker.m_GeneticLocation, 0, 'f', 1 ).
					arg( marker.m_Heterozygosity, 0, 'f', 1 );
					
					

                LocusDisplay* display = AddLocus( marker.m_MinimumSize, yValue, 
                                                  marker.m_MaximumSize - marker.m_MinimumSize, 
                                                  color, label,
												  toolTip, 
                                                  labelFont, true );

                
												  display->SetMarkerInfo( & marker );
                numOfMarkers++;
                annealTemp += marker.m_AnnealingTemp;
				minAnnealTemp = min( minAnnealTemp, marker.m_AnnealingTemp );
				maxAnnealTemp = max( maxAnnealTemp, marker.m_AnnealingTemp );
				
            }
            yValue += PxBetweenDyes;
            
        }

        annealTemp /= numOfMarkers;

        averageAnnealTemps << annealTemp;
		minAnnealTemps << minAnnealTemp;
		maxAnnealTemps << maxAnnealTemp;

        
        numberOfMarkersList << numOfMarkers;
        yValue += PxBetweenReactions;
    }

    for ( qint32 d = 0; d < usedDyes.size(); ++d )
    {
        DyeInfo* dye = usedDyes[d];
        for ( qint32 a = 0; a < dye->m_Artifacts.size(); a++ )
        {
            const ArtifactInfo & spike = dye->m_Artifacts[a];
            AddDyeSpike( spike.m_Size, spike.m_Width, yValue, dye->m_Color );
        }
    }

 

    QRectF size = itemsBoundingRect();

    int minX = (int) ( size.left() / m_HorizScale );
    int maxX = (int) ( minX + size.width() / m_HorizScale );

    
    for ( int scale = 0; scale < topOfReactions.size(); ++scale )
    {
        AddScale( minX, maxX, topOfReactions[scale], labelFont );
    }

   
    for( int t = 0; t < topOfReactions.size(); ++t )
    {
        
        float top = topOfReactions[t];
        float bottom = t == topOfReactions.size() - 1 ? yValue  : topOfReactions[t + 1];
        
        bottom -= PxBetweenReactions;

        float mid = ( top + bottom ) * 0.5f;     
        float lineX = ( minX * m_HorizScale ) - 55.0f;
        QString reactionLabel = QString( REACTION_LABEL ).
            arg( numberOfMarkersList[t] ).arg( t + 1 ).arg( averageAnnealTemps[t], 0, 'f', 1 );
		float minTemp = minAnnealTemps[t];
		float maxTemp = maxAnnealTemps[t];
        QGraphicsTextItem* countLabel = new QGraphicsTextItem();
        countLabel->setHtml( reactionLabel );
        countLabel->setFont( labelFont );
		countLabel->setToolTip( REACTION_LABEL_TOOLTIP.arg( minTemp, 0, 'f', 1 ).arg( maxTemp, 0, 'f', 1 ) );
        addItem( countLabel );
        
        m_Labels << countLabel;
        countLabel->setTextWidth( 100 );

        QRectF bounds = countLabel->boundingRect();
        float sizeToOffsetLines = ( bounds.height()  * 0.5f ) + 5.0f;

        countLabel->moveBy( lineX - bounds.width() * 0.5f,
							mid - bounds.height() * 0.5f );
	
        addLine( QLineF( lineX - 5, top, lineX + 5, top ) );
        
        addLine( QLineF( lineX - 5, bottom, lineX + 5, bottom ) );

        addLine( QLineF( lineX, top, lineX, mid - sizeToOffsetLines ) ); 
        addLine( QLineF( lineX, mid + sizeToOffsetLines, lineX, bottom ) ); 
    }
    


    // add a legend
    float bottomOfPage = yValue + 30;
    float legendX =  ( minX * m_HorizScale ) - 20.0f;

    QGraphicsTextItem* legend = new QGraphicsTextItem();
    legend->setHtml( "<b>" + QObject::tr("Legend" ) + "</b>" );
    legend->setFont( labelFont );
    addItem( legend );
    legend->moveBy( legendX,  bottomOfPage );
        
    float legendY = bottomOfPage + 25;
	// make the legend items a fixed
	// size regardless of the scale
	float hackScale = 10.0f;
	float oldScale = m_HorizScale;
	m_HorizScale = hackScale;
    for ( qint32 d = 0; d < usedDyes.size(); ++d )
    {
        DyeInfo* dye = usedDyes[d];
        AddLocus( legendX / hackScale, legendY, 
                  10, 
                  dye->m_Color, dye->m_Name, "",
                  labelFont, false ); 
        legendX += 15 * hackScale;
        
    }
	m_HorizScale = oldScale;

    QRectF rect = itemsBoundingRect();
    rect.adjust( 0.0f, 0.0f, 0.0f, 40.0f );
	setSceneRect( rect );


    QFont cantDropFont( QApplication::font() );
    cantDropFont.setPixelSize( 10 );
    cantDropFont.setBold( true );
    m_CantDropHereHint.setFont( cantDropFont );
    m_CantDropHereHint.setText( "Bort" );
	m_CantDropHereHint.setBrush( QBrush( Qt::black ) );
    m_CantDropHereHint.setZValue( 10 );


    m_CantDropHereHintBox.setZValue( 11 );
	m_CantDropHereHintBox.setPen( QPen() );
	m_CantDropHereHintBox.setBrush( QBrush( Qt::white ) );
	m_CantDropHereHintBox.setVisible( false );

	m_CantDropHereHintBox.setRect( Expand( m_CantDropHereHint.boundingRect(), 5 ));
	m_CantDropHereHint.setParentItem( &m_CantDropHereHintBox );
	m_CantDropHereHint.setPos( 0, 0 );
    m_CantDropHereHint.setVisible( true );
    addItem( &m_CantDropHereHintBox );
	



}

//------------------------------------------------------------------------------    
LocusDisplay* ResultsDisplay::AddLocus(  float x, float y, int size, 
                                         const QColor & color, const QString & label, 
										 const QString & toolTip, 
                                         const QFont & font, bool interactive )  
{
    LocusDisplay* ds = new  LocusDisplay( color, size, label, toolTip, m_HorizScale, font, interactive );
    addItem( ds );
    ds->moveBy( x * m_HorizScale, y );
  
    return ds;
}

//------------------------------------------------------------------------------    
void ResultsDisplay::AddDyeSpike( int x, int width, float height,
                                  const QColor & color )  
{
    addItem( new DyeSpikeDisplay( color, x, width, height, m_HorizScale ) );
}

//------------------------------------------------------------------------------    
void ResultsDisplay::AddScale( int minx, int maxx, 
                               float top,  const QFont & font )
{


    // put labels every 50 pixels or so
    int indicatorDistance = 50;

    // go a bit past the end 
    maxx += 10;

    for ( int x = minx; x < maxx; x += 1 )
    {
        float xPixel = x * m_HorizScale;
        addLine( QLineF( xPixel, top - 15, xPixel, top - 5 ) );
        if ( x % indicatorDistance == 0 )
        {
            QGraphicsTextItem* label = addText( QString::number( x ), font );
            QRectF bounds = label->boundingRect();
            label->setTransform(QTransform::fromTranslate(xPixel - bounds.width() * 0.5f, top -15 - bounds.height()), true);
        }
    }

}

static const float SNAP_DISTANCE = 10.0f;

//------------------------------------------------------------------------------    
void BlendColors( const QColor & mainColor, const QColor& otherColor, QColor* outputColor, float blend1 )
{
    
    qreal r1, g1, b1;
    mainColor.getRgbF ( &r1, &g1, &b1 );
    qreal r2, g2, b2;
    otherColor.getRgbF ( &r2, &g2, &b2 );

    float blend2 = ( 1.0f - blend1 );
    qreal r = clamp( blend1 * r1 +  blend2 * r2, 0.0f, 1.0f );
    qreal g = clamp( blend1 * g1 +  blend2 * g2, 0.0f, 1.0f );
    qreal b = clamp( blend1 * b1 +  blend2 * b2, 0.0f, 1.0f );

    outputColor->setRgbF ( r, g, b );
}

void ResultsDisplay::UpdateLocusOnDrag( LocusDisplay* locus, float mouseY )
{
    float x = locus->x();
    float y = mouseY - MIDDLE_OFFSET;

    // lower bound does a binary search and finds the place where we would
    // want to insert the element, i.e. just after the closest but smallest y value 
    QList<RowInfo>::iterator best = qLowerBound( m_Rows.begin(), m_Rows.end(), y );
    // if it's after the end or before the beginning, 
    // you can't drop it there, and hence show black
    if ( ( best == m_Rows.end() ) || ( y < m_Rows.front().m_YValue ) )
    {
        locus->setPos( x, y - MIDDLE_OFFSET );
        locus->SetBrush( Qt::black );
       
        m_CantDropHereHintBox.setPos( x, y + 25 );
        m_CantDropHereHintBox.setVisible( true );
        m_CantDropHereHint.setText( tr("Can't drop - outside all reactions") );
		m_CantDropHereHintBox.setRect( Expand( m_CantDropHereHint.boundingRect(), 5 ) );
		
        return;
    }
	
    m_CantDropHereHintBox.setVisible( false );
	
    QColor colorA = best->GetColor();
    QColor colorB = colorA;   
    float tween = 1.0f;
	
    //qDebug( "Best: %f %f", best->m_YValue, y );
    float distance = fabs( y - best->m_YValue );
	
    if ( best != m_Rows.begin() )
    {
        // check if the previous element exists,
        // generate a color and tween
        // and store if the previous one is actually
        // closer
        QList<RowInfo>::iterator prev = best - 1;    
        //qDebug( "Prev: %f %f", prev->m_YValue, y );
		
        float prevDistance = fabs( y - prev->m_YValue );
		
        float totalDistance = prevDistance + distance;
        if ( totalDistance > 0 )
        {
            colorB = prev->GetColor();
            tween = 1.0f - ( distance / totalDistance );
        }
		
        if  ( prevDistance < distance )
        {
            best = prev;
            distance = prevDistance;
        }        
    }
	

    QColor color = colorA;
    // create a color blend
    BlendColors( colorA, colorB, &color, tween );
	
    locus->SetBrush( color );

    QString reason;

    
    float outputY;
    // this does a bit of "snapping" so if you're close to 
    // a row for dropping purposes, you can tell
    if ( distance < SNAP_DISTANCE )
    {
        outputY =  best->m_YValue;
    }
    else
    {
        outputY = y;
    }

    locus->setPos( x, outputY - MIDDLE_OFFSET );

    bool ok = best->m_Results->OkToDropMarker( best->m_ReactionIndex, best->m_DyeIndex, locus->GetMarkerInfo(), reason );

    if ( !ok )
    {
        m_CantDropHereHintBox.setPos( x, outputY + 25 );
        m_CantDropHereHintBox.setVisible( true );
        m_CantDropHereHint.setText( reason  );
		m_CantDropHereHintBox.setRect( Expand( m_CantDropHereHint.boundingRect(), 5 ) );
    }

}

QColor RowInfo::GetColor() const
{
    return m_Results->m_Reactions[m_ReactionIndex].m_Dyes[m_DyeIndex].m_Dye->m_Color;
}

bool RowInfo::ContainsMarker( MarkerInfo * marker )
{
    return m_Results->m_Reactions[ m_ReactionIndex ].m_Dyes[ m_DyeIndex ].m_Markers.contains( marker );

}
void ResultsDisplay::DropLocusIntoReaction( LocusDisplay* locus, float mouseY, bool addUndoCommand )
{
    m_CantDropHereHintBox.setVisible( false );

    float x = locus->x();
    float y = mouseY - MIDDLE_OFFSET;


    // lower bound does a binary search and finds the place where we would
    // want to insert the element, i.e. just after the closest but smallest y value 
    QList<RowInfo>::iterator best = qLowerBound( m_Rows.begin(), m_Rows.end(), y );
    // if it's after the end or before the beginning, 
    // you can't drop it there, and hence show black
    if ( ( best == m_Rows.end() ) || ( y < m_Rows.front().m_YValue ) )
    {
        assert( addUndoCommand ); // we should never "miss" dropping if we're 
		// comming from inside an Undo-redo call 

		QPropertyAnimation* p = new QPropertyAnimation( locus, "pos" );
		p->setDuration( 500 );
		//p->setStartValue( locus->pos() );
		p->setEndValue( locus->GetStartDragPos() );

		
		QRectF bRect = locus->sceneBoundingRect();
		bRect.setTopLeft( locus->GetStartDragPos() );
		bRect.setRight( bRect.left() + 50 );
		
		locus->scene()->views()[0]->ensureVisible( bRect );
		
		p->setEasingCurve(QEasingCurve::OutBounce);
        p->start( QAbstractAnimation::DeleteWhenStopped );

		QPropertyAnimation* p2 = new QPropertyAnimation( locus, "brush" );
		p2->setDuration( 500 );
		//p2->setStartValue( locus->GetBrush() );
		p2->setEndValue( locus->GetDyeColor() );
		p2->setEasingCurve(QEasingCurve::OutBounce);
        p2->start( QAbstractAnimation::DeleteWhenStopped );

        return;
    }
	
	
    float distance = fabs( y - best->m_YValue );
	
    if ( best != m_Rows.begin() )
    {
        // check if the previous element exists,
        QList<RowInfo>::iterator prev = best - 1;    
        //qDebug( "Prev: %f %f", prev->m_YValue, y );
		
        float prevDistance = fabs( y - prev->m_YValue );
		
        if  ( prevDistance < distance )
        {
            best = prev;
            distance = prevDistance;
        }        
    }

    
	
	QPropertyAnimation* p = new QPropertyAnimation( locus, "pos" );
	p->setDuration( 500 );
	p->setEndValue( QPointF( x, best->m_YValue - MIDDLE_OFFSET ) );		
	p->setEasingCurve(QEasingCurve::OutBounce);
	p->start( QAbstractAnimation::DeleteWhenStopped );
	
	QPropertyAnimation* p2 = new QPropertyAnimation( locus, "brush" );
	p2->setDuration( 500 );
	p2->setEndValue( best->GetColor() );	
	p2->setEasingCurve(QEasingCurve::OutBounce);	
	p2->start( QAbstractAnimation::DeleteWhenStopped );
	
	// now actually update the reaction object itself, if you
    // dropped it somewhere different


    MarkerInfo* marker = locus->GetMarkerInfo();

    if ( best->ContainsMarker( marker ) )
    {
        return; // it's where it used to be, no action required
    }


    float oldMouseY = -1;
    
    qint32 rowId = 0;
    for( int r = 0; r < m_Results.m_Reactions.size(); ++r )
    {
        MultiplexResults::Reaction & reaction = m_Results.m_Reactions[r];
        for ( int d = 0; d < reaction.m_Dyes.size(); d++ )
        {                
            MultiplexResults::Dye & dye = reaction.m_Dyes[d];

            qint32 index = dye.m_Markers.indexOf( marker );

            if ( index != -1 )
            {
                dye.m_Markers.removeAt( index );
                oldMouseY = m_Rows[rowId].m_YValue;
                break;
            }

            ++rowId;

        }
    }
    
    assert( oldMouseY != -1 );

    best->m_Results->m_Reactions[best->m_ReactionIndex].m_Dyes[best->m_DyeIndex].m_Markers << marker;

    for( int r = 0; r < m_Results.m_Reactions.size(); ++r )
    {
        qint32 totalMarkers = 0;
        MultiplexResults::Reaction & reaction = m_Results.m_Reactions[r];
        float annealTemp = 0;
		float minTemp = FLT_MAX;
		float maxTemp = -FLT_MAX;
        for ( int d = 0; d < reaction.m_Dyes.size(); d++ )
        {                
            MultiplexResults::Dye & dye = reaction.m_Dyes[d];
            totalMarkers +=  dye.m_Markers.size();
            for ( qint32 m = 0; m < dye.m_Markers.size(); ++m )
            {
                annealTemp += dye.m_Markers[m]->m_AnnealingTemp;
				minTemp = min( dye.m_Markers[m]->m_AnnealingTemp, minTemp );
				maxTemp = max( dye.m_Markers[m]->m_AnnealingTemp, maxTemp );
            }
        }

		if ( totalMarkers )
		{
			annealTemp /= totalMarkers;
		}
		else
		{
			annealTemp = 0.0f;
		}

        // fix the labels for the number count
        m_Labels[r]->setHtml( REACTION_LABEL.arg( totalMarkers ).arg( r + 1 ).arg( annealTemp, 0, 'f', 1 ) );
        m_Labels[r]->setToolTip( REACTION_LABEL_TOOLTIP.arg( minTemp, 0, 'f', 1 ).arg( maxTemp, 0, 'f', 1 ) );

    }

	
    QRectF bRect = locus->sceneBoundingRect();
	bRect.setTopLeft( QPointF( x, best->m_YValue - MIDDLE_OFFSET ) );  
	bRect.setRight( bRect.left() + 50 );

	locus->scene()->views()[0]->ensureVisible( bRect );

    if ( addUndoCommand )
    {
        DragDropUndoCommand* undoCmd = new DragDropUndoCommand( this, locus, mouseY, oldMouseY + MIDDLE_OFFSET  );
        m_UndoStack->push( undoCmd );
    }


    
}
