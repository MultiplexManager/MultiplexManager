#include "MainWindow.h"
#include "MarkerModel.h"
#include "DyeModel.h"
#include "DyeModelDelegate.h"
#include "MarkerModelDelegate.h"
#include "MarkerModelProxy.h"
#include "ArtifactModelDelegate.h"

#include <QMessageBox>
#include <QDialog>
#include <QItemSelection>
#include <QStandardItemModel>

#include <QDesktopServices>

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QFileDialog>
#include <QCloseEvent>

#include "utils.h"


#include "DomTemplateHelper.h"
#include <MultiplexSolver.h>
#include "VectorModelUndoCommands.h"
#include "ControlUndoCommands.h"

#include "ResultsDisplay.h"
#include "ModelUndoCommand.h"

#include "ui_About.h"

#include <cassert>

#include <QUndoStack>

#include "Criteria.h"
#include "CalculateAnnealingTemp.h"
#include "MyGraphicsView.h"

#include "AnnealTempParams.h"

#include <QPrinter>

static const float VERSION = 1.2f;
static const QString APPLICAITON_NAME = QObject::tr("Multiplex Manager %1").arg( VERSION, 0, 'f', 1 );

MainWindow* MainWindow::s_Instance = NULL;

// forwad ref
QString GetSaveFileNameHelper( QWidget* parent, const QString & title, 
                         const QString & currentFile, const QString & filter,
                         const QString & defaultSuffix );

MainWindow::MainWindow( )
{
	assert( s_Instance == NULL );
	s_Instance = this;

    setupUi( this );
    m_UndoStack = new QUndoStack();

    setWindowIcon( QIcon( QPixmap( ":images/MultiplexManagerIcon.png" ) ) );

    QAction* undoAction = m_UndoStack->createUndoAction(this);
    QAction* redoAction = m_UndoStack->createRedoAction(this);


    undoAction->setShortcut(  Qt::CTRL + Qt::Key_Z );
    redoAction->setShortcut(  Qt::CTRL + Qt::Key_Y );

    menu_Edit->addAction( undoAction );
    menu_Edit->addAction( redoAction );

    action_Save->setShortcut( Qt::CTRL + Qt::Key_S );
    action_Open->setShortcut( Qt::CTRL + Qt::Key_O );
    action_New->setShortcut( Qt::CTRL + Qt::Key_N );
    action_Exit->setShortcut( Qt::CTRL + Qt::Key_Q );

    menu_Results->setEnabled( false );


    m_MarkerModel = new MarkerModel( m_MarkerData, m_DyeData );
    m_DyeModel = new DyeModel( m_DyeData );


    m_ArtifactModel = NULL; // we create this and delete it as needed when selecting dyes
    MarkerModelStandardProxy* standardFilter = new MarkerModelStandardProxy();
    standardFilter->setSourceModel( m_MarkerModel );
    markerTableView->setModel( standardFilter );
    dyeTableView->setModel( m_DyeModel );

    MarkerModelDyeEditProxy* dyeEditFilter = new MarkerModelDyeEditProxy();
    dyeEditFilter->setSourceModel( m_MarkerModel );

    markerToDyeMapping->setModel( dyeEditFilter );

    


    dyeTableView->setItemDelegate( new DyeModelDelegate( m_UndoStack ) );

    markerTableView->setItemDelegate( new MarkerModelDelegate( m_UndoStack, standardFilter, m_DyeData ) );

    artifactTableView->setItemDelegate( new ArtifactModelDelegate( m_UndoStack ) );
	
    markerToDyeMapping->setItemDelegate( new MarkerModelDelegate( m_UndoStack, dyeEditFilter, m_DyeData ) );
	
    QObject::connect( markerTableView->selectionModel(), SIGNAL( selectionChanged(QItemSelection,QItemSelection) ),
                      this, SLOT( markerTableViewSelectionChanged(QItemSelection, QItemSelection) ) );

    QObject::connect( dyeTableView->selectionModel(), SIGNAL( selectionChanged(QItemSelection,QItemSelection) ),
                      this, SLOT( dyeTableViewSelectionChanged(QItemSelection, QItemSelection) ) );


    QObject::connect( m_MarkerModel, SIGNAL( dataChanged( QModelIndex, QModelIndex) ),
                      markerTableView, SLOT( resizeColumnsToContents() ) );


    QObject::connect( m_MarkerModel, SIGNAL( rowsInserted( const QModelIndex &, int, int ) ),
                      this, SLOT( updateSelectBestSpinner() ) );

    QObject::connect( m_MarkerModel, SIGNAL( rowsRemoved( const QModelIndex &, int, int ) ),
                      this, SLOT( updateSelectBestSpinner() ) );
    

    // this forces a repaint of the marker->dye mapping when deleting or undeleting a dye
    QObject::connect( m_DyeModel, SIGNAL( rowsRemoved( const QModelIndex &, int, int ) ), 
                      m_MarkerModel, SLOT( RefreshData() ));
    QObject::connect( m_DyeModel, SIGNAL( rowsInserted( const QModelIndex &, int, int ) ), 
                      m_MarkerModel, SLOT( RefreshData() ));


    spnSelectBest->setMinimum( 1 );
    ResetData();
  


    m_Solver = NULL;

	// check for new version?

}

