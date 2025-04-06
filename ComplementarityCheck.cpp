#include "ComplementarityCheck.h"

#include "utils.h"

#include <QObject>
#include <QRegExp>

#include <cassert>

Checker::Checker( const QString & name, const QString & sequence )
    : m_Name( name )

{
    QString strand = sequence.toUpper();
    // remove anything we don't know what it is
    strand.remove( QRegExp("[^ATGC]") );

    m_StrandDisplay = strand;

    const QByteArray & byteArray = strand.toLatin1(); 
    for ( qint32 i = 0 ; i < byteArray.size(); ++i )
    {
        m_Strand << byteArray[i];
        switch( byteArray[i] )
        {
        case 'C':
            m_ComplementaryStrand << 'G';
            break;
        case 'G':
            m_ComplementaryStrand << 'C';
            break;
        case 'A':
            m_ComplementaryStrand << 'T';
            break;
        case 'T':
            m_ComplementaryStrand << 'A';
            break;
        default:
            break;
        }
    }
}

Checker::CheckerResults Checker::CheckAgainst( const Checker & other ) const
{
    CheckerResults res;
    const QVector<char> & otherStrand = other.m_Strand;
 
    qint32 strandSize = m_Strand.size();
    qint32 otherStrandSize = other.m_Strand.size();

    qint32 maxScore = -INT_MAX;
    qint32 bestOffset = 0;
    bool bestIsForward = false;

    qint32 offset = m_ComplementaryStrand.size() - 1;
    while ( offset >= -strandSize )
    {
        for ( int forward = 1; forward >= 0; --forward )
        {
            qint32 score = 0;
            qint32 otherStrandIndex = 0;
            
            for ( qint32 strandIndex = offset; strandIndex < strandSize; ++strandIndex, ++otherStrandIndex )
            {
                qint32 otherStrandIndex = strandIndex - offset;
                if ( !forward )
                {
                    otherStrandIndex = ( otherStrandSize - 1 ) - otherStrandIndex;
                }
                
                if ( ( strandIndex ) >= 0 && ( strandIndex < strandSize ) 
                     && ( otherStrandIndex  >= 0 ) && ( otherStrandIndex < otherStrandSize ) )
                {
                    
                    if ( m_ComplementaryStrand[strandIndex] == otherStrand[otherStrandIndex] )
                    {
                        ++score;
                    }
                    else
                    {
                        --score;
                    }
                }
            }
            
            if ( score > maxScore )
            {
                bestOffset = offset;
                maxScore = score;
                bestIsForward = forward;
               
            }
          
        }

        --offset;
    }
	
    res.score = maxScore;
    res.offset  = bestOffset;
    res.forward = bestIsForward;
    return res;
}

QString Checker::Display( const Checker & other, const Checker::CheckerResults & results ) const
{

    QString result("\n");
    result.append( QObject::tr( "Checking %1 %2 against %3 %4 \n\n").
                   arg( m_Name, m_StrandDisplay, other.m_Name, other.m_StrandDisplay ) );

    qint32 strandSize = m_Strand.size();
    qint32 otherStrandSize = other.m_Strand.size();

    qint32 offset = results.offset;
    bool forward = results.forward;

    if ( offset < 0 )
    {
        qint32 antiOffset =  -offset;
        for ( qint32 i = 0; i < antiOffset; ++i )
        {
            result.append(" ");
        }
    }

    result.append("5'-");
    for ( qint32 i =0; i < strandSize; ++i )
    {
        result.append( m_Strand[i] );
    }
    result.append("-3'");
    result.append("\n");


    result.append("   ");
    if ( offset > 0 )
    {
        //qint32 antiOffset =  -offset;
        for ( qint32 i = 0; i < offset; ++i )
        {
            result.append(" ");
        }
    }


    qint32 score = 0;

    for ( qint32 strandIndex = offset; strandIndex < strandSize; ++strandIndex )
    {

        qint32 otherStrandIndex = strandIndex - offset;
        if ( !forward )
        {
            otherStrandIndex = ( otherStrandSize  - 1 ) - otherStrandIndex;
        }
        
        if ( ( strandIndex ) >= 0 && ( strandIndex < strandSize ) 
             && ( otherStrandIndex >= 0 ) && ( otherStrandIndex < otherStrandSize ) )
        {
            if ( m_ComplementaryStrand[strandIndex] == other.m_Strand[otherStrandIndex] )
            {
                result.append( "|" );
                ++score;
            }
            else
            {
                result.append( "x" );
                --score;
            }

        }
        else
        {
            result.append( " " );
        }

    }

    result.append("   ");
    result.append("\n");

    if ( offset > 0 )
    {
        for ( qint32 i = 0; i < offset; ++i )
        {
            result.append(" ");
        }
    }


    if ( forward )
    {
        result.append("5'-");
        for ( qint32 i = 0; i < otherStrandSize; ++i )
        {
            result.append( other.m_Strand[i] );
        }
        result.append("-3'");
    }
    else
    {
        result.append("3'-");
        for ( qint32 i = otherStrandSize - 1; i >= 0; --i )
        {
            result.append( other.m_Strand[i] );
        }
        result.append("-5'");
    }
    result.append("\n");
        

    assert( score == results.score );

    result.append( QString("Score: %1\n\n").arg( score ) );
    
    return result;

}
