#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cassert>

#include "board_dirsplit.h"
#include <ctime>          // Timekeeping
#include <ratio>          // Timekeeping
#include <chrono>         // Timekeeping
using std::chrono::high_resolution_clock;
using std::chrono::time_point;
using std::chrono::duration;

duration<double, std::milli> delta(std::string msg);

using std::chrono::high_resolution_clock;
using std::chrono::time_point;
using std::chrono::system_clock;
using std::chrono::duration;
high_resolution_clock::time_point start_time = high_resolution_clock::now();
int time_first = 1;

duration<double, std::milli> delta(std::string msg = ""){
  duration<double, std::milli> del;
  int silent = 0;
  if(msg == ""){silent = 1;}
  if(time_first){
    start_time = high_resolution_clock::now();
    time_first = 0;
    del = (high_resolution_clock::now() - high_resolution_clock::now()) / 1000;
  }
  else{
    del = (high_resolution_clock::now() - start_time) / 1000;
    if(!silent){std::cout << "  " << msg << ": " << del.count() << " s" << std::endl;}
    start_time = high_resolution_clock::now();
  }
  return del;
}

// ==================================================================================
// ==================================================================================

// ==================================================================================
// ==================================================================================

// This function is called if there was an error with the command line arguments
bool usage(const std::string &executable_name) {
  std::cerr << "Usage: " << executable_name << " <puzzle_file>" << std::endl;
  std::cerr << "       " << executable_name << " <puzzle_file> -max_moves <#>" << std::endl;
  std::cerr << "       " << executable_name << " <puzzle_file> -all_solutions" << std::endl;
  std::cerr << "       " << executable_name << " <puzzle_file> -max_moves <#> -all_solutions" << std::endl;
  exit(0);
}

// ==================================================================================
// ==================================================================================

int main(int argc, char* argv[]) {

  // There must be at least one command line argument, the input puzzle file
  if (argc < 2) {
    usage(argv[0]);
  }

  // By default, the maximum number of moves is unlimited
  int max_moves = -1;

  // By default, output one solution using the minimum number of moves
  bool all_solutions = false;

  // Read in the other command line arguments
  for (int arg = 2; arg < argc; arg++) {
    if (argv[arg] == std::string("-all_solutions")) {
      // find all solutions to the puzzle that use the fewest number of moves
      all_solutions = true;
    } else if (argv[arg] == std::string("-max_moves")) {
      // the next command line arg is an integer, a cap on the  # of moves
      arg++;
      assert (arg < argc);
      max_moves = atoi(argv[arg]);
      assert (max_moves > 0);
    } 
    else {
      std::cout << "unknown command line argument" << argv[arg] << std::endl;
      usage(argv[0]);
    }
  }

  // Load the puzzle board from the input file
  Board_Threads boards_obj;
  boards_obj.set_orig_board(argv[0],argv[1]);

  Board_Threads::Board& original = boards_obj.getOrig();
/*
  if(!all_solutions && max_moves != -1){
    //Prints out one solution (the shortest, or one of the shortest in the case of ties)

    //print initial board
    // board.print();

    //Check to make sure there are robots and goals
    if(original.numRobots() == 0 || original.numGoals() == 0){
      std::cout << "no solutions with " << max_moves << " or fewer moves" << std::endl;
    }
    else{
      //Call the one_solution recursive driver function
      boards_obj.one_solution(max_moves, true);
    }
  }

  else if(!all_solutions && max_moves == -1){
    //Prints out one solution (the shortest, or one of the shortest in the case of ties)

    //print initial board
    // board.print();

    //Algorithm for finding the max_moves
    max_moves = original.getCols() + original.getRows();
    if(max_moves > 11){ //caps max_moves so it doesn't take too long
      max_moves = 11;
    }

    //Check to make sure there are robots and goals
    if(original.numRobots() == 0 || original.numGoals() == 0){
      std::cout << "no solutions" << std::endl;
    }
    else{
      //Call the one_solution recursive driver function
      boards_obj.one_solution(max_moves, false);
    }
  }
*/
  if(all_solutions && max_moves != -1){
    delta();
    // prints out total number of solutions and then all of the shortest solutions
    // intermediate solutions are not printed out

    //print initial board
    // board.print();

    //Check to make sure there are robots and goals
    if(original.numRobots() == 0 || original.numGoals() == 0){
      std::cout << "no solutions with " << max_moves << " or fewer moves" << std::endl;
    }
    else{
      //Call the all_solutions recursive driver function
      boards_obj.all_solutions(max_moves, true); //max_moves given
    
    delta("time to solve all_solutions with finite moves");
    }
  }

  else if(all_solutions && max_moves == -1){
    delta();
    // prints out total number of solutions and then all of the shortest solutions
    // intermediate solutions are not printed out
    // This finds the shortest solutions without a max_moves argument given

    //print initial board
    boards_obj.getOrig().print();

    //Algorithm to find maximum number of moves possible
    max_moves = original.getCols() + original.getRows();
    if(max_moves > 12){ //caps max_moves so it doesn't take too long
      max_moves = 12;
    }

    //Check to make sure there are robots and goals
    if(original.numRobots() == 0 || original.numGoals() == 0){
      std::cout << "no solutions" << std::endl;
    }
    else{
      //Call the all_solutions recursive driver function
      boards_obj.all_solutions(max_moves, false); //max_moves not given
    }
    delta("time to solve all_solutions with unlimited moves!");
  }

}

// ================================================================
// ================================================================