void MainWindow::ResetData()
{
    m_MarkerData.clear();
    m_MarkerModel->ResetData();
    
    m_DyeData.clear();
    m_DyeModel->PopulateDefaultDyes();
    m_DyeModel->ResetData();

    btnRemoveMarker->setEnabled( false );
    btnRemoveDye->setEnabled( false );
    
    btnAddArtifact->setEnabled( false );
    btnRemoveArtifact->setEnabled( false );
    
    chkUseAllMarkers->setChecked( true );
    
    new (&m_OptionData) OptionData(); // inplace new the option data
    
    UpdateOptionData();

    updateSelectBestSpinner();

    m_CurrentFileName = "";   

    setWindowTitle( APPLICAITON_NAME );

    mainTabs->setCurrentIndex( 0 );
    mainTabs->setTabEnabled( 2, false );

    markerTableView->resizeColumnsToContents();
    markerToDyeMapping->resizeColumnsToContents();

    m_UndoStack->clear();

	mainTabs->setTabToolTip( 0, "Switch to the Marker Data tab." );
	mainTabs->setTabToolTip( 1, "Switch to the Dyes an Options Tab to configure options and analyse your multiplex.");
	mainTabs->setTabToolTip( 2, "Switch to the Results Tab ( only enabled after you have generated some results )" );

}

MainWindow::~MainWindow()
{
    delete m_MarkerModel;
    delete m_DyeModel;
    delete m_UndoStack;
	s_Instance = NULL;

}


void MainWindow::on_btnAddMarker_clicked()
{
    UndoAddRowCommand<MarkerInfo>* cmd = new UndoAddRowCommand<MarkerInfo>( m_MarkerModel, markerTableView );
    
    m_UndoStack->push( cmd );
    

}

void MainWindow::on_btnRemoveMarker_clicked()
{
    QMessageBox::StandardButtons result =  
        QMessageBox::question( this, 
                               tr("Remove Markers?"), 
                               tr("Are you sure you want to delete the selected markers(s)?"), 
                               QMessageBox::Ok | QMessageBox::Cancel );
    
    if ( result == QMessageBox::Ok )
    {
        m_UndoStack->beginMacro(tr("Remove Marker(s)"));

        int rowCount = m_MarkerModel->rowCount();
        // going backward makes this work right when removing from a vector
        for( int i = rowCount - 1; i >= 0; --i )
        {
            if ( markerTableView->selectionModel()->isRowSelected( i, QModelIndex() ) )
            {

                UndoRemoveRowCommand<MarkerInfo>* cmd = new UndoRemoveRowCommand<MarkerInfo>( m_MarkerModel, markerTableView, i );
                m_UndoStack->push( cmd );

            }	
        }
        m_UndoStack->endMacro();
    }


}

void MainWindow::on_btnAddDye_clicked()
{
    UndoAddRowCommand<DyeInfo>* cmd = new UndoAddRowCommand<DyeInfo>( m_DyeModel, dyeTableView );    
    m_UndoStack->push( cmd );

}

void MainWindow::on_btnRemoveDye_clicked()
{
     QMessageBox::StandardButtons result =  
        QMessageBox::question( this, 
                               tr("Remove Dye?"), 
                               tr("Are you sure you want to delete the selected dye(s)?"), 
                               QMessageBox::Ok | QMessageBox::Cancel );
    
    if ( result == QMessageBox::Ok )
    {
        m_UndoStack->beginMacro(tr("Remove Dye(s)"));

        int rowCount = m_DyeModel->rowCount();
        // going backward makes this work right when removing from a vector
        for( int i = rowCount - 1; i >= 0; --i )
        {
            if ( dyeTableView->selectionModel()->isRowSelected( i, QModelIndex() ) )
            {
                UndoRemoveRowCommand<DyeInfo>* cmd = new UndoRemoveRowCommand<DyeInfo>( m_DyeModel, dyeTableView, i );
                m_UndoStack->push( cmd );

            }
	
        }
        
        m_UndoStack->endMacro();
    }
}

void MainWindow::markerTableViewSelectionChanged ( const QItemSelection & /*selected*/, const QItemSelection & /*deselected*/ )
{

    int rowCount = m_MarkerModel->rowCount();

    bool anyRowSelected = false;
    for( int i = 0; i < rowCount; ++i )
    {
        if ( markerTableView->selectionModel()->isRowSelected( i, QModelIndex() ) )
        {
            anyRowSelected = true;
            break;
        }
	
    }
    btnRemoveMarker->setEnabled( anyRowSelected );
}

void MainWindow::dyeTableViewSelectionChanged ( const QItemSelection & /*selected*/, const QItemSelection & /*deselected*/ )
{

    int rowCount = m_DyeModel->rowCount();

    bool anyRowSelected = false;
    for( int i = 0; i < rowCount; ++i )
    {
        if ( dyeTableView->selectionModel()->isRowSelected( i, QModelIndex() ) )
        {
            anyRowSelected = true;
            break;
        }
	
    }
    btnRemoveDye->setEnabled( anyRowSelected );

    int rowSelected = -1;
    bool moreThanOne = false;

    const QItemSelection & selection = dyeTableView->selectionModel()->selection();
    
    const QModelIndexList & indexList =  selection.indexes();

    QModelIndex idx;
    foreach ( idx, indexList )
    {
        if ( rowSelected == -1 )
        {
            rowSelected = idx.row();
        }
        else if ( rowSelected != idx.row() )
        {
            moreThanOne = true;
            break;
        }
    }
 
    if ( ( rowSelected == -1 ) || ( moreThanOne ) ) // if we have no row selected, or more than one, hide the artifact view
    {
        if ( m_ArtifactModel != 0 )
        {
            QObject::disconnect( artifactTableView->selectionModel(), SIGNAL( selectionChanged(QItemSelection,QItemSelection) ),
                              this, SLOT( artifactTableViewSelectionChanged(QItemSelection, QItemSelection) ) );

            artifactTableView->setModel( NULL );
            delete m_ArtifactModel;
            m_ArtifactModel = NULL;
            btnAddArtifact->setEnabled( false );
            btnRemoveArtifact->setEnabled( false );
        }
    }
    else
    {
        m_ArtifactModel = new ArtifactModel( m_DyeData[rowSelected].m_Artifacts );
        artifactTableView->setModel( m_ArtifactModel );
        btnAddArtifact->setEnabled( true );
        btnRemoveArtifact->setEnabled( false );
        QObject::connect( artifactTableView->selectionModel(), SIGNAL( selectionChanged(QItemSelection,QItemSelection) ),
                          this, SLOT( artifactTableViewSelectionChanged(QItemSelection, QItemSelection) ) );


    }

    artifactTableView->resizeColumnsToContents();

}



