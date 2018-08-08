/** @file game.c
    @author K. Koh, M. Fenwick UC
    @date 17 October 2017
    @brief A simple pong game between two ucfk4 kits.
*/

#include "system.h"
#include "pacer.h"
#include "paddle.h"
#include "navswitch.h"
#include "pio.h"
#include "display.h"
#include "ledmat.h"
#include "task.h"
#include "ball.h"
#include "tinygl.h"
#include "../fonts/font3x5_1.h"
#include "ir_uart.h"
#include "usart1.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define BALL_RATE 5
#define PADDLE_RATE 100
#define UPDATE_RATE 250
#define MESSAGE_RATE 10
#define LOOP_RATE 300
#define NAV_RATE 20
#define GAME_RATE 1


//states for program to switch between
typedef enum {STATE_START, STATE_SELECT, STATE_READY,
              STATE_PLAYING, STATE_RECEIVING, STATE_OVER} state_t;


static state_t state = STATE_START; //default state of program


char* display_message; //text variable for storing text displayed on tinygl 


//to capture text from tinygl
void set_message (char* message)
{
    display_message = message;
}


//retrieves text from tinygl
char* get_message (void)
{
    return display_message;
}


//function to allow user to select between player 1 and 2 using navswitch
void switch_player (void)
{
    if (strcmp(get_message(), "SELECT PLAYER") == 0) //if tinygl displays SELECT PLAYER
    {
        set_message("P1");
        tinygl_clear ();
        tinygl_update ();
        tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
        tinygl_text ("P1");
    }

    else if (strcmp(get_message(), "P1") == 0) //if it is at P1, switch
    {
        set_message("P2");
        tinygl_clear ();
        tinygl_update ();
        tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
        tinygl_text ("P2");
    }

    else if (strcmp(get_message(), "P2") == 0) //if it is at P2, switch
    {
        set_message("P1");
        tinygl_clear ();
        tinygl_update ();
        tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
        tinygl_text ("P1");
    }
}  


//task to initialise paddle 
static void paddle_init_task (void) 
{
    paddle_init ();
}


//task to initialise led matrix
static void led_init_task (void)
{
    ledmat_init ();
}


//task to initialise ball
static void ball_init_task (void)
{
    ball_init ();
}


//task to update display
static void display_refresh (__unused__ void *data)
{
    display_update ();
}


//task to allow user to move paddle
static void paddle_task (void)
{
    navswitch_update();

    if (navswitch_push_event_p (NAVSWITCH_SOUTH)) //if left-relative direction is pressed
    {
        move_paddle_left ();
    }

    else if (navswitch_push_event_p (NAVSWITCH_NORTH)) //if right-relative direction is pressed
    {
        move_paddle_right ();
    }

}


//ball task that moves the ball and sends it over to next player's kit if over top bound
static void ball_task (void)
{   
    ball_move ();

    if (ball_col() < 0 && state == STATE_PLAYING) //if current user in PLAYING state and ball exceeds top bound
    {
        ir_uart_putc('.'); //sends a specific character so receiver can check and only responds if same character received
        state = STATE_RECEIVING; //changes state to receiving

    }
}


