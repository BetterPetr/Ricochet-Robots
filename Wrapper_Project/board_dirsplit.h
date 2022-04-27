#ifndef __board_h_
#define __board_h_

#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <thread>
#include <map>

// ==================================================================
// ==================================================================
// A tiny all-public helper class to record a 2D board position

class Position {
public:
  // the coordinate (-1,-1) is invalid/unitialized
  Position(int r=-1,int c=-1) : row(r),col(c) {}
  int row,col;
};

// convenient functions to print and equality compare Positions
std::ostream& operator<<(std::ostream &ostr, const Position &p);
bool operator==(const Position &a, const Position &b);
bool operator!=(const Position &a, const Position &b);


// ==================================================================
// ==================================================================
// A tiny all-public helper class to store the position & label of a
// robot.  A robot label must be a capital letter.

class Robot {
public:
  Robot(Position p, char w) : pos(p), which(w) { 
    assert (isalpha(w) && isupper(w)); }
  Robot(const Robot &old) : pos(old.pos), which(old.which){};
  Position pos;
  char which;
};


// ==================================================================
// ==================================================================
// A tiny all-public helper class to store the position & label of a
// goal.  A goal label must be an upper case letter or '?' indicating
// that any robot can occupy this goal to solve the puzzle.

class Goal {
public:
  Goal(Position p, char w) : pos(p), which(w) { 
    assert (w == '?' || (isalpha(w) && isupper(w))); 
  }
  Goal(const Goal &old) : pos(old.pos), which(old.which) {}
  Position pos;
  char which;
};

//Helper class, keeps the new position moved to and direction of movement
class Move{
public:
  Move(int i, std::string dir) : robot_index(i), direction(dir){}
  Move(const Move &old) : robot_index(old.robot_index), direction(old.direction){}
  int robot_index; //index in list
  std::string direction;
};
//Helper class, keeps solutions of paths (vectors of moves)
class Solution{
public:
  Solution();
  Solution(std::vector<Move> p) : path(p) {}
  Solution(const Solution &old) : path(old.path) {}
  std::string getKey();
  std::string getKey2();
  std::vector<Move> path;
  // added by Allan
  // bool operator==(const Solution& lhs){
  //   return lhs.path == path;
  // };

};

// Global Singleton All Solutions class
class Sol_Singleton{
public:
  Sol_Singleton(){ init = true; };
  ~Sol_Singleton(){ init = false; };
  static Sol_Singleton* getInstance();
  void addState(std::string key, Solution sol);
  std::map<std::string, Solution>& getSolutions(){ return all_solutions; };
  // new functions, added by Allan
  bool checkState(std::string key, Solution new_sol);
  bool checkState2(std::string key);
  std::map<std::string, Solution> all_states;
  void addStateToAllStates(std::string key, Solution sol);
  void addBoardToAllBoards(std::string key);

private:
  static Sol_Singleton* instance;
  static bool init;

  std::map<std::string, Solution> all_solutions;
  std::map<std::string , bool> all_boards;
  
};


// ==================================================================
// ==================================================================
// A class to hold information about the puzzle board including the
// dimensions, the location of all walls, the current position of all
// robots, the goal location, and the robot (if specified) that must
// reach that position

class Board_Threads
{
public:
  
  class Board {
  public:
    // CONSTRUCTOR
    Board();
    ~Board();
    Board(int num_rows, int num_cols);
    Board(const Board& old_board);
    void operator=(const Board& old_board);

    // ACCESSORS related the board geometry
    int getRows() const { return rows; }
    int getCols() const { return cols; }
    bool getHorizontalWall(double r, int c) const;
    bool getVerticalWall(int r, double c) const;

    // ACCESSORS related to the robots and their current positions
    unsigned int numRobots() const { return robots.size(); }
    char getRobot(int i) const { assert (i >= 0 && i < (int)numRobots()); return robots[i].which; }
    std::vector<Robot>& getRobots(){ return robots; };
    Position getRobotPosition(int i) const { assert (i >= 0 && i < (int)numRobots()); return robots[i].pos; }
    
    // ACCESSORS related to the overall puzzle goals
    unsigned int numGoals() const { return goals.size(); }
    // (if any robot is allowed to reach the goal, this value is '?')
    char getGoalRobot(int i) const { assert (i >= 0 && i < (int)numGoals()); return goals[i].which; }
    Position getGoalPosition(int i) const { assert (i >= 0 && i < (int)numGoals()); return goals[i].pos; }
    
    std::string getKey3();
    
    // MODIFIERS related to board geometry
    void addHorizontalWall(double r, int c);
    void addVerticalWall(int r, double c);

    // MODIFIERS related to robot position
    // initial placement of a new robot
    void placeRobot(const Position &p, char a);
    // move an existing robot
    bool moveRobot(int i, const std::string &direction);

    // MODIFIER related to puzzle goals
    void addGoal(const std::string &goal_robot, const Position &p);

    // PRINT
    void print();

    // CHECK LAST IN PATH
    bool checkLast(std::vector<Move>& path, const std::string& curr_dir, int bot);

    //CHECK GOALS
    bool checkGoals(); //goes through goals and sees if they are all satisfied

    // ONE-SOLUTION
    void one_sol_helper(int i, int move, int* max_moves, std::vector<Move>& path);
    void one_sol_helper_other(int i, int move, int* max_moves,
      std::vector<Move>& path);

    void one_sol_north(int i, int move, int* max_moves, std::vector<Move>& path); 
    void one_sol_east(int i, int move, int* max_moves, std::vector<Move>& path);
    void one_sol_west(int i, int move, int* max_moves, std::vector<Move>& path);
    void one_sol_south(int i, int move, int* max_moves, std::vector<Move>& path);

    void one_sol_other_north(int i, int move, int* max_moves, std::vector<Move>& path); 
    void one_sol_other_east(int i, int move, int* max_moves, std::vector<Move>& path);
    void one_sol_other_west(int i, int move, int* max_moves, std::vector<Move>& path);
    void one_sol_other_south(int i, int move, int* max_moves, std::vector<Move>& path);

  private:

    // private helper functions
    char getspot(const Position &p) const;
    void setspot(const Position &p, char a);
    char isGoal(const Position &p) const;

    // REPRESENTATION

    // the board geometry
    int rows;
    int cols;
    std::vector<std::vector<char> > board;
    std::vector<std::vector<bool> > vertical_walls;
    std::vector<std::vector<bool> > horizontal_walls;

    // the names and current positions of the robots
    std::vector<Robot> robots;

    // the goal positions & the robots that must reach them
    std::vector<Goal> goals;

    // vector of solutions
    std::vector<Solution> solutions;

    // states

  };
  
  Board_Threads();
  ~Board_Threads();

  void make_copies();

  bool usage(const std::string &executable_name);

  Board load(const std::string &executable, const std::string &filename);

  void set_orig_board(const std::string &executable, const std::string &filename);
  Board& getOrig();

  void one_solution(int max_movs, bool moves_given);

  // ALL-SOLUTIONS
  void all_solutions(int max_movs, bool moves_given); //uses one_solution functions

private:
  Board orig_board;
  std::vector<Board*> thread_boards;
  int num_robots;

  std::vector<std::thread> threads;
};

#endif