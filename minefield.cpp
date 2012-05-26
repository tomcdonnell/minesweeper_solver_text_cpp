/**************************************************************************************************\
*
* vim: ts=3 sw=3 et wrap co=100 go-=b
*
* Filename: "minefield.cpp"
*
* Project: Minesweeper Text
*
* Purpose: Function definitions for class "mineField".
*
* Author: Tom McDonnell 2003
*
\**************************************************************************************************/

// Includes. ///////////////////////////////////////////////////////////////////////////////////////

#include "minefield.h"

#include <iostream>
#include <cassert>

// Public function definitions. ////////////////////////////////////////////////////////////////////

namespace minesweeper
{

 /*
  * Constructor.
  */
 mineField::mineField(const int &height, const int &width, const int &n)
 : n_mines(n), squaresExplored(0)
 {
    // Resize columns.
    map.resize(height);
    expMap.resize(height);

    // Resize rows.
    for (int r = 0; r < getHeight(); ++r)
    {
       map[r].resize(width);
       expMap[r].resize(width);
    }

    reset();
 }

 /*
  * Reset the exploration map to unexplored state and
  * lay a new set of mines in a random configuration.
  */
 void mineField::reset(void)
 {
    // Reset maps.
    for (int r = 0; r < getHeight(); ++r)
    {
       for (int c = 0; c < getWidth(); ++c)
       {
          map   [r][c] = false; // Unmined.
          expMap[r][c] = -1;    // Unexplored.
       }
    }

    squaresExplored = 0; // Reset squaresExplored.

    layMines();
 }

 /*
  * Explore square (i, j) and update mineFieldMap[i][j].  If square has
  * no neighboring mines, explores all adjacent squares recursively.
  */
 bool mineField::explore(const square &s)
 {
    assert(squareInsideMap(s));

    if (squareMined(s))
    {
       return false;
    }

    if (not squareExplored(s) and not squareFlagged(s))
    {
       ++squaresExplored;

       expMap[s.row][s.col] = countMinedNbours(s);

       if (n_minedNbours(s) == 0)
       {
          exploreNbours(s);
       }
    }

    return true;
 }

 /*
  * Print map of minefield to screen as text, hiding unexplored territory.
  */
 void mineField::printMap(void) const
 {
    using std::cout;
    using std::endl;

    square s;

    cout << "Minefield map (unexplored territory hidden)." << endl;

    for (s.row = 0; s.row < getHeight(); ++s.row)
    {
       for (s.col = 0; s.col < getWidth(); ++s.col)
       {
          if (squareExplored(s))
          {
             cout << n_minedNbours(s) << " ";
          }
          else
          {
             if (squareFlagged(s)) {cout << "F ";}
             else                  {cout << "- ";}
          }
       }

       cout << endl;
    }
 }

} // End namespace minesweeper.

// Private function definitions. ///////////////////////////////////////////////////////////////////

namespace minesweeper
{

 /*
  * Lay 'nMines' mines at random positions in minefield.
  */
 void mineField::layMines(void)
 {
    int  r, c, n;
    bool mineLayed;

    srand(time(NULL));

    // lay mines
    for (n = 0; n < getNmines(); ++n)
    {
       mineLayed = false;
       while (!mineLayed)
       {
          r = rand() % getHeight();
          c = rand() % getWidth();

          if (!squareMined(r, c))
          {
             map[r][c] = true;
             mineLayed = true;
          }
       }
    }
/*
    map[0][0] = 1; map[0][1] = 1; map[0][2] = 1; map[0][3] = 0; map[0][4] = 1;
    map[1][0] = 0; map[1][1] = 1; map[1][2] = 1; map[1][3] = 0; map[1][4] = 1;
    map[2][0] = 1; map[2][1] = 0; map[2][2] = 0; map[2][3] = 0; map[2][4] = 1;
    map[3][0] = 1; map[3][1] = 1; map[3][2] = 0; map[3][3] = 1; map[3][4] = 0;
*/
 }

 /*
  * Returns the sum of the values in the eight squares surrounding (i, j).
  */
 int mineField::countMinedNbours(const square &s) const
 {
    assert(squareInsideMap(s));

    int t = s.row - 1, tSafe = s.row >               0, // Top    is within map.
        b = s.row + 1, bSafe = s.row < getHeight() - 1, // Bottom is within map.
        l = s.col - 1, lSafe = s.col >               0, // Left   is within map.
        r = s.col + 1, rSafe = s.col < getWidth()  - 1, // Right  is within map.
        count = 0;

    if (tSafe)
    {
       if (lSafe) {count += squareMined(t,     l);} // Top left.
                  {count += squareMined(t, s.col);} // Top middle.
       if (rSafe) {count += squareMined(t,     r);} // Top right.
    }

       if (lSafe) {count += squareMined(s.row, l);} // Middle left.
       if (rSafe) {count += squareMined(s.row, r);} // Middle right.

    if (bSafe)
    {
       if (lSafe) {count += squareMined(b,     l);} // Bottom left.
                  {count += squareMined(b, s.col);} // Bottom middle.
       if (rSafe) {count += squareMined(b,     r);} // Bottom right.
    }

    return count;
 }

 /*
  *
  */
 void mineField::exploreNbours(const square &s)
 {
    assert(squareInsideMap(s));

    int t = s.row - 1, tSafe = s.row >               0, // Top    is within map.
        b = s.row + 1, bSafe = s.row < getHeight() - 1, // Bottom is within map.
        l = s.col - 1, lSafe = s.col >               0, // Left   is within map.
        r = s.col + 1, rSafe = s.col < getWidth()  - 1; // Right  is within map.

    if (tSafe)
    {
       if (lSafe) {explore(t, l    );} // Top left.
                  {explore(t, s.col);} // Top middle.
       if (rSafe) {explore(t, r    );} // Top right.
    }

       if (lSafe) {explore(s.row, l);} // Middle left.
       if (rSafe) {explore(s.row, r);} // Middle right.

    if (bSafe)
    {
       if (lSafe) {explore(b, l    );} // Bottom left.
                  {explore(b, s.col);} // Bottom middle.
       if (rSafe) {explore(b, r    );} // Bottom right.
    }
 }

} // End namespace minesweeper.

/*******************************************END*OF*FILE********************************************/