void MainWindow::on_btnAddArtifact_clicked()
{
    assert( m_ArtifactModel );

    UndoAddRowCommand<ArtifactInfo>* cmd = new UndoAddRowCommand<ArtifactInfo>( m_ArtifactModel, artifactTableView );    
    m_UndoStack->push( cmd );

}

void MainWindow::on_btnRemoveArtifact_clicked()
{
    assert( m_ArtifactModel );

    QMessageBox::StandardButtons result =  
        QMessageBox::question( this, 
                               tr("Remove Artifact?"), 
                               tr("Are you sure you want to delete the selected artifacts(s)?"), 
                               QMessageBox::Ok | QMessageBox::Cancel );
    
    if ( result == QMessageBox::Ok )
    {
        m_UndoStack->beginMacro(tr("Remove Artifact(s)"));

        int rowCount = m_ArtifactModel->rowCount();
        // going backward makes this work right when removing from a vector
        for( int i = rowCount - 1; i >= 0; --i )
        {
            if ( artifactTableView->selectionModel()->isRowSelected( i, QModelIndex() ) )
            {
                UndoRemoveRowCommand<ArtifactInfo>* cmd = new UndoRemoveRowCommand<ArtifactInfo>( m_ArtifactModel, artifactTableView, i );
                m_UndoStack->push( cmd );

            }
	
        }

        m_UndoStack->endMacro();
    }
}

void MainWindow::artifactTableViewSelectionChanged ( const QItemSelection & /*selected*/, const QItemSelection & /*deselected*/ )
{

    int rowCount = m_ArtifactModel->rowCount();

    bool anyRowSelected = false;
    for( int i = 0; i < rowCount; ++i )
    {
        if ( artifactTableView->selectionModel()->isRowSelected( i, QModelIndex() ) )
        {
            anyRowSelected = true;
            break;
        }
	
    }
    btnRemoveArtifact->setEnabled( anyRowSelected );    
}

void MainWindow::on_action_Save_triggered()
{
 
    if ( m_CurrentFileName.isEmpty() )
    {
        on_action_Save_As_triggered();
    }
    else
    {
        Save( m_CurrentFileName );
    }

      
}

QString MainWindow::GetSaveFileName()
{
    QString fileName = GetSaveFileNameHelper(this, tr("Save File"),
                                       m_CurrentFileName,
                                       tr("XML files (*.xml)"), "xml" );
    return fileName;

}
void MainWindow::on_action_Save_As_triggered()
{
    QString fileName = GetSaveFileName();

    if ( fileName.isEmpty() )
    {
        return;
    }


    Save( fileName );
}

void MainWindow::Save( const QString & fileName )
{
    QDomDocument doc( "voldemort-version-1" );
      
    QDomElement data = doc.createElement( "data" );
    doc.appendChild( data );
    QDomElement markers = doc.createElement( "markers" );
    data.appendChild( markers );

    MarkerInfo m;
    foreach( m, m_MarkerData )
    {
        m.Save( &doc, &markers );
    }

    QDomElement dyes = doc.createElement( "dyes" );
    data.appendChild( dyes );

    DyeInfo d;
    foreach( d, m_DyeData )
    {
        d.Save( &doc, &dyes );
    }

    m_OptionData.Save( &doc, &data );

    
    QDomElement results = doc.createElement( "results" );
    
    for ( qint32 i = 0; i < m_CurrentResults.size(); ++i )
    {
        m_CurrentResults[i].Save( &doc, &results );
    }

    
    QDomElement log = doc.createElement( "log" );

    log.appendChild( doc.createCDATASection ( m_ActualLog ) );

    results.appendChild( log );

    data.appendChild( results );


    QFile file( fileName );
    file.open( QIODevice::WriteOnly | QIODevice::Text );
    file.write( doc.toString().toLatin1() );
    file.close();

    m_CurrentFileName = fileName;

    setWindowTitle( APPLICAITON_NAME + " - " + m_CurrentFileName );

    m_UndoStack->clear();
    
}

