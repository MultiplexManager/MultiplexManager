#ifndef UTILS_H
#define UTILS_H
#include <limits.h>


template< typename T > T min( const T & a, const T & b )
{
    return ( a < b ? a : b );
}


template< typename T> T max( const T & a, const T & b )
{
    return ( a > b ? a : b );
}

template< class T, class U, class V >
inline T clamp( T const& x, U const& lo, V const& hi )
{
    return min( max( x, static_cast<T>(lo) ), static_cast<T>(hi) );
}


// I implement my own rand function 
// here so I can ensure different platforms perform in exactly the same way,
int my_rand();

void my_srand(quint32 seed );


template <typename T> T* FindDeref( QList<T*> & objects, quint32 id )
{
    for ( qint32 i = 0; i < objects.size(); ++i )
    {
        if ( objects[i]->m_Id == id )
        {
            return objects[i];
        }
    }

    return NULL;
}


#endif
