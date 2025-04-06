#include "AnnealTempParams.h"



AnnealTempParams::AnnealTempParams ( QWidget * parent , Qt::WindowFlags f  )
	: QDialog( parent, f )
{
	setupUi( this );
}

void AnnealTempParams::setParams( float concentration, float degrees )
{
	spinConcentration->setValue( (quint32) concentration );
	spinDegrees->setValue( degrees );
}


void AnnealTempParams::getParams( float & concentration, float & degrees )
{
	concentration = (float) spinConcentration->value(); 
	degrees = spinDegrees->value();
}
