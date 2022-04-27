#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cassert>
#include <ctime>          // Timekeeping
#include <ratio>          // Timekeeping
#include <chrono>         // Timekeeping
#include "board.h"


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

// load a Ricochet Robots puzzle from the input file
Board load(const std::string &executable, const std::string &filename) {

  // open the file for reading
  std::ifstream istr (filename.c_str());
  if (!istr) {
    std::cerr << "ERROR: could not open " << filename << " for reading" << std::endl;
    usage(executable);
  }

  // read in the board dimensions and create an empty board
  // (all outer edges are automatically set to be walls
  int rows,cols;
  istr >> rows >> cols;
  assert (rows > 0 && cols > 0);
  Board answer(rows,cols);

  // read in the other characteristics of the puzzle board
  std::string token;
  while (istr >> token) {
    if (token == "robot") {
      char a;
      int r,c;
      istr >> a >> r >> c;
      answer.placeRobot(Position(r,c),a);
    } else if (token == "vertical_wall") {
      int i;
      double j;
      istr >> i >> j;
      answer.addVerticalWall(i,j);
    } else if (token == "horizontal_wall") {
      double i;
      int j;
      istr >> i >> j;
      answer.addHorizontalWall(i,j);
    } else if (token == "goal") {
      std::string which_robot;
      int r,c;
      istr >> which_robot >> r >> c;
      answer.addGoal(which_robot,Position(r,c));
    } else {
      std::cerr << "ERROR: unknown token in the input file " << token << std::endl;
      exit(0);
    }
  }

  // return the initialized board
  return answer;
}

// ==================================================================================
// ==================================================================================

int main(int argc, char* argv[]) {
  // starting timer
  delta();

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
  Board board = load(argv[0],argv[1]);
  delta("time to read in arguments and load board");

  if(!all_solutions && max_moves != -1){
    //Prints out one solution (the shortest, or one of the shortest in the case of ties)

    //print initial board
    board.print();

    //Check to make sure there are robots and goals
    if(board.numRobots() == 0 || board.numGoals() == 0){
      std::cout << "no solutions with " << max_moves << " or fewer moves" << std::endl;
    }
    else{
      //Call the one_solution recursive driver function
      board.one_solution(max_moves, true);
    }
  }

  else if(!all_solutions && max_moves == -1){
    //Prints out one solution (the shortest, or one of the shortest in the case of ties)

    //print initial board
    board.print();

    //Algorithm for finding the max_moves
    max_moves = board.getCols() + board.getRows();
    if(max_moves > 11){ //caps max_moves so it doesn't take too long
      max_moves = 11;
    }

    //Check to make sure there are robots and goals
    if(board.numRobots() == 0 || board.numGoals() == 0){
      std::cout << "no solutions" << std::endl;
    }
    else{
      //Call the one_solution recursive driver function
      board.one_solution(max_moves, false);
    }
  }
  
  else if(all_solutions && max_moves != -1){
    // prints out total number of solutions and then all of the shortest solutions
    // intermediate solutions are not printed out

    //print initial board
    board.print();

    //Check to make sure there are robots and goals
    if(board.numRobots() == 0 || board.numGoals() == 0){
      std::cout << "no solutions with " << max_moves << " or fewer moves" << std::endl;
    }
    else{
      //Call the all_solutions recursive driver function
      board.all_solutions(max_moves, true); //max_moves given
    }
  }

  else if(all_solutions && max_moves == -1){
    delta();
    // prints out total number of solutions and then all of the shortest solutions
    // intermediate solutions are not printed out
    // This finds the shortest solutions without a max_moves argument given

    //print initial board
    board.print();

    //Algorithm to find maximum number of moves possible
    max_moves = board.getCols() + board.getRows();
    if(max_moves > 12){ //caps max_moves so it doesn't take too long
      max_moves = 12;
    }

    //Check to make sure there are robots and goals
    if(board.numRobots() == 0 || board.numGoals() == 0){
      std::cout << "no solutions" << std::endl;
    }
    else{
      //Call the all_solutions recursive driver function
      board.all_solutions(max_moves, false); //max_moves not given
    }
    delta("time to call all solutions and max moves = -1");
  }

}

// ================================================================
// ================================================================
