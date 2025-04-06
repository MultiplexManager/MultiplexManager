#ifndef __ANNEAL_TEMP_PARAMS_H__
#define __ANNEAL_TEMP_PARAMS_H__

#include <QDialog>
#include "ui_AnnealingTempParams.h"

class AnnealTempParams : public QDialog, Ui::AnnealTempParams
{
public:
	AnnealTempParams ( QWidget * parent = 0, Qt::WindowFlags f = 0 );

	void setParams( float concentration, float degrees );
	void getParams( float & concentration, float & degrees );
};

#endif
