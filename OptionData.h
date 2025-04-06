#ifndef OPTION_DATA_H
#define OPTION_DATA_H

class QDomDocument;
class QDomNode;

#include <QtGui>

struct OptionData
{
    OptionData()
    {
        m_UseAllMarkers = true;
        m_MaxLociPerReaction = 6;
        m_SelectBest = 6;
        m_MinDistanceBetweenDyesOfSameColor = 40;
        m_ComplementarityThreshold = 7;
        m_IterationsToRun = 1;

		m_PrimerConcentration = 200; // nanomolar
		m_DegreesLowerForAnnealing = 10;

		m_Criteria << 0;  // note that 1 is missing, we decided we don't like that criteria
		m_Criteria << 2;
		m_Criteria << 3;
		m_Criteria << 4;
		m_Criteria << 5;
		m_Criteria << 6;
    }

    bool m_UseAllMarkers;
    qint32 m_MaxLociPerReaction;
    qint32 m_SelectBest;
    qint32 m_MinDistanceBetweenDyesOfSameColor;
    qint32 m_ComplementarityThreshold;

    qint32 m_IterationsToRun;

	float m_PrimerConcentration;
	float m_DegreesLowerForAnnealing;
	QList<qint32> m_Criteria;

    void Save( QDomDocument* doc, QDomNode* parent );
    void Load( QDomNode* node );
};

#endif
