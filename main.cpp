/**************************************************************************************************\
*
* vim: ts=3 sw=3 et wrap co=100 go-=b
*
* Filename: "main.cpp"
*
* Project: Minesweeper Text
*
* Purpose: Main function for text version of minesweeper game.
*
* Author: Tom McDonnell 2003
*
\**************************************************************************************************/

// Includes. ///////////////////////////////////////////////////////////////////////////////////////

#include "minefield.h"
#include "mineprob.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>

// File-scope function declarations. ///////////////////////////////////////////////////////////////

namespace
{
 using namespace minesweeper;

 bool autoExplore(mineField &M, mineFieldProbMap &P);
 int playGame(int, int, int);
}

// Main function definition. ///////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
   using namespace minesweeper;

   using std::cin;
   using std::cout;
   using std::endl;

   int n_rows, n_cols, n_mines;
   std::string settingsTypeStr;

   switch (argc)
   {
    case 1:
      // Use default arguments.
      settingsTypeStr = "default";
      n_rows  =  32;
      n_cols  =  32;
      n_mines = 200;
      break;
    case 4:
      // Use user-supplied arguments.
      settingsTypeStr = "user supplied";
      n_rows  = atoi(argv[1]);
      n_cols  = atoi(argv[2]);
      n_mines = atoi(argv[3]);
      break;
    default:
      std::cout << "Minsweeper Text\n"
                << "Usage: minesweeper_text <int n_rows> <int n_cols> <int n_mines>\n";
      exit(EXIT_SUCCESS);
   }

   std::cout << "Using " << settingsTypeStr << " settings of "
             << n_rows  << " rows, "
             << n_cols  << " cols, "
             << n_mines << " mines.\n\n";

   return playGame(n_rows, n_cols, n_mines);
}

// File-scope function definitions. ////////////////////////////////////////////////////////////////

namespace
{

 using namespace minesweeper;

 /*
  *
  */
 int playGame(int n_rows, int n_cols, int n_mines)
 {
    using std::cout;
    using std::cin;
    using std::endl;

    mineField        M(n_rows, n_cols, n_mines);
    mineFieldProbMap P(&M);

    square s;
 
    bool gameOver, exitGame = false, semiAutomate = true;

    char choice;

    while (!exitGame)
    {
       M.reset();
       P.reset();
 
       gameOver = false;
       while (!gameOver)
       {
          M.printMap();
 
          cout << "Explore which square? (row col) ";
          cin >> s.row >> s.col;
 
          if (M.squareInsideMap(s) && !M.explore(s))
          {
             cout                                   << endl
                  << "That square was mined dummy!" << endl
                                                    << endl;
             gameOver = true;
          }
          else
          {
             if (semiAutomate)
             {
                cout << endl;
                M.printMap();
                cout << endl;
 
                autoExplore(M, P);
                cout << endl;
             }
 
             if (M.gameWon())
             {
                cout << "Congratulations."                                   << endl
                     << "You have successfully mapped the entire minefield." << endl
                                                                             << endl;
                gameOver = true;
             }
          }
       }
 
       cout << "Play again? (y/n) ";
       cin  >> choice;
       cout << endl;
 
       if (choice == 'n' || choice == 'N')
       {
          exitGame = true;
       }
    }
 
    return EXIT_SUCCESS;
 }

 /*
  *
  */
 bool autoExplore(mineField &M, mineFieldProbMap &P)
 {
    using std::cout;
    using std::endl;

    square s;
    bool mapChanged = false;

    while (P.update())
    {
       mapChanged = true;

       cout << "Exploring confirmed clear squares..." << endl;

       for (s.row = 0; s.row < M.getHeight(); ++s.row)
       {
          for (s.col = 0; s.col < M.getWidth(); ++s.col)
          {
             if (P.squareClear(s) and not M.squareExplored(s))
             {
                M.explore(s);
             }
             else
             {
                if (P.squareMined(s) and not M.squareFlagged(s))
                {
                   M.flagSquare(s);
                }
             }
          }
       }

       cout << endl;
       M.printMap();
       cout << "Press enter to continue.";
       std::cin.get();
       cout << endl;

       if (M.gameWon())
       {
          break;
       }
    }

    return mapChanged;
 }

} // End anonymous namespace.

/*******************************************END*OF*FILE********************************************/
