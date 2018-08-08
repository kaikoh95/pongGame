/** @file ball.h
    @author K. Koh, M. Fenwick UC
    @date 17 October 2017
    @brief This si the interface for the ball "pong"
*/

#ifndef BALL_H
#define BALL_H

void ball_init (void); //initalises ball position

void ball_move (void); //ball movement

int ball_col (void); //column location of ball

int ball_row (void); //row location of ball

#endif
