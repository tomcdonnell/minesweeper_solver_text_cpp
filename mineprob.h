/**************************************************************************************************\
*
* vim: ts=3 sw=3 et wrap co=100 go-=b
*
* Filename: "mineprob.h"
*
* Project: Minesweeper Text
*
* Purpose: Header file for "mineprob.cpp"..
*
* Author: Tom McDonnell 2003
*
\**************************************************************************************************/

#ifndef MINEPROB_H
#define MINEPROB_H

// Includes. ///////////////////////////////////////////////////////////////////////////////////////

#include "minefield.h"

#include <bitset>
#include <iostream>

#include <cassert>

// Inline operator function definitions. ///////////////////////////////////////////////////////////

namespace minesweeper
{

 inline bool operator==(square s1, square s2) {return (s1.row == s2.row && s1.col == s2.col);}
 inline bool operator!=(square s1, square s2) {return !(s1 == s2);}

}

// Class definition. ///////////////////////////////////////////////////////////////////////////////

namespace minesweeper
{

 /*
  * Class whose purpose is to keep track of how many mines have
  * been accounted for in the neighbourhood of an explored square.
  * 
  * Used for tests of order > 1.
  */
 class unknownNboursSharedRec
 {
  public:
    void reset(void) {minMines = 0; maxMines = 0; list.reset();}

    int getMinMines(void) const {return minMines;}
    int getMaxMines(void) const {return maxMines;}

    int getCount(void) const {return list.count();}

    std::bitset<8> getList(void) const {return list;}

    void addToMinMines(const int &n) {assert(n >= 0); minMines += n;}
    void addToMaxMines(const int &n) {assert(n >= 0); maxMines += n;}

    void addSquareToList(const square &s)
    {
       assert(-1 <= s.row and s.row <= 1);
       assert(-1 <= s.col and s.col <= 1);
       list.set(convertSquareToBitPos(s));
    }

    void addSquaresToList(const std::bitset<8> &l)
    {std::bitset<8> andL = list & l; assert(not andL.any()); list or_eq l;}

    bool squareOnList(const square &s) const
    {
       assert(-1 <= s.row and s.row <= 1);
       assert(-1 <= s.col and s.col <= 1);
       return list.test(convertSquareToBitPos(s));
    }

    void print(void) const
    {
       for (int i = 0; i < 8; ++i)
         std::cout << list.test(i) << ", ";
       std::cout << "(" << minMines << " <= n_mines <= " << maxMines << ")" << std::endl;
    }

  private:

    int convertSquareToBitPos(const square &s) const
    {
       switch (s.row)
       {
        case -1: switch (s.col) {case -1: return 0; case 0: return 1; case 1: return 2;}
        case  0: switch (s.col) {case -1: return 3;                   case 1: return 4;}
        case  1: switch (s.col) {case -1: return 5; case 0: return 6; case 1: return 7;}
       }
       return -999; // An error has occurred.
    }

    int minMines, // Maximum number of mines that could be contained in unknown shared neighbours.
        maxMines; // Minimum number of mines that could be contained in unknown shared neighbours.

    std::bitset<8> list; // List of neighbours included in unknown shared neighbours
                         // bits are arranged like so (for square s) -> 0 1 2
                         //                                             3 s 4
                         //                                             5 6 7
 };

 /*
  *
  */
 class mineFieldProbMap
 {
  public:

    /* Constructor. */
    mineFieldProbMap(const mineField *);

    /* Reset all probability map values to unknown. */
    void reset(void)
    {
       for (int r = 0; r < Mptr->getHeight(); ++r)
       {
          for (int c = 0; c < Mptr->getWidth(); ++c)
          {
             probMap[r][c] = -1.0;
          }
       }
    }

    /* Update the probability map to take into account    *
     * knowledge from all squares that have been explored */
    bool update(void);

    /* Print probability map to screen as text. */
    void printProbMap() const;

