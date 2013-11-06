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

struct CacheLine
{
    int tag;    // The tag of this block
    char flags; // This contains the valid/dirty bits.
    int  block; // This contains the actual block.
};

struct CacheSet
{
    std::vector< CacheLine > blocks; // The blocks in this set
};

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
	int offsetMask;
	int setMask;
	int tagMask;
        std::vector< CacheSet > cache;
        

        struct addr
        {
           int tag;
           int set;
	   int offset;
        } lineAddress; 

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
            this->numSets = 
		this->dataSize /(this->associativity * this->lineSize);

	    // Set up the masks
	    offsetMask = 0;
	    for( int i = 0; i < offsetBits; ++i )
		offsetMask = offsetMask * 2 + 1;

	    setMask = 0;
	    for( int i = 0; i < setBits; ++i )
		setMask = setMask * 2 + 1;
	    setMask <<= offsetBits;

	    tagMask = 0;
	    for( int i = 0; i < tagBits; ++i )
		tagMask = tagMask * 2 + 1;
	    tagMask <<= (setBits + offsetBits);

            // Build the cache
	    int blocksPerSet = numBlocks / numSets;
            this->cache.resize( numSets );
            for( int i = 0; i < numSets; ++i )
            {
                CacheSet* cs = new CacheSet();
                cs->blocks.resize( blocksPerSet );
                for( int j = 0; j < blocksPerSet; ++j )
                {
                    CacheLine* cl = new CacheLine();
                    cl->tag = 0;
                    cl->flags = 0;
                    cl->block = 0;
                    cs->blocks[j] = *cl;
                }
                this->cache[i] = *cs;
	    }
        }

        /* Empty and clear out the values in the cache */
        void emptyCache()
        {
            int blocksPerSet = this->numBlocks/this->numSets;
            for( int i = 0; i < this->numSets; ++i )
            {
                for( int j = 0; j < blocksPerSet; ++j )
                {
                    this->cache[i].blocks[j].tag = 0;
                    this->cache[i].blocks[j].flags = 0;
                    this->cache[i].blocks[j].block = 0;
                }
            }
        }

	/* Loads a memory address into the cache */
        void load( unsigned int address )
        {
	    /* TODO:
	     *	We need a way to determine:
	     *	    1) which line in a set to use
	     *	    2) which line to replace if they're all full...
	     */

	    splitAddress( address );
	    cache[set].blocks[0].tag = lineAddress.tag;
	    cache[set].blocks[0].flags = 0;

	    // Is this right? hmmm... the intent is to 
	    // align the start address in the block to be a
	    // multiple of linesize, regardless of what offset
	    // we're loading into the cache...
	    cache[set].blocks[0].block = offset - (linesize % offset);
        }

	/* Checks if a memory address is already in memory */
	bool isLoaded( unsigned int address )
	{
	    /* Solve load first, then figure out a way to make this
	     * search the cache for the address provided.
	     */
	    splitAddress( address );
	    return true;
	}

	/* Will split a 32 bit int into the correct fields */
	void splitAddress( int addr )
	{
	    lineAddress.set = (addr & setMask) >> offsetBits;
	    lineAddress.tag = (addr & tagMask) >> (offsetBits + setBits);
	    lineAddress.offset = (addr & offsetMask);
	}
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
        int numSets;
        std::string traceFile;
        std::string outputFile;
        std::fstream outputFileStream;
        std::fstream inputFileStream;
        Cache cache2Simulate;
        struct line
        {
           std::string accessType;
           unsigned int address;
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
        inputFileStream.open(traceFile.c_str(), std::fstream::in);
        this->numSets = this->dataSize /(this->associativity * this->lineSize);
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
       inputFileStream >> line2Simulate.accessType 
		       >> std::hex >> line2Simulate.address 
		       >> line2Simulate.numInstnsLastMem;
    }
        

    /* function to simulate contents of file */
    void simulate()
    {
        int cycles = 0;
        int hits = 0;
        int accesses = 0;
        int loadHits = 0;
        int loads = 0;
        int storeHits = 0;
        int stores = 0;
        int mal = 0;

        while( !inputFileStream.eof() )
	{
	    readInTrace();
	    accesses++;

            if( line2Simulate.accessType == "s" )
	    {
	        cycles += writeCost( line2Simulate.address );
	        stores++;
	    } else if( line2Simulate.accessType == "l" )
	    {
	        cycles += loadCost( line2Simulate.address );
	        loads++;
	    }

	    cycles += line2Simulate.numInstnsLastMem;

            /* Zero out the simulated line to prevent an 
	     * error that would otherwise occur in the output
	     */
            line2Simulate.accessType = "";
            line2Simulate.address = 0;
            line2Simulate.numInstnsLastMem = 0;
    	}

	outputFileStream << hits/accesses << std::endl
			 << loadHits/loads << std::endl
			 << storeHits/stores << std::endl
			 << cycles << std::endl
			 << mal << std::endl;
    }

    int loadCost( unsigned int address )
    {
	if( cache2Simulate.isLoaded( address ) )
	{
	    return 1;
	} else
	{
	    cache2Simulate.load( address );
	    return missPenalty;
	}
    }

    int writeCost( unsigned int address )
    {
	return 1;
    }
};
