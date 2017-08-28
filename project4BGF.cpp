/*
*Author:Benjamin Fields
*Class: CS 575
*Date: May 17, 2017
*Description: This program simulates an environment containing
*Deer, grass, rain, tempurature and fires. Each environmental
*variable is controlled by its own private thread, but depends 
*on the state of the environment and the other variables
*/

#include <iostream>
#include <omp.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <cmath> 
#include <fstream>

std::ofstream outFile;

unsigned int seed = 0;  // a thread-private variable
int totalMonth = 1;

int	NowYear;		// 2017 - 2022
int	NowMonth;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// grain height in inches
int	NowNumDeer;		// number of deer in the current population
int NowNumFires;

const float GRAIN_GROWS_PER_MONTH =		8.0;
const float ONE_DEER_EATS_PER_MONTH =		0.5;

const float AVG_PRECIP_PER_MONTH =		6.0;	// average
const float AMP_PRECIP_PER_MONTH =		6.0;	// plus or minus
const float RANDOM_PRECIP =			2.0;	// plus or minus noise

const float AVG_TEMP =				50.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise

const float MIDTEMP =				40.0;
const float MIDPRECIP =				10.0;

const int MAX_FIRES = 				15;
const float INCHES_PER_FIRE =       0.5;



float
SQR( float x )
{
        return x*x;
}

float
Ranf( unsigned int *seedp,  float low, float high )
{
        float r = (float) rand_r( seedp );              // 0 - RAND_MAX

        return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}

int
Ranf( unsigned int *seedp, int ilow, int ihigh )
{
        float low = (float)ilow;
        float high = (float)ihigh + 0.9999f;

        return (int)(  Ranf(seedp, low,high) );
}

void GrainDeer()
{
	while( NowYear < 2023 )
	{
		// compute a temporary next-value for this quantity
		// based on the current state of the simulation:
		int numDeerTemp = NowNumDeer;
		if (numDeerTemp>NowHeight)
		{
			numDeerTemp = numDeerTemp-1;
		}
		else if (numDeerTemp<=NowHeight)
		{
			numDeerTemp = numDeerTemp+1;
		}

		if (NowNumFires>=9)
		{
			numDeerTemp = numDeerTemp-1;
		}
		
		if (numDeerTemp<0)
		{
			numDeerTemp=0;
		}

		// DoneComputing barrier:
		#pragma omp barrier
		
		NowNumDeer = numDeerTemp;
		// DoneAssigning barrier:
		#pragma omp barrier
		

		// DonePrinting barrier:
		#pragma omp barrier
	
	}
}

void Grain()
{
	while( NowYear < 2023 )
	{
		// compute a temporary next-value for this quantity
		// based on the current state of the simulation:
		float NowHeightTemp = NowHeight;
		float tempFactor = exp(   -SQR(  ( NowTemp - MIDTEMP ) / 10.  )   );

		float precipFactor = exp(   -SQR(  ( NowPrecip - MIDPRECIP ) / 10.  )   );

		NowHeightTemp += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
 		NowHeightTemp -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;

 		NowHeightTemp -= (float)NowNumFires*INCHES_PER_FIRE;

 		if( NowHeightTemp < 0. )
			NowHeightTemp = 0.;

		// DoneComputing barrier:
		#pragma omp barrier
		
		NowHeight = NowHeightTemp;
		// DoneAssigning barrier:
		#pragma omp barrier
		

		// DonePrinting barrier:
		#pragma omp barrier
		
	}
}

void Watcher()
{
	while( NowYear < 2023 )
	{
	

		// DoneComputing barrier:
		#pragma omp barrier
		

		// DoneAssigning barrier:
		#pragma omp barrier
		std::cout<<"***********************************************\n";
		std::cout<<"Year: "<<NowYear<<" Month: "<<NowMonth<<" Elapsed Months: "<<totalMonth<<"\n";
		std::cout<<"Precipitation: "<<NowPrecip<<"\n";
		std::cout<<"Temperature: "<<NowTemp<<"\n";
		std::cout<<"Grain Height: "<<NowHeight<<"\n";
		std::cout<<"Number of Grain Deer: "<<NowNumDeer<<"\n";
		std::cout<<"Number of Fires: "<<NowNumFires<<"\n";
		std::cout<<"***********************************************\n";
		outFile<<totalMonth<<","<<NowPrecip*2.54<<","<<(5./9.)*(NowTemp-32.0)<<","<<NowHeight*2.54<<","<<NowNumDeer<<","<<NowNumFires<<"\n";
		NowMonth++;
		totalMonth++;
		if (NowMonth==12)
		{
			NowMonth=0;
			NowYear++;
		}

		float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

		float temp = AVG_TEMP - AMP_TEMP * cos( ang );
		
		NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

		float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
		NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
		if( NowPrecip < 0. )
			NowPrecip = 0.;

		// DonePrinting barrier:
		#pragma omp barrier
	
	}
}

void FireAgent()
{
	while( NowYear < 2023 )
	{
		// compute a temporary next-value for this quantity
		// based on the current state of the simulation:
		//compute fires based on tempurature grass height and precipitation
		int tempNumFires = ((5./9.)*(NowTemp-32.0)) + ((NowHeight*2.54)/3) - (NowPrecip*2.54);
		tempNumFires = tempNumFires/2;
		if (tempNumFires<0)
		{
			tempNumFires = 0;
		}
		if (tempNumFires > MAX_FIRES)
		{
			tempNumFires = MAX_FIRES;
		}

		// DoneComputing barrier:
		#pragma omp barrier
		
		NowNumFires = tempNumFires;
		// DoneAssigning barrier:
		#pragma omp barrier
		

		// DonePrinting barrier:
		#pragma omp barrier
		
	}
}

int main(int argc, char **argv)
{
	#ifndef _OPENMP
	fprintf(stderr, "OpenMP is not supported here -- sorry.\n");
	return 1;
	#endif

	outFile.open("Results.csv",std::ios::app);
	if (!outFile.is_open())
	{
		std::cout<<"FILE ERROR: Could not open file \n";
		return 1;
	}

	
	NowMonth =    0;
	NowYear  = 2017;

	
	NowNumDeer = 1;
	NowHeight =  1.;
	NowNumFires = 2;

	omp_set_num_threads( 4 );	
	#pragma omp parallel sections
	{
		#pragma omp section
		{
			GrainDeer( );
		}

		#pragma omp section
		{
			Grain( );
		}

		#pragma omp section
		{
			Watcher( );
		}

		#pragma omp section
		{
			FireAgent( );	
		}
	}       

	std::cout<<"\n*********Simulation Complete************\n";
	outFile.close();
	return 0;
}