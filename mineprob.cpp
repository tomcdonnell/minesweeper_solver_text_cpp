/**************************************************************************************************\
*
* vim: ts=3 sw=3 et wrap co=100 go-=b
*
* Filename: "mineprob.cpp"
*
* Project: Minesweeper Text
*
* Purpose: Functions for calculating probabilites that squares in minefield are
*          mined based on information from already explored minefield territory.
*
* Author: Tom McDonnell 2003
*
\**************************************************************************************************/

// Includes. ///////////////////////////////////////////////////////////////////////////////////////

#include "mineprob.h"
#include "minefield.h"

#include <iostream>
#include <bitset>
#include <algorithm>

#include <cassert>

// File-scope inline function definitions. /////////////////////////////////////////////////////////

namespace
{

 template<class T>
 T minimum(T a, T b, T c) {return std::min(std::min(a, b), c);}

 template<class T>
 T maximum(T a, T b, T c) {return std::max(std::max(a, b), c);}

}

// Class mineFieldProbMap public member functions. /////////////////////////////////////////////////

namespace minesweeper
{

 /*
  * Constructor.
  */
 mineFieldProbMap::mineFieldProbMap(const mineField *_Mptr)
 : Mptr(_Mptr)
 {
    // Resize rows.
    probMap.resize(Mptr->getHeight());

    int r, c;
    for (r = 0; r < Mptr->getHeight(); ++r)
    {
       // Resize columns.
       probMap[r].resize(Mptr->getWidth());

       // Initialise rows.
       for (c = 0; c < Mptr->getWidth(); ++c)
       {
          probMap[r][c] = -1.0; // Unmined.
       }
    }
 }

 /*
  * Update the probability map to take into account
  * knowledge from all squares that have been explored.
  */
 bool mineFieldProbMap::update(void)
 {
    using std::cout;
    using std::endl;

    setProbOfExploredSquaresToZero();

    cout << "Updating probability map." << endl;

    bool success, probMapChanged = false;

    do
    {
       do
       {
          cout << " Applying simple tests." << endl;
          success = applySimpleTestsToAllSquares();

          if (success)
          {
             probMapChanged = true; // a simple test on at least one square was successful
          }
       }
       while (success);

       cout << " Applying tests involving 1 other square." << endl;
       success = applyComplexTestsUntilSuccess(1);

       if (not success)
       {
          cout << " Applying tests involving 2 other squares." << endl;
          success = applyComplexTestsUntilSuccess(2);
       }

       if (not success)
       {
          cout << " Applying tests involving 3 other squares." << endl;
          success = applyComplexTestsUntilSuccess(3);
       }

       if (success)
       {
          probMapChanged = true; // a higher order test on one square was successful
       }
    }
    while (success);

    return probMapChanged;
}

 /*
  * Print the probability map to the screen as text.
  */
 void mineFieldProbMap::printProbMap(void) const
 {
    using std::cout;
    using std::endl;

    square s;

    for (s.row = 0; s.row < Mptr->getHeight(); ++s.row)
    {
       for (s.col = 0; s.col < Mptr->getWidth(); ++s.col)
       {
          if (!probKnown(s)) {cout << "* ";                  }
          else               {cout << getProbMined(s) << " ";}
       }

       cout << endl;
    }

    cout << endl;
 }

} // End namespace minesweeper.

// Class mineFieldProbMap private function definitions. ////////////////////////////////////////////

namespace minesweeper
{

 // Counting functions. -----------------------------------------------------------------------//

 /*
  * Return the number of nieghbours s that are known to be mined.
  */
 int mineFieldProbMap::n_knownMinedNbours(const square &s) const
 {
    assert(Mptr->squareExplored(s));

    int t = s.row - 1, tSafe = s.row >                     0, // Top    is within map.
        b = s.row + 1, bSafe = s.row < Mptr->getHeight() - 1, // Bottom is within map.
        l = s.col - 1, lSafe = s.col >                     0, // Left   is within map.
        r = s.col + 1, rSafe = s.col < Mptr->getWidth()  - 1, // Right  is within map.
        count = 0;

    if (tSafe)
    {
       if (lSafe) {count += squareMined(t,     l);} // Top left
                  {count += squareMined(t, s.col);} // Top middle
       if (rSafe) {count += squareMined(t,     r);} // Top right
    }

       if (lSafe) {count += squareMined(s.row, l);} // Middle left
       if (rSafe) {count += squareMined(s.row, r);} // Middle right

    if (bSafe)
    {
       if (lSafe) {count += squareMined(b,     l);} // Bottom left.
                  {count += squareMined(b, s.col);} // Bottom middle.
       if (rSafe) {count += squareMined(b,     r);} // Bottom right.
    }

    return count;
 }

