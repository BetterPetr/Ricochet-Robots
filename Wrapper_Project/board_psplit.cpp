#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "board_psplit.h"

// ==================================================================
// ==================================================================
// Implementation of the Position class


// allows a position to be output to a stream
std::ostream& operator<<(std::ostream &ostr, const Position &p) {
  ostr << '(' << p.row << "," << p.col << ')';
  return ostr;
}


// equality and inequality comparision of positions
bool operator==(const Position &a, const Position &b) {
  return (a.row == b.row && a.col == b.col);
}
bool operator!=(const Position &a, const Position &b) {
  return !(a==b);
}

Sol_Singleton* Sol_Singleton::instance = NULL;
bool Sol_Singleton::init = false;

Sol_Singleton* Sol_Singleton::getInstance(){
  if(instance == NULL){
    instance = new Sol_Singleton();
  }
  return instance;
}

void Sol_Singleton::addState(std::string key, Solution sol){
  all_solutions.insert(std::make_pair(key, sol));
}

std::string Solution::getKey(){
  std::string key = "";
  for(Move mv : path){
    key += std::to_string(mv.robot_index);
    key += mv.direction;
  }
  return key;
}

// ===================
// CONSTRUCTOR
// ===================

Board_Threads::Board::Board(){
  rows = 0;
  cols = 0;
}

Board_Threads::Board::~Board(){}

Board_Threads::Board::Board(int r, int c) { 
  // initialize the dimensions
  rows = r; 
  cols = c; 

  // allocate space for the contents of each grid cell
  board = std::vector<std::vector<char> >(rows,std::vector<char>(cols,' '));

  // allocate space for booleans indicating the presense of each wall
  // by default, these are false == no wall
  // (note that there must be an extra column of vertical walls
  //  and an extra row of horizontal walls)
  vertical_walls = std::vector<std::vector<bool> >(rows,std::vector<bool>(cols+1,false));
  horizontal_walls = std::vector<std::vector<bool> >(rows+1,std::vector<bool>(cols,false));

  // initialize the outermost edges of the grid to have walls
  for (int i = 0; i < rows; i++) {
    vertical_walls[i][0] = vertical_walls[i][cols] = true;
  }
  for (int i = 0; i < cols; i++) {
    horizontal_walls[0][i] = horizontal_walls[rows][i] = true;
  }
}


Board_Threads::Board_Threads(){}

Board_Threads::~Board_Threads(){}

Board_Threads::Board::Board(const Board& old_board){
  rows = old_board.rows;
  cols = old_board.cols;
  board = old_board.board;
  vertical_walls = old_board.vertical_walls;
  horizontal_walls = old_board.horizontal_walls;
  robots = old_board.robots;
  goals = old_board.goals;
  solutions = old_board.solutions;
}

void Board_Threads::Board::operator=(const Board_Threads::Board& old_board){
  rows = old_board.rows;
  cols = old_board.cols;
  board = old_board.board;
  vertical_walls = old_board.vertical_walls;
  horizontal_walls = old_board.horizontal_walls;
  robots = old_board.robots;
  goals = old_board.goals;
  solutions = old_board.solutions;
}


// This function is called if there was an error with the command line arguments
bool Board_Threads::usage(const std::string &executable_name) {
  std::cerr << "Usage: " << executable_name << " <puzzle_file>" << std::endl;
  std::cerr << "       " << executable_name << " <puzzle_file> -max_moves <#>" << std::endl;
  std::cerr << "       " << executable_name << " <puzzle_file> -all_solutions" << std::endl;
  std::cerr << "       " << executable_name << " <puzzle_file> -max_moves <#> -all_solutions" << std::endl;
  exit(0);
}

void Board_Threads::set_orig_board(const std::string &executable, const std::string &filename){
  orig_board = load(executable, filename);
  num_robots = orig_board.getRobots().size();
}

Board_Threads::Board& Board_Threads::getOrig(){
  return orig_board;
}

void Board_Threads::make_copies(){
  int num_threads = num_robots * 2;
  for(int i=0; i < num_threads; ++i){
    Board* new_instance = new Board(orig_board);
    thread_boards.push_back(new_instance);
  }
}

