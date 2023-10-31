(this is not part of the license)

Project description:

# Project 2a - The Walking Deadline

Due Thursday, October 12, 2023 at 11:59 PM

## Overview

Zombies are invading the Bob & Betty Beyster Building!

As the Hero of EECS 281, you have been entrusted with protecting the building, its contents, its occupants, and everything that CS stands for. You have your trusty bow, a seemingly infinite supply of arrows, and a collection of priority queues limited only by main memory itself. You need to hold off the zombie horde - and hope that they don’t kill you while you’re refilling your quiver.

### Project Goals

- Understand and implement several kinds of priority queues
- Be able to independently read, learn about, and implement an unfamiliar data structure
- Be able to develop efficient data structures and algorithms
- Implement an interface that uses templated “generic” code
- Implement an interface that uses inheritance and basic dynamic polymorphism
- Become more proficient at testing and debugging your code

## Command Line Options

Your executable must be named `zombbb` (pronounced “zombies”). You may assume the command line is well-formed: you do not need to error-check it. It takes the following command line flags:

- `-v`, `--verbose`: The `-v` flag is optional. If provided, indicates that you should print out extra messages during the operation of the program about zombies being created, moved, or destroyed. See [Verbose Output](https://eecs281staff.github.io/p2-the-walking-deadline/#verbose-output).
- `-s <num>`, `--statistics <num>`: The `-s` flag is optional. If it is provided, then `<num>` is a required `uint32_t`. If provided, you should print out statistics at the end of the program. You’ll print out `<num>` entries for each type of statistic. The `-s` flag will always be followed by a positive number. See [Statistics Output](https://eecs281staff.github.io/p2-the-walking-deadline/#statistics-output).
- `-m`, `--median`: The `-m` flag is optional. If provided, indicates that you should print out extra messages during the program indicating the median time that zombies have been active before being destroyed. See [Median Output](https://eecs281staff.github.io/p2-the-walking-deadline/#median-output).
- `-h`, `--help`: When passed `-h`, print a useful help message explaining how to use the program to `cout`. Ignore all other input (including other command line flags and `cin`) and exit with status `0`.

### Legal Command Line Examples

|      | ./zombbb -mv < infile.txt   |
| ---- | --------------------------- |
|      | ./zombbb --statistics 10 -v |

### Illegal Command Line Examples

|      | ./zombbb -M (The -m option must be lower case)               |
| ---- | ------------------------------------------------------------ |
|      | ./zombbb -s (When provided, the -s option has a required argument <num>) |

Zombies are not particularly attentive to detail, so we will not be checking your command line handling. But it is to your benefit to add a reasonable amount of error-checking, just in case you mistype it.

## The Zombies

A zombie is defined by five attributes:

1. A `string` name. Names will always be unique in our test cases (and you do not need to check for this).
2. A `uint32_t` distance between the zombie and the player
3. A `uint32_t` speed (`>0`) at which the zombie moves toward you
4. A `uint32_t` health (`>0`), the amount of damage the zombie can take before being destroyed
5. The number of rounds the zombie has been active. This is measured as the number of rounds, from and including the round it was created, to and including the round it was shot or when the game ends. If the zombie was created in round 2 and was shot or the player was eaten in round 5, it was active for 4 rounds (rounds 2, 3, 4, and 5).

## The Battle

### The Schedule

The player operates in ‘rounds’, beginning with round 1. Each round:

1. The player refills their quiver so that it contains `quiver_capacity` arrows
2. Existing zombies move toward the player and attack if they have reached the player
3. New zombies appear
4. The player shoots zombies with arrows

### Zombie Offense

At the beginning of each round (just after you reload your quiver), the zombies move in closer. Every zombie has a base speed, and they move a distance equal to their base speed. Use an expression similar to the following to calculate each zombie’s new distance:

Updating a zombie's distance

|      | new_distance = distance - min(distance, speed); |
| ---- | ----------------------------------------------- |
|      | // or                                           |
|      | distance -= min(distance, speed);               |

This will prevent the distance from wrapping around zero. When subtraction is used in a `uint32_t` and the result would be negative, the value stored ends up as a very large postive number. You must move every zombie that has not yet been destroyed, even if the player is dead, because verbose mode must print an accurate move message.

You should perform this update on the zombies in the order in which they were initially created. You should not update the number of rounds active for zombies that are inactive (dead). This will require that you keep a data structure in addition to your priority queue to have all of the zombies available in the order of their creation. Your priority queue should then refer to elements inside this data structure (think about how).

As each zombie moves, if it has reached the player (distance is 0) it attacks, eating the player’s brain. If the player has their brain eaten, they die and the game is lost. After the existing zombies have moved and attacked, if the player is still alive new zombies may appear, but they do not move or attack until the next round.

### Player Defense

You’ve strategically taken position behind a chokepoint in order to maximize your chances of survival. However, this limits your movement options: you may not move during the course of the game.

Each round, you fill your quiver of arrows to its maximum capacity (`quiver_capacity`) and shoot as many times as you have arrows in your quiver. You have an infinite supply of arrows overall, but you can only fire one quiver per round and you only get the opportunity to refill your quiver in between rounds. Each arrow does one point of damage; a zombie is destroyed if its health reaches 0.

You must prioritize how to shoot the zombies in order to survive for as long as possible using a priority queue. In particular, you should approximate each zombie’s `uint32_t` estimated time of arrival (ETA) using the following formula:

Calculating the estimated time of arrival (ETA)

|      | ETA = distance / speed; |
| ---- | ----------------------- |
|      |                         |

Prioritize shooting the zombie with the lowest ETA. You may shoot the same zombie with several arrows during a round, but do not continue to shoot a zombie that has been destroyed (i.e. after its health has reached zero).

In the event of ties in ETA, you should shoot the zombie with the lower health. If zombies are also tied in health, you should shoot the zombie with the lexicographically smaller name. For example, if zombies paoletti and darden are equally close to you with equal health, you should shoot darden first since he has the lexicographically smaller name. You should not ignore case for this comparison: given zombies PAOLETTI and darden, you should shoot PAOLETTI first, as he has the lexicographically smaller name (by ASCII value). Use `std::string::operator<()` for this: it handles the comparison correctly.

When you shoot a zombie, its health is decreased by one. When it reaches zero, it has been destroyed and is no longer active.

If you manage to destroy all of the zombies, then you have won the battle.

### Round Breakdown

The following summarizes the actions that need to occur in a round and the order in which they should happen:

1. If the `--verbose` flag is enabled, print `Round:` followed by the round number.
2. You refill your quiver. Set the number of arrows you have equal to your quiver capacity.
3. All active zombies advance toward you, updated in the order that they were created.
   1. Update the zombie and move it closer to you.
   2. If the `--verbose` flag is enabled, print the zombie name, speed, distance, and health, along with `Moved:` For example:
      `Moved: paoletti0 (distance: 0, speed: 20, health: 1)`
   3. If at this point the zombie has reached you (has `distance == 0`), then it attacks you and you die.
   4. If you die, the first zombie that reached you is the one that has “eaten” you (see [Output](https://eecs281staff.github.io/p2-the-walking-deadline/#output)). You still need to update the other zombies, so don’t exit the loop yet!
4. At this point, if the Player was killed in Step 3, the battle ends.
   1. Print any required messages and statistics.
5. New zombies appear:
   1. Random zombies are created.
   2. Named zombies are created.
   3. If the `--verbose` flag is enabled, print new active zombie name, speed, distance, and health information in the order they were created, along with `“Created.”` This is true for both random and named zombies. For example: `Created: paoletti0 (distance: 25, speed: 20, health: 1)`
6. Player shoots zombies.
   1. Shoot the zombie with the lowest ETA (tie breaking described above) until your quiver is empty.
   2. If you destroy a zombie AND the `--verbose` flag is enabled, display a message; for example:
      `Destroyed: paoletti0 (distance: 4, speed: 1, health: 0)`
7. If the `--median` flag is enabled, AND any zombies have been destroyed so far, display a message about the median time that zombies have been active. If there are an even number, take the average of the “middle two”, with integer division. For example:
   `At the end of round 1, the median zombie lifetime is 1`
8. If there are no more zombies and none will be generated in a future round, then you have won the battle.
   1. Print any required messages and statistics. The battle ends.

## Input File Format

Information about the zombies will be given on standard input (`cin`). The input consists of a header, followed by any number of rounds. You may assume the input is well-formed: you do not need to error-check it. A full input/output example is provided in the appendices.

### Header Format

Each header starts with exactly one comment line, beginning with a `#`. You should ignore this line. The file then contains the following parameters, all of which are followed by `uint32_t` values. The `max-rand-` values provided will not be zero.

- `quiver-capacity`: The number of arrows you can fit into your quiver. You refill your quiver to this number at the beginning of every round.
- `random-seed`: A “seed”, which initializes our patent-pending Random Zombie Generator™ (see [Random Zombie Generation](https://eecs281staff.github.io/p2-the-walking-deadline/#random-zombie-generation)). The Random Zombie Generator™ always produces the same zombies when provided with the same seed.
- `max-rand-distance`: The maximum starting distance of a randomly-generated zombie.
- `max-rand-speed`: The maximum speed of a randomly-generated zombie.
- `max-rand-health`: The maximum health of a randomly-generated zombie.

Named zombies are not constrained to the `max-rand-` parameters: they may be farther, faster, or healthier.

### Example Header

Example header

|      | # my test case        |
| ---- | --------------------- |
|      | quiver-capacity: 10   |
|      | random-seed: 123456   |
|      | max-rand-distance: 50 |
|      | max-rand-speed: 10    |
|      | max-rand-health: 5    |

### Round Format

After the header is a list of rounds. There is always at least one round. Each round starts with a delimiting triple hyphen (“—”). The round information contains a round number, information about random zombies, and possibly a list of named zombies.

The round numbers in the round list strictly increase. However some round numbers may be skipped! You should simulate all of the currently active zombies as normal during unlisted rounds, but you should not create any new ones. The only information provided about the random zombies is the number of them; they will be generated by the Random Zombie Generator™ (see the Random Zombie Generation section). Information about the named zombies is provided on the following lines. If the number of named zombies is given as 0, there will be no lines describing named zombies.

### Example Round

Example round

|      | ---                                          |
| ---- | -------------------------------------------- |
|      | round: 3                                     |
|      | random-zombies: 10                           |
|      | named-zombies: 3                             |
|      | darden    distance: 20  speed: 10  health: 3 |
|      | paoletti  distance: 30  speed: 10  health: 5 |
|      | garcia    distance: 40  speed: 15  health: 4 |

### Note on Parameter Names

For your convenience in writing test files, the name of any parameter may be abbreviated: it may be replaced with any non-empty string without whitespace. Additionally, whitespace is to be ignored, except that there is at least one whitespace character between a parameter name and value, and the comment line always ends with a newline character. For example, the input below is also valid:

Valid input with extra whitespace

|      | # my test case                               |
| ---- | -------------------------------------------- |
|      | quiver    10  seed   123456                  |
|      | rand-dist 50  rand-speed 10  rand-health 5   |
|      | ---                                          |
|      | r 3                                          |
|      | rndzmbs 10                                   |
|      | named 3                                      |
|      |                                              |
|      | darden    distance: 20  speed: 10  health: 3 |
|      | paoletti  distance 30 speed 10 health 5      |
|      | garcia    d 40 s 15 h 4                      |

Hint: read with `>>`, not `getline()`! The text box below contains a complete and valid sample input file.

### Example Input File

spec.txt

|      | # Project 2 specification example                 |
| ---- | ------------------------------------------------- |
|      | quiver-capacity: 10                               |
|      | random-seed: 2049231                              |
|      | max-rand-distance: 50                             |
|      | max-rand-speed: 60                                |
|      | max-rand-health: 1                                |
|      | ---                                               |
|      | round: 1                                          |
|      | random-zombies: 25                                |
|      | named-zombies: 3                                  |
|      | MarkSchlissel distance: 150 speed: 300 health: 15 |
|      | MarySueColeman distance: 2 speed: 3 health: 6     |
|      | LeeBollinger distance: 100 speed: 1 health: 100   |
|      | ---                                               |
|      | round: 3                                          |
|      | random-zombies: 10                                |
|      | named-zombies: 1                                  |
|      | JamesDuderstadt distance: 20 speed: 10 health: 20 |

## Random Zombie Generation

To standardize random number generation across all development platforms in the course we have created the P2random files (P2random.h and P2random.cpp). They generate random numbers using the [Mersenne Twister](http://en.wikipedia.org/wiki/Mersenne_Twister). Before generating any random numbers, call `P2random::initialize()` passing it the `random-seed`, `max-rand-distance`, `max-rand-speed`, and `max-rand-health`, values (read from the start of the input file), in that order. Call this function only once. Include the header file in any module where random numbers are needed. There is no need to modify either of the P2random files.

Each round, you should generate random zombies before the player starts shooting zombies but after existing zombies have been updated. When randomly generating zombies, you should generate them as follows:

Random zombie generation function calls

|      | std::string name  = P2random::getNextZombieName();     |
| ---- | ------------------------------------------------------ |
|      | uint32_t distance = P2random::getNextZombieDistance(); |
|      | uint32_t speed    = P2random::getNextZombieSpeed();    |
|      | uint32_t health   = P2random::getNextZombieHealth();   |

You should always generate the zombie’s statistics in the order given above, because the ordering will affect the result of the pseudo-random number generation. **DO NOT** put these 4 function calls into another function call or constructor call without the intermediate variables! Some compilers evaluate function parameters right-to-left, which would result in incorrect values.

**Also:** Explicitly specified zombies are always created **after** the random ones.

## Output

If the player lives after all zombies have been destroyed and no new zombies will be generated in a later round, the output (sent to `cout`) should be:

Survival (success) message format

|      | VICTORY IN ROUND <ROUND_NUMBER>! <NAME_OF_LAST_ZOMBIE_KILLED> was the last zombie. |
| ---- | ------------------------------------------------------------ |
|      |                                                              |

If the player is killed, the output should be:

Demise (failure) message format

|      | DEFEAT IN ROUND <ROUND_NUMBER>! <NAME_OF_KILLER_ZOMBIE> ate your brains! |
| ---- | ------------------------------------------------------------ |
|      |                                                              |

If multiple zombies would be able to eat your brains in the same round, you should print the name of the one who was updated first.

Example:

Demise message

|      | DEFEAT IN ROUND 2! FoxMcCloud ate your brains! |
| ---- | ---------------------------------------------- |
|      |                                                |

### Verbose Output

You should be working on the verbose output as you write the program, so that you can fix any potential problems as soon as possible, rather than wondering why the player is dead when they’re supposed to be alive.

If `--verbose` is specified, print the following during every round:

- `Round: <round#>`.
- For each zombie updated, print `Moved: `and its name distance, speed and health.
- After a zombie is created, print `Created: `and its name, distance, speed and health.
- Whenever a zombie is killed, print `Destroyed: `and its name, distance, speed and health.

For example, your output generated via the `--verbose` flag may look like:

Sample verbose output

|      | Round: 1                                                     |
| ---- | ------------------------------------------------------------ |
|      | Created: paoletti0 (distance: 25, speed: 20, health: 1)      |
|      | Created: juett1 (distance: 17, speed: 48, health: 1)         |
|      | Created: FoxMcCloud (distance: 10, speed: 10, health: 100)   |
|      | Created: FalcoLombardi (distance: 12, speed: 12, health: 100) |
|      | Round: 2                                                     |
|      | Moved: FoxMcCloud (distance: 0, speed: 10, health: 100)      |
|      | Moved: FalcoLombardi (distance: 0, speed: 12, health: 92)    |
|      | DEFEAT IN ROUND 2! FoxMcCloud ate your brains!               |

Again, this is not the correct output for the example given above; see the appendices for that.

### Statistics Output

If and only if the `--statistics N` option is specified on the command line, the following additional output should be printed after the `DEFEAT` or `VICTORY` line in the following order without any blank lines separating them:

- The number of zombies still active at the end, in the format:

Remaining active zombies message format

|      | Zombies still active: <NUMBER_OF_ZOMBIES> |
| ---- | ----------------------------------------- |
|      |                                           |

Where `NUMBER_OF_ZOMBIES` is the number of zombies still active (created but not destroyed)

- The names of the first `N` zombies that were killed, each followed by a space and the number (1,2,3,…,`N`) corresponding to the relative order they were killed in. These zombies should be printed in order, such that the very first one killed is printed first, and the Nth first one is printed Nth.
- The names of the last `N` zombies that were killed, followed by a space, and then the number (`N`,`N`-1,…,1) corresponding to the relative order they were killed in. These zombies should be printed in order, such that the very last one killed is printed first, and the Nth-to-last zombie is printed Nth.
- The names of the `N` zombies who were active for the most number of rounds followed by a space and then the number of rounds. These zombies should be printed in order such that the zombie who was in the most rounds appears first. In the event that there is more than one zombie who survived for the same amount of time, you should print the one who has a lexicographically smaller name first (use the `<` operator of `std::string` for this).
- The names of the `N` zombies who were active for the least number of rounds, followed by a space and then the number of rounds. These zombies should be printed in order such that the zombie who was in the least rounds appears first. Break ties using the same lexicographic rule as before.

**Hint:** Be efficient about how the last two statistics (that deal with how many rounds a zombie has been active) are calculated.

If for any of the above statistics you do not have at least `N` zombies to print, you should print the data for as many as you can.

Example: If the program is run with `--statistics 10`, there were only three zombies in the game, and you killed FoxMcCloud before FalcoLombardi and they were both active for 3 rounds and then you killed DarkLink in round 5, its fourth round of being active, you should print the following statistics output (after the victory/defeat line):

Printing statistics example

|      | Zombies still active: 0 |
| ---- | ----------------------- |
|      | First zombies killed:   |
|      | FoxMcCloud 1            |
|      | FalcoLombardi 2         |
|      | DarkLink 3              |
|      | Last zombies killed:    |
|      | DarkLink 3              |
|      | FalcoLombardi 2         |
|      | FoxMcCloud 1            |
|      | Most active zombies:    |
|      | DarkLink 4              |
|      | FalcoLombardi 3         |
|      | FoxMcCloud 3            |
|      | Least active zombies:   |
|      | FalcoLombardi 3         |
|      | FoxMcCloud 3            |
|      | DarkLink 4              |

Note that `"FalcoLombardi" < "FoxMcCloud"` lexicographically and that some zombies may be included in several lists. This is not the output of the sample input given above; for that output, see the appendices.

### Median Output

If `--median` is specified, whenever a zombie is **destroyed**, include its number of rounds active in the median data. It must contain only destroyed zombies, because the `std::priority_queue` has no way to update its ordering if the values it contains change, and active zombies still have their number of rounds active changing.

In [Step 7](https://eecs281staff.github.io/p2-the-walking-deadline/#round-breakdown), if any zombies have been destroyed, print the median time that destroyed zombies were active.

For example, your output generated via the `--median` flag may look like:

Sample median output

|      | At the end of round 1, the median zombie lifetime is 1 |
| ---- | ------------------------------------------------------ |
|      |                                                        |

Again, this is not the correct output for the example given above; see the appendices for that.

## Logistics

### The `std::priority_queue<>`

**For Part A, you should always use `std::priority_queue<>`, not your templates from Part B. In Part A you need to use pointers for keeping track of zombies (both “living” and dead); in Part B you will have to use pointers for the Pairing Heap.**

The STL `priority_queue<>` data structure is an efficient implementation of the binary heap which you are also coding in BinaryPQ.h (Part B). To declare a `priority_queue<>` you need to state either one or three types:

1. The data type, `T`, to be stored in the container. If this type has a natural sort order that meets your needs, this is the only type required.
2. The type of underlying container to use, usually just a `vector<T>`.
3. The type of comparator the container will use to determine what is considered the highest priority element.

If the type that you store in the container has a natural sort order (i.e. it supports `operator<()`), the `priority_queue<>` will be a max-heap of the declared type. For example, if you just want to store integers, and have the largest integer be the highest priority:

|      | priority_queue<int> pqMax; |
| ---- | -------------------------- |
|      |                            |

When you declare this, by default the underlying storage type is `vector<int>` and the default comparator is `less<int>`. If you want the smallest integer to be the highest priority:

|      | priority_queue<int, vector<int>, greater<int>> pqMin; |
| ---- | ----------------------------------------------------- |
|      |                                                       |

If you want to store something other than integers, define a custom comparator as described below.

### About Comparators

The functor must accept two of whatever is stored in your priority queue: if your PQ stores integers, the functor would accept two integers. If your PQ stores pointers to objects, your functor would accept two pointers to objects (actually two const pointers, since you don’t have to modify objects to compare them).

Your functor receives two parameters, let’s call them a and b. It must always answer the following question: is the priority of a less than the priority of b? What does lower priority mean? It depends on your application.

When you would have wanted to write a comparison, such as:

|      | if (data[i] < data[j]) |
| ---- | ---------------------- |
|      |                        |

You would instead write:

|      | if (this->compare(data[i], data[j])) |
| ---- | ------------------------------------ |
|      |                                      |

Your priority queues must work in general. It’s important to realize that a priority queue has no idea what kind of data is inside of it. That’s why it uses `this->compare()` instead of `<`. What if you wanted to perform the comparison `if (data[i] > data[j])`? Use the following:

|      | if (this->compare(data[j], data[i])) |
| ---- | ------------------------------------ |
|      |                                      |

## Libraries and Restrictions

We highly encourage the use of the STL for part A, with the exception of these prohibited features:

- The thread/atomics libraries (e.g., boost, pthreads, etc) which spoil runtime measurements.
- Smart pointers (both unique and shared).

You are allowed to use `std::vector<>`, `std::priority_queue<>` and `std::deque<>`.

You are not allowed to use other STL containers. Specifically, this means that use of `std::stack<>`, `std::queue<>`, `std::list<>`, `std::set<>`, `std::map<>`, `std::unordered_set<>`, `std::unordered_map<>`, and the ‘multi’ variants of the aforementioned containers are forbidden.

## Testing and Debugging

Part of this project is to prepare several test files that will expose defects in the program. We strongly recommend that you first try to catch a few of our buggy solutions with your own test files, before beginning your solutions. This will be extremely helpful for debugging. The autograder will also tell you if one of your test files exposes bugs in your own solution, and for the first such file encountered we will provide you with the correct output and your program’s output.

Each test file should consist of a valid GAMEFILE. We will run your test files on several buggy project solutions. If your test file causes a correct program and the incorrect program to produce different output, the test file is said to expose that bug.

Test files should be named test-n-OPTION.txt where 0≤�≤9. The autograder’s buggy solutions will run your test files in the specified OPTION. Valid options are v, m, or s, only one can be used per input file (ie: a file named test-1-v.txt will run your test on a solution using the verbose flag). If you specify the s option, we will automatically pick a number of statistics for you (greater than zero).

Your test files may not generate more than 50 zombies (counting both random and explicitly specified zombies). Further, no test file can run more than 1000 rounds (to prevent running too long). You may include up to 10 test files per submission (though it is possible to get full credit with fewer test files). The test cases that the autograder runs on your solution are NOT limited to 50 zombies or 1000 rounds; your solution should not impose any size limits (as long as sufficient system memory is available).

## Submission to the Autograder

Do all of your work (with all needed files, as well as test files) in some directory other than your home directory. This will be your “submit directory”. You can make two separate projects inside of your IDE: one for part A, another for part B. Before you turn in your code, be sure that:

- Every source code and header file contains the following project identifier in a comment at the top of the file: // Project Identifier: 9504853406CBAC39EE89AA3AD238AA12CA198043
- The Makefile must also have this identifier (in the first TODO block).
- DO NOT copy the above identifier from the PDF! It might contain hidden characters. Copy it from the README file instead (this file is included with the starter files).
  - You have deleted all .o files and any executables. Typing ‘make clean’ should accomplish this.
  - Your makefile is called Makefile. Typing `make -R -r` builds your code without errors and generates an executable file called zombbb. The command line options `-R` and `-r` disable automatic build rules, which will not work on the autograder.
  - Your Makefile specifies that you are compiling with the gcc optimization option `-O3`. This is extremely important for getting all of the performance points, as `-O3` can often speed up execution by an order of magnitude. You should also ensure that you are not submitting a Makefile to the autograder that compiles with `-g`, as this will slow your code down considerably. If your code “works” when you don’t compile with `-O3` and breaks when you do, it means you have a bug in your code!
  - Your test files are named test-n-MODE.txt and no other project file names begin with test. Up to 10 test files may be submitted.
  - The total size of your solution and test files does not exceed 2MB.
  - You don’t have any unnecessary files (including temporary files created by your text editor and compiler, etc) or subdirectories in your submit directory (i.e. the .git folder used by git source code management).
  - Your code compiles and runs correctly using version 11.3.0 of the g++ compiler. This is available on the CAEN Linux systems (that you can access via login.engin.umich.edu). Even if everything seems to work on another operating system or with different versions of GCC, the course staff will not support anything other than GCC 11.3.0 running on Linux (students using other compilers and OS did observe incompatibilities). In order to compile with g++ version 11.3.0 on CAEN you must put the following at the top of your Makefile (or use the one that we’ve provided):

|      | PATH := /usr/um/gcc-11.3.0/bin:$(PATH)      |
| ---- | ------------------------------------------- |
|      | LD_LIBRARY_PATH := /usr/um/gcc-11.3.0/lib64 |
|      | LD_RUN_PATH := /usr/um/gcc-11.3.0/lib64     |

Turn in all of the following files:

- All your .h and .cpp files for the project (solution and priority queues)
- Your Makefile
- Your test files

You must prepare a compressed tar archive (.tar.gz file) of all of your files to submit to the autograder. One way to do this is to have all of your files for submission (and nothing else) in one directory. Go into this directory and run one of these two commands (assuming you are using our Makefile):

- `make fullsubmit`
  (builds a “tarball” named `fullsubmit.tar.gz` that contains all source and header files, test files, and the `Makefile`; this file is to be submitted to the autograder for any completely graded submission)
- `make partialsubmit`
  (builds a “tarball” named `partialsubmit.tar.gz` that contains only source and header files, and the `Makefile`; test files are not included, which will speed up the autograder by not checking for bugs; this should be used when testing the simulation only)

For Part A, you can submit test files (zombie simulation gamefiles), for Part B you only submit code.

These commands will prepare a suitable file in your working directory. Submit your project files directly to the autograders at: https://eecs281staff.github.io/ag-status/.

You can safely ignore and override any warnings about an invalid security certificate. When the autograders are turned on and accepting submissions, there will be an announcement on Piazza. The autograders are identical and your daily submission limit will be shared (and kept track of) between them. You may submit up to 2 times per calendar day, per part, with autograder feedback (more in Spring). For this purpose, days begin and end at midnight (Ann Arbor local time). We will use your best submission when running final grading. Part of programming is knowing when you are done (when you have achieved your task and have no bugs); this is reflected in this grading policy. We strongly recommend that you use some form of revision control (ie: SVN, GIT, etc) and that you ‘commit’ your files every time you upload to the autograder so that you can always retrieve an older version of the code as needed. Please refer to your discussion slides and Canvas regarding the use of version control.

If you use late days on this project, it is applied to both parts. So if you use one late day for Part A, you automatically get a 1-day extension for Part B, but are only charged for the use of one late day.

Please make sure that you read all messages shown at the top section of your autograder results! These messages will help explain some of the issues you are having (such as losing points for having a bad Makefile). Check for the word “Hint” (without quotes) in the autograder feedback for help on some test cases. Also be sure to check whether the autograder shows that one of your own test files exposes a bug in your solution (at the bottom of the “Scoring student test files” section).

## Grading

- 60 pts total for part A (all your code, using the STL, but not using your priority queues)
  - 45 pts — correctness & performance
  - 5 pts — no memory leaks
  - 10 pts — student-provided test files
- 40 pts total for part B (our main, your priority queues)
  - 20 pts — pairing heap correctness & performance
  - 5 pts — pairing heap has no memory leaks
  - 10 pts — binary heap correctness & performance
  - 5 pts — sorted heap correctness & performance

Although we will not be grading your code for style, we reserve the right to give you limited help in office hours if your code is unreadable.

It is extremely helpful to compile your code with the gcc options: `-Wall -Wextra -pedantic -Werror -Wconversion`. This will help you catch bugs in your code early by having the compiler point out when you write code that is either of poor style or might result in unintended behavior. This can make it a little more difficult to get your code compiling, but makes it less likely to have runtime errors.

## Part B: Priority Queues

The [Part B Spec](https://eecs281staff.github.io/p2b-priority-queues) is in a separate document. The solution to Part B will be submitted separately from Part A, and should be developed separately in your development environment with the files from both solutions stored in separate directories.