void MainWindow::OpenFile( const QString & fileName )
{
    if ( fileName.isEmpty() )
    {
        return;
    }

    QFile file( fileName );
    if ( !file.exists() )
    {
        return;
    }
    
    m_MarkerData.clear();
    m_DyeData.clear();
	m_CurrentResults.clear();

    menu_Results->setEnabled( false );
    mainTabs->setTabEnabled( 2, false );

    for ( qint32 i = 0; i < m_TabWidgets.size(); ++i )
    {
        delete m_TabWidgets[i];
    }
    m_TabWidgets.clear();
    

    QDomDocument doc( "voldemort-version-1" );


    bool opened = file.open( QIODevice::ReadOnly | QIODevice::Text );
    assert( opened );

    doc.setContent(&file);
    file.close();

            
    QDomElement docElem = doc.documentElement(); // data tag


    const QDomElement & markers = docElem.firstChildElement( "markers" );
    if ( !markers.isNull() )
    {
        QDomNode markerNode = markers.firstChild();
        while ( !markerNode.isNull() )
        {
            MarkerInfo m;
            m.Load( &markerNode );
            m_MarkerData << m; // insert into list
            markerNode = markerNode.nextSibling();
            
        }

    }

    const QDomElement & dyes = docElem.firstChildElement( "dyes" );
    if ( !dyes.isNull() )
    {
        QDomNode dyeNode = dyes.firstChild();
        while ( !dyeNode.isNull() )
        {
            DyeInfo m;
            m.Load( &dyeNode );
            m_DyeData << m; // insert into list
            dyeNode = dyeNode.nextSibling();
            
        }

    }

    updateSelectBestSpinner();

    m_OptionData.Load( &docElem );

    UpdateOptionData();

    QList<DyeInfo*> dyePointers;
    QList<MarkerInfo*> markerPointers;

    for ( qint32 i = 0; i < m_MarkerData.size(); ++i )
    {
        markerPointers << &m_MarkerData[i];
    }


    for ( qint32 i = 0; i < m_DyeData.size(); ++i )
    {
        dyePointers << &m_DyeData[i];
    }

    const QDomElement & results = docElem.firstChildElement( "results" );
    if ( !results.isNull() )
    {
        QDomNode resultNode = results.firstChildElement("result");
        while ( !resultNode.isNull() )
        {
            MultiplexResults r;
            r.Load( &resultNode, dyePointers, markerPointers );
            m_CurrentResults << r; // insert into list
            resultNode = resultNode.nextSiblingElement("result");
            
        }

        QDomNode log = results.firstChildElement("log");
        if ( !log.isNull() )
        {
            m_ActualLog = log.firstChild().toCDATASection().data();
            txtLog->setHtml( m_ActualLog ); 
        }
        
        menu_Results->setEnabled( true );
    }

    

    if ( !m_CurrentResults.empty() )
    {
        CreateResultsDisplay();
        mainTabs->setTabEnabled( 2, true );
    }





    m_MarkerModel->ResetData();
    m_DyeModel->ResetData();

    m_CurrentFileName = fileName;

    setWindowTitle( APPLICAITON_NAME + " - " + m_CurrentFileName );
 

    mainTabs->setCurrentIndex( 0 );

    markerTableView->resizeColumnsToContents();
    markerToDyeMapping->resizeColumnsToContents();

    m_UndoStack->clear();


}

void MainWindow::UpdateOptionData()
{
    chkUseAllMarkers->setChecked( m_OptionData.m_UseAllMarkers );
    spnNumLoci->setValue( m_OptionData.m_MaxLociPerReaction );
    spnSelectBest->setValue( m_OptionData.m_SelectBest  );
    spnMinDistance->setValue( m_OptionData.m_MinDistanceBetweenDyesOfSameColor );
    spnCompThreshold->setValue( m_OptionData.m_ComplementarityThreshold );
    spnIterationsToRun->setValue( m_OptionData.m_IterationsToRun );

}

void MainWindow::on_action_Open_triggered()
{
    if ( MaybeSave( tr( "open a new file" ) ) )
    {

		QString dirToOpen = ".";
		if ( !m_CurrentFileName.isEmpty( ) )
		{
			QDir dir( m_CurrentFileName );
			dirToOpen = dir.absolutePath();
		}

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                        dirToOpen,
                                                        tr("XML files (*.xml)"));

        
        OpenFile( fileName );
    }


}


void MainWindow::on_action_New_triggered()
{

    if ( MaybeSave( tr( "start a new Multiplex Plan" ) ) )
    {
        ResetData();        
    }
}

void MainWindow::on_action_Online_Help_triggered()
{
	// fire off the browser to point to our URL.
	// this makes it easy to keep documention up to date etc
	QDesktopServices::openUrl( QUrl( "http://multiplexmanager.com/help.php" ) );
}

void MainWindow::on_action_About_triggered()
{
    QDialog about( this );
    Ui::AboutDialog ui;
    ui.setupUi( &about );
    about.exec();

    
}

void MainWindow::on_action_Export_Results_triggered()
{
    
    QWidget* currentTab = resultsTab->currentWidget();
    if ( !currentTab )
    {
        // no tab, something is very odd
        return;
    }

    
    QGraphicsView* view =  currentTab->findChild<QGraphicsView*>();
    if ( !view )
    {
        QMessageBox::information( resultsTab, tr("Seelct Tab"), tr("Please Select a results tab to export.") ); 
        return;
    }
    
    QGraphicsScene* display = qobject_cast<QGraphicsScene*>( view->scene() );
    if ( !display )
    {
        // can't cast it? also very odd
        return;
    }

    QString fileName = GetSaveFileNameHelper(this, tr("Export Results"),
                                             m_CurrentFileName.replace(".xml", ".pdf" ),
                                             tr("PDF files (*.pdf)"), 
                                             "pdf");


    if ( !fileName.isEmpty() )
    {        
   
        QPrinter printer( QPrinter::HighResolution ) ;
        printer.setOutputFormat( QPrinter::PdfFormat );
        printer.setOutputFileName( fileName );
        printer.setOrientation( QPrinter::Landscape );

        //printer.setPageSize( QPrinter::Custom );
        
        QPainter painter(&printer);
        painter.setRenderHint( QPainter::Antialiasing);
        painter.setRenderHint( QPainter::TextAntialiasing );
        painter.setRenderHint( QPainter::SmoothPixmapTransform );
        
        display->render(&painter);
    }
    

    
    
}

