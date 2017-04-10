#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
using namespace std;

#define SIZE 10
#define NUM_TILES 19
#define BOARD_SIZE (SIZE*SIZE)

// #define TIMED
#ifdef TIMED
    // #define TIMED_ACT
    // #define TIMED_VALID_ACTIONS
#endif

typedef struct {
    bool board[BOARD_SIZE];
    int tile;
} State;

typedef struct {
    State afterstate;
    int reward;
    bool valid;
} ActionOutput;

// profiling
#ifdef TIMED
const int NUM_TIMES = 20;
clock_t times[NUM_TIMES] = {0};
#endif

// tiles parameters
static const int tilesWidth[]  = {1, 2, 1, 3, 1, 2, 2, 2, 2, 1, 4, 2, 1, 5, 3, 3, 3, 3, 3};
static const int tilesHeight[] = {1, 1, 2, 1, 3, 2, 2, 2, 2, 4, 1, 2, 5, 1, 3, 3, 3, 3, 3};
static const int tilesSize[]   = {1, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 5, 9};
static const int tilesSizeCumSum[] = {0, 1, 3,  5,  8, 11, 14, 17, 20, 23, 27, 31, 35, 40, 45, 50, 55, 60, 65}; // starting index of the tile
static const int tiles[] = {
    0,                              // 0: #
    0, 1,                           // 1: ##
    0, 10,                          // 2: #/#
    0, 1, 2,                        // 3: ###
    0, 10, 20,                      // 4: #/#/#
    1, 10, 11,                      // 5: .#/##
    0, 10, 11,                      // 6: #./##
    0, 1, 10,                       // 7: ##/#.
    0, 1, 11,                       // 8: ##/.#
    0, 10, 20, 30,                  // 9: #/#/#/#
    0, 1, 2, 3,                     // 10: ####
    0, 1, 10, 11,                   // 11: ##/##
    0, 10, 20, 30, 40,              // 12: #/#/#/#/#
    0, 1, 2, 3, 4,                  // 13: #####
    0, 1, 2, 12, 22,                // 14: ###/..#/..#
    0, 1, 2, 10, 20,                // 15: ###/#../#..
    0, 10, 20, 21, 22,              // 16: #../#../###
    2, 12, 20, 21, 22,              // 17: ..#/..#/###
    0, 1, 2, 10, 11, 12, 20, 21, 22 // 18: ###/###/###
};
static const long tilesInt[] = {
    1,
    3,
    1025,
    7,
    1049601,
    3074,
    3073,
    1027,
    2051,
    1074791425,
    15,
    3075,
    1100586419201,
    31,
    4198407,
    1049607,
    7341057,
    7344132,
    7347207
};

// game class
class Game1010 {
  public:
    State state;
    int score;
    
    Game1010();
    
    // next state predictions
    ActionOutput getAfterState(int, State);
    State getNextState(State);
    
    // action
    void act(int);
    vector<int> getValidActions2(State);
    vector<int> getValidActions(State);
    
    // termination and restart
    bool isTerminated(State);
    void restart();
    
    // user-interaction
    void render();
};

Game1010::Game1010() {
    restart();
}