 /*
  * Return the number of neighbours of s that are unknown (NOTE: unknown not unexplored).
  */
 int mineFieldProbMap::n_unknownNbours(const square &s) const
 {
    assert(Mptr->squareExplored(s));

    int t = s.row - 1, tSafe = s.row >                     0, // Top    is within map.
        b = s.row + 1, bSafe = s.row < Mptr->getHeight() - 1, // Bottom is within map.
        l = s.col - 1, lSafe = s.col >                     0, // Left   is within map.
        r = s.col + 1, rSafe = s.col < Mptr->getWidth()  - 1, // Right  is within map.
        count = 0;

    if (tSafe)
    {
       if (lSafe) {count += not squareKnown(t,     l);} // Top left.
                  {count += not squareKnown(t, s.col);} // Top middle.
       if (rSafe) {count += not squareKnown(t,     r);} // Top right.
    }

       if (lSafe) {count += not squareKnown(s.row, l);} // Middle left.
       if (rSafe) {count += not squareKnown(s.row, r);} // Middle right.

    if (bSafe)
    {
       if (lSafe) {count += not squareKnown(b,     l);} // Bottom left.
                  {count += not squareKnown(b, s.col);} // Bottom middle.
       if (rSafe) {count += not squareKnown(b,     r);} // Bottom right.
    }

    return count;
 }

 // Miscellaneous functions. ------------------------------------------------------------------//

 /*
  * This function is a special case of function applyTests()
  * where the list of unknown neighbours shared has no members
  * (ie. we are looking at the square s alone).
  * This function exists for efficiency reasons.
  * Update the probability map if anything is learned.
  */
 bool mineFieldProbMap::applySimpleTests(const square &s)
 {
    assert(Mptr->squareExplored(s));

    const int n_unkMinedNbs = n_unknownMinedNbours(s);

    // Condition for detecting mined squares.
    if (n_unkMinedNbs == n_unknownNbours(s))
    {
       std::cout << "  Success at " << s << "." << std::endl;
       setProbsOfUnknownNbours(s, 1.0);
       return true;
    }

    // Condition for detecting clear squares.
    if (n_unkMinedNbs == 0)
    {
       std::cout << "  Success at " << s << "." << std::endl;
       setProbsOfUnknownNbours(s, 0.0);
       return true;
    }

    return false;
 }

 /*
  *
  */
 bool mineFieldProbMap::applySimpleTestsToAllSquares(void)
 {
    bool probMapChanged = false;
    square s;

    // For each square in the mineField...
    for (s.row = 0; s.row < Mptr->getHeight(); ++s.row)
    {
       for (s.col = 0; s.col < Mptr->getWidth(); ++s.col)
       {
          if (Mptr->squareExplored(s) and n_unknownNbours(s))
          {
             if (applySimpleTests(s))
             {
                probMapChanged = true;
             }
          }
       }
    }

    return probMapChanged;
 }

 /*
  * Apply test conditions to square s involving the squares s shares neighbours
  * with (n1, n2, n3) that may result in knowing that the unknown neighbours s does
  * not share with (n1 or n2 or n3) are definitely mined or not mined.
  * Update the probability map if anything is learned.
  */
 bool mineFieldProbMap::applyComplexTests(const square &s, unknownNboursSharedRec &unkNbsShared)
 {
    assert(Mptr->squareExplored(s));

    // Set constants to avoid calculating results twice.
    const int min_n_minesInUnkNbsNotShared = n_unknownMinedNbours(s) - unkNbsShared.getMaxMines(),
              max_n_minesInUnkNbsNotShared = n_unknownMinedNbours(s) - unkNbsShared.getMinMines(),
              n_unkNbsNotShared            = n_unknownNbours(s)      - unkNbsShared.getCount();

    if (n_unkNbsNotShared)
    {
       // Condition for detecting mined squares.
       if (min_n_minesInUnkNbsNotShared == n_unkNbsNotShared)
       {
          setProbsOfUnknownNboursNotShared(s, unkNbsShared.getList(), 1.0);
          return true;
       }

       // Condition for detecting clear squares.
       if (max_n_minesInUnkNbsNotShared == 0)
       {
          setProbsOfUnknownNboursNotShared(s, unkNbsShared.getList(), 0.0);
          return true;
       }
    }

    return false;
 }

