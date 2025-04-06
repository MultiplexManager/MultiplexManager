#ifndef ARTIFACT_MODEL_DELEGATE_H
#define ARTIFACT_MODEL_DELEGATE_H

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QSpinBox>

class QUndoStack;

class ArtifactModelDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    ArtifactModelDelegate(QUndoStack* undoStack, QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData( QWidget *editor,
                       const QModelIndex &index) const;


    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

private:
    QUndoStack* m_UndoStack;

};

#endif
