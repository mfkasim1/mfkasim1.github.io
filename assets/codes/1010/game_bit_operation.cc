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
    __int128 board;
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
static const long tilesInt[] = { // mask of the tiles
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

static const __int128 rowMask = (1 << SIZE) - 1;
static const __int128 colMask = ((__int128)(1239150)*1000000+146850)*1000000000000000+664126585242625;

// game class
class Game1010 {
  public:
    State state;
    int score;
    
    Game1010();
    
    // next state predictions
    ActionOutput getAfterState(int, State);
    State getNextState(State);
    vector<State> getAllStates(State);
    
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
    const int tileSize = tilesSize[inputstate.tile];
    const int tileHeight = tilesHeight[inputstate.tile];
    const int tileWidth = tilesWidth[inputstate.tile];
    const int tileInt = tilesInt[inputstate.tile];
    
    #ifdef TIMED_ACT
    times[4] += clock() - t4;
    
    clock_t t5 = clock();
    #endif
    
    // check if the action is valid
    bool isValid = true;
    if ((row < 0) || (col < 0) || (row > SIZE - tileHeight) || (col > SIZE - tileWidth)) { // check if the tiles are inside the box
        isValid = false;
    }
    // check if all tiles to be placed are blank
    else if ((long)(inputstate.board >> action) & tileInt) {
        isValid = false;
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
    // obtain the tile mask to be placed on board
    __int128 tileOnBoard = (__int128)(tileInt) << action;
    
    // if it's valid, then place the tiles on the board
    State nextstate = inputstate;
    nextstate.board = inputstate.board | tileOnBoard;
    
    // copy the board
    __int128 boardRow = nextstate.board;
    __int128 boardCol = nextstate.board;
    
    #ifdef TIMED_ACT
    times[6] += clock() - t6;
    
    clock_t t7 = clock();
    #endif
    // check the completed rows and columns, then remove them
    int ncomplete = 0;
    for (int i = row*SIZE; i < (row+tileHeight)*SIZE; i+=SIZE) {
        if (((nextstate.board >> i) & rowMask) == rowMask) {
            ++ncomplete;
            boardRow ^= (rowMask << i);
        }
    }
    for (int j = col; j < (col+tileWidth); ++j) {
        if (((nextstate.board >> j) & colMask) == colMask) {
            ++ncomplete;
            boardCol ^= (colMask << j);
        }
    }
    #ifdef TIMED_ACT
    times[7] += clock() - t7;
    
    clock_t t8 = clock();
    #endif
    
    // copy back to the nextstate board
    nextstate.board = boardRow & boardCol;
    
    #ifdef TIMED_ACT
    times[8] += clock() - t8;
    
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

vector<State> Game1010::getAllStates(State afterstate) {
    vector<State> allStates;
    allStates.resize(NUM_TILES);
    
    // the board for the next state is the same with afterstate, but the tile is different
    for (int i = 0; i < NUM_TILES; ++i) {
        State ss = afterstate;
        ss.tile = i;
        allStates[i] = ss;
    }
    
    return allStates;
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
    
    int hspaceSIZE = (SIZE-tilesHeight[inputstate.tile])*SIZE;
    int wspace = SIZE-tilesWidth[inputstate.tile];
    
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
    
    // check for every possible actions
    long boardInt64;
    long tileInt64 = tilesInt[inputstate.tile];
    for (int row = 0; row <= hspaceSIZE; row+=SIZE) {
        
        // cast the lsb 64 bits to long integer to speed up
        boardInt64 = static_cast<long>(inputstate.board >> row);
        for (int col = 0; col <= wspace; ++col) {
            
            // if the tile positions are all zeros, then cast it as a valid action
            if (!(boardInt64 & tileInt64)) validActions[nActions++] = row + col;
            boardInt64 >>= 1;
        }
    }
    
    validActions.resize(nActions);
    return validActions;
}

// check if it's terminated (if there's no possible action)
bool Game1010::isTerminated(State inputstate) {
    // extract the tile information
    int tileWidth = tilesWidth[inputstate.tile];
    long tileInt = tilesInt[inputstate.tile];
    
    int hspace = SIZE-tilesHeight[inputstate.tile];
    int wspace = SIZE-tileWidth;
    
    // check all possible actions
    __int128 boardInt128 = inputstate.board;
    unsigned long boardInt64;
    for (int row = 0; row <= hspace; ++row) {
        
        // cast the lsb 64 bits to long integer to speed up
        boardInt64 = static_cast<unsigned long>(boardInt128);
        for (int col = 0; col <= wspace; ++col) {
            
            // for a certain action, check if the tiles are blank
            if (!(boardInt64 & tileInt)) return false;
            boardInt64 >>= 1;
        }
        boardInt128 >>= SIZE;
    }
    return true;
}

// restart the game: clear the board, assign a random tile, and reset the score
void Game1010::restart() {
    state.board = 0;
    state.tile = rand() % NUM_TILES; // generate random number between 0 and NUM_TILES-1
    
    // reset the score
    score = 0;
}

void Game1010::render() {
    // print the score
    printf("Score: %d\n", score);
    
    // print the board
    __int128 board = state.board;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board & 1) printf("#");
            else printf(".");
            board >>= 1;
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


void play(bool random=false, bool wait=true, bool render=true) {
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

void simulate(int N) {
    
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
    // play(false, true, true);
    simulate(100000);
}































