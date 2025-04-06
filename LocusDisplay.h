
#ifndef LOCUS_DISPLAY
#define LOCUS_DISPLAY

#include <QGraphicsPathItem>
#include <QGraphicsTextItem>

struct MarkerInfo;

class LocusDisplay : public QObject, public QGraphicsPathItem 
{
	Q_OBJECT
	Q_PROPERTY(QPointF pos READ pos WRITE setPos)
	Q_PROPERTY(QColor brush READ GetBrush WRITE SetBrush)
		
public:
    LocusDisplay( const QColor & mainColor, int width,
                  const QString & name, 
				  const QString & toolTip, 
				  float horizScale, const QFont & font, 
                  bool interactive ) ;
	virtual ~LocusDisplay();

    void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );

	bool eventFilter(QObject *obj, QEvent *event);
    void SetMarkerInfo( MarkerInfo* marker )
    {
        m_MarkerInfo = marker;
    }

    MarkerInfo* GetMarkerInfo() const
    {
        return m_MarkerInfo;
    }

    void SetBrush( const QColor & mainColor );
	const QColor & GetBrush() const;

    const QPointF & GetStartDragPos() const
    {
        return m_StartDragPos;
    } 

    const QColor & GetDyeColor() const
    {
        return m_DyeColor;
    }

    void SetDyeColor( const QColor & color )
    {
        m_DyeColor = color;
        SetBrush( m_DyeColor );
    }

private:

    
    QGraphicsTextItem m_Text;
    QColor m_DyeColor;

    QPointF m_TopLeftGrad;
    QPointF m_BottomRightGrad;

    bool m_Dragging;
    QPointF m_StartDragPos;
    QColor m_FadeToColor;
   
    MarkerInfo * m_MarkerInfo;

    bool m_Interactive;
	QColor m_BrushColor;
};

#endif
