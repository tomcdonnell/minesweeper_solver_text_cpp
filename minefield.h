/**************************************************************************************************\
*
* vim: ts=3 sw=3 et wrap co=100 go-=b
*
* Filename: "minefield.h"
*
* Project: Minesweeper Text
*
* Purpose: Class "mineField" definition.
*
* Author: Tom McDonnell 2003
*
\**************************************************************************************************/

#ifndef MINEFIELD_H
#define MINEFIELD_H

// Includes. ///////////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <iostream>
#include <cassert>

// Global class definitions. ///////////////////////////////////////////////////////////////////////

namespace minesweeper
{

 class square
 {
  public:
    square(const int _row = 0, const int _col = 0)
    : row(_row), col(_col)
    {}

    int row, col;
 };

 inline std::ostream &operator<<(std::ostream &out, const square &s)
 {
    return out << '(' << s.row << ',' << s.col << ')';
 }

 /*
  *
  */
 class mineField
 {
  public:
    // Public function declarations / inline definitions. -------------------------------------//

    /** Initialisation functions. **/

    /* Constructor. */
    mineField(const int &h = 8, const int &w = 8, const int &n = 10);

    void reset(void);

    /** Get functions. **/
    int getHeight(void) const {return map.size();}
    int getWidth(void)  const {return map[0].size();} // rely on rows being same width
    int getNmines(void) const {return n_mines;}

    /** Boolean test functions. **/

    /* Test whether game has been won. */
    bool gameWon(void) const {return squaresExplored == getWidth() * getHeight() - getNmines();}

    /* Test whether square is inside map. */
    bool squareInsideMap(const int &r, const int &c) const
    {return (0 <= r && r < getHeight() && 0 <= c && c < getWidth());}
    bool squareInsideMap(const square &s) const {return squareInsideMap(s.row, s.col);}

    /* Test whether square has been flagged. */
    bool squareFlagged(const int &r, const int &c) const
    {assert(squareInsideMap(r, c)); return expMap[r][c] == -2;}
    bool squareFlagged(const square &s) const {return squareFlagged(s.row, s.col);}

    /* Test whether square has been explored. */
    bool squareExplored(const int &r, const int &c) const
    {assert(squareInsideMap(r, c)); return expMap[r][c] >=  0;}
    bool squareExplored(const square &s) const {return squareExplored(s.row, s.col);}

    /** Counting functions. **/

    /* Returns the number of mines surrounding that square (use only if square explored). */
    int n_minedNbours(const int &r, const int &c) const
    {assert(squareExplored(r, c)); return expMap[r][c];}
    int n_minedNbours(const square &s) const {return n_minedNbours(s.row, s.col);}

    /** Functions corresponding to actions. **/

    /* Flag (mark) square as being mined (To avoid accidentally uncovering it later). */
    void flagSquare(const int &r, const int &c) {expMap[r][c] = -2;}
    void flagSquare(const square &s) {flagSquare(s.row, s.col);}

    /*
     * Explore square (i, j).  If square is mined returns false, else returns true.
     * If square has no surrounding mines, explores all surrounding squares recursively.
     * If all clear squares have been explored game is won.
     */
    bool explore(const square &);
    bool explore(const int &r, const int &c) {return explore(square(r, c));}

    /** Misc. functions. **/

    /*
     * Print map of minefield to screen as text, hiding unexplored territory.
     */
    void printMap(void) const;

  private:
    // Private function declarations / inline definitions. /////////////////////////////////////////

    bool squareMined(const int &r, const int &c) const {return map[r][c];}
    bool squareMined(const square &s) const {return squareMined(s.row, s.col);}

    int countMinedNbours(const square &) const;

    void layMines(void);
    void exploreNbours(const square &);

    // Private constant & variable declarations. ///////////////////////////////////////////////////

    const int n_mines;                      // Total number of mines in mineField.

    std::vector< std::vector<int> > expMap; // Map of explored territory.
                                            // (expMap[r][c] = [0 - 8] If explored,
                                            //                         meaning that many mines
                                            //                         lie in surrounding squares.
                                            //                 -1 If unexplored
                                            //                 -2 If flagged

    std::vector< std::vector<bool> > map;   // Map of minefield (if square is mined true,
                                            //                   if square is clear false)

    int squaresExplored;
 };

} // End namespace minesweeper.

#endif

/*******************************************END*OF*FILE********************************************/
