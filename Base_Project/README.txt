HOMEWORK 6: RICOCHET ROBOTS RECURSION


NAME:  Thomas Petr


COLLABORATORS AND OTHER RESOURCES:
List the names of everyone you talked to about this assignment
(classmates, TAs, ALAC tutors, upperclassmen, students/instructor via
LMS, etc.), and all of the resources (books, online reference
material, etc.) you consulted in completing this assignment.

COLLABORATORS:
My Dad (he just helped me conceptualize what I had to do with recursion)

RESOURCES:
cplusplus.com, geeksforgeeks.org
I just used these to help understand some basic things

Data Structures Lecture Notes
Mainly just lecture 12, some basic recursion help

Remember: Your implementation for this assignment must be done on your
own, as described in "Academic Integrity for Homework" handout.


ESTIMATE OF # OF HOURS SPENT ON THIS ASSIGNMENT:  45


ANALYSIS OF PERFORMANCE OF YOUR ALGORITHM:
(order notation & concise paragraph, < 200 words)

i & j = dimensions of the board
    r = number of robots on the board
    g = number of goals
    w = number of interior walls
    m = maximum total number of moves allowed

O( 4^m * m^r )
REASONING:
First Part - For every robot, the robot can move 4 different directions each move
(4^m). This does not take into account the fact that any robot can move however.
To take into account for that I added the ... * m^r, which is saying that for any
one of the moves up to the maximum move can be any one of the robots on the board.
It is obviously not an efficient solution.
Also this analysis is true for both one_solution and all_solutions because both do
the same thing (one solution just takes the first shortest one and prints it out).
It likely will not take this long though because my code works so that if it finds
a solution it re-sets the maximum amount of moves to the amount of moves for that
solution.
Obviously a bigger board means the moveRobot function takes longer as well, but
I don't think that it is necessary to put that in the O notation.


SUMMARY OF PERFORMANCE OF YOUR PROGRAM ON THE PROVIDED PUZZLES:
Correctness & approximate wall clock running time for various command
line arguments.

CORRECTNESS:
My program can find all of the shortest solutions to a given board, the only problems
are that (1) my algorithm for finding the max moves when not given one is not so much
of an algorithm as it is an arbitrary number and (2) it's slow. I really just did that
to put a cap on how long the program can take at a maximum. 

APPROXIMATE TIMES:
All were done using -all_solutions

puzzle1.txt
real    0m0.047s
user    0m0.031s
sys     0m0.016s

puzzle2.txt
real    0m0.087s
user    0m0.078s
sys     0m0.016s

puzzle3.txt
real    0m56.419s
user    0m56.406s
sys     0m0.000s

puzzle4.txt
real    0m0.008s
user    0m0.000s
sys     0m0.000s

puzzle5.txt
real    0m0.010s
user    0m0.000s
sys     0m0.016s

puzzle6.txt
real    0m1.633s
user    0m1.625s
sys     0m0.000s

puzzle7.txt
real    0m6.603s
user    0m6.594s
sys     0m0.016s

puzzle8.txt - I know this one is slow
real    6m13.005s
user    6m13.000s
sys     0m0.000s

MISC. COMMENTS TO GRADER:  
I know my code is not efficient, but it works!!!
I would hope that counts for something at least.
The example given in lecture was far less complex as this one, and now
that I understand recursion better I realize that I should have structured
this homework differently.


