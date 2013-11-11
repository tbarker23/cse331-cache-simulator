/******************************************************************************
/   filename:   main.cpp
/
/   description: Implementation of the cache simulator
/
/   authors: Barker, Thomas
/            Delaney, Kent
/	     Poston, Jake
/
/   class:      CSE331
/   instructor: Zheng
/   assignment: Lab Project #2
/
/   assigned:   Oct 28, 2013
/   due:        Nov 11, 2013
/
/*****************************************************************************/
#include <iostream>
#include <fstream>
#include "simulator.h"

using namespace std;

/* Calculates if a number is a power of 2. */
bool isPowOf2( unsigned int i );

int main( int argc, char** argv )
{
    /* Check for proper usage */
    if( argc != 3 )
    { 
        cout<< "Usage:\n\t";
        cout<< "cachesim configFile traceFile\n";
        return -1;
    }

    /* Ensure the configuration is correct */
    fstream config( argv[1], fstream::in );

    int configs[6];
    for( int i = 0; i < 6; ++i )
    {
        config>> configs[i];

        if( i <= 2 && (configs[i] < 0 || !isPowOf2( configs[i] )) )
        {
            cout<< "Invalid configuration. Line " << i+1
                << " should be a non-negative power of 2.\n";
            return -2;
        }
        if( i == 3 && configs[i] != 0 && configs[i] != 1 )
        {
            cout<< "Invalid configuration. Line " << i+1 
                << " should be 0 (for random) or 1 (for FIFO).\n";
            return -2;
        }
        if( i == 4 && configs[i] < 0 )
        {
            cout<< "Invalid configuration. Line " << i+1
                << " should be a positive integer.\n";
            return -2;
        }
        if( i == 5 && configs[i] != 0 && configs[i] != 1 )
        {
            cout<< "Invalid configuration. Line " << i+1
                << " should be 0 (for no-write-allocate) "
                << "or 1 (for write-allocate).\n";
            return -2;
        }
    }

    config.close();

    /* Construct the simulator */
    Simulator s( configs[0], configs[1], configs[2], configs[3]
               , configs[4], configs[5], string( argv[2] ) );

    /* SIMULATE... THAT... CACHE!!!! (trace given by argv[2]) */
    s.simulate();

    return 0;
}

bool isPowOf2( unsigned int i )
{
    /* This snippet was found at:
     * http://stackoverflow.com/questions/600293/
     *    how-to-check-if-a-number-is-a-power-of-2
     */
    return (i != 0) && ((i & (i - 1)) == 0);
}
