#ifndef MARKER_MODEL_PROXY
#define MARKER_MODEL_PROXY

#include <QSortFilterProxyModel>

// In theory I could perform a similar operation
// by hiding columns in the view, for the this case
class MarkerModelStandardProxy : public QSortFilterProxyModel
{
    Q_OBJECT;

protected:
    virtual bool filterAcceptsColumn ( int source_column, const QModelIndex & source_parent ) const;

};


// However, in this case, I override the editing flags so that the 
// user isn't allowed to edit the name column, just the dye mapping column
// whish is very handy
class MarkerModelDyeEditProxy : public QSortFilterProxyModel
{
    Q_OBJECT;

protected:
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;

    virtual bool filterAcceptsColumn ( int source_column, const QModelIndex & source_parent ) const;

};


#endif
