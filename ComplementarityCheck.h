#ifndef COMPLEMENTARITY_CHECK_H
#define COMPLEMENTARITY_CHECK_H

#include <QVector>
#include <QString>



class Checker
{
public:

    struct CheckerResults
    {
        qint32 score;
        qint32 offset;
        bool forward;
    };


    // this constructor filters out spaces
    // etc
    Checker( const QString & name, const QString & sequence );

    CheckerResults CheckAgainst( const Checker & other ) const ;

    QString Display( const Checker & other, const CheckerResults & results ) const;



private:
    QString m_Name;
    QString m_StrandDisplay;
    QVector<char> m_Strand;
    // we compute the complementary sequence
    // up front to save time later
    QVector<char> m_ComplementaryStrand;

};

#endif
