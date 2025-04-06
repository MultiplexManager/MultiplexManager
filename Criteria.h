#ifndef CRITERIA_H
#define CRITERIA_H

#include <QDialog>
#include <QList>

#include <QStandardItemModel>

#include "ui_Criteria.h"

class CriteriaDialog : public QDialog, public Ui::Criteria
{
	Q_OBJECT;
public:
    CriteriaDialog( const QList<qint32> & criteria,
                    bool selectBest,
					QWidget * parent = NULL);
											
	void GetCriteria( QList<qint32> & criteria );
 
																			   
private slots:
    void on_btnMoveUp_clicked();
    void on_btnMoveDown_clicked();
	void on_btnDefaults_clicked();

    void on_listWidget_currentRowChanged( int currentRow );
private:
	bool m_SelectBest;

};


#endif
