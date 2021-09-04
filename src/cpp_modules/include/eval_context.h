#include "types.h"
#include <math.h>

const EvalContext DEBUG_CONTEXT = {
  /* aiMode= */ STANDARD,
  /* fastEvalWeights= */ MAIN_WEIGHTS,
  /* pieceRangeContext= */ {},
  /* countWellHoles= */ false,
  /* maxDirtyTetrisHeight= */ 1,
  /* maxSafeCol9Height= */ 6,
  /* scareHeight= */ 5,
  /* shouldRewardLineClears= */ false,
  /* wellColumn= */ 9,
};

int isLineout(GameState gameState){
  return false;
  // return gameState.level >= 29;
}

int hasHoleBlockingTetrisReady(int board[20], int col10Height){
  if (col10Height > 16) {
    return 0;
  }
  // Check that the four rows where a right well tetris would happen are all full except col 10
  for (int r = 0; r <= 4; r++) {
    if (board[19 - col10Height - r] & ALL_HOLE_BITS) {
      return true;
    }
  }
  return false;
}

/** Gets the number of holes that are holes and not tuck setups*/
int getNumTrueHoles(float adjustedNumHoles){
  while (std::abs(adjustedNumHoles - round(adjustedNumHoles)) > __FLT_EPSILON__){
    adjustedNumHoles -= TUCK_SETUP_HOLE_PROPORTION;
  }
  return (int) adjustedNumHoles;
}

AiMode getAiMode(GameState gameState, int max5TapHeight) {
  if (max5TapHeight < 4) {
    return LINEOUT;
  }
  if (gameState.lines > 220 && gameState.level < 29){
    if (hasHoleBlockingTetrisReady(gameState.board, gameState.surfaceArray[9])){
      return DIRTY_NEAR_KILLSCREEN;
    }
    return NEAR_KILLSCREEN;
  }
  if (getNumTrueHoles(gameState.adjustedNumHoles) >= 1) {
    return DIG;
  }
  return STANDARD;
}

const EvalContext getEvalContext(GameState gameState, const PieceRangeContext pieceRangeContextLookup[]){
  EvalContext context = {};
  
  // Copy the piece range context from the global lookup
  context.pieceRangeContext = pieceRangeContextLookup[getGravity(gameState.level) - 1];
  
  // Set the mode
  AiMode aiMode = getAiMode(gameState, context.pieceRangeContext.max5TapHeight);
  context.aiMode = aiMode;
  context.weights = getWeights(context.aiMode);
  
  // Set the scare heights
  if (aiMode == LINEOUT) {
    context.scareHeight = 0;
    context.maxSafeCol9 = -1;
  } else {
    context.scareHeight = context.pieceRangeContext.max5TapHeight - 3;
    context.maxSafeCol9 = context.pieceRangeContext.max4TapHeight - 5;
  }

  // Set the well column
  if (aiMode == LINEOUT) {
    context.wellColumn = -1;
  } else {
    context.wellColumn = 9;
  }
  
  // Misc other properties
  context.maxDirtyTetrisHeight = 0;
  context.countWellHoles = context.aiMode == DIG;
  context.shouldRewardLineClears = (aiMode == LINEOUT || aiMode == DIRTY_NEAR_KILLSCREEN);
  
  return context;
}