// get afterstate for a given action and the state
ActionOutput Game1010::getAfterState(int action, State inputstate) {
    // determine the row and column of the action
    #ifdef TIMED_ACT
    clock_t t4 = clock();
    #endif
    int row = action / SIZE;
    int col = action % SIZE;
    
    // extract the tile information
    const int* tile = &tiles[tilesSizeCumSum[inputstate.tile]];
    const int tileSize = tilesSize[inputstate.tile];
    const int tileHeight = tilesHeight[inputstate.tile];
    const int tileWidth = tilesWidth[inputstate.tile];
    
    #ifdef TIMED_ACT
    times[4] += clock() - t4;
    
    clock_t t5 = clock();
    #endif
    // check if the action is valid
    bool isValid = true;
    if ((row < 0) || (col < 0) || (row > SIZE - tileHeight) || (col > SIZE - tileWidth)) { // check if the tiles are inside the box
        isValid = false;
    }
    else {
        // check if all tiles to be placed are blank
        for (int i = 0; i < tileSize; ++i) {
            if (inputstate.board[tile[i]+action]) {
                isValid = false;
                break;
            }
        }
    }
    #ifdef TIMED_ACT
    times[5] += clock() - t5;
    #endif
    
    // if the action is not valid, return the next state as it is
    if (!isValid) {
        ActionOutput ao;
        ao.afterstate = inputstate;
        ao.reward = 0;
        ao.valid = false;
        return ao;
    }
    
    #ifdef TIMED_ACT
    clock_t t6 = clock();
    #endif
    // if it's valid, then place the tiles on the board
    State nextstate = inputstate;
    for (int i = 0; i < tileSize; ++i) {
        nextstate.board[tile[i]+action] = true;
    }
    #ifdef TIMED_ACT
    times[6] += clock() - t6;
    
    clock_t t7 = clock();
    #endif
    // check the completed rows and columns
    bool columnsIncomplete[tileWidth] = {false};
    bool rowsIncomplete[tileHeight] = {false};
    for (int i = 0; i < tileHeight; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (nextstate.board[(i+row)*SIZE+j] == false) {
                rowsIncomplete[i] = true;
                columnsIncomplete[j] = true;
                break;
            }
        }
    }
    for (int j = 0; j < tileWidth; ++j) {
        if (columnsIncomplete[j]) continue;
        for (int i = 0; i < SIZE; ++i) {
            if (nextstate.board[i*SIZE+j+col] == false) {
                columnsIncomplete[j] = true;
                break;
            }
        }
    }
    #ifdef TIMED_ACT
    times[7] += clock() - t7;
    
    clock_t t8 = clock();
    #endif
    // remove the completed rows and columns
    for (int i = 0; i < tileHeight; ++i) {
        if (!rowsIncomplete[i]) { // row is complete
            for (int j = 0; j < SIZE; ++j) {
                nextstate.board[(i+row)*SIZE+j] = false;
            }
        }
    }
    for (int j = 0; j < tileWidth; ++j) {
        if (!columnsIncomplete[j]) { // the column is complete
            for (int i = 0; i < SIZE; ++i) {
                nextstate.board[i*SIZE+j+col] = false;
            }
        }
    }
    #ifdef TIMED_ACT
    times[8] += clock() - t8;
    
    clock_t t9 = clock();
    #endif
    // calculate the number of completed rows and cols
    int ncomplete = 0;
    for (int i = 0; i < tileHeight; ++i) {
        if (!rowsIncomplete[i]) ++ncomplete;
    }
    for (int i = 0; i < tileWidth; ++i) {
        if (!columnsIncomplete[i]) ++ncomplete;
    }
    #ifdef TIMED_ACT
    times[9] += clock() - t9;
    clock_t t10 = clock();
    times[10] += clock() - t10;
    #endif
    
    // calculate the reward
    int reward = tileSize + (ncomplete+1)*ncomplete*5;
    
    // return the output_iterator
    ActionOutput ao;
    ao.afterstate = nextstate;
    ao.reward = reward;
    ao.valid = true;
    return ao;
}

// get one possible next state after the afterstate
State Game1010::getNextState(State afterstate) {
    State ss = afterstate;
    ss.tile = rand() % NUM_TILES;
    return ss;
}

// do an action to the board
void Game1010::act(int action) {
    // get the next state
    ActionOutput ao = getAfterState(action, state);
    if (ao.valid) {
        state = getNextState(ao.afterstate);
        
        // add the score with the reward
        score += ao.reward;
    }
}