    /* Return the probability of a square being mined.                  *
     * Only valid if probMap has been update()ed since last exploration */
    double getProbMined(const square &s) const {return probMap[s.row][s.col];}

    /** Boolean test functions. **/

    /* Test whether square is known to be definitely clear. */
    bool squareClear(const square &s) const
    {assert(Mptr->squareInsideMap(s)); return probMap[s.row][s.col] == 0.0;}

    /* Test whether square is known to be definitely mined. */
    bool squareMined(const square &s) const
    {assert(Mptr->squareInsideMap(s)); return probMap[s.row][s.col] == 1.0;}
    bool squareMined(const int &r, const int &c) const {return squareMined(square(r, c));}

    /* Test whether squares state is known definitely (mined or clear). */
    bool squareKnown(const square &s) const {return squareClear(s) or squareMined(s);}
    bool squareKnown(const int & r, const int &c) const {return squareKnown(square(r, c));}

    /* Test whether squares probability is known. */
    bool probKnown(const square &s) const
    {assert(Mptr->squareInsideMap(s)); return probMap[s.row][s.col] != -1.0;}

  private:
    // Private function declarations / inline definitions. //////////////////////////////////////

    /** Boolean test functions. **/

    /* Test whether two squares are neighbours (touching) */
    bool neighbours(const square &s1, const square &s2) const
    {
       assert(Mptr->squareInsideMap(s1));
       assert(Mptr->squareInsideMap(s2));

       return
       (
          s2.row - 1 <= s1.row and s1.row <= s2.row + 1 and
          s2.col - 1 <= s1.col and s1.col <= s2.col + 1 and
          s1 != s2
       );
    }

    /** Counting functions. **/

    int n_knownMinedNbours(const square &s) const;

    /* Return number of mines surrounding s that have not been located definitely. */
    int n_unknownMinedNbours(const square &s) const
    {assert(Mptr->squareExplored(s)); return Mptr->n_minedNbours(s) - n_knownMinedNbours(s);}

    int n_unknownNbours(const square &) const;

    /** Misc. functions. **/

    bool applySimpleTests(const square &s);
    bool applySimpleTestsToAllSquares(void);

    bool applyComplexTests(const square &s, unknownNboursSharedRec &unkNbsShared);
    bool findAndApplyAllComplexTests(const square &s, const int &n_otherSquares);
    bool applyComplexTestsUntilSuccess(const int &n_otherSquares);

    void setProbsOfUnknownNbours(const square &s, const double &p);
    void setProbsOfUnknownNboursNotShared
    (
       const square &s, const std::bitset<8> &L,
       const double &p
    );

    bool findAnotherSquare
    (
       const square &s, unknownNboursSharedRec &unkNbsShared,
       square &startPos, square &n
    ) const;

    bool squareMeetsCriteria
    (
       const square &s, const square &n,
       const unknownNboursSharedRec &unkNbsShared,
       unknownNboursSharedRec &unkNbsSharedWn
    ) const;

    void updateUnkNbsShared
    (
       unknownNboursSharedRec &, const unknownNboursSharedRec &,
       const square &s, const square &n
    ) const;

    void setProbOfExploredSquaresToZero(void);

    void setProbMined(const square &s, const double &p)
    {assert(0.0 <= p and p <= 1.0); assert(Mptr->squareInsideMap(s)); probMap[s.row][s.col] = p;}
    void setProbMined(const int &r, const int &c, const double &p) {setProbMined(square(r, c), p);}

    // Private constants & variables. ///////////////////////////////////////////////////////////

    const mineField *Mptr;

    std::vector< std::vector<double> > probMap; // Probability of square being mined.
                                                //  map[r][c] = 1.0 if definitely mined
                                                //              range(0.0, 1.0) if prob. uncertain
                                                //              0.0 if definitely clear
                                                //             -1.0 if probability unknown
 };

} // End namespace minesweeper.

#endif

/*****************************************END*OF*FILE*********************************************/
