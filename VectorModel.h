#ifndef VECTOR_MODEL_H
#define VECTOR_MODEL_H


#include <QAbstractTableModel>
#include <QVector>

#include <cassert>

template <typename T> 
class VectorModel :  public QAbstractTableModel
{

public:
        
    inline VectorModel( QVector<T> & source )
        : m_Vector( source )
    { 
        
    }

    virtual ~VectorModel() {}

    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const
    {
        (void) parent;

        return m_Vector.size();
    }
    
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const
    {
        (void) parent;

        return T::FieldCount();
    }

    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const
    {
        if (!index.isValid())
            return QVariant();

        return m_Vector[ index.row() ].data( index.column(), role );
    }
    
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const
    {
        return T::HeaderData( section, orientation, role );
    }

    virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole )
    {
  
        if (!index.isValid())
            return false;
        else if (role != Qt::EditRole)
            return false;

        m_Vector[ index.row() ].setData( index.column(), value );
   
        emitDataChanged( index, index );
        return true;
    }

    void SetRowContents( int row, const T & data )
    {
        assert( row < m_Vector.size() );
        m_Vector[row] = data;
        QModelIndex startIndex = index( row, 0 );
        QModelIndex endIndex = index( row, T::FieldCount() - 1 );

        emitDataChanged( startIndex, endIndex );
    }


    void emitDataChanged( const QModelIndex & startIndex, const QModelIndex & endIndex )
    {
        emit( dataChanged( startIndex, endIndex ) );
    }

    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const
    {
        return QAbstractTableModel::flags( index ) | Qt::ItemIsEditable;
    }


    virtual bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex())
    {
        (void) index;

        beginInsertRows(QModelIndex(), position, position+rows-1);
        for (int row = 0; row < rows; ++row) 
        {
            m_Vector.insert( position, T() );
        }


        endInsertRows();
        return true;
    }

    virtual bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex())
    {
        (void) index;

        beginRemoveRows(QModelIndex(), position, position+rows-1);

        m_Vector.remove( position, rows );

        endRemoveRows();
        return true;

    }


    int AddRow()
    {
        insertRows( m_Vector.size(), 1 );
        return m_Vector.size() - 1;
    }
    
    void RemoveRow( int index )
    {
        removeRows( index, 1 );
    }
    
    QVector<T> & GetData()
    {
        return m_Vector;
    }

    const QVector<T> & GetData() const
    {
        return m_Vector;
    }

    void ResetData() 
    {
        reset();
    }

    
protected:
    QVector<T> & m_Vector;
};

#endif