 /*
  *
  */
 bool mineFieldProbMap::findAndApplyAllComplexTests(const square &s, const int &n_otherSquares)
 {
    assert(Mptr->squareExplored(s));

    using std::cout;
    using std::endl;

    square s1, s2, s3;

    unknownNboursSharedRec unkNbsShared1;

    unkNbsShared1.reset();
    square startPos1(s.row - 2, s.col - 2);
    while (findAnotherSquare(s, unkNbsShared1, startPos1, s1))
    {
       if (n_otherSquares > 1)
       {
          unknownNboursSharedRec unkNbsShared2 = unkNbsShared1;
          square startPos2(startPos1);
          while (findAnotherSquare(s, unkNbsShared2, startPos2, s2))
          {
             if (n_otherSquares > 2)
             {
                unknownNboursSharedRec unkNbsShared3 = unkNbsShared2;
                square startPos3(startPos2);
                while (findAnotherSquare(s, unkNbsShared3, startPos3, s3))
                {
                   if (applyComplexTests(s, unkNbsShared3))
                   {
                      cout << "  Success at " << s << " using "
                           << s1 << ", " << s2 << ", " << s3 << "." << endl;
                      return true;
                   }

                   unkNbsShared3 = unkNbsShared2;
                }
             }
             else
             {
                if (applyComplexTests(s, unkNbsShared2))
                {
                   cout << "  Success at " << s << " using " << s1 << ", " << s2 << "." << endl;
                   return true;
                }
             }

             unkNbsShared2 = unkNbsShared1;
          }
       }
       else
       {
          if (applyComplexTests(s, unkNbsShared1))
          {
             cout << "  Success at " << s << " using " << s1 << "." << endl;
             return true;
          }
       }

       unkNbsShared1.reset();
    }

    return false;
 }

 /*
  *
  */
 bool mineFieldProbMap::applyComplexTestsUntilSuccess(const int &n_otherSquares)
 {
    assert(1 <= n_otherSquares && n_otherSquares <= 3);

    unknownNboursSharedRec unkNbsShared;
    square s;

    // For each square in the mineField...
    for (s.row = 0; s.row < Mptr->getHeight(); ++s.row)
    {
       for (s.col = 0; s.col < Mptr->getWidth(); ++s.col)
       {
          if
          (
             Mptr->squareExplored(s) and
             n_unknownNbours(s)      and
             findAndApplyAllComplexTests(s, n_otherSquares)
          )
          {
             return true;
          }
       }
    }

    return false;
 }

 /*
  * Sets the probability of unknown neighbours of square s to p.
  * This function is a special case of the function setProbsOfUnknownNboursNotShared()
  * where no neighbours are shared (ie. we are looking at s alone).
  * This function exists for efficiency reasons.
  */
 void mineFieldProbMap::setProbsOfUnknownNbours(const square &s, const double &p)
 {
    assert(Mptr->squareExplored(s));

    int t = s.row - 1, tSafe = s.row >                     0, // Top    is within map.
        b = s.row + 1, bSafe = s.row < Mptr->getHeight() - 1, // Bottom is within map.
        l = s.col - 1, lSafe = s.col >                     0, // Left   is within map.
        r = s.col + 1, rSafe = s.col < Mptr->getWidth()  - 1; // Right  is within map.

    if (tSafe)
    {
       if (lSafe and not squareKnown(t,     l)) {setProbMined(t,     l, p);} // tl
       if (          not squareKnown(t, s.col)) {setProbMined(t, s.col, p);} // tm
       if (rSafe and not squareKnown(t,     r)) {setProbMined(t,     r, p);} // tr
    }

       if (lSafe and not squareKnown(s.row, l)) {setProbMined(s.row, l, p);} // ml
       if (rSafe and not squareKnown(s.row, r)) {setProbMined(s.row, r, p);} // mr

    if (bSafe)
    {
       if (lSafe and not squareKnown(b,     l)) {setProbMined(b,     l, p);} // bl
       if (          not squareKnown(b, s.col)) {setProbMined(b, s.col, p);} // bm
       if (rSafe and not squareKnown(b,     r)) {setProbMined(b,     r, p);} // br
    }
 }

