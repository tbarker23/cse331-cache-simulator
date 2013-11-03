/******************************************************************************
/
/   filename:   simulator.h
/
/   description: Implementation of the cache simulator
/
/   authors: Barker, Thomas
/            Delaney, Kent
/
/   class:      CSE331
/   instructor: Zheng
/   assignment: Lab Project #2
/
/   assigned:   Oct 28, 2013
/   due:        Nov 11, 2013
/
/*****************************************************************************/
#include<iostream>
#include<fstream>
#include<math.h>
#include<vector>

class Cache
{
    /* Frames = lineSize / dataSize;
     * For an address;
     *     offsetBits = log_2(lineSize)
     *     setBits    = log_2(frames) / associativity (if assoc > 0 )
     *     tagBits    = 32 - offsetBits - setBits
     *
     * Visually:
     *     | tag            | set     |   offset |
     */
    protected:
        int lineSize;
        int associativity;
        int dataSize;
        int replacePolicy;
        int frames;
        int offsetBits;
        int setBits;
        int tagBits;
        int numBlocks;
        int numSets;
        std::vector< std::vector< int > > cache;
        

    public:
        Cache()
        {
        }
        Cache( int linesize, int assoc, int datasize, int replacement )
        {
            this->lineSize = linesize;
            this->associativity = assoc;
            this->dataSize = datasize;
            this->replacePolicy = replacement;

            this->frames = this->lineSize / this->dataSize;
            this->offsetBits = log2( this->lineSize );
            if( this->associativity > 0 )
            {
                this->setBits = log2( this->frames ) / this->associativity;
            } else
            {
                this->setBits = log2( this->frames );
            }
            this->tagBits = 32 - setBits - offsetBits;
            
            this->numBlocks = this->dataSize / this->lineSize;
            this->numSets = this->dataSize /(this->associativity * this->lineSize);
        }

        /* Empty and clear out the values in the cache */
        void emptyCache()
        {
            for( int i = 0; i < this->numSets; ++i )
            {
                for( int j = 0; j < this->associativity; ++j)
                {
                    cache[i][j] = 0;
                }
            }
        }
        void replace( /* not sure what args are needed... */ );
        void load( unsigned int address );
};

class Simulator 
{
    protected:
        int lineSize;
        int associativity;
        int dataSize;
        int replacePolicy;
        int missPenalty;
        int writeAllocate;
        std::string traceFile;
        std::string outputFile;
        std::fstream outputFileStream;
        std::fstream inputFileStream;
        Cache cache2Simulate;
        struct line
        {
           std::string accessType;
           int address;
           int numInstnsLastMem;
        } line2Simulate;
       
             
    public:
    /* CTOR */
    Simulator(int lsize, int asctvty, int dsize, 
              int rpol, int mpenal, int walloc, std::string fname
              )
    {
        this->lineSize = lsize;
        this->associativity = asctvty;
        this->dataSize = dsize;
        this->replacePolicy = rpol;
        this->missPenalty = mpenal;
        this->writeAllocate = walloc;
        this->traceFile = fname;
        this->outputFile = fname + ".out";
        outputFileStream.open(outputFile.c_str(), std::fstream::out);
        inputFileStream.open(traceFile.c_str(), std::fstream::out);
        this->cache2Simulate = Cache(this->lineSize, this->associativity, 
                                     this->dataSize, this->replacePolicy
                                     );
    }

    ~Simulator()
    {
        outputFileStream.close();
        inputFileStream.close();
    }

    /* Read in trace file data */
    void readInTrace()
    {
       inputFileStream >> line2Simulate.accessType >> 
                        line2Simulate.address >> line2Simulate.numInstnsLastMem;
    }

    /* function to simulate contents of file */
    void simulate()
    {
    }
};
