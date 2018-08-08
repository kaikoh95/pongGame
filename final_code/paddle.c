/** @file paddle.c
    @author K. Koh, M. Fenwick UC
    @date 17 October 2017
    @brief The module to control the paddle
*/

#include "system.h"
#include "paddle.h"
#include "display.h"

int PAD_ROW_LEFT, PAD_ROW_RIGHT, PAD_COL;


//initialises paddle
void paddle_init (void) 
{
    PAD_ROW_LEFT = 3;
    PAD_ROW_RIGHT = 2;
    PAD_COL = 4;
    
    //sets left and right LEDs for paddle
    display_pixel_set (PAD_COL, PAD_ROW_LEFT, 1);
    display_pixel_set (PAD_COL, PAD_ROW_RIGHT, 1); 
}


//function to move paddle left
void move_paddle_left (void)
{
    if (PAD_ROW_LEFT < 6) //if paddle within left bound of LED matrix width
    {
        display_pixel_set(PAD_COL, PAD_ROW_RIGHT, 0); 
        
        //re-configures new position of paddle
        PAD_ROW_LEFT++;
        PAD_ROW_RIGHT++;

        display_pixel_set(PAD_COL, PAD_ROW_LEFT, 1);
    }
}


//function to move paddle right
void move_paddle_right (void)
{
    if (PAD_ROW_RIGHT > 0) //if paddle within right bound of LED matrix width
    {
        display_pixel_set(PAD_COL, PAD_ROW_LEFT, 0);

        //re-configures new position of paddle
        PAD_ROW_RIGHT--;
        PAD_ROW_LEFT--;

        display_pixel_set(PAD_COL, PAD_ROW_RIGHT, 1);
    }
}
