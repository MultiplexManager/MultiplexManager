#include <AnnealTempWidget.h>
#include "CalculateAnnealingTemp.h"

#include "MainWindow.h"

AnnealTempWidget::AnnealTempWidget( const QString & forward, const QString & reverse, 
                                    QObject* eventFilter, QWidget* parent )
    : QWidget( parent )
    , m_Forward( forward )
    , m_Reverse( reverse )
    , m_EventFilter( eventFilter )
{
    setupUi( this );
    if ( m_Forward.isEmpty() || m_Reverse.isEmpty() )
    {
        btnCalcTemp->setEnabled( false );
    }
    
    setFocusProxy( edtTmp );
    //btnCalcTemp->installEventFilter( m_EventFilter );
    //edtTmp->installEventFilter( m_EventFilter );
}


void AnnealTempWidget::on_btnCalcTemp_clicked()
{

    if ( m_Forward.isEmpty() || m_Reverse.isEmpty() )
    {
        return;
    }

	const OptionData & optionData = MainWindow::Instance().GetOptionData();

    float temp  = CalculateAnnealingTemp( m_Forward, m_Reverse, 
										  optionData.m_PrimerConcentration, 
										  optionData.m_DegreesLowerForAnnealing );

    //edtTmp->removeEventFilter( m_EventFilter );

    edtTmp->setText( QString::number ( temp , 'f', 2 ));
    
    //edtTmp->installEventFilter( m_EventFilter );
}