// load a Ricochet Robots puzzle from the input file
Board_Threads::Board Board_Threads::load(const std::string &executable, const std::string &filename) {

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
  Board_Threads::Board answer(rows,cols);

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


// ===================
// ACCESSORS related to board geometry
// ===================

// Query the existance of a horizontal wall
bool Board_Threads::Board::getHorizontalWall(double r, int c) const {
  // verify that the requested wall is valid
  // the row coordinate must be a half unit
  assert (fabs((r - floor(r))-0.5) < 0.005);
  assert (r >= 0.4 && r <= rows+0.6);
  assert (c >= 1 && c <= cols);
  // subtract one and round down because the corner is (0,0) not (1,1)
  return horizontal_walls[floor(r)][c-1];
}

// Query the existance of a vertical wall
bool Board_Threads::Board::getVerticalWall(int r, double c) const {
  // verify that the requested wall is valid
  // the column coordinate must be a half unit
  assert (fabs((c - floor(c))-0.5) < 0.005);
  assert (r >= 1 && r <= rows);
  assert (c >= 0.4 && c <= cols+0.6);
  // subtract one and round down because the corner is (0,0) not (1,1)
  return vertical_walls[r-1][floor(c)];
}


// ===================
// MODIFIERS related to board geometry
// ===================

// Add an interior horizontal wall
void Board_Threads::Board::addHorizontalWall(double r, int c) {
  // verify that the requested wall is valid
  // the row coordinate must be a half unit
  assert (fabs((r - floor(r))-0.5) < 0.005);
  assert (r >= 0 && r <= rows);
  assert (c >= 1 && c <= cols);
  // verify that the wall does not already exist
  assert (horizontal_walls[floor(r)][c-1] == false);
  // subtract one and round down because the corner is (0,0) not (1,1)
  horizontal_walls[floor(r)][c-1] = true;
}

// Add an interior vertical wall
void Board_Threads::Board::addVerticalWall(int r, double c) {
  // verify that the requested wall is valid
  // the column coordinate must be a half unit
  assert (fabs((c - floor(c))-0.5) < 0.005);
  assert (r >= 1 && r <= rows);
  assert (c >= 0 && c <= cols);
  // verify that the wall does not already exist
  assert (vertical_walls[r-1][floor(c)] == false);
  // subtract one and round down because the corner is (0,0) not (1,1)
  vertical_walls[r-1][floor(c)] = true;
}


// ===================
// PRIVATE HELPER FUNCTIONS related to board geometry
// ===================

char Board_Threads::Board::getspot(const Position &p) const {
  // verify that the requested coordinate is valid
  assert (p.row >= 1 && p.row <= rows);
  assert (p.col >= 1 && p.col <= cols);
  // subtract one from each coordinate because the corner is (0,0) not (1,1)
  return board[p.row-1][p.col-1];
}


void Board_Threads::Board::setspot(const Position &p, char a) {
  // verify that the requested coordinate is valid
  assert (p.row >= 1 && p.row <=  rows);
  assert (p.col >= 1 && p.col <= cols);
  // subtract one from each coordinate because the corner is (0,0) not (1,1)
  board[p.row-1][p.col-1] = a;
}

char Board_Threads::Board::isGoal(const Position &p) const {
  // verify that the requested coordinate is valid
  assert (p.row >= 1 && p.row <= rows);
  assert (p.col >= 1 && p.col <= cols);
  // loop over the goals, see if any match this spot
  for (unsigned int i = 0; i < goals.size(); i++) {
    if (p == goals[i].pos) { return goals[i].which; }
  }
  // else return space indicating that no goal is at this location
  return ' ';
}


// ===================
// MODIFIERS related to robots
// ===================

// for initial placement of a new robot
void Board_Threads::Board::placeRobot(const Position &p, char a) {

  // check that input data is reasonable
  assert (p.row >= 1 && p.row <= rows);
  assert (p.col >= 1 && p.col <= cols);
  assert (getspot(p) == ' ');
  
  // robots must be represented by a capital letter
  assert (isalpha(a) && isupper(a));

  // make sure we don't already have a robot with the same name
  for (unsigned int i = 0; i < robots.size(); i++) {
    assert (robots[i].which != a);
  }

  // add the robot and its position to the vector of robots
  robots.push_back(Robot(p,a));

  // mark the robot on the board
  setspot(p,a);
}

// ==================================================================================
// CHECK GOALS
// ==================================================================================

//Checks if all goals on the board are satisfied or not
bool Board_Threads::Board::checkGoals(){
  for(unsigned int i = 0; i < goals.size(); ++i){
    //char the the position on the 2D board of char's
    char pos = board[(goals[i].pos).row-1][(goals[i].pos).col-1];
    //If a goal is not satisfied, return false
    if(pos == ' '){ //If pos is an open space, return false
      return false;
    }
    else if(goals[i].which == '?'){ //pos != ' ' && it's an "any" goal
      continue; //means it is a satisfied goal
    }
    else if(pos != goals[i].which){ //if pos != the goal type, return false
      return false;
    }
  }
  //If all goals are satisfied, return true
  return true;
}

// ==================================================================================
// CHECK LAST IN PATH
// ==================================================================================

bool Board_Threads::Board::checkLast(std::vector<Move>& path, const std::string& curr_dir, int bot){
  //Purpose: if the last move of a robot was "north", going "south" next is pointless
  if(path.size() == 0){ //if path is empty, return true
    return true;
  }
  if(bot != path.back().robot_index){ //if the last robot to move was a diff robot
    return true; //return true
  }
  //if the next move is north, the last move can be anything but south
  else if(curr_dir == "north"){
    if(path.back().direction != "south"){
      return true;
    }
    return false;
  }
  //Same Idea for all the rest
  else if(curr_dir == "south"){
    if(path.back().direction != "north"){
      return true;
    }
    return false;
  }
  else if(curr_dir == "west"){
    if(path.back().direction != "east"){
      return true;
    }
    return false;
  }
  else if(curr_dir == "east"){
    if(path.back().direction != "west"){
      return true;
    }
    return false;
  }
  else{ //Should never happen (just to prevent warnings in compilation)
    std::cerr << "Not a correct direction" << std::endl;
    return false;
  }
}


// ==================================================================================
// ONE SOLUTION
// ==================================================================================

void Board_Threads::Board::one_sol_helper(int i, int move, int* max_moves, std::vector<Move>& path){
  Robot* curr_robot;
  curr_robot = &robots[i];
  Position orig_pos = curr_robot->pos;
  //If move is greater than max_moves, end recursion step
  if(move > *max_moves){
    return;
  }

  if(checkLast(path, "north", i)){ //Checks the last move to prevent pointless moves
    //moveRobot fn trys to move robot, returns true if it does move
    if(moveRobot(i, "north")){
      //Add move to solution
      Move new_move(i, "north");
      path.push_back(new_move);
      //if all goals satisfied, add solution to solutions and return
      if(checkGoals()){
        Solution new_sol(path);
        Sol_Singleton::getInstance()->addState(new_sol.getKey(), new_sol);
        *max_moves = move; //resets max_moves
        path.pop_back();//deletes the last move before moving on and trying the next one
        setspot(curr_robot->pos, ' '); //resets the board to how it was
        curr_robot->pos = orig_pos;
        setspot(curr_robot->pos, curr_robot->which);
        return;
      }
      one_sol_helper(i, move+1, max_moves, path); //recursive call (increment move)

      path.pop_back();//deletes the last move before moving on and trying the next one
      setspot(curr_robot->pos, ' '); //resets the board to how it was
      curr_robot->pos = orig_pos;
      setspot(curr_robot->pos, curr_robot->which);
    }
  }
  if(checkLast(path, "west", i)){ //Checks the last move to prevent pointless moves
    if(moveRobot(i, "west")){
      //Add move to solution
      Move new_move(i, "west");
      path.push_back(new_move);
      //if all goals satisfied, add solution to solutions and return
      if(checkGoals()){
        Solution new_sol(path);
        Sol_Singleton::getInstance()->addState(new_sol.getKey(), new_sol);
        *max_moves = move; //resets max_moves
        path.pop_back();//deletes the last move before moving on and trying the next one
        setspot(curr_robot->pos, ' '); //resets the board to how it was
        curr_robot->pos = orig_pos;
        setspot(curr_robot->pos, curr_robot->which);
        return;
      }
      one_sol_helper(i, move+1, max_moves, path); //recursive call (increment move)

      path.pop_back();//deletes the last move before moving on and trying the next one
      setspot(curr_robot->pos, ' '); //resets the board to how it was
      curr_robot->pos = orig_pos;
      setspot(curr_robot->pos, curr_robot->which);
    }
  }
  if(checkLast(path, "south", i)){ //Checks the last move to prevent pointless moves
    if(moveRobot(i, "south")){
      //Add move to solution
      Move new_move(i, "south");
      path.push_back(new_move);
      //if all goals satisfied, add solution to solutions and return
      if(checkGoals()){
        Solution new_sol(path);
        Sol_Singleton::getInstance()->addState(new_sol.getKey(), new_sol);
        *max_moves = move; //resets max_moves
        path.pop_back();//deletes the last move before moving on and trying the next one
        setspot(curr_robot->pos, ' '); //resets the board to how it was
        curr_robot->pos = orig_pos;
        setspot(curr_robot->pos, curr_robot->which);
        return;
      }
      one_sol_helper(i, move+1, max_moves, path); //recursive call (increment move)

      path.pop_back();//deletes the last move before moving on and trying the next one
      setspot(curr_robot->pos, ' '); //resets the board to how it was
      curr_robot->pos = orig_pos;
      setspot(curr_robot->pos, curr_robot->which);
    }
  }
  if(checkLast(path, "east", i)){ //Checks the last move to prevent pointless moves
    if(moveRobot(i, "east")){
      //Add move to solution
      Move new_move(i, "east");
      path.push_back(new_move);
      //if all goals satisfied, add solution to solutions and return
      if(checkGoals()){
        Solution new_sol(path);
        Sol_Singleton::getInstance()->addState(new_sol.getKey(), new_sol);
        *max_moves = move; //resets max_moves
        path.pop_back();//deletes the last move before moving on and trying the next one
        setspot(curr_robot->pos, ' '); //resets the board to how it was
        curr_robot->pos = orig_pos;
        setspot(curr_robot->pos, curr_robot->which);
        return;
      }
      one_sol_helper(i, move+1, max_moves, path); //recursive call (increment move)

      path.pop_back();//deletes the last move before moving on and trying the next one
      setspot(curr_robot->pos, ' '); //resets the board to how it was
      curr_robot->pos = orig_pos;
      setspot(curr_robot->pos, curr_robot->which);
    }
  }
}

void Board_Threads::Board::one_sol_helper_other(int i, int move, int* max_moves,
    std::vector<Move>& path){
  //If move is greater than max_moves, end recursion step
  if(move > *max_moves-1){
    return;
  }

  for(unsigned int l = 0; l < robots.size(); ++l){
    Robot* other_robot;
    other_robot = &robots[l];
    Position orig_pos = other_robot->pos;

    if(checkLast(path, "north", l)){ //Checks the last move to prevent pointless moves
      //moveRobot fn trys to move robot, returns true if it does move
      if(moveRobot(l, "north")){
        //Add move to solution
        Move new_move(l, "north");
        path.push_back(new_move);

        //if statement cuts out duplicate solutions + increases efficiency
        if(l != (unsigned int)i) one_sol_helper(i, move+1, max_moves, path);
        one_sol_helper_other(i, move+1, max_moves, path); //recursive call (move+1)

        path.pop_back(); //deletes last move
        setspot(other_robot->pos, ' '); //resets the board to how it was
        other_robot->pos = orig_pos;
        setspot(other_robot->pos, other_robot->which);
      }
    }
    if(checkLast(path, "west", l)){ //Checks the last move to prevent pointless moves
      if(moveRobot(l, "west")){
        //Add move to solution
        Move new_move(l, "west");
        path.push_back(new_move);
        
        //if statement cuts out duplicate solutions + increases efficiency
        if(l != (unsigned int)i) one_sol_helper(i, move+1, max_moves, path);
        one_sol_helper_other(i, move+1, max_moves, path); //recursive call (move+1)

        path.pop_back(); //deletes last move
        setspot(other_robot->pos, ' '); //resets the board to how it was
        other_robot->pos = orig_pos;
        setspot(other_robot->pos, other_robot->which);
      }
    }
    if(checkLast(path, "south", l)){ //Checks the last move to prevent pointless moves
      if(moveRobot(l, "south")){
        //Add move to solution
        Move new_move(l, "south");
        path.push_back(new_move);
        
        //if statement cuts out duplicate solutions + increases efficiency
        if(l != (unsigned int)i) one_sol_helper(i, move+1, max_moves, path);
        one_sol_helper_other(i, move+1, max_moves, path); //recursive call (move+1)

        path.pop_back(); //deletes last move
        setspot(other_robot->pos, ' '); //resets the board to how it was
        other_robot->pos = orig_pos;
        setspot(other_robot->pos, other_robot->which);
      }
    }
    if(checkLast(path, "east", l)){ //Checks the last move to prevent pointless moves
      if(moveRobot(l, "east")){
        //Add move to solution
        Move new_move(l, "east");
        path.push_back(new_move);
        
        //if statement cuts out duplicate solutions + increases efficiency
        if(l != (unsigned int)i) one_sol_helper(i, move+1, max_moves, path);
        one_sol_helper_other(i, move+1, max_moves, path); //recursive call (move+1)

        path.pop_back(); //deletes last move
        setspot(other_robot->pos, ' '); //resets the board to how it was
        other_robot->pos = orig_pos;
        setspot(other_robot->pos, other_robot->which);
      }
    }
  }
}

//For Sorting Solutions to find the smallest solutions
bool sortSolutions(const Solution& a, const Solution& b){
  int aSize = a.path.size(); //lengths of paths for a and b
  int bSize = b.path.size();
  if(a.path[0].robot_index == -1) aSize = 0; //special case 0 length
  if(b.path[0].robot_index == -1) bSize = 0; //special case 0 length
  return aSize < bSize;
}

// void Board_Threads::one_solution(int max_movs, bool moves_given){
//   std::vector<Move> path; //creates path of moves
//   int* max_moves = new int; //max_moves made into dynamic memory
//   *max_moves = max_movs;
//   int move;
//   for(unsigned int i = 0; i < num_robots; ++i){
//     if(checkGoals()){ //checks if the board is already solved
//       std::vector<Move> blank_path;
//       blank_path.push_back(Move(-1, "zero_length")); //special case solution
//       Solution new_sol(blank_path);
//       solutions.push_back(new_sol);
//       break;
//     }

//     if(*max_moves == 0) continue; //special case
//     move = 1;
//     one_sol_helper(i, move, max_moves, path); //case where only one robot moves
//     one_sol_helper_other(i, move, max_moves, path); //all robots can move
//   }
//   if(solutions.size() == 0 && moves_given){ //max_moves given
//     std::cout << "no solutions with " << *max_moves << " or fewer moves" << std::endl;
//   }
//   else if(solutions.size() == 0 && !moves_given){ //all_solutions case
//     std::cout << "no solutions" << std::endl;
//   }
//   else{
//     //Sort the solutions (shortest solutions first)
//     std::sort(solutions.begin(), solutions.end(), sortSolutions);

//     std::vector<Move> one_sol = solutions[0].path;
//     for(unsigned int q = 0; q < one_sol.size(); ++q){
//       int bot_index = one_sol[q].robot_index; //finds all of the necessary variables
//       std::string bot_dir = one_sol[q].direction;
//       char bot_char = robots[bot_index].which;

//       moveRobot(bot_index, bot_dir); //makes next move + prints out move
//       std::cout << "robot " << bot_char << " moves " << bot_dir << std::endl;

//       print(); //prints out the board
//     }
//     std::cout << "All goals are satisfied after " << one_sol.size() << 
//       " moves" << std::endl; //prints out the number of moves used
//   }
//   delete max_moves; //cleans up dynamic memory
// }


// ==================================================================================
// ALL SOLUTIONS
// ==================================================================================


void Board_Threads::all_solutions(int max_movs, bool moves_given){
  std::vector<Move> path; //creates path of moves
  int move;
  int* max_moves = new int; //makes max_moves dynamic memory
  *max_moves = max_movs;
  make_copies(); // Make the copies of the original board for the threads

  auto one_sol = [&](Board* board, int robot_ind, int move, int* max_moves, std::vector<Move> path){
    board->Board::one_sol_helper(robot_ind, move, max_moves, path);
  };

  auto one_sol_other = [&](Board* board, int robot_ind, int move, int* max_moves, std::vector<Move> path){
    board->Board::one_sol_helper_other(robot_ind, move, max_moves, path);
  };

  for(int i = 0; i < num_robots; ++i){
    // Does not check for already solved boards anymore (unlikely case anyway)
    if(*max_moves == 0) continue; //special case
    move = 1;
    
    threads.push_back(std::thread(one_sol, thread_boards[2*i], i, move, max_moves, path));
    threads.push_back(std::thread(one_sol_other, thread_boards[2*i+1], i, move, max_moves, path));
  }
  for(std::thread & th : threads){
    if(th.joinable())
      th.join();
  }

  std::map<std::string, Solution>& solutions_map = Sol_Singleton::getInstance()->getSolutions();
  if(solutions_map.size() == 0 && moves_given){ //no solutions found and max_moves given
    std::cout << "no solutions with " << *max_moves << " or fewer moves" << std::endl;
  }
  else if(solutions_map.size() == 0 && !moves_given){ //no solutions and max_moves not given
    std::cout << "no solutions" << std::endl;
  }
  else{
    std::map<std::string, Solution>::iterator itr = solutions_map.begin();
    std::vector<Solution> solutions;
    for(; itr != solutions_map.end(); ++itr){
      solutions.push_back(itr->second);
    }
    //Sort the solutions (shortest solutions first)
    std::sort(solutions.begin(), solutions.end(), sortSolutions);

    unsigned int index = 0;
    unsigned int prev_len = solutions[index].path.size(); //smallest length solution
    unsigned int num_solutions = 0;

    //Find the number of solutions (number of ties, if there are any)
    while(solutions[index].path.size() == prev_len){
      ++num_solutions; //increment number of solutions
      ++index; //increment index to the next solution
    }

    //Print out total number of solutions
    std::cout << num_solutions << " different " << prev_len <<
      " move solutions:" << std::endl << std::endl;

    index = 0; //resets index to beginning of solutions
    //Go through each of the shortest solutions and print them out
    std::vector<Robot>& robots = orig_board.getRobots();
    while(index < num_solutions){
      std::vector<Move> one_solu = solutions[index].path; //single solution
      for(unsigned int q = 0; q < one_solu.size(); ++q){
        int bot_index = one_solu[q].robot_index;
        std::string bot_dir = one_solu[q].direction;
        char bot_char = robots[bot_index].which;

        //Print out each move
        std::cout << "robot " << bot_char << " moves " << bot_dir << std::endl;
      }
      //Print out the number of moves to solve the board
      std::cout << "All goals are satisfied after " << one_solu.size() << 
        " moves" << std::endl << std::endl;

      //increment index to next solution
      ++index;
    }
  }
  delete max_moves; //cleans up dynamic memory
}

// ==================================================================================
// MOVE ROBOT FUNCTION
// ==================================================================================

bool Board_Threads::Board::moveRobot(int i, const std::string &direction) {
  Robot* curr_robot;
  curr_robot = &robots[i];
  Position old_position = curr_robot->pos; //for checking if the position changed
  int robot_row = (curr_robot->pos).row; //current row
  int robot_col = (curr_robot->pos).col; //current column
  bool stop = false;
  setspot(curr_robot->pos, ' '); //sets old spot to blank initially

  if(direction == "north"){
    while(!stop){
      if(robot_row <= 0){ //out of bounds
        stop = true;
        continue;
      }
      else if(getHorizontalWall(double(robot_row) - 0.5, robot_col)){//checking for wall
        stop = true;
        continue;
      }
      else if(getspot(Position(robot_row-1, robot_col)) != ' '){ //checking for robot
        stop = true;
        continue;
      }
      else{
        robot_row -= 1; //robot_row decrement
      }
    }
  }
  else if(direction == "south"){
    while(!stop){
      if(robot_row >= rows){ //out of bounds
        stop = true;
        continue;
      }
      else if(getHorizontalWall(double(robot_row) + 0.5, robot_col)){//checking for wall
        stop = true;
        continue;
      }
      else if(getspot(Position(robot_row+1, robot_col)) != ' '){//checking for robot
        stop = true;
        continue;
      }
      else{
        robot_row += 1; //robot_row increment
      }
    }
  }
  else if(direction == "east"){
    while(!stop){
      if(robot_col >= cols){ //out of bounds
        stop = true;
        continue;
      }
      else if(getVerticalWall(robot_row, double(robot_col) + 0.5)){//checking for wall
        stop = true;
        continue;
      }
      else if(getspot(Position(robot_row, robot_col + 1)) != ' '){//checking for robot
        stop = true;
        continue;
      }
      else{
        robot_col += 1; //robot_col increment
      }
    }
  }
  else if(direction == "west"){
    while(!stop){
      if(robot_col <= 0){ //out of bounds
        stop = true;
        continue;
      }
      else if(getVerticalWall(robot_row, double(robot_col) - 0.5)){//checking for wall
        stop = true;
        continue;
      }
      else if(getspot(Position(robot_row, robot_col - 1)) != ' '){//checking for robot
        stop = true;
        continue;
      }
      else{
        robot_col -= 1; //robot_col decrement
      }
    }
  }
  curr_robot->pos = Position(robot_row, robot_col); //sets new position
  setspot(curr_robot->pos, curr_robot->which); //updates the board
  //if the position did not change, just resets board to how it was
  if(curr_robot->pos == old_position){
    return false; //false if the robot did not move
  }
  else{
    return true; //true if the robot did move
  }
}


// ===================
// MODIFIER related to the puzzle goal
// ===================

void Board_Threads::Board::addGoal(const std::string &gr, const Position &p) {

  // check that input data is reasonable
  assert (p.row >= 1 && p.row <= rows);
  assert (p.col >= 1 && p.col <= cols);

  char goal_robot;
  if (gr == "any") {
    goal_robot = '?';
  } else {
    assert (gr.size() == 1);
    goal_robot = gr[0];
    assert (isalpha(goal_robot) && isupper(goal_robot));
  }

  // verify that a robot of this name exists for this puzzle
  if (goal_robot != '?') {
    int robot_exists = false;
    for (unsigned int i = 0; i < robots.size(); i++) {
      if (getRobot(i) == goal_robot) 
        robot_exists = true;
    }
    assert (robot_exists);
  }
  
  // make sure we don't already have a robot at that location
  assert (isGoal(p) == ' ');

  // add this goal label and position to the vector of goals
  goals.push_back(Goal(p,goal_robot));
}


// ==================================================================
// PRINT THE BOARD
// ==================================================================

void Board_Threads::Board::print() {

  // print the column headings
  std::cout << " ";
  for (int j = 1; j <= cols; j++) {
    std::cout << std::setw(5) << j;
  }
  std::cout << "\n";
  
  // for each row
  for (int i = 0; i <= rows; i++) {

    // don't print row 0 (it doesnt exist, the first real row is row 1)
    if (i > 0) {
      
      // Note that each grid rows is printed as 3 rows of text, plus
      // the separator.  The first and third rows are blank except for
      // vertical walls.  The middle row has the row heading, the
      // robot positions, and the goals.  Robots are always uppercase,
      // goals are always lowercase (or '?' for any).
      std::string first = "  ";
      std::string middle;
      for (int j = 0; j <= cols; j++) {

        if (j > 0) { 
          // determine if a robot is current located in this cell
          // and/or if this is the goal
          Position p(i,j);
          char c = getspot(p);
          char g = isGoal(p);
          if (g != '?') g = tolower(g);
          first += "    ";
          middle += " "; 
          middle += c; 
          middle += g; 
          middle += " ";
        }

        // the vertical walls
        if (getVerticalWall(i,j+0.5)) {
          first += "|";
          middle += "|";
        } else {
          first += " ";
          middle += " ";
        }
      }

      // output the three rows
      std::cout << first << std::endl;
      std::cout << std::setw(2) << i << middle << std::endl;
      std::cout << first << std::endl;
    }

    // print the horizontal walls between rows
    std::cout << "  +";
    for (double j = 1; j <= cols; j++) {
      (getHorizontalWall(i+0.5,j)) ? std::cout << "----" : std::cout << "    ";
      std::cout << "+";
    }
    std::cout << "\n";
  }
}

// ==================================================================
// ==================================================================


