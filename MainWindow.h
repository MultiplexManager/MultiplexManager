#ifndef MY_WINDOW_H
#define MY_WINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "ui_Main.h"

#include "MarkerModel.h"
#include "DyeModel.h"
#include "ArtifactModel.h"

#include "OptionData.h"

#include "MultiplexResults.h"

class QUndoStack;
class MultiplexSolver;

class MainWindow : public QMainWindow, private Ui::MainWindow 
{
    Q_OBJECT;
    
public:
    MainWindow();
    virtual ~MainWindow();
              
public:
    void OpenFile( const QString & fileName ); // called my main if a command line argument exists
	
	static MainWindow & Instance() 
	{
		assert( s_Instance != NULL );
		return *s_Instance;
	}

	const OptionData & GetOptionData() const
	{
		return m_OptionData;
	}

private slots:
    void on_btnAddMarker_clicked();
    void on_btnRemoveMarker_clicked();

    void on_btnAddDye_clicked();
    void on_btnRemoveDye_clicked();
    
    void on_btnAddArtifact_clicked();
    void on_btnRemoveArtifact_clicked();

    void markerTableViewSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );
    void dyeTableViewSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );

    void artifactTableViewSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );

    void on_action_Save_triggered();
    void on_action_Save_As_triggered();
    void on_action_Open_triggered();

    void on_action_New_triggered();

    void on_action_Export_Results_triggered();
	void on_action_Export_Results_as_text_triggered();

    void on_action_Export_All_Results_as_PDF_triggered();
    void on_action_Export_All_Results_as_Text_triggered();

	void on_action_Export_Results_Log_as_Text_triggered();

	void on_action_Online_Help_triggered();
    void on_action_About_triggered();
    
    void on_action_Export_CSV_Template_triggered();

    void on_action_Import_CSV_Data_triggered();

	void on_actionCalculate_All_Annealing_Temperatures_triggered();
	void on_actionSet_Parameters_for_Annealing_Temp_Calculation_triggered();

    void on_chkUseAllMarkers_toggled();

    void on_spnNumLoci_valueChanged( int i );
    void on_spnSelectBest_valueChanged( int i );
    void on_spnMinDistance_valueChanged( int i );
    void on_spnCompThreshold_valueChanged( int i );
    void on_spnIterationsToRun_valueChanged( int i );

    void on_btnEditCriteria_clicked();

    void on_btnSolveMultiplex_clicked();

    void solverProgress( QString text, float value );
    void solverComplete( bool result );
    void solverCancelled( );
    void solverLog( const QString & html );
	void flushSolverLog();

    void updateSelectBestSpinner();

    void on_btnCancelSolver_clicked();
	
	void scale_slider_changed( int value );


protected:
    virtual void closeEvent( QCloseEvent* event );

private:

    QString GetSaveFileName();

    bool MaybeSave( const QString & message );

    void Save( const QString & filename );
    void ResetData();

    void CleanUpSolver();

    void UpdateOptionData();


    void CreateResultsDisplay();

	

    // Models for the table views
    MarkerModel* m_MarkerModel;
    DyeModel*    m_DyeModel;
    ArtifactModel* m_ArtifactModel;

    // actual data
    QVector<MarkerInfo> m_MarkerData;
    QVector<DyeInfo> m_DyeData;

    OptionData m_OptionData;

    QString m_CurrentFileName;


    // Undo System
    QUndoStack* m_UndoStack;

    // solver
    MultiplexSolver* m_Solver;
    QList<QWidget*> m_TabWidgets;

    QList<MultiplexResults> m_CurrentResults;

	QString m_TempLog;
    QString m_ActualLog;
	QTimer  m_Timer;

	static MainWindow* s_Instance;
};



#endif
