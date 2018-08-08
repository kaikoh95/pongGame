/** @file ball.c
    @author K. Koh, M. Fenwick UC
    @date 17 October 2017
    @brief The module to control the ball, "pong"
*/

#include "system.h"
#include "ball.h"
#include "display.h"

int ROW, COL, ROW_INC,COL_INC; //define global ints for row and column of ball position and row/col increment


//initiatialises ball
void ball_init (void)
{
    ROW = COL = 2;
    ROW_INC = COL_INC = 1;
    display_pixel_set (COL, ROW, 1); //light up ball location on LED matrix
    
}


//returns ball column (to be used in other modules)
int ball_col (void)
{
    return COL;
}


//returns ball row (to be used in other modules)
int ball_row (void)
{
    return COL;
}


//function for ball movement
void ball_move (void)
{
    display_pixel_set (COL, ROW, 0);
    
    //changes row/col location of ball
    COL += COL_INC;
    ROW += ROW_INC;
        
    if (ROW > 6 || ROW < 0) //if exceeds width of LED matrix
    {
        //reverses ball
        ROW -= ROW_INC * 2; 
        ROW_INC = -ROW_INC;
    }
        
    if (COL > 3) //if exceeds top length (paddle location) of LED matrix
    {
        //reverses ball movement
        COL -= COL_INC *2;
        COL_INC = -COL_INC;
    }
    
    display_pixel_set (COL, ROW, 1);
}