void MainWindow::on_action_Export_Results_as_text_triggered()
{
	
    QWidget* currentTab = resultsTab->currentWidget();
    if ( !currentTab )
    {
        // no tab, something is very odd
        return;
    }

    QGraphicsView* view =  currentTab->findChild<QGraphicsView*>();
    if ( !view )
    {
        QMessageBox::information( resultsTab, tr("Seelct Tab"), tr("Please Select a results tab to export.") ); 
        return;
    }
    
    ResultsDisplay* display = qobject_cast<ResultsDisplay*>( view->scene() );
    if ( !display )
    {
        // can't cast it? also very odd
        return;
    }

    QString fileName = GetSaveFileNameHelper(this, tr("Export Results"),
                                             m_CurrentFileName.replace(".xml", ".txt" ),
                                             tr("Text files (*.txt)"), 
                                             "txt");


    if ( !fileName.isEmpty() )
    {        
		
		display->GetResults().SaveResultsAsText( fileName );
    }
    

    
    
}

void MainWindow::on_action_Export_All_Results_as_PDF_triggered()
{

    if ( m_TabWidgets.size() == 0 )
    {
        return;
    }
    
    QString fileName = GetSaveFileNameHelper(this, tr("Export Results"),
                                             m_CurrentFileName.replace(".xml", ".pdf" ),
                                             tr("PDF files (*.pdf)"), 
                                             "pdf");
                                                 
    if ( fileName.isEmpty() )
    { 
        return;
    }
    
    QPrinter printer( QPrinter::HighResolution ) ;
    printer.setOutputFormat( QPrinter::PdfFormat );
    printer.setOutputFileName( fileName );
    printer.setOrientation( QPrinter::Landscape );
    //printer.setPageSize( QPrinter::Custom );
    
    QPainter painter(&printer);
    painter.setRenderHint( QPainter::Antialiasing);
    painter.setRenderHint( QPainter::TextAntialiasing );
    painter.setRenderHint( QPainter::SmoothPixmapTransform );
                
    for ( qint32 i = 0; i < m_TabWidgets.size(); ++i )
    {
        QGraphicsView* view =  m_TabWidgets[i]->findChild<QGraphicsView*>();

        if ( !view )
        {
            QMessageBox::information( resultsTab, tr("Seelct Tab"), tr("Please Select a results tab to export.") ); 
            return;
        }

        QGraphicsScene* display = qobject_cast<QGraphicsScene*>( view->scene() );
        if ( !display )
        {
            // can't cast it? also very odd
            return;
        }
        
        display->render(&painter);
        if ( i != m_TabWidgets.size() - 1 )
        {
            printer.newPage();
        }
    }
        
}
void MainWindow::on_action_Export_All_Results_as_Text_triggered()
{

    if ( m_TabWidgets.size() == 0 )
    {
        return;
    }
    
        
    QString fileName = GetSaveFileNameHelper(this, tr("Export Results"),
                                             m_CurrentFileName.replace(".xml", ".txt" ),
                                             tr("Text files (*.txt)"), 
                                             "txt");    

    if ( fileName.isEmpty() )
    { 
        return;
    }

    QFile file( fileName );
    file.open( QIODevice::WriteOnly | QIODevice::Text );
    QTextStream out( &file );


    for ( qint32 i = 0; i < m_TabWidgets.size(); ++i )
    {
        QGraphicsView* view =  m_TabWidgets[i]->findChild<QGraphicsView*>();
        if ( !view )
        {
            QMessageBox::information( resultsTab, tr("Seelct Tab"), tr("Please Select a results tab to export.") ); 
            return;
        }

        ResultsDisplay* display = qobject_cast<ResultsDisplay*>( view->scene() );
        if ( !display )
        {
            // can't cast it? also very odd
            return;
        }
    
        display->GetResults().SaveResultsAsText( out, i + 1 );

    }

    
}


void MainWindow::on_action_Export_Results_Log_as_Text_triggered()
{
	        
    QString fileName = GetSaveFileNameHelper(this, tr("Export Results Log"),
                                             m_CurrentFileName.replace(".xml", "_log.txt" ),
                                             tr("Text files (*.txt)"), 
                                             "txt");  

	if ( fileName.isEmpty() ) return;

	QFile file( fileName );
    file.open( QIODevice::WriteOnly | QIODevice::Text );
    QTextStream out( &file );

	out << txtLog->toPlainText();
}

void MainWindow::on_action_Export_CSV_Template_triggered()
{

    QString fileName = GetSaveFileNameHelper( this, tr("Select CSV File to Export"),
                                              m_CurrentFileName,
                                              tr("CSV files (*.csv)"), "csv" );

    if ( fileName.isEmpty()  )
    {
        return;
    }

    
    // create a CSV file using the column names from the MarkerModel.
    // the nice thing about this is that if we do translations, the
    // column headings will match the language in question

    QFile file( fileName );
    file.open( QIODevice::WriteOnly | QIODevice::Text );
    QTextStream out( &file );
	// don't include the Enforce Marker-Dye column
    for ( qint32 i = 0; i < MarkerInfo::FieldCount() - 1; ++i )
    {
     
        out <<  MarkerInfo::HeaderData( i, Qt::Horizontal, Qt::DisplayRole ).toString();

        if ( i < MarkerInfo::FieldCount() - 2 )
        {
			out  <<  QString(", ");
        }
    }
    file.close();
    
}

