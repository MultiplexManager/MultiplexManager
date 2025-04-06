#include <QApplication>


#include "MainWindow.h"

int main(int argc, char *argv[]) 
{

    QApplication app(argc, argv);
#if defined(Q_OS_MAC)
	// this makes sure that the stand-alone version doesn't
	// try and load plugins from the default install locaiton
	QDir dir(QApplication::applicationDirPath());
	dir.cdUp();
	dir.cd("Plugins");
	QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif
    MainWindow* window = new MainWindow();
    window->show();
    
    if ( argc == 2 ) // argv[0] is always the program name
    {
        // open file deals gracefully with the filename specified not existing
        window->OpenFile( QString( argv[1] ) );
    }

    return app.exec();

}
