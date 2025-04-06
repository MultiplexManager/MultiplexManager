
#ifndef DYE_MODEL_DELEGATE_H
#define DYE_MODEL_DELEGATE_H

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QSpinBox>

class QUndoStack;

class DyeModelDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    DyeModelDelegate(QUndoStack* undoStack, QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;


    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

private:
    QUndoStack* m_UndoStack;

};

#endif
