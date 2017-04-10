import random
import math
import time
# import numpy as np

class Game1010:
    def __init__(self):
        self.size = 10
        self.numActions = 100
        self.tilesMask = [
            0b1,                        # 0
            0b11,                       # 1
            0b10000000001,              # 2
            0b111,                      # 3
            0b100000000010000000001,    # 4
            0b010000000011,             # 5
            0b100000000011,             # 6
            0b110000000010,             # 7
            0b110000000001,             # 8
            0b1000000000100000000010000000001, # 9
            0b1111,                     # 10
            0b110000000011,             # 11
            0b10000000001000000000100000000010000000001, # 12
            0b11111,                    # 13
            0b11100000000010000000001,  # 14
            0b11100000001000000000100,    # 15
            0b10000000001000000000111,  # 16
            0b00100000000010000000111,  # 17
            0b11100000001110000000111,  # 18
        ]
        self.tilesSize   = [1, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 5, 9]
        self.tilesWidth  = [1, 2, 1, 3, 1, 2, 2, 2, 2, 1, 4, 2, 1, 5, 3, 3, 3, 3, 3]
        self.tilesHeight = [1, 1, 2, 1, 3, 2, 2, 2, 2, 4, 1, 2, 5, 1, 3, 3, 3, 3, 3]
        self.tiles = [ # for render purpose only
            [[1]], # 1 tile                 # 0
            [[1,1]], # 2 tiles              # 1
            [[1],[1]],                      # 2
            [[1,1,1]], # 3 tiles            # 3
            [[1],[1],[1]],                  # 4
            [[0,1],[1,1]],                  # 5
            [[1,0],[1,1]],                  # 6
            [[1,1],[1,0]],                  # 7
            [[1,1],[0,1]],                  # 8
            [[1],[1],[1],[1]], # 4 tiles    # 9
            [[1,1,1,1]],                    # 10
            [[1,1],[1,1]],                  # 11
            [[1],[1],[1],[1],[1]], # 5 tiles# 12
            [[1,1,1,1,1]],                  # 13
            [[1,1,1],[0,0,1],[0,0,1]],      # 14
            [[1,1,1],[1,0,0],[1,0,0]],      # 15
            [[1,0,0],[1,0,0],[1,1,1]],      # 16
            [[0,0,1],[0,0,1],[1,1,1]],      # 17
            [[1,1,1],[1,1,1],[1,1,1]]       # 18
        ]
        self.numTiles = len(self.tilesHeight)
        
        # rowMask = 0b1111111111 << i*10
        # colMask = 0b1000000000100000000010000000001000000000100000000010000000001000000000100000000010000000001 << i
        self.rowMasks = [1023, 1047552, 1072693248, 1098437885952, 1124800395214848, 1151795604700004352, 1179438699212804456448L, 1207745227993911763402752L, 1236731113465765645724418048L, 1266412660188944021221804081152L]
        self.colMasks = [1239150146850664126585242625L, 2478300293701328253170485250L, 4956600587402656506340970500L, 9913201174805313012681941000L, 19826402349610626025363882000L, 39652804699221252050727764000L, 79305609398442504101455528000L, 158611218796885008202911056000L, 317222437593770016405822112000L, 634444875187540032811644224000L]
        
        self.restart()
    
    def restart(self):
        board = 0 # encoded by 10x10 = 100 bits
        tile = random.randrange(self.numTiles)
        self.state = (board << 5) + tile # additional 5 bits for the tile
        self.score = 0
        self.times = [0.0 for i in range(10)]
    
    def predictNextState(self, action, state=None): # return one possible state, reward, afterstate, and validity of the action
        # t0 = time.time()
        if state == None: state = self.state
        row = action // self.size
        col = action % self.size
        # self.times[0] += time.time() - t0
        
        # get the tile
        # t1 = time.time()
        tile = state & 0b11111
        board = (state >> 5)
        tileWidth = self.tilesWidth[tile]
        tileHeight = self.tilesHeight[tile]
        # self.times[1] += time.time() - t1
        
        # t8 = time.time()
        if (row < 0) or (col < 0) or (row > self.size - tileHeight) or (col > self.size - tileWidth):
            return (state, 0, state, 0)
        
        tileOnBoard = (self.tilesMask[tile] << ((self.size-tileWidth-col)+(self.size-tileHeight-row)*10))
        if (board & tileOnBoard) != 0:
            return (state, 0, state, 0)
        
        # self.times[8] += time.time() - t8
        
        
        # put the tile on the board
        # t2 = time.time()
        nextBoard = board | tileOnBoard
        nextBoard2 = nextBoard # make a copy of the next board
        # self.times[2] += time.time() - t2
        
        # check the completed rows or cols only on the filled rows and cols
        # t3 = time.time()
        completedRowsCols = 0
        for r in self.rowMasks[self.size-(row+tileHeight):self.size-row]:
            if (nextBoard & r) == r:
                nextBoard2 ^= r
                completedRowsCols += 1
        # self.times[3] += time.time() - t3
        
        # t4 = time.time()
        for c in self.colMasks[self.size-(col+tileWidth):self.size-col]:
            if (nextBoard & c == c):
                nextBoard ^= c
                completedRowsCols += 1
        # self.times[4] += time.time() - t4
        
        # remove the completed rows and columns
        # t5 = time.time()
        nextBoard &= nextBoard2
        
        # calculate the reward
        reward = self.tilesSize[tile] + completedRowsCols * (completedRowsCols+1) * 5
        # self.times[5] += time.time() - t5
        
        # generate one the possible state
        # t6 = time.time()
        afterstate = (nextBoard << 5)
        # self.times[6] += time.time() - t6
        # t7 = time.time()
        nextState = afterstate + int(random.random()*self.numTiles)
        # self.times[7] += time.time() - t7
        
        return (nextState, reward, afterstate, 1)
    
    def getNextState(self, afterstate): #return one possible state, given an afterstate
        return afterstate + random.randrange(self.numTiles)
    
    def act(self, action):
        # generate a possible next state
        [nextState, reward, afterstate, valid] = self.predictNextState(action, self.state)
        
        # update the score and the state
        self.score += reward
        self.state = nextState
        
        return reward
        
    def isTerminated(self, state=None):
        if state == None: state = self.state
        
        tile = state & 0b11111
        board = (state >> 5)
        mask = self.tilesMask[tile]
        tileHeight = self.tilesHeight[tile]
        tileWidth = self.tilesWidth[tile]
        
        rowMask = (1 << (self.size * tileHeight)) - 1
        
        colArray = xrange(self.size-tileWidth+1)
        for row in xrange(self.size-tileHeight+1):
            
            board2 = int(board & rowMask)
            
            for col in colArray:
                if (board2 >> col) & mask == 0: return False
            
            board >>= self.size
        return True
    
    def getValidActions(self, state=None):
        if state == None: state = self.state
        
        validActions = []
        tile = state & 0b11111
        board = (state >> 5)
        mask = self.tilesMask[tile]
        tileWidth = self.tilesWidth[tile]
        tileHeight = self.tilesHeight[tile]
        
        # get the mask for the nrows (n = tileHeight)
        rowMask = (1 << (self.size * tileHeight)) - 1
        
        # save some variables for optimisation
        append = validActions.append
        colArray = xrange(self.size-tileWidth,-1,-1)
        
        # iterate for every possible rows and columns
        for row10 in xrange((self.size-tileHeight)*10,-1,-10):
            
            # get the nrows of the board
            board2 = int(board & rowMask)
            
            for col in colArray:
                if board2 & mask == 0: append(row10+col)
                board2 >>= 1
            
            board >>= self.size
        
        return validActions
    
    def render(self):
        board = self.state >> 5
        tile = self.state & 0b11111
        shift = self.size*self.size-1
        print('Score: %d' % self.score)
        
        # print the board
        s = ''
        for r in range(self.size):
            for c in range(self.size):
                s += '#' if (board >> shift) & 0b1 else '.'
                shift -= 1
            s += '\n'
        print(s)
        print('')
        
        # print the tile
        t = [['.' for i in range(self.size)] for j in range(5)]
        for i in range(len(self.tiles[tile])):
            for j in range(len(self.tiles[tile][i])):
                t[i][j] = '#' if self.tiles[tile][i][j] == 1 else '.'
        b = [''.join(tt) for tt in t]
        print('\n'.join(b))

# play the game
def main():
    game = Game1010()
    while not game.isTerminated():
        game.render()
        action = input("Your action: ")
        game.act(action)

# simulate and time
def simulate(N=100):
    game = Game1010()
    # times = [0 for i in range(4+len(game.times))]
    nsteps = 0
    
    # tt0 = time.time()
    for j in range(N):
        game.restart()
        while True:
            nsteps += 1
            # t0 = time.time()
            possibleActions = game.getValidActions()
            # t1 = time.time()
            action = possibleActions[random.randrange(len(possibleActions))]
            # t2 = time.time()
            game.act(action)
            # t3 = time.time()
            terminated = game.isTerminated()
            # t4 = time.time()
            
            # times[0] += t1 - t0
            # times[1] += t2 - t1
            # times[2] += t3 - t2
            # times[3] += t4 - t3
            if terminated: break
        
        # for i in range(len(game.times)): times[4+i] += game.times[i]
    # print("Finishes in %f s" % (time.time() - tt0))
    # print(times)
    # print(nsteps)

if __name__ == "__main__":
    simulate(10000)
    # main()