#include <iostream>
#include <fstream>

using namespace std;

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

    /* SIMULATE... THAT... CACHE!!!! */

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
