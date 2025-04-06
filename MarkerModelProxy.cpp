#include "MarkerModelProxy.h"
#include <MarkerModel.h>


bool MarkerModelStandardProxy::filterAcceptsColumn( 
    int source_column, const QModelIndex & /*source_parent*/ ) const
{
    return source_column != MarkerInfo::FORCE_DYE;
}



Qt::ItemFlags MarkerModelDyeEditProxy::flags ( const QModelIndex & index ) const
{
    // find source index
    QModelIndex source = mapToSource( index );

    // use that to find the flags
    Qt::ItemFlags f = sourceModel()->flags( source  );

    if ( source.column() == MarkerInfo::NAME )
    {
        // you can't edit column 0
        f &= ~Qt::ItemIsEditable;
        
    }

    return f;

}

bool MarkerModelDyeEditProxy::filterAcceptsColumn( 
    int source_column, const QModelIndex & /*source_parent*/ ) const
{
    // only show name and dye
    return source_column == MarkerInfo::NAME || source_column == MarkerInfo::FORCE_DYE;
}


