#include "keyboard.h"

unsigned int shift_status = 0;
unsigned int caps_status = 0;
unsigned int ctrl_status = 0;
unsigned int alt_status = 0;
int terminal_status[2] = {0};
unsigned int buffer_index[3] = {0};
unsigned char buffer[3][MAX_BUFFER];
int terminal = 0;
volatile unsigned int enter_flag[3] = {};

int rtc_freq = 1;



volatile int rtc_stop = 1;


//osdev source

//if none of caps lock and shift is KEY
static uint8_t keyboard_case_zero[KEYBOARD_INPUT] =
   {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l' , ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm',',', '.', '/', '\0', '*', '\0', ' ', '\0'};
//if only shift is pressed
static uint8_t keyboard_case_one[KEYBOARD_INPUT] =
   {'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L' , ':', '"', '~', '\0', '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0'};

// if only caps lock is pressed
static uint8_t keyboard_case_two[KEYBOARD_INPUT] =
   {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L' , ';', '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', '\0', '*', '\0', ' ', '\0'};

// if both caps lock and shift is pressed
static uint8_t keyboard_case_three[KEYBOARD_INPUT] =
   {'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l' , ':', '"', '~', '\0', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', '<', '>', '?', '\0', '*', '\0', ' ', '\0'};

/*
 * void reset_buffer(uint9_t *buffer_array)
 * input: pointer of buffer
 * output: none
 * result : clear all the value of buffer by initialize to '\0'
 */
void reset_buffer(unsigned char* buffer_array)
{
  int i;
    /* clear buffer */
    for(i = 0; i < MAX_BUFFER; i++) {
      buffer_array[i] = '\0';
    }
}

/*
 * void keyboard_handler()
 * input: none
 * output: none
 * result : take input value from port and print out scanned value to terminal
 */
void keyboard_handler()
{
  cli();
  int ter = cursor_on_terminal - 1;
  unsigned char keyboard_input = inb(IOPORT);
  //check if input is shift key
  if(keyboard_input == LEFT_SHIFT || keyboard_input == RIGHT_SHIFT)
    shift_status = 1;
  else if (keyboard_input == LEFT_SHIFT_OFF || keyboard_input == RIGHT_SHIFT_OFF)
    shift_status = 0;

  if(keyboard_input == CTRL_KEY)
    ctrl_status = 1;
  else if(keyboard_input == CTRL_OFF)
    ctrl_status = 0;

  if(keyboard_input == ALT_KEY)
    alt_status = 1;
  else if (keyboard_input == ALT_OFF)
    alt_status = 0;

  //check if input is capslock key
  if(keyboard_input == CAPS_KEY){
    caps_status = !(caps_status);
  }

  //interpret input value to appropriate keyboard value
  uint8_t interpretted = keyboard_interpret(keyboard_input);

  //Ctrl+l
  if((ctrl_status == 1 ) && ((interpretted == 'l') || (interpretted == 'L')))
  {
    //set cursur to (0,0) and clear buffer and clear the screen and reinitialize buffer index
    putc('\n');
    reset_buffer(buffer[cursor_on_terminal - 1]);
    buffer_index[cursor_on_terminal - 1] = 0;
    clear();
  }

  else if((ctrl_status == 1 ) && (interpretted == '4'))
  {
    // start RTC test and change frequency of RTC by pressing again
    cli();
    // set the rtc flag to zero
    rtc_stop = 0;
    if (rtc_freq == MAX_RTC){
      rtc_freq = 1;
    }

    rtc_freq = rtc_freq * 2;

    write_rtc(rtc_freq);
    
  }

  else if((ctrl_status == 1 )&& (interpretted == '5'))
  {
    //stop RTC test
    rtc_freq = 1;

    rtc_freq = 1;
    write_rtc(2);
    rtc_stop = 1;
  }
  //functioanlity key
    if ((alt_status == 1) && ((keyboard_input == F1_KEY) || (keyboard_input == F2_KEY) || (keyboard_input == F3_KEY))){
        switch(keyboard_input){
          case F1_KEY:
          terminal = 1;
          break;
          case F2_KEY:
          terminal = 2;
          break;
          case F3_KEY:
          terminal = 3;
          break;
      }

    if (cursor_on_terminal != terminal) {
      next_terminal(terminal, cursor_on_terminal);
      cursor_on_terminal = terminal;
      sched_terminal = terminal;
      relocate_cursor();
      if ((terminal != 1) && (terminal_status[terminal-2] == 0)) {
        terminal_status[terminal-2] = 1;
        base_flag = 1;
        send_eoi(1);
        execute((uint8_t*)"shell");
      }
    }
  }
  //enter key
  else if(keyboard_input == ENTER_KEY)
  {
    //clear buffer and reinitialize buffer_index

    enter_flag[ter] = 0;

    putc('\n');
    /*reset_buffer(buffer);*/
    /*buffer_index = 0;*/
  }
  //check if input is in valid range (when KEY)
  else if (keyboard_input < KEYBOARD_INPUT && keyboard_input != LEFT_SHIFT && keyboard_input != RIGHT_SHIFT && keyboard_input != ENTER_KEY) {
    //back space
    if (keyboard_input == BACKSPACE_KEY)
    {
      //only if there is at least one character in the buffer
      if(buffer_index[cursor_on_terminal - 1] > 0 )
      {
        buffer_index[cursor_on_terminal - 1] --;
        buffer[cursor_on_terminal - 1][buffer_index[cursor_on_terminal - 1]] = '\0';
        putc('\b');
      }
    }
    else
    {
      //only if buffer index is less than buffer size, there can be appended character on the screen and buffer
        if((buffer_index[cursor_on_terminal - 1] < MAX_BUFFER) && (ctrl_status ==0) && (interpretted != '\0'))
        {
          buffer[cursor_on_terminal - 1][buffer_index[cursor_on_terminal - 1]] = interpretted;
          buffer_index[cursor_on_terminal - 1] ++;
          putc(interpretted);
        }
    }
  }
  send_eoi(1);
  sti();
}


/*
 * void keyboard_init()
 * input: none
 * output: none
 * result : initialize keyboard interrupt
 */
void keyboard_init() {
  enable_irq(1);
}




/*
 * void keyboard_interpret(unsigned char input)
 * input: input from IOport
 * output: actual character
 * result : display the character
 */
unsigned char keyboard_interpret(unsigned char input)
{
  unsigned char output;
  //if none of caps lock and shift is pressed
  if((shift_status == 0) && (caps_status ==0))
    output = keyboard_case_zero[input];
  //if only shift is pressed
  else if ((shift_status == 1 )&& (caps_status == 0))
    output = keyboard_case_one[input];
  // if only caps lock is pressed
  else if ((shift_status == 0 )&& (caps_status == 1))
    output = keyboard_case_two[input];
  // if both caps lock and shift is pressed
  else if ((shift_status ==1 )&& (caps_status == 1))
    output = keyboard_case_three[input];

  return output;
}
