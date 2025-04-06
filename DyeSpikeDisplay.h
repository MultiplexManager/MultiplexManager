#ifndef DYE_SPIKE_DISPLAY_H
#define DYE_SPIKE_DISPLAY_H

#include <QGraphicsRectItem>


class DyeSpikeDisplay : public QGraphicsRectItem
{
 public:
  DyeSpikeDisplay( const QColor & mainColor, int position, 
                    int width, float height,  float horizScale );

	virtual ~DyeSpikeDisplay() { }
 private:
  
};


#endif