void MainWindow::on_action_Import_CSV_Data_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import CSV File"),
                                                    ".",
                                                    tr("CSV files (*.csv)"));

    if ( fileName.isEmpty() )
    {
        return;
    }

	QFile file( fileName );
	
	file.open( QIODevice::ReadOnly);

	QTextStream in( &file );


	bool first = true;
	while ( !in.atEnd() )
	{
		
		QString chunk = in.readLine();
		QStringList lines = chunk.split("\r");
		for ( qint32 i =  0; i < lines.size(); ++i )
		{

			if ( first )
			{
				QStringList header = lines[i].trimmed().split( ",", QString::SkipEmptyParts );
				if ( header.size() != MarkerInfo::FieldCount() - 1 )
				{
					QMessageBox::warning( this, tr("Wrong Number of Columns"), 
										  tr("Unexpected number of columns %1. Expected %2.\nPlease Check your input file.").
										  arg( header.size() ).arg( MarkerInfo::FieldCount() -1 ) ); 
					return;
				}
				
				first = false;
				continue;
			}

			QStringList data = lines[i].trimmed().split(",");			
			qint32 newRow = m_MarkerModel->AddRow();
			for ( qint32 i = 0; i < data.size(); ++i )
			{
				QString element = data[i];
				element = element.trimmed();
				// remove leading and trailing quotes if the exist
				element = element.remove( QRegExp( "(^\"|\"$)" ) );
				if ( element.isEmpty() ) continue;

				m_MarkerModel->setData( m_MarkerModel->index( newRow, i ), element, Qt::EditRole );
			}
		}
			
	}
	file.close();

}


void MainWindow::on_actionCalculate_All_Annealing_Temperatures_triggered()
{

	QMessageBox::StandardButtons result =  
		QMessageBox::question( this, 
							   tr("Calculate all Annealing Temperatures?"), 
							   tr("Are you sure you want to recalculate all annealing temperatures?"),
							   QMessageBox::Ok | QMessageBox::Cancel );        
	
	if ( result == QMessageBox::Cancel )
	{
		return;
	}

	
	QList<float> newValues;
	for ( qint32 i = 0; i < m_MarkerData.size(); ++i )
	{
		if ( m_MarkerData[i].m_Forward.isEmpty() || 
			 m_MarkerData[i].m_Reverse.isEmpty() )
		{
			continue;
		}
		float temp = CalculateAnnealingTemp( m_MarkerData[i].m_Forward,  
											 m_MarkerData[i].m_Reverse, 
											 m_OptionData.m_PrimerConcentration, 
											 m_OptionData.m_DegreesLowerForAnnealing );
		newValues << temp;
		

	}

	
	m_UndoStack->push( new RecalculateAnnealTempCommand( *m_MarkerModel, newValues ) );



}

void MainWindow::on_actionSet_Parameters_for_Annealing_Temp_Calculation_triggered()
{
	AnnealTempParams paramBox;
	
	paramBox.setParams( m_OptionData.m_PrimerConcentration, 
						m_OptionData.m_DegreesLowerForAnnealing );

	int res = paramBox.exec();
	if ( res == QDialog::Accepted )
	{
		paramBox.getParams( m_OptionData.m_PrimerConcentration, 
							m_OptionData.m_DegreesLowerForAnnealing );
		
	}
	setFocus();
	//delete paramBox;
}

void MainWindow::on_chkUseAllMarkers_toggled()
{
    if ( !CheckUndoCommand::InUndoRedo() )
    {
        m_UndoStack->push( new CheckUndoCommand( chkUseAllMarkers, chkUseAllMarkers->isChecked(), 
                                                 tr("Use All Markers" ) ) );
    }
    m_OptionData.m_UseAllMarkers = chkUseAllMarkers->isChecked();

    spnSelectBest->setEnabled( !chkUseAllMarkers->isChecked() );
    updateSelectBestSpinner();
    spnSelectBest->setValue( m_OptionData.m_SelectBest );
}


void MainWindow::on_spnNumLoci_valueChanged( int i )
{
    if ( !SpinnerUndoCommand::InUndoRedo() )
    {
        m_UndoStack->push( new SpinnerUndoCommand( spnNumLoci, m_OptionData.m_MaxLociPerReaction, i, tr("Num Loci Per Reaction" ) ) );
    }
    m_OptionData.m_MaxLociPerReaction = i;

}

void MainWindow::on_spnSelectBest_valueChanged( int i )
{
    if ( !SpinnerUndoCommand::InUndoRedo() )
    {
        m_UndoStack->push( new SpinnerUndoCommand( spnSelectBest, m_OptionData.m_SelectBest, i, tr("Select Best" ) ) );
    }

    m_OptionData.m_SelectBest = i;
}


void MainWindow::updateSelectBestSpinner()
{   
    spnSelectBest->setMaximum( max( 1, m_MarkerData.size() ) );
}

void MainWindow::on_spnMinDistance_valueChanged( int i )
{
    if ( !SpinnerUndoCommand::InUndoRedo() )
    {
        m_UndoStack->push( new SpinnerUndoCommand( spnMinDistance, m_OptionData.m_MinDistanceBetweenDyesOfSameColor, i, tr("Min Distance" ) ) );
    }

    m_OptionData.m_MinDistanceBetweenDyesOfSameColor = i;
}

void MainWindow::on_spnCompThreshold_valueChanged( int i )
{
    if ( !SpinnerUndoCommand::InUndoRedo() )
    {
        m_UndoStack->push( new SpinnerUndoCommand( spnCompThreshold, m_OptionData.m_ComplementarityThreshold, i, tr("Complementarity Threshold" ) ) );
    }
    
    m_OptionData.m_ComplementarityThreshold = i;
}


void MainWindow::on_spnIterationsToRun_valueChanged( int i )
{
    if ( !SpinnerUndoCommand::InUndoRedo() )
    {
        m_UndoStack->push( new SpinnerUndoCommand( spnIterationsToRun, m_OptionData.m_IterationsToRun, i, tr("Iterations To Run" ) ) );
    }
    
    m_OptionData.m_IterationsToRun = i;
}