 /*
  * Sets the probability of squares surrounding s whose
  * probabilities are unknown and which are not neighbours of n1 or n2 or n3.
  * L here is a list of unknown neighbours s shares with other squares.
  */
 void mineFieldProbMap::setProbsOfUnknownNboursNotShared
 (
    const square &s, const std::bitset<8> &L, const double &p
 )
 {
    assert(Mptr->squareExplored(s));

    int t = s.row - 1, tSafe = s.row >                     0, // Top    is within map.
        b = s.row + 1, bSafe = s.row < Mptr->getHeight() - 1, // Bottom is within map.
        l = s.col - 1, lSafe = s.col >                     0, // Left   is within map.
        r = s.col + 1, rSafe = s.col < Mptr->getWidth()  - 1; // Right  is within map.

    if (tSafe)
    {
       if (lSafe and not squareKnown(t,     l) and not L.test(0)) {setProbMined(t,     l, p);} // tl
       if (          not squareKnown(t, s.col) and not L.test(1)) {setProbMined(t, s.col, p);} // tm
       if (rSafe and not squareKnown(t,     r) and not L.test(2)) {setProbMined(t,     r, p);} // tr
    }

       if (lSafe and not squareKnown(s.row, l) and not L.test(3)) {setProbMined(s.row, l, p);} // ml
       if (rSafe and not squareKnown(s.row, r) and not L.test(4)) {setProbMined(s.row, r, p);} // mr

    if (bSafe)
    {
       if (lSafe and not squareKnown(b,     l) and not L.test(5)) {setProbMined(b,     l, p);} // bl
       if (          not squareKnown(b, s.col) and not L.test(6)) {setProbMined(b, s.col, p);} // bm
       if (rSafe and not squareKnown(b,     r) and not L.test(7)) {setProbMined(b,     r, p);} // br
    }
 }

 /*
  *
  */
 bool mineFieldProbMap::findAnotherSquare
 (
    const square &s, unknownNboursSharedRec &unkNbsShared, square &startPos, square &n
 ) const
 {
    assert(Mptr->squareInsideMap(s));
    assert(s.row - 2 <= startPos.row and startPos.row <= s.row + 3); // NOTE: startPos.row must be
    assert(s.col - 2 <= startPos.col and startPos.col <= s.col + 2); //       allowed to overrun
                                                                     //       by one so that we
    // Test for overrun.                                             //       know when all
    if (startPos.row > s.row + 2)                                    //       eligible squares have
    {                                                                //       been tested.
       // All eligible squares have already been tested.
       return false;
    }

    using std::max;
    using std::min;

    // For each square that possibly shares neighbours with s.
    // (starting at (startRow, startCol))
    for (n.row = startPos.row; n.row <= min(s.row + 2, Mptr->getHeight()); ++n.row)
    {
       for (n.col = max(s.col - 2, 0); n.col <= min(s.col + 2, Mptr->getWidth()); ++n.col)
       {
          if (n.row == startPos.row and n.col == max(s.col - 2, 0))
          {
             n.col = max(startPos.col, 0); // If on 1st run, set n.col to startPos.col
          }                                // (on subsequent runs, n.col must start from s.col - 2)

          if
          (
              Mptr->squareInsideMap(n) and
              Mptr->squareExplored(n)  and
              n_unknownNbours(n)       and
              n != s
          )
          {
             unknownNboursSharedRec unkNbsSharedWn; // Unknown neighbours s shares with n list
                                                    // to be compiled by squareMeetsCriteria().

             // If square n meets criteria, update unkNbsShared
             // and startPos  to reflect n's inclusion
             if (squareMeetsCriteria(s, n, unkNbsShared, unkNbsSharedWn))
             {
                updateUnkNbsShared(unkNbsShared, unkNbsSharedWn, s, n);

                // Update startPos to one after n.
                startPos.row = n.row;
                startPos.col = n.col + 1;
                if (startPos.col > s.col + 2)
                {
                   startPos.row++;
                   startPos.col = s.col - 2;
                }

                return true;
             }
          }
       }
    }

    return false;
 }

