#include <QGraphicsView>

class MyGraphicsView : public QGraphicsView
{
	Q_OBJECT;

public:
    MyGraphicsView( QWidget * parent = NULL ) :
        QGraphicsView( parent )
    {
    }

	// the default QGaphicsView returns a size based on the scene size
	// size for the size hint, it wants to try and fit everything
	// in. But I want scrollbars, and it not to rescale the window
    virtual QSize sizeHint() const
    {
        return QSize();
	}
	
};
