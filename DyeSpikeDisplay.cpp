#include "DyeSpikeDisplay.h"

#include <QBrush>

DyeSpikeDisplay::DyeSpikeDisplay( const QColor & mainColor, int position, 
                                    int width, float height, float horizScale)
    : QGraphicsRectItem( QRectF( position * horizScale , 
                               -30, width * horizScale , height ) )
{
    QColor c = mainColor;
    c.setAlphaF( 0.25f );
    setBrush( QBrush( c ) );
    setZValue( -1.0f );
    setToolTip( QString("Dye spike at %1").arg( position ) );
}