 /*
  * Test whether square n has unknown neighbours not in list
  * containing a definite number of mines.
  */
 bool mineFieldProbMap::squareMeetsCriteria
 (
    const square &s, const square &n,
    const unknownNboursSharedRec &unkNbsShared,
    unknownNboursSharedRec &unkNbsSharedWn
 ) const
 {
    assert(Mptr->squareExplored(s));
    assert(Mptr->squareExplored(n));
    assert(s != n);

    square t; // Temporary square.

    // For t = squares surrounding s...
    //    Find all unknown neighbours s shares with n
    for (t.row = s.row - 1; t.row <= s.row + 1; ++t.row)
    {
       for (t.col = s.col - 1; t.col <= s.col + 1; ++t.col)
       {
          if
          (
             Mptr->squareInsideMap(t)
             and not squareKnown(t) // Will exclude s and n because they are explored & hence known
             and neighbours(t, n)
          )
          {
             unkNbsSharedWn.addSquareToList(square(t.row - s.row, t.col - s.col));
          }
       }
    }

    // Cull candidate squares.

    if (unkNbsSharedWn.getCount() == 0)
    {
       // There are no unknown neighbours shared by s and n.
       return false;
    }

    std::bitset<8> doubles = unkNbsSharedWn.getList() & unkNbsShared.getList();
    if (doubles.any())
    {
       // At least one square from unkNbsSharedWn is also in unkNbsShared.
       return false;
    }

    // Accept candidate square, and so update unkNbsSharedList and n_unknownMinedNbours.
    return true;
 }

 /*
  *
  */
 void mineFieldProbMap::updateUnkNbsShared
 (
    unknownNboursSharedRec &unkNbsShared,
    const unknownNboursSharedRec &unkNbsSharedWn,
    const square &s, const square &n
 ) const
 {
    int n_unkNbsSsharesWithN = unkNbsSharedWn.getCount();

    if (n_unkNbsSsharesWithN == n_unknownNbours(n))
    {
       // We know for certain how many mines are in the unkNbsSharedWn squares.
       unkNbsShared.addToMinMines(n_unknownMinedNbours(n));
       unkNbsShared.addToMaxMines(n_unknownMinedNbours(n));
    }
    else
    {
       // Must investigate max & min possible number of mines in the unkNbsSharedWn squares.

       // Assign constants to avoid calculating them twice.
       const int n_unkNbsOfSnotSharedWithN = n_unknownNbours(s) - n_unkNbsSsharesWithN,
                 n_unkNbsOfNnotSharedWithS = n_unknownNbours(n) - n_unkNbsSsharesWithN,
                 n_unkMinedNbsOfS          = n_unknownMinedNbours(s),
                 n_unkMinedNbsOfN          = n_unknownMinedNbours(n);

       // To calculate the minimum number of unknown mined neighbours shared by s & n:
       //   From s's perspective:
       //     Assume all s's unknown mined neighbours are in squares not shared with n, so:
       //       min = n_unkMinedNbsOfS - n_unkNbsOfSnotSharedWithN (valid only if >= 0)
       //   From n's perspective:
       //     Similarly:
       //       min = n_unkMinedNbsOfN - n_unkNbsOfNnotSharedWithS (valid only if >= 0)
       //   If neither valid, min = 0,
       //     so in order to be most restrictive we take the maximum.
       int min = maximum
       (
          0, n_unkMinedNbsOfS - n_unkNbsOfSnotSharedWithN,
          n_unkMinedNbsOfN - n_unkNbsOfNnotSharedWithS
       );

       // To calculate the maximum number of unknown mined neighbours shared by s & n:
       //   From s's perspective:
       //     Assume all s's unknown mined neighbours are in squares shared with n, so
       //       max = n_unkMinedNbsOfS (valid only if <= n_unkNbsSsharesWithN)
       //   From n's perspective:
       //     Similarly
       //       max = n_unkMinedNbsOfN (valid only if <= n_unkNbsSsharesWithN)
       //   If neither valid, max = n_unkNbsSsharesWithN,
       //     so in order to be most restrictive, we take the minimum.
       int max = minimum(n_unkNbsSsharesWithN, n_unkMinedNbsOfS, n_unkMinedNbsOfN);

       // NOTE: min may equal max
       unkNbsShared.addToMinMines(min);
       unkNbsShared.addToMaxMines(max);
    }

    unkNbsShared.addSquaresToList(unkNbsSharedWn.getList()); // Update unkNbsShared.
 }

 /*
  *
  */
 void mineFieldProbMap::setProbOfExploredSquaresToZero(void)
 {
    // Set prob to: -1.0 for all unexplored squares,
    //               0.0 for all   explored squares.
    for (int r = 0; r < Mptr->getHeight(); ++r)
    {
       for (int c = 0; c < Mptr->getWidth(); ++c)
       {
          if (Mptr->squareExplored(r, c))
          {
             setProbMined(r, c,  0.0);
          }
       }
    }
 }

} // End namespace minesweeper.

/*******************************************END*OF*FILE********************************************/