// list all possible valid actions
vector<int> Game1010::getValidActions(State inputstate) {
    // extract the tile information
    #ifdef TIMED_VALID_ACTIONS
    clock_t t11 = clock();
    #endif
    
    const int* tile = &tiles[tilesSizeCumSum[inputstate.tile]];
    int tileSize = tilesSize[inputstate.tile];
    int tileHeight = tilesHeight[inputstate.tile];
    int tileWidth = tilesWidth[inputstate.tile];
    
    #ifdef TIMED_VALID_ACTIONS
    times[11] += clock() - t11;
    
    clock_t t12 = clock();
    #endif
    // prepare the output vector
    vector<int> validActions;
    validActions.resize(BOARD_SIZE);
    
    #ifdef TIMED_VALID_ACTIONS
    times[12] += clock() - t12;
    #endif
    
    // check all possible actions
    int nActions = 0;
    int action = -1;
    bool isValid;
    for (int row = 0; row <= SIZE-tileHeight; ++row) {
        for (int col = 0; col <= SIZE-tileWidth; ++col) {
            // for a certain action, check if the tiles are blank
            #ifdef TIMED_VALID_ACTIONS
            clock_t t13 = clock();
            #endif
            
            action++;
            if (inputstate.board[tile[0]+action]) continue;
            isValid = true;
            
            #ifdef TIMED_VALID_ACTIONS
            times[13] += clock() - t13;
            #endif
            
            for (int i = 1; i < tileSize; ++i) {
                #ifdef TIMED_VALID_ACTIONS
                clock_t t14 = clock();
                #endif
                
                if (inputstate.board[tile[i]+action]) {
                    isValid = false;
                    break;
                }
                
                #ifdef TIMED_VALID_ACTIONS
                times[14] += clock() - t14;
                #endif
            }
            
            // if the action is valid, then it is not terminated
            #ifdef TIMED_VALID_ACTIONS
            clock_t t15 = clock();
            #endif
            
            if (isValid) {
                validActions[nActions++] = action;
            }
            
            #ifdef TIMED_VALID_ACTIONS
            times[15] += clock() - t15;
            #endif
        }
        action += tileWidth - 1;
    }
    
    // resize the valid actions
    #ifdef TIMED_VALID_ACTIONS
    clock_t t16 = clock();
    #endif
    
    validActions.resize(nActions);
    
    #ifdef TIMED_VALID_ACTIONS
    times[16] += clock() - t16;
    #endif
    return validActions;
}

vector<int> Game1010::getValidActions2(State inputstate) {
    // extract the tile information
    #ifdef TIMED_VALID_ACTIONS
    clock_t t11 = clock();
    #endif
    
    const int* tile = &tiles[tilesSizeCumSum[inputstate.tile]];
    int tileSize = tilesSize[inputstate.tile];
    int tileHeight = tilesHeight[inputstate.tile];
    int tileWidth = tilesWidth[inputstate.tile];
    long tileInt = tilesInt[inputstate.tile];
    
    #ifdef TIMED_VALID_ACTIONS
    times[11] += clock() - t11;
    
    clock_t t12 = clock();
    #endif
    // prepare the output vector
    vector<int> validActions;
    validActions.resize(BOARD_SIZE);
    int nActions = 0;
    
    #ifdef TIMED_VALID_ACTIONS
    times[12] += clock() - t12;
    #endif
    
    // convert the board into __int128 bits
    unsigned __int128 boardInt128 = 0;
    for (int i = BOARD_SIZE-1; i >= 0; --i) {
        boardInt128 <<= 1;
        if (inputstate.board[i]) boardInt128 |= 1;
    }
    
    unsigned long boardInt64;
    for (int row = 0; row <= SIZE-tileHeight; ++row) {
        boardInt64 = static_cast<unsigned long>(boardInt128);
        for (int col = 0; col <= SIZE-tileWidth; ++col) {
            if (!(boardInt64 & tileInt)) validActions[nActions++] = row*SIZE + col;
            boardInt64 >>= 1;
        }
        boardInt128 >>= SIZE;
    }
    
    validActions.resize(nActions);
    return validActions;
}