void MainWindow::on_btnEditCriteria_clicked()
{
    
    CriteriaDialog criteria( m_OptionData.m_Criteria, !m_OptionData.m_UseAllMarkers, this );
    
    criteria.exec();

	int result = criteria.result();
	if ( result == QDialog::Accepted )
	{
		QList<qint32> newItems;
		criteria.GetCriteria( newItems );
		if ( newItems != m_OptionData.m_Criteria )
		{
			m_UndoStack->push( new CriteriaUndoCommand( &m_OptionData, m_OptionData.m_Criteria, newItems ) );
		}
		
	}
}

void MainWindow::on_btnSolveMultiplex_clicked()
{
    if ( m_MarkerData.size() == 0 )
    {
		QMessageBox::warning( this, tr("Enter Marker Data"), tr("Please enter some information before trying to analyse.") ); 
	    return;
    }
	
	if ( m_DyeData.size() == 0 )
	{
		QMessageBox::warning( this, tr("Enter Dye Data"), tr("Please enter some dye information before trying to analyse.") ); 
	    return;
    }
	
	QStringList validationErrors;

	// validate input
	for ( qint32 i = 0; i < m_MarkerData.size(); ++ i )
	{
		MarkerInfo & info = m_MarkerData[i];
		if ( info.m_MinimumSize >= info.m_MaximumSize )
		{
			validationErrors << QString( tr( "Marker %1 : Minimum Allele size is greater than Maximum Allele Size" ) ).arg( info.m_Name );
		}
		
	}


	for ( qint32 i = 0; i < m_TabWidgets.size(); ++i )
    {
        delete m_TabWidgets[i];
    }
    m_TabWidgets.clear();
    
	if ( validationErrors.size() > 0)
	{
		mainTabs->setTabEnabled( 2, true );
		mainTabs->setCurrentIndex( 2 );
		resultsTab->setCurrentIndex( 0 );
		txtLog->clear();
		QString errorString = tr( "<h2>Data Validation Errors</h2><ul>");
		foreach ( QString line, validationErrors )
		{
			errorString += "<li>" + line + "</li>";
		}
		errorString += tr( "</ul><br/>Please rectify these errors before continuing." );
		txtLog->setHtml( errorString );
		return;
	}
	   

    // the multiplex solver inherits from QThread, so
    // we can do heavy work in there without blocking
    // the main application thread
    m_Solver = new MultiplexSolver( m_MarkerData, m_DyeData, &m_OptionData );

    mainTabs->setTabEnabled( 2, true );
    mainTabs->setCurrentIndex( 2 );

    mainTabs->setTabEnabled( 0, false );
    mainTabs->setTabEnabled( 1, false );

    btnCancelSolver->setEnabled( true );

    QObject::connect( m_Solver, SIGNAL( progress( QString, float ) ),
                      this, SLOT( solverProgress( QString, float ) ) );

    QObject::connect( m_Solver, SIGNAL( complete( bool ) ),
                      this, SLOT( solverComplete( bool ) ) );


    QObject::connect( m_Solver, SIGNAL( cancelled() ),
                      this, SLOT( solverCancelled( ) ) );
  

    QObject::connect( m_Solver, SIGNAL( log( QString ) ),
                      this, SLOT( solverLog( QString ) ) );
  
    QObject::connect( &m_Timer, SIGNAL( timeout() ),
                      this, SLOT( flushSolverLog() ) );

	m_Timer.start( 1000 );

    txtLog->clear();
    txtLog->setHtml("<br/>");

    m_ActualLog = "";

    m_Solver->start();

    
}


void MainWindow::solverProgress( QString  text, float value )
{
    
    txtProgress->setText( text );
    solveProgressBar->setValue( (int) ( value * 100.0f ) );


}

void MainWindow::solverComplete( bool result )
{
    assert( m_Solver );
    m_Solver->wait();
    assert( m_Solver->isFinished() );

    if ( result )
    {

        // take a copy of the results so we can delete
        // the solver        
        m_CurrentResults = m_Solver->GetResults();

        CreateResultsDisplay();

       
        menu_Results->setEnabled( true );

	


    }
    else
    {
        QMessageBox::warning( this, tr("Unable to find a solution"), 
                              tr("Unable to find a solution for your criteria. Please check your input and make sure "
                                 "that your size ranges and dye artifacts are correct." ) );

    }
    
    btnCancelSolver->setEnabled( false );

    CleanUpSolver();
    
      
}

