#ifndef CALC_ANNEALING_TEMP
#define CALC_ANNEALING_TEMP

#include <QString>



// takes 2 sequences, nanomolar concentration and number of degress
// to subtract off the melting temperature.
float CalculateAnnealingTemp( const QString & sequenceA,  const QString & sequenceB, 
							  float concentration, float degrees);



#endif
