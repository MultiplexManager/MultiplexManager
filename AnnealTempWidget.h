#ifndef __ANNEAL_TEMP_WIDGET_H__
#define __ANNEAL_TEMP_WIDGET_H__

#include <QWidget>

#include "ui_AnnealTempWidget.h"

class AnnealTempWidget : public QWidget, private Ui::AnnealTempWidget
{
    Q_OBJECT
    
public:
    AnnealTempWidget( const QString & forward, const QString & reverse, 
                      QObject* eventFilter, QWidget* parent = NULL );

    QLineEdit* GetLineEdit()
    {
        return edtTmp;
    }

    QPushButton* GetButton()
    {
        return btnCalcTemp;
    }

private slots:
    void on_btnCalcTemp_clicked();

private:
    QString m_Forward;
    QString m_Reverse;
    QObject* m_EventFilter;
}; 


#endif