void MainWindow::CreateResultsDisplay()
{
	for ( qint32 i = 0; i < m_TabWidgets.size(); ++i )
    {
        delete m_TabWidgets[i];
    }
    m_TabWidgets.clear();
        
    for ( qint32 r = 0; r < m_CurrentResults.size(); ++r )
    {

        QGraphicsView* gview = new MyGraphicsView( );
		
		int rowCount = m_MarkerModel->rowCount();
		qint32 maxSize = 0;
		for( int i = 0; i < rowCount; ++i )
		{
			qint32 size = m_MarkerData[ i ].m_MaximumSize - m_MarkerData[ i ].m_MinimumSize;
			maxSize = max( size, maxSize );

		}

		// this may seem a bit odd but its based on real world examples
		// multiplex data size ranges.
		float horizontalScale = 3.0f +  ( 1.0f - ( ( (float) maxSize - 40.f ) / 180.0f ) ) * 7.0f;

		// clamp between 3 and 10 as scales outside
		// this range look a bit odd
		horizontalScale = min( 10.0f, max( horizontalScale, 3.0f ) );
		// whole numbers look best
		horizontalScale = (float) ((int) horizontalScale );
		
        ResultsDisplay* display = new ResultsDisplay( &m_CurrentResults[r], horizontalScale, m_UndoStack );
     
        gview->setScene( display );
		QHBoxLayout* layout = new QHBoxLayout();
        
        layout->setContentsMargins( 0,0,0,0 );
        layout->addWidget( gview );
        QSlider* slider = new QSlider( Qt::Vertical );
		slider->setMinimum( 10 );
		slider->setMaximum( 100 );
		slider->setValue( 10 );
        layout->addWidget( slider );
        QWidget* frame = new QWidget();
		frame->setLayout( layout );
        resultsTab->addTab( frame, tr("Result %1").arg( r + 1 ) );        
        
        gview->centerOn( 0.0f, 0.0f );
		connect( slider, SIGNAL( valueChanged( int ) ), this, SLOT( scale_slider_changed( int ) ) );

		m_TabWidgets << frame;
        
    }

    if ( m_CurrentResults.size() )
    {
	    resultsTab->setCurrentIndex( 1 );
    }
}

void MainWindow::scale_slider_changed( int value )
{
	float scale = 10.0f  / (float) value;
	QTransform t;
	t.scale( scale, scale );

    QGraphicsView* view =  sender()->parent()->findChild<QGraphicsView*>();
	if( view )
	{
		view->setTransform( t );
	}
	
	
}

void MainWindow::solverCancelled()
{
    mainTabs->setCurrentIndex( 1 );
    mainTabs->setTabEnabled( 2, false );

    CleanUpSolver();

}
void MainWindow::CleanUpSolver()
{
    QObject::disconnect( m_Solver, SIGNAL( progress( QString, float ) ),
                         this, SLOT( solverProgress( QString, float ) ) );
    QObject::disconnect( m_Solver, SIGNAL( complete( bool ) ),
                         this, SLOT( solverComplete( bool ) ) );

    QObject::disconnect( m_Solver, SIGNAL( cancelled() ),
                         this, SLOT( solverCancelled( ) ) );
  
    QObject::disconnect( m_Solver, SIGNAL( log( QString ) ),
                         this, SLOT( solverLog( QString ) ) );

	QObject::disconnect( &m_Timer, SIGNAL( timeout() ),
						 this, SLOT( flushSolverLog() ) );

	m_Timer.stop();
    
    // make sure the thread is infact deceased
    // we've already emited the signal that said we are done
    // but it's possible the thread is still cleaning itself up
    m_Solver->wait();

    delete m_Solver;
    m_Solver = NULL;

    mainTabs->setTabEnabled( 0, true );
    mainTabs->setTabEnabled( 1, true );

	flushSolverLog();

}

void MainWindow::flushSolverLog()
{
    m_ActualLog += m_TempLog;
    QTextCursor cursor = txtLog->textCursor();
    cursor.movePosition( QTextCursor::End );
    txtLog->setTextCursor( cursor );

	txtLog->insertHtml( m_TempLog  );
    
	m_TempLog = "";
}

void MainWindow::solverLog( const QString & html )
{
	// it seems that repeatedly inserting many small items
	// into the text area causes Qt to really chug.
	// so we store the html, and have a timer connected
	// to the flushSolverLog() slot
	// we reset the timer in here, so it only adds the values
	// after a second has elapsed of no log messages
	static const QString pSytleStart = "<p style=\"font-size:12pt\">";
    static const QString pSytleEnd = "</p>";

	m_TempLog +=  pSytleStart + html + pSytleEnd;
	// reset the timer
	m_Timer.start( 1000 );
    
}

void MainWindow::on_btnCancelSolver_clicked()
{
    if ( m_Solver )
    {
        m_Solver->Cancel();
        btnCancelSolver->setEnabled( false );
    }
}

void MainWindow::closeEvent( QCloseEvent* event )
{
    if ( MaybeSave( tr( "quit" ) ) )
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

bool MainWindow::MaybeSave( const QString & message ) 
{
    if ( m_UndoStack->canUndo() ) // we have some changes since we saved
    {
        QMessageBox::StandardButtons result =  
            QMessageBox::question( this, 
                                   tr("Unsaved changes!"), 
                                   tr("You have unsaved changes. Are you sure you want to %1?").arg( message ), 
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );        

        if ( result == QMessageBox::Cancel )
        {
            return false;
        }

        if ( result == QMessageBox::Save )
        {
            QString fileName = GetSaveFileName(); // prompt the user, make sure the have a chance 
            if ( fileName.isEmpty() )             // to pick a new filename if they want
            {
                return false; // in this case, they hit canel in the save as box, so don't do anything
            }
            Save( fileName );
            return true;
        }

        return true;
    }
    else
    {        
        return true;
    }
} 


QString GetSaveFileNameHelper( QWidget* parent, const QString & title, 
                         const QString & currentFile, const QString & filter,
                         const QString & defaultSuffix )
{
	
	QString dir = ".";
	if ( !currentFile.isEmpty() )
	{
		dir = QDir( currentFile ).absolutePath();
	}
	
    QFileDialog dialog( parent, title, dir, filter );
    dialog.setDefaultSuffix( defaultSuffix );

    dialog.setFileMode( QFileDialog::AnyFile );
    dialog.setConfirmOverwrite( true );
    dialog.setAcceptMode( QFileDialog::AcceptSave );

    if ( dialog.exec() )
    {
        QStringList fileNames;
        fileNames = dialog.selectedFiles();
        return fileNames[0];
    }
    return QString();

}
