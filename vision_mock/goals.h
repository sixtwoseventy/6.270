#ifndef GOALS_H
#define GOALS_H

#include "vision_mock1.h"

void checkGoals(int x, int y);
CvPoint getGoal();
void resetRound(int randomSeed);
int getScore();

#endif
