
#ifndef MARKER_MODEL_DELEGATE_H
#define MARKER_MODEL_DELEGATE_H

#include "DyeModel.h"

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QSpinBox>

class QUndoStack;
class QSortFilterProxyModel;
class QTextEdit;

class MarkerModelDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    MarkerModelDelegate(QUndoStack* undoStack, QSortFilterProxyModel* proxy, 
			const QVector<DyeInfo> & dyeData, 
			QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData( QWidget *editor,
                       const QModelIndex &index) const;

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;


protected:
	virtual bool eventFilter ( QObject * editor, QEvent * event );


private:
    static QString FilterOutNonSequenceCharacters( const QString & string );

    QUndoStack* m_UndoStack;
    QSortFilterProxyModel* m_Proxy;
    const QVector<DyeInfo> & m_DyeData;
	mutable bool m_EditingSequence;

};

#endif
