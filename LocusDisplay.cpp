#include "LocusDisplay.h"

#include "utils.h"
#include <QPen>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QGraphicsSceneMouseEvent>
#include <QFont>

#include <math.h>

#include "ResultsDisplay.h"
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QApplication>

//------------------------------------------------------------------------------    
LocusDisplay::LocusDisplay( const QColor & mainColor, int markerWidth, 
                            const QString & name, 
							const QString & toolTip, 
							float horizScale, const QFont & font, bool interactive ) :
    m_Text( this ),
    m_DyeColor( mainColor ),
    m_Dragging( false ),
    m_Interactive( interactive )
{

    float top = 0.0f;
    float left = 0.0f;
    float width = markerWidth * horizScale;
    float height = 30.0f;

    float right = left + width;
    float bottom = top + height;

    float arcSize = min( 20.0f, ( min( height ,  width ) * 0.5f ) );
  
    QFont myFont( font );
    myFont.setBold( true );

    m_Text.setPlainText( name );
    m_Text.setFont( myFont );


    QRectF bounds = m_Text.boundingRect();
    QRectF boundsCopy( bounds );
    // if it's too big, chop some stuff off
    if ( bounds.width() > width - arcSize ) 
    {
        // this is pretty inefficent, but I don't envisage a million
        // of these  objects being created
        // check that the ...s will 
        m_Text.setPlainText( "..." );
        bounds = m_Text.boundingRect();
      
        // if at least the ... will fit, try putting some text in
        if ( bounds.width() < width - arcSize ) 
        {
            bounds = boundsCopy;
            QString chopped( name );
            while ( bounds.width() > width - arcSize ) 
            {
                chopped.chop( 1 );
                m_Text.setPlainText( chopped + "..." );
                bounds = m_Text.boundingRect();
            }
        }
        else
        {
            // otherwise don't bother - the tooltip is there
            m_Text.setPlainText( "" );
        }
    }

    m_Text.translate(  arcSize * 0.5f, 
                       ( height * 0.5f ) - ( bounds.height() * 0.5f ) );

    //m_Text.setTextWidth( width );
    setToolTip( toolTip );
  

    QPen myPen;
    myPen.setWidth( 2 );
  
    QPainterPath roundRectPath;

    roundRectPath.moveTo(right, top + arcSize );
    roundRectPath.arcTo( right - arcSize, top, arcSize, arcSize, 0.0, 90.0);
  
    roundRectPath.lineTo( left + arcSize , top );
 
    roundRectPath.arcTo( left, top, arcSize, arcSize, 90.0, 90.0);
  
    roundRectPath.lineTo( left, bottom - arcSize );
  
    roundRectPath.arcTo( left , bottom - arcSize, arcSize, arcSize, 180.0, 90.0);
  
    roundRectPath.lineTo( right - arcSize, bottom );
  
    roundRectPath.arcTo( right - arcSize, bottom - arcSize, arcSize, arcSize, 270.0, 90.0);
  
    roundRectPath.closeSubpath();


    
    setPath( roundRectPath );
    setPen ( myPen );
    
    m_TopLeftGrad = QPointF(left, top);
    m_BottomRightGrad = QPointF(left + width * 0.5f , bottom);


    SetBrush( m_DyeColor );


	QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
	effect->setOffset( 4, 4 );
	effect->setBlurRadius( 6 );
	setGraphicsEffect( effect );
}

LocusDisplay:: ~LocusDisplay()
{
}


//------------------------------------------------------------------------------    
void LocusDisplay::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{
    if (!m_Interactive ) return;
	
	if ( !event->buttons().testFlag( Qt::LeftButton ) ) 
	{
		QGraphicsPathItem::mouseMoveEvent( event );
		return;
	}

	if ( !m_Dragging ) return;

    QPointF scenePos = event->scenePos( );
    
    ResultsDisplay* display = static_cast<ResultsDisplay*>( scene() );

    QRectF bRect = boundingRect();
	bRect.setRight( bRect.left() + 50 );
	this->ensureVisible( bRect );
	display->UpdateLocusOnDrag( this, scenePos.y() );




	QGraphicsPathItem::mouseMoveEvent( event );

    
	
}


bool LocusDisplay::eventFilter(QObject * /*obj*/, QEvent * event)
{
	if ( event->type() == QEvent::MouseButtonRelease )
	{
		QMouseEvent * me = (QMouseEvent*) event;
		if ( me->button() != Qt::LeftButton ) 
		{
            return false;
		}
		if ( !m_Dragging ) return false;
		
		qApp->removeEventFilter( this );
		mouseReleaseEvent( NULL ); // doesn't use the event anyway
		return true;

	}

	return false;
}




//------------------------------------------------------------------------------    
void LocusDisplay::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    if (!m_Interactive ) return;

	if ( !event->buttons().testFlag( Qt::LeftButton ) ) 
	{
		QGraphicsPathItem::mouseMoveEvent( event );
		return;
	}

    m_Dragging = true;
    m_StartDragPos = pos();
    setZValue( 1.0f );

#ifdef Q_OS_MAC
	qApp->installEventFilter( this );
#endif

}

//------------------------------------------------------------------------------    
void LocusDisplay::mouseReleaseEvent ( QGraphicsSceneMouseEvent * /*event*/ )
{
    if (!m_Interactive ) return;
	if ( !m_Dragging ) return;

    ResultsDisplay* display = static_cast<ResultsDisplay*>( scene() );
    float y = sceneBoundingRect().bottomLeft().y();
    display->DropLocusIntoReaction( this, y );

    m_Dragging = false;
    setZValue( 0.0f );


}

//------------------------------------------------------------------------------    
void LocusDisplay::SetBrush( const QColor & mainColor  )
{   
	m_BrushColor = mainColor;
    QLinearGradient grad( m_TopLeftGrad, m_BottomRightGrad );
    grad.setColorAt( 0, Qt::white );
    grad.setColorAt( 1, mainColor );
    
    QBrush brush( grad );

    setBrush( brush );

}

const QColor & LocusDisplay::GetBrush() const
{
	return m_BrushColor;
}
