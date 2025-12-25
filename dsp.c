#include <stdio.h>
#include "inttypes.h"


void filter_lpf(double * raw_data, double * filter_data, uint32_t buff_size)
{
	#define FILTER_ORDER		4
	static const double filter_deneminator[FILTER_ORDER+1] = 
	{
		1,   -1.968427786939,    1.735860709209,  -0.7244708295074,
		0.1203895998962
	};
	static const double filter_numerator[FILTER_ORDER+1] = 
	{
		0.0102094807912,  0.04083792316481,  0.06125688474722,  0.04083792316481,
		0.0102094807912
	};
	static double v[FILTER_ORDER+1];
	
	uint32_t i;
	for(i = 0; i < buff_size; i++)
	{
		
		uint32_t j = 0;
		
		for(j = FILTER_ORDER; j > 0; j--)
		{
			v[j] = v[j-1];
		}
		v[0] = raw_data[i];
		
		for(j = 1; j < (FILTER_ORDER +1); j++)
		{
			v[0] = v[0] - filter_deneminator[j] * v[j];
		}
		double filtered_data = 0;
		for(j = 0; j < (FILTER_ORDER +1); j++)
		{
			filtered_data += filter_numerator[j] * v[j];
		}
		filter_data[i] = filtered_data;	
	}
}
