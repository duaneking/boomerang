/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc  
 * Copyright (C) 2002, Trent Waddington
 *
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:        frontend.h
 * OVERVIEW:    This file contains the definition for the FrontEnd class,
 *              which implements the source indendent parts of the front end
 *              of UQBT: decoding machine instructions into a control flow
 *              graph populated with low and high level RTLs.
 *              Also has some prototypes and structs for switch.cc
 *============================================================================*/

/* $Revision$
 *
 * 17 Apr 02 - Mike: Mods to adapt UQBT code to boomerang
 */


#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include <list>
#include <map>
#include <queue>
#include <fstream>
#include "types.h"

class UserProc;
class Proc;
class TargetQueue;
class RTL;
class NJMCDecoder;
class BasicBlock;
typedef BasicBlock* PBB;
class Exp;
class TypedExp;
class Cfg;

// Control flow types
enum INSTTYPE {
    I_UNCOND,                // unconditional branch
    I_COND,                  // conditional branch
    I_N_COND,                // case branch
    I_CALL,                  // procedure call
    I_RET,                   // return
    I_COMPJUMP,              // computed jump
    I_COMPCALL               // computed call
};

typedef bool (*PHELPER)(ADDRESS dest, ADDRESS addr, std::list<RTL*>* lrtl); 

class FrontEnd {
protected:
    int delta;                  // Text host address - native address difference
    ADDRESS uUpper;             // Upper address for the text segment
//    const int NOP_SIZE;         // Size of a no-op instruction (in bytes)
//    const int NOP_INST;         // No-op pattern
	std::map<ADDRESS, Proc *> processed;

public:
    /*
     * Constructor. Takes some parameters to save passing these around a lot
     */
    FrontEnd(int delta, ADDRESS uUpper);
    // If you know which machine you want.
    static FrontEnd* instantiate(MACHINE machine, int delta, ADDRESS uUpper);

    /**
     * Destructor. Virtual to mute a warning
     */
virtual ~FrontEnd();

// returns a string identifer for this frontend
virtual const char *getFrontEndId() = 0;

// returns a frontend given a string
static FrontEnd *createById(std::string &str, int delta, ADDRESS uUpper);

    /*
     * Function to fetch the smallest machine instruction
     */
virtual int     getInst(int addr);

    /*
     * Accessor function to get the decoder.
     */
virtual NJMCDecoder *getDecoder() = 0;

    /*
     * processProc. This is the main function for decoding a procedure.
     * It is usually overridden in the derived class to do
     * source machine specific things.
     * If spec is set, this is a speculative decode
     * Returns true on a good decode
     */
virtual bool    processProc(ADDRESS uAddr, UserProc* pProc, std::ofstream &os, 
  bool spec = false, PHELPER helperFunc = NULL);

    /*
     * Locate the starting address of "main", returning a native address
     */
virtual ADDRESS getMainEntryPoint( bool &gotMain ) = 0;

    /*
     * getInstanceFor. Get an instance of a class derived from FrontEnd,
     * returning a pointer to the object of that class.
     * Do this by guessing the machine for the binary file whose name is
     * sName, loading the appropriate library using dlopen/dlsym, running
     * the "construct" function in that library, and returning the result.
     */
static FrontEnd* getInstanceFor( const char* sName, void*& dlHandle,
  int delta, ADDRESS uUpper, NJMCDecoder*& decoder);

    /*
     * Close the library opened by getInstanceFor
     */
static void closeInstance(void* dlHandle);

};


/*==============================================================================
 * These functions do the analysis required to see if a register jump
 * is actually a switch table.
 *============================================================================*/

/*
 * Initialise the switch analyser.
 */
void initSwitch();

/*
 * Attempt to determine whether this DD instruction is a switch
 * statement. If so, return true, and set iLower, uUpper to the
 * switch range, and set uTable to the native address of the
 * table. If it is form O (the table is an array of offsets from
 * the start of the table), then chForm will be 'O', etc.
 * If it is form H (hash table), then iNumTable will be the
 * number of entries in the table (not the number of cases,
 * or max-min).
 */
bool isSwitch(PBB pBB, Exp* pDest, UserProc* pProc);

/*
 * Make use of the switch info. Should be incorporated into isSwitch.
 */
void processSwitch(PBB pBB, int delta, Cfg* pCfg, TargetQueue& targetQueue,
    UserProc* proc);


/*==============================================================================
 * These functions are the machine specific parts of the front end. They consist
 * of those that actually drive the decoding and analysis of the procedures of
 * the program being translated.
 * These functions are implemented in the files front<XXX> where XXX is a
 * plaform name such as sparc or 386.
 *============================================================================*/

/*
 * Intialise the procedure decoder and analyser.
 */
void initFront();

/*
 * Needed by the switch logic. Here because it's source machine specific.
 */
unsigned fetch4(unsigned char* ptr);

/*
 * Decode one RTL
 */
RTL* decodeRtl(ADDRESS address, int delta, NJMCDecoder* decoder);

/*
 * This decodes a given procedure. It performs the
 * analysis to recover switch statements, call
 * parameters and return types etc.
 * If keep is false, discard the decoded procedure (only need this to find code
 *  other than main that is reachable from _start, for coverage and speculative
 *  decoding)
 * If spec is true, then we are speculatively decoding (i.e. if there is an
 *  illegal instruction, we just bail out)
 */
bool decodeProc(ADDRESS uAddr, FrontEnd& fe, bool keep = true,
    bool spec = false);

// Put the target queue logic into this small class
class TargetQueue {
    std::queue<ADDRESS>  targets;

public:

/*
 * FUNCTION:    visit
 * OVERVIEW:    Visit a destination as a label, i.e. check whether we need to
 *              queue it as a new BB to create later.
 *              Note: at present, it is important to visit an address BEFORE
 *              an out edge is added to that address. This is because adding
 *              an out edge enters the address into the Cfg's BB map, and it
 *              looks like the BB has already been visited, and it gets
 *              overlooked. It would be better to have a scheme whereby the
 *              order of calling these functions (i.e. visit() and
 *              AddOutEdge()) did not matter.
 * PARAMETERS:  pCfg - the enclosing CFG
 *              uNewAddr - the address to be checked
 *              pNewBB - set to the lower part of the BB if the address
 *                already exists as a non explicit label (BB has to be split)
 * RETURNS:     <nothing>
 */
    void visit(Cfg* pCfg, ADDRESS uNewAddr, PBB& pNewBB);
/*
 * Provide an initial address (can call several times if there are several
 *  entry points)
 */
    void initial(ADDRESS uAddr);


/*
 * FUNCTION:      nextAddress
 * OVERVIEW:      Return the next target from the queue of non-processed
 *                targets.
 * PARAMETERS:    cfg - the enclosing CFG
 * RETURNS:       The next address to process, or 0 if none (queue is empty)
 */
    ADDRESS nextAddress(Cfg* cfg);

};






#endif      // #ifndef __FRONTEND_H__
