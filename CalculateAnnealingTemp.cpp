#include "CalculateAnnealingTemp.h"

#include "utils.h"
#include <math.h>
#include <cassert>

#include "ComplementarityCheck.h"
#include <QRegExp>

/*

SantaLucia Jr., J. 1998. A unified view of polymer, dumbbell, and oligonucleotide 
DNA nearest-neighbor thermodynamics. Proc. Nat. Acad. Sci. USA 95:1460-1465. 14 

http://ws1.izbi.uni-leipzig.de/pdf/sant-96-35-3555-oligo.pdf
and http://www.dna.caltech.edu/courses/cs191/paperscs191/santalucia1998a.pdf

They propose the equation:

TM = Delta H°/(Delta S°+ R ln CT)

which results in a temperature in Kelvin.

DeltaH and DeltaS are calulated as the sum of the 
values for each nearest neighbour pair, R is the gas constant
( 1.987 cal/(K mol) ) and CT is the concentration of the oglio.
 

https://www.operon-biotech.com/oligos/custom_oligos_calculations.php?

How can I calculate the annealing Temperature for PCR?
Calculate the melting temperature as above and subtract 5 to 10 °C 
If the two Oligos have different melting temperatures, do NOT average the numbers. 
Use the lower number so that both of the Oligos can anneal.


*/

enum Pair
{
	AA_TT,
	AT_TA,
	TA_AT,
	CA_GT,
	GT_CA,
	CT_GA,
	GA_CT,
	CG_GC,
	GC_CG,
	GG_CC
};

float DeltaH[] = { -7.9, -7.2, -7.2, -8.5, -8.4, -7.8, -8.2, -10.6, -9.8, -8.0 };

float DeltaS[] = { -22.2, -20.4, -21.3, -22.7, -22.4, -21.0, -22.2, -27.2, -24.4, -19.9 };


static const float RGasConstant = 1.987;

Pair GetPair( char current, char next )
{

	switch ( current )
	{
		case 'a':
			switch ( next )
			{
				case 'a': return AA_TT;
				case 't': return AT_TA;
				case 'g': return CT_GA;
				case 'c': return GT_CA
;
			}
		case 't':
			switch ( next )
			{
				case 'a': return TA_AT;
				case 't': return AA_TT;
				case 'g': return CA_GT;
				case 'c': return GA_CT;
			}
		case 'g':
			switch ( next )
			{
				case 'a': return GA_CT;
				case 't': return GT_CA;
				case 'g': return GG_CC;
				case 'c': return GC_CG;
			}
		case 'c':
			switch ( next )
			{
				case 'a': return CA_GT;
				case 't': return CT_GA;
				case 'g': return CG_GC;
				case 'c': return GG_CC;
			}
	};


	assert( 0 && "doesn't match!");

	return AA_TT;

}
float CalculateMeltingTemp( const QString & sequence, float concentration )
{

	QString strand = sequence.trimmed().toLower();
	strand.remove( QRegExp("[^atgc]") );

    const QByteArray & letterArray = strand.toAscii();

    const char* letters = letterArray.data();
	
	qint32 numLetters = letterArray.size();


	Checker self( "self1", sequence.trimmed() );
	Checker selfAgain( "self2", sequence.trimmed() );

	Checker::CheckerResults results = self.CheckAgainst( selfAgain );

	bool selfComplentary = results.score >= numLetters ;

	float currentDeltaH = 0.0f;
	float currentDeltaS = 0.0f;

	bool containsGC = false;
	
	for ( qint32 i = 0; i < numLetters - 1; ++i )
	{
		char current = letters[i];
		char next = letters[ i + 1 ];
		Pair p = GetPair( current, next );

		containsGC |= ( current == 'c' ) || ( current == 'g' );
	   
		currentDeltaH += DeltaH[ p ];
		currentDeltaS += DeltaS[ p ];

	}

	if ( containsGC )
	{
		currentDeltaH += 0.1f; // initiation when containing GC
		currentDeltaS -= 2.8; 

	}
	else
	{
		currentDeltaH += 2.3f; // initiation when not containing GC
		currentDeltaS += 4.1; 

	}

	if ( selfComplentary )
	{
		currentDeltaS -= 1.4f;
	}

	if ( letters[0] == 't' ) 
	{
		currentDeltaH += 0.4f;

	}

	if ( letters[ numLetters  -1 ] == 'a' ) 
	{
		currentDeltaH += 0.4f;

	}
 
	//printf( "DeltaH %f\n", currentDeltaH );
	//printf( "DeltaS %f\n", currentDeltaS );
	// convert nanomolar to moles
	float moles = concentration * 1e-9;

	float temp = currentDeltaH * 1000 / ( currentDeltaS + RGasConstant * log( moles ) );

	temp -= 273.15f; // convert to celsius
	//printf(" Temp: %f\n", temp );


	return temp;

}

float CalculateAnnealingTemp( const QString & sequenceA,  const QString & sequenceB, 
							  float concentration, float degrees  )
{
	float a = CalculateMeltingTemp( sequenceA, concentration );
	float b = CalculateMeltingTemp( sequenceB, concentration );

	return min ( a, b ) - degrees; 
}
