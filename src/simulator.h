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
#include<cstdlib> // rand
#include<vector>

// Describes a line in a cache
struct CacheLine
{
    int tag;    // The tag of this block
    int age;	// The age of the block
    char flags; // This contains the valid bit.
    int  block; // This contains the actual block.
};

// Describes a set of lines in a cache
struct CacheSet
{
    std::vector< CacheLine > blocks; // The blocks in this set
    int nextLineUsed; // The next line to replace during loads
};

// Describes an entire cache
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
        int lineSize;	    // The size of a block
        int associativity;  // The associativity of the cache
        int dataSize;	    // The total size of the cache
        int replacePolicy;  // Replacement policy
        int frames;	    // The number of lines in the cache
        int offsetBits;	    // The number of offset bits in an address
        int setBits;	    // The number of set bits in an address
        int tagBits;	    // The number of tag bits in an address
        int numSets;	    // The total number of sets
	int offsetMask;	    // The mask of the offset
	int setMask;	    // The mask of the set
	int tagMask;	    // The mask of the tag
        std::vector< CacheSet > cache; // The cache
        
	// Holds information about an address
        struct addr
        {
           int tag;
           int set;
	   int offset;
        } lineAddress; 

    public:
	/* Default CTOR. Does nothing. */
        Cache()
        {
        }

	/* CTOR */
        Cache( int linesize, int assoc, int datasize, int replacement )
        {
	    /* Straight assignments */
            this->lineSize = linesize;
            this->associativity = assoc;
            this->dataSize = datasize;
            this->replacePolicy = replacement;

	    /* Calculate cache properties */
            this->frames = this->dataSize / this->lineSize;

	    /* Calculate the breakdown of the address bits */
            this->offsetBits = log2( this->lineSize );

            if( this->associativity > 0 )
            {
                this->setBits = log2( this->frames ) / this->associativity;
            } else
            {
                this->setBits = log2( this->frames );
            }

            this->tagBits = 32 - setBits - offsetBits;

	    /* Get the number of sets in the cache */          
            this->numSets = 
		this->dataSize /(this->associativity * this->lineSize);

	    // Set up the masks
	    offsetMask = 0;
	    for( int i = 0; i < offsetBits; ++i )
		offsetMask = (offsetMask << 1) + 1;

	    setMask = 0;
	    for( int i = 0; i < setBits; ++i )
		setMask = (setMask << 1) + 1;
	    setMask <<= offsetBits;

	    tagMask = 0;
	    for( int i = 0; i < tagBits; ++i )
		tagMask = (tagMask << 1) + 1;
	    tagMask <<= (setBits + offsetBits);

            // Build the actual cache
	    int blocksPerSet = frames / numSets;
            this->cache.resize( numSets );
            for( int i = 0; i < numSets; ++i )
            {
                CacheSet* cs = new CacheSet();
		cs->nextLineUsed = 0;
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
            int blocksPerSet = this->frames/this->numSets;
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
	    splitAddress( address );

	    // Figure out the line to use
	    int lineNo = 0;
	    switch( replacePolicy )
	    {
		case 0:
		default:
		    lineNo = rand() % associativity;
		    break;
		case1:
		    {
			// This is FIFO replacement...the way
			// we implement this implies that the
			// order of replacement follows 0 <= i < blocks
			// and wraps around back to 0 again...
			int blocks = cache[lineAddress.set].blocks.size();
			lineNo = cache[lineAddress.set].nextLineUsed;
			cache[lineAddress.set].nextLineUsed = 
			    (cache[lineAddress.set].nextLineUsed + 1) % blocks;
		    }
		    break;
	    };

	    /* Set up the block */
	    cache[lineAddress.set].blocks[lineNo].tag = lineAddress.tag;
	    cache[lineAddress.set].blocks[lineNo].flags |= 0x1;

	    // Is this right? hmmm... the intent is to 
	    // align the start address in the block to be a
	    // multiple of linesize, regardless of what offset
	    // we're loading into the cache...
	    cache[lineAddress.set].blocks[lineNo].block = 
		lineAddress.offset - (lineAddress.offset % lineSize);
        }

	/* Checks if a memory address is already in memory */
	bool isLoaded( unsigned int address )
	{
	    // Assume it's not loaded. This seems safe.
	    bool loaded = false;

	    splitAddress( address );

	    // Look through the blocks in this set to see if the
	    // tag matches the one of the address. If we find one,
	    // it's loaded.
	    int set = lineAddress.set;
	    int blocks = cache[set].blocks.size();
	    for( int i = 0; i < blocks; ++i )
	    {
		if( cache[set].blocks[i].tag == lineAddress.tag )
		    loaded = true;
	    }

	    return loaded;
	}

	/* Will split a 32 bit int into the correct fields */
	void splitAddress( int addr )
	{
	    lineAddress.set = (unsigned int)(addr & setMask) >> offsetBits;
	    lineAddress.tag = 
		(unsigned int)(addr & tagMask) >> (offsetBits + setBits);
	    lineAddress.offset = (addr & offsetMask);
	}
};

/* Our actual simulator! */
class Simulator 
{
    protected:
	// Why do we store some of these here, again?
        int lineSize;	    // block size
        int associativity;  // associativity
        int dataSize;	    // full cache size
        int replacePolicy;  // replacement policy
        int missPenalty;    // miss penalty in the cache
        int writeAllocate;  // write-alloc or no-write-alloc
        int numSets;	    // number of sets in the cache
        std::string traceFile;	// The trace file
        std::string outputFile;	// The output file
        std::fstream outputFileStream;	// output stream
        std::fstream inputFileStream;	// input stream
        Cache cache2Simulate;		// The cache

	// Line information from input file
        struct line
        {
           std::string accessType;
           unsigned int address;
           int numInstnsLastMem;
        } line2Simulate;

	// Information for stats calculations
	struct cacheStatData
	{
	    int cycles;
	    bool hit;
	};
             
    public:
    /* CTOR */
    Simulator(int lsize, int asctvty, int dsize, 
              int rpol, int mpenal, int walloc, std::string fname
              )
    {
	// Cache information setup
        this->lineSize = lsize;
        this->associativity = asctvty;

	// We multiply this by 2^10 because dsize is given in KB
        this->dataSize = dsize << 10;

	// simulation information
        this->replacePolicy = rpol;
        this->missPenalty = mpenal;
        this->writeAllocate = walloc;

	// Set up the input/output files
        this->traceFile = fname;
        this->outputFile = fname + ".out";
        outputFileStream.open(outputFile.c_str(), std::fstream::out);
        inputFileStream.open(traceFile.c_str(), std::fstream::in);

	// Calculate set number
        this->numSets = this->dataSize /(this->associativity * this->lineSize);

	// Create the cache
        this->cache2Simulate = Cache(this->lineSize, this->associativity, 
                                     this->dataSize, this->replacePolicy
                                     );
    }

    ~Simulator()
    {
	// Clean these up
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
	int hittime = 1;
	int misstime = missPenalty;

	// Read through the input file.  This performs one more loop
	// than we really would like, but that last iteration should
	// be pretty much irrelevant.
        while( !inputFileStream.eof() )
	{
	    cacheStatData csd;

	    readInTrace();

	    // Take different actions based on store (s) or load (l)
            if( line2Simulate.accessType.compare("s") == 0 )
	    {
		accesses++;
	        csd = writeCost( line2Simulate.address );
		if( csd.hit )
		{
		    hits++;
		    storeHits++;
		}
	        stores++;
		cycles += csd.cycles;
	    } else if( line2Simulate.accessType.compare("l") == 0 )
	    {
		accesses++;
	        csd = loadCost( line2Simulate.address );
		if( csd.hit )
		{
		    hits++;
		    loadHits++;
		}
	        loads++;
		cycles += csd.cycles;
	    }

	    // Assume 1 cycle for all other instructions
	    cycles += line2Simulate.numInstnsLastMem;

            /* Zero out the simulated line to prevent an 
	     * error that would otherwise occur in the output
	     */
            line2Simulate.accessType = "";
            line2Simulate.address = 0;
            line2Simulate.numInstnsLastMem = 0;
    	}

	// Once we're done with the input file
	float missRate = 1.0f - (1.0f*hits/(float)accesses);
	mal = hittime + missRate * misstime;

	// Write our stats to the output file
	outputFileStream << hits*100.0f/(float)accesses << std::endl
			 << loadHits*100.0f/(float)loads << std::endl
			 << storeHits*100.0f/(float)stores << std::endl
			 << cycles << std::endl
			 << mal << std::endl;
    }

    /* Calculates statistics for loading into the cache */
    cacheStatData loadCost( unsigned int address )
    {
	cacheStatData csd;

	if( cache2Simulate.isLoaded( address ) )
	{
	    // we hit, so no load penalty is accrued
	    csd.hit = true;
	    csd.cycles = 1;
	    return csd;
	} else
	{
	    cache2Simulate.load( address );

	    csd.hit = false;

	    if( this->writeAllocate == 1 )
		csd.cycles = missPenalty + 1;	// +1 for the write-back
	    else
		csd.cycles = missPenalty;

	    return csd;
	}
    }

    /* Calculates statistics data for writes */
    cacheStatData writeCost( unsigned int address )
    {
	cacheStatData csd;

	// We take different actions based on the write allocation scheme
	switch( this->writeAllocate )
	{
	    case 0: // No-write-allocate
		if( cache2Simulate.isLoaded( address ) )
		{
		    csd.hit = true;
		    csd.cycles = 2; // 1 for write in cache, 1 for write
		    return csd;
		} else
		{
		    csd.hit = false;
		    // 1 for write in memory
		    csd.cycles = 1;
		    return csd;
		}
		break;
	    case 1: // write-allocate
		if( cache2Simulate.isLoaded( address ) )
		{
		    csd.hit = true;
		    // write once now
		    csd.cycles = 1;

		    return csd;
		} else
		{
		    cache2Simulate.load( address );
    
		    csd.hit = false;
		    // miss penalty plus 1 for the write
		    csd.cycles = missPenalty + 1;

		    return csd;
		}
		break;
	    default: // Assume a weird case here?
		csd.hit = false;
		csd.cycles = 1;
		return csd;
		break;
	}
    }
};