// check if it's terminated (if there's no possible action)
bool Game1010::isTerminated(State inputstate) {
    // extract the tile information
    const int* tile = &tiles[tilesSizeCumSum[inputstate.tile]];
    int tileSize = tilesSize[inputstate.tile];
    int tileWidth = tilesWidth[inputstate.tile];
    
    int hspace = SIZE-tilesHeight[inputstate.tile];
    int wspace = SIZE-tileWidth;
    
    // check all possible actions
    int action = 0;
    bool isValid;
    for (int row = 0; row <= hspace; ++row) {
        for (int col = 0; col <= wspace; ++col) {
            
            // for a certain action, check if the tiles are blank
            isValid = true;
            for (int i = 0; i < tileSize; ++i) {
                if (inputstate.board[tile[i]+action]) {
                    isValid = false;
                    break;
                }
            }
            
            // if the action is valid, then it is not terminated
            if (isValid) return false;
            action++;
        }
        action += tileWidth - 1;
    }
    return true;
}

// restart the game: clear the board, assign a random tile, and reset the score
void Game1010::restart() {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        state.board[i] = false;
    }
    state.tile = rand() % NUM_TILES; // generate random number between 0 and NUM_TILES-1
    
    // reset the score
    score = 0;
}

void Game1010::render() {
    // print the score
    printf("Score: %d\n", score);
    
    // print the board
    bool* boardPtr = state.board;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (*boardPtr++) printf("#");
            else printf(".");
        }
        printf("\n");
    }
    printf("\n");
    
    // print the tile
    const int* tile = &tiles[tilesSizeCumSum[state.tile]];
    int tileSize = tilesSize[state.tile];
    int nTile = 0;
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            if ((nTile < tileSize) && (tile[nTile] == i*SIZE+j)) {
                printf("#");
                ++nTile;
            }
            else printf(".");
        }
        printf("\n");
    }
    
}


int play(bool random=false, bool wait=true, bool render=true) {
    int action;
    Game1010* g = new Game1010();
    if (render) g->render();
    while (!g->isTerminated(g->state)) {
        if (wait)
            scanf("%i", &action);
        if (random) {
            vector<int> validActions = g->getValidActions(g->state);
            action = validActions[rand() % validActions.size()];
        }
        if (render) printf("Your action: %d\n", action);
        g->act(action%BOARD_SIZE);
        if (render) g->render();
    }
    delete g;
}

int simulate(int N) {
    
    int action;
    Game1010* g = new Game1010();
    int nsteps = 0;
    bool isTerminated;
    
    for (int i = 0; i < N; ++i) {
        while (true) {
            #ifdef TIMED
            clock_t t0 = clock();
            #endif
            vector<int> validActions = g->getValidActions(g->state);
            #ifdef TIMED
            clock_t t1 = clock();
            #endif
            action = validActions[rand() % validActions.size()];
            #ifdef TIMED
            clock_t t2 = clock();
            #endif
            g->act(action);
            #ifdef TIMED
            clock_t t3 = clock();
            #endif
            isTerminated = g->isTerminated(g->state);
            #ifdef TIMED
            clock_t t4 = clock();
            #endif
            
            #ifdef TIMED
            times[0] += t1 - t0;
            times[1] += t2 - t1;
            times[2] += t3 - t2;
            times[3] += t4 - t3;
            #endif
            
            ++nsteps;
            if (isTerminated) break;
        }
        g->restart();
    }
    printf("Total steps: %d\n", nsteps);
    #ifdef TIMED
    for (int i = 0; i < NUM_TIMES; ++i) {
        printf("%f, ", (float)times[i] / CLOCKS_PER_SEC);
    }
    #endif
    printf("\n");
    delete g;
}

int main() {
    // play(true, true, true);
    simulate(100000);
}































