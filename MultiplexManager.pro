######################################################################
# Automatically generated by qmake (2.01a) Fri Aug 13 21:42:57 2010
######################################################################

QT+=xml
mac { ICON=application.icns }
win32 { RC_FILE=application.rc }
TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += AnnealTempParams.h \
           AnnealTempWidget.h \
           ArtifactModel.h \
           ArtifactModelDelegate.h \
           CalculateAnnealingTemp.h \
           ComplementarityCheck.h \
           ControlUndoCommands.h \
           Criteria.h \
           DomTemplateHelper.h \
           DyeModel.h \
           DyeModelDelegate.h \
           DyeSpikeDisplay.h \
           LocusDisplay.h \
           MainWindow.h \
           MarkerModel.h \
           MarkerModelDelegate.h \
           MarkerModelProxy.h \
           ModelUndoCommand.h \
           MultiplexResults.h \
           MultiplexSolver.h \
           MyGraphicsView.h \
           OptionData.h \
           ResultsDisplay.h \
           utils.h \
           VectorModel.h \
           VectorModelUndoCommands.h
FORMS += About.ui AnnealingTempParams.ui AnnealTempWidget.ui Criteria.ui Main.ui
SOURCES += AnnealTempParams.cpp \
           AnnealTempWidget.cpp \
           ArtifactModel.cpp \
           ArtifactModelDelegate.cpp \
           CalculateAnnealingTemp.cpp \
           ComplementarityCheck.cpp \
           ControlUndoCommands.cpp \
           Criteria.cpp \
           DyeModel.cpp \
           DyeModelDelegate.cpp \
           DyeSpikeDisplay.cpp \
           LocusDisplay.cpp \
           main.cpp \
           MainWindow.cpp \
           MarkerModel.cpp \
           MarkerModelDelegate.cpp \
           MarkerModelProxy.cpp \
           ModelUndoCommand.cpp \
           MultiplexResults.cpp \
           MultiplexSolver.cpp \
           OptionData.cpp \
           ResultsDisplay.cpp \
           utils.cpp
RESOURCES += application.qrc