//game task to handle different states of the game
static void game_task (__unused__ void *data)
{
    //captures ball location
    int b_col = ball_col(); 
    int b_row = ball_row();    

    switch (state)
    {      
        //Home Screen
        case STATE_START:

            navswitch_init (); //initialises navswitch to be used subsequently
            pacer_init (LOOP_RATE); //to be used to keep tinygl text in time

            //initialises tinygl
            tinygl_init (LOOP_RATE);
            tinygl_font_set (&font3x5_1);
            tinygl_text_speed_set (MESSAGE_RATE);
            tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
            tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);

            tinygl_text ("WELCOME TO GAME"); //text to be displayed

            while (1)
            {
                pacer_wait ();
                
                //necessary updates
                tinygl_update ();
                navswitch_update ();

                if (navswitch_push_event_p (NAVSWITCH_PUSH)) //if navswitch button pressed 
                {
                    tinygl_clear();
                    tinygl_update ();
                    state = STATE_SELECT; //moves into next state of game, ie selecting player
                    break;
                }

            }
        
        //Selecting Player Screen
        case STATE_SELECT:

            tinygl_text ("SELECT PLAYER");
            set_message("SELECT PLAYER"); //captures text displayed using tinygl
            pacer_init (LOOP_RATE);
            
            while (1)
            {
                pacer_wait ();
                tinygl_update ();
                navswitch_update ();
                
                //if user pushes left- or right-relative button on navswitch
                if (navswitch_push_event_p (NAVSWITCH_SOUTH) || navswitch_push_event_p (NAVSWITCH_NORTH)) 
                {
                    switch_player();
                }
                
                //selects player 1 or 2 accordingly if user presses navswitch button
                else if (navswitch_push_event_p (NAVSWITCH_PUSH))
                {
                    if (strcmp(get_message(), "P1") == 0) //if player 1 selected
                    {
                        tinygl_clear();
                        tinygl_update ();
                        display_clear ();
                        state = STATE_READY; //go into ready to send ball state
                        break;
                    }
                    else if (strcmp(get_message(), "P2") == 0) //if player 2
                    {
                        tinygl_clear ();
                        tinygl_update ();
                        display_clear ();
                        state = STATE_RECEIVING; //go into ready to receive ball state
                        break;
                    }
                }
                
            }

        //Ready (to send ball) Screen
        case STATE_READY:

            //clears LED display and sets up necessary inits
            display_clear ();
            led_init_task (); 
            paddle_init_task ();
            ball_init_task ();

        //Playing (or Sending) Screen
        case STATE_PLAYING:

            ir_uart_init (); //initialises IR
            paddle_task(); //allows user to move paddle
            ball_task(); //allows for ball movement and sending

        //Receiving Screen
        case STATE_RECEIVING:

            display_update();
            paddle_task();
            ir_uart_init();

            if (ir_uart_read_ready_p ()) //if IR ready to receive
            {
                if (ir_uart_getc() == '.') //if received char same as sent char
                {   
                    //move the ball towards user's paddle
                    display_pixel_set (b_col, b_row, 1);
                    display_pixel_set (b_col, b_row, 0);
 
                    b_col--;
                    b_row--;

                    display_pixel_set (b_col, b_row, 1);

                    //go into playing state to bounce and send ball over
                    state = STATE_PLAYING; 
                } 
           }
        default:
            break;           
    }

}


//navswitch task for different states of game
static void navswitch_task (__unused__ void *data)
{
    //initialises navswitch if not already
    static bool init = 0;

    if (!init)
    {
        navswitch_init ();
        init = 1;
    }

    navswitch_update ();
    
    //if navswitch button pressed, changes current state to next state accordingly
    if (navswitch_push_event_p (NAVSWITCH_PUSH))
    {
        switch (state)
        {
            case STATE_START:
                state = STATE_SELECT;
                break;
                
            case STATE_READY:
                state = STATE_PLAYING;
                break;
                
            case STATE_OVER:
                state = STATE_START;
                break;
            
            default:
                break;
        }
    }
    
    //if right-relative button is pushed
    if (navswitch_push_event_p (NAVSWITCH_NORTH))
    {
        switch (state)
        {   
            //moves paddle right if at playing/receiving states, do nothing otherwise         
            case STATE_PLAYING:
                move_paddle_right ();
                break;
                
            case STATE_RECEIVING:
                move_paddle_right ();
                break;
                
            default:
                break;
        }
    }
    
    //if left-relative button is pushed
    if (navswitch_push_event_p (NAVSWITCH_SOUTH))
    {
        switch (state)
        {
            //moves paddle left or do nothing otherwise
            case STATE_PLAYING:
                move_paddle_left ();
                break;
            
            case STATE_RECEIVING:
                move_paddle_left ();
                break;
                
            default:
                break;
        }
    }
}


//main function to run the game
int main (void)
{  
    task_t tasks[] =
    {
        {.func = display_refresh, .period = TASK_RATE / UPDATE_RATE},
        {.func = navswitch_task, .period = TASK_RATE / NAV_RATE},
        {.func = game_task, .period = TASK_RATE / GAME_RATE},
        
    };

    system_init ();

    task_schedule (tasks, ARRAY_SIZE (tasks));
    
    return 0;
}
  
