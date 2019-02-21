/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab
 */

#include "lib.h"
#include "scheduling.h"
#define VIDEO 0xB8000
#define NUM_COLS 80
#define NUM_ROWS 25
#define ATTRIB 0x7
#define VIDEO_SIZE (NUM_COLS*NUM_ROWS)
#define BLUESCREEN 0x11
#define BUFF_SIZE 128
#define BITMASK 0xFF

static int screen_x[3];
static int screen_y[3];
static char* video_mem = (char *)VIDEO;

volatile int base_flag = 0;
volatile int sched_flag = 0;
volatile int cursor_on_terminal = 1;
volatile int cursor_past_terminal = 1;

/*
 * void clear()
 * input: none
 * output: none
 * result : clear screen and relocate cursor to (x,y) = (0,0)
 */
void
clear(void)
{
    int32_t i;
    int ter = cursor_on_terminal - 1;
    screen_x[ter] = 0;
    screen_y[ter] = 0;
    for(i=0; i<NUM_ROWS*NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }

    //relocate cursor location to (x,y)=(0,0)
    screen_x[ter] = 0;
    screen_y[ter] = 0;
    relocate_cursor();
}


/*
 * void realocate_cursor(void)
 * input: none
 * output: none
 * result : update position of cursor refer to value of screen_x and screen_y
 * source : http://wiki.osdev.org/Text_Mode_Cursor
 */
void
relocate_cursor(void)
{
	//calculate location
	unsigned short location = (screen_y[cursor_on_terminal-1] * NUM_COLS) + screen_x[cursor_on_terminal-1];

	//port data and address
	// cursor Low Port to VGA Index Register
	outb(0x0F,0x03D4);
  	outb((unsigned char)(location & BITMASK),0x03D5);

	// cursor High Port to VGA Index Register
  	outb(0x0E,0x03D4);
  	outb((unsigned char)((location >> 8) & BITMASK),0x03D5);

}

/*
 * void scroll_up(void)
 * input: none
 * output: none
 * result : scroll up the screen when screen_y is on the last line
 */
void scroll_up(void)
{
	int prev;
	int curr;
	unsigned int i,j;
	for(i = 0; i<NUM_ROWS-1; i++)
	{
		for(j=0; j<NUM_COLS; j++)
		{
			prev = (i+1)*NUM_COLS+j;
			curr = (i)*NUM_COLS+j;
			*(unsigned char *)(video_mem + (curr<<1)) = *(unsigned char *)(video_mem+(prev<<1));
		}
	}

	for(j=0; j<NUM_COLS; j++)
	{
		curr = (NUM_ROWS-1)*NUM_COLS+j;

		*(unsigned char *)(video_mem + (curr<<1)) = ' ';
		screen_x[sched_terminal-1] = 0;
		screen_y[sched_terminal-1] = NUM_ROWS-1;
	}


}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output.
 * */
int
printf(char *format, ...)
{
	/* Pointer to the format string */
	char* buf = format;

	/* Stack pointer for the other parameters */
	int* esp = (void *)&format;
	esp++;

	while(*buf != '\0') {
		switch(*buf) {
			case '%':
				{
					int alternate = 0;
					buf++;

format_char_switch:
					/* Conversion specifiers */
					switch(*buf) {
						/* Print a literal '%' character */
						case '%':
							putc('%');
							break;

						/* Use alternate formatting */
						case '#':
							alternate = 1;
							buf++;
							/* Yes, I know gotos are bad.  This is the
							 * most elegant and general way to do this,
							 * IMHO. */
							goto format_char_switch;

						/* Print a number in hexadecimal form */
						case 'x':
							{
								char conv_buf[64];
								if(alternate == 0) {
									itoa(*((unsigned int *)esp), conv_buf, 16);
									puts(conv_buf);
								} else {
									int starting_index;
									int i;
									itoa(*((unsigned int *)esp), &conv_buf[8], 16);
									i = starting_index = strlen(&conv_buf[8]);
									while(i < 8) {
										conv_buf[i] = '0';
										i++;
									}
									puts(&conv_buf[starting_index]);
								}
								esp++;
							}
							break;

						/* Print a number in unsigned int form */
						case 'u':
							{
								char conv_buf[36];
								itoa(*((unsigned int *)esp), conv_buf, 10);
								puts(conv_buf);
								esp++;
							}
							break;

						/* Print a number in signed int form */
						case 'd':
							{
								char conv_buf[36];
								int value = *((int *)esp);
								if(value < 0) {
									conv_buf[0] = '-';
									itoa(-value, &conv_buf[1], 10);
								} else {
									itoa(value, conv_buf, 10);
								}
								puts(conv_buf);
								esp++;
							}
							break;

						/* Print a single character */
						case 'c':
							putc( (unsigned char) *((int *)esp) );
							esp++;
							break;

						/* Print a NULL-terminated string */
						case 's':
							puts( *((char **)esp) );
							esp++;
							break;

						default:
							break;
					}

				}
				break;

			default:
				putc(*buf);
				break;
		}
		buf++;
	}

	return (buf - format);
}

/*
* int puts(char* s);
*   Inputs: int_8* s = pointer to a string of characters
*   Return Value: Number of bytes written
*	Function: Output a string to the console
*/

int
puts(char* s)
{
	register int index = 0;
	while(s[index] != '\0') {
		putc(s[index]);
		index++;
	}

	return index;
}

/*
* void putc(unsigned char c);
*   Inputs: uint_8* c = character to print
*   Return Value: void
*	Function: Output a character to the console depends on its c value
*/

void
putc(unsigned char c)
{

	cli();
	video_mem = (int8_t*)(VIDEO + 0x1000*cursor_on_terminal);
	//if keyboard input is new line or ctrl+L
  	int ter = cursor_on_terminal - 1;
  	/*ter = cursor_on_terminal - 1;*/
    if(c == '\n' || c == '\r') {
    	if(screen_y[ter] >= NUM_ROWS-1) {
    		// if row count is over 25 scroll up to more lines on the bottom of screen
    		scroll_up();
    	} else {
    		//if new line need to be added
    		//increase screen_y and stick x to zero
    		screen_y[ter]++;
    		screen_x[ter] = 0;
    	}

    } // if keyboard input is backspace
    else if(c == '\b') {
    	// if the character we have to erase is not null, decrement screen_x and fill it as '\0'
    	if(*(uint8_t *)(video_mem + ((NUM_COLS*screen_y[ter] + screen_x[ter]-1) << 1)) != '\0') {
    		*(uint8_t *)(video_mem + ((NUM_COLS*screen_y[ter] + screen_x[ter]-1) << 1)) = '\0';
    		if(screen_x[ter] == 0) {
    			screen_x[ter] = NUM_COLS-1;
    			screen_y[ter] --;
    		} else {
    			screen_x[ter]--;
    		}
    	}

        screen_x[ter] %= NUM_COLS;
        screen_y[ter] = (screen_y[ter] + (screen_x[ter] / NUM_COLS)) % NUM_ROWS;
    }
    else {
    	//if keyboard input is not functional and is character
    	//put character as right video memory
        *(uint8_t *)(video_mem + ((NUM_COLS*screen_y[ter] + screen_x[ter]) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS*screen_y[ter] + screen_x[ter]) << 1) + 1) = ATTRIB;

        screen_x[ter]++;

        //if new character is end of colomns and row is out of range -> scroll up
        if(screen_y[ter] == NUM_ROWS-1 && screen_x[ter] == NUM_COLS)
        {
        	scroll_up();
        }
        else if((screen_y[ter] < NUM_ROWS-1) && (screen_x[ter] == NUM_COLS))
        {
        	screen_y[ter]++;
        }
        screen_x[ter] %= NUM_COLS;
        screen_y[ter] = (screen_y[ter] + (screen_x[ter] / NUM_COLS)) % NUM_ROWS;
    }

    // upper functions chage screen_x and screen_y value. so we have to change cursor location
    relocate_cursor();
    sti();
}

/*
* char* itoa(unsigned int value, char* buf, int radix);
*   Inputs: unsigned int value = number to convert
*			char* buf = allocated buffer to place string in
*			int radix = base system. hex, oct, dec, etc.
*   Return Value: number of bytes written
*	Function: Convert a number to its ASCII representation, with base "radix"
*/

char*
itoa(unsigned int value, char* buf, int radix)
{
	static char lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	char *newbuf = buf;
	int i;
	unsigned int newval = value;

	/* Special case for zero */
	if(value == 0) {
		buf[0]='0';
		buf[1]='\0';
		return buf;
	}

	/* Go through the number one place value at a time, and add the
	 * correct digit to "newbuf".  We actually add characters to the
	 * ASCII string from lowest place value to highest, which is the
	 * opposite of how the number should be printed.  We'll reverse the
	 * characters later. */
	while(newval > 0) {
		i = newval % radix;
		*newbuf = lookup[i];
		newbuf++;
		newval /= radix;
	}

	/* Add a terminating NULL */
	*newbuf = '\0';

	/* Reverse the string and return */
	return strrev(buf);
}

/*
* char* strrev(char* s);
*   Inputs: char* s = string to reverse
*   Return Value: reversed string
*	Function: reverses a string s
*/

char*
strrev(char* s)
{
	register char tmp;
	register int beg=0;
	register int end=strlen(s) - 1;

	while(beg < end) {
		tmp = s[end];
		s[end] = s[beg];
		s[beg] = tmp;
		beg++;
		end--;
	}

	return s;
}

/*
* unsigned int strlen(const char* s);
*   Inputs: const char* s = string to take length of
*   Return Value: length of string s
*	Function: return length of string s
*/

unsigned int
strlen(const char* s)
{
	register unsigned int len = 0;
	while(s[len] != '\0')
		len++;

	return len;
}

/*
* void* memset(void* s, int c, unsigned int n);
*   Inputs: void* s = pointer to memory
*			int c = value to set memory to
*			unsigned int n = number of bytes to set
*   Return Value: new string
*	Function: set n consecutive bytes of pointer s to value c
*/

void*
memset(void* s, int c, unsigned int n)
{
	c &= 0xFF;
	asm volatile("                  \n\
			.memset_top:            \n\
			testl   %%ecx, %%ecx    \n\
			jz      .memset_done    \n\
			testl   $0x3, %%edi     \n\
			jz      .memset_aligned \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			subl    $1, %%ecx       \n\
			jmp     .memset_top     \n\
			.memset_aligned:        \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			movl    %%ecx, %%edx    \n\
			shrl    $2, %%ecx       \n\
			andl    $0x3, %%edx     \n\
			cld                     \n\
			rep     stosl           \n\
			.memset_bottom:         \n\
			testl   %%edx, %%edx    \n\
			jz      .memset_done    \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			subl    $1, %%edx       \n\
			jmp     .memset_bottom  \n\
			.memset_done:           \n\
			"
			:
			: "a"((c << (NUM_ROWS-1)) | (c << 16) | (c << 8) | c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/*
* void* memset_word(void* s, int c, unsigned int n);
*   Inputs: void* s = pointer to memory
*			int c = value to set memory to
*			unsigned int n = number of bytes to set
*   Return Value: new string
*	Function: set lower 16 bits of n consecutive memory locations of pointer s to value c
*/

/* Optimized memset_word */
void*
memset_word(void* s, int c, unsigned int n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			rep     stosw           \n\
			"
			:
			: "a"(c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/*
* void* memset_dword(void* s, int c, unsigned int n);
*   Inputs: void* s = pointer to memory
*			int c = value to set memory to
*			unsigned int n = number of bytes to set
*   Return Value: new string
*	Function: set n consecutive memory locations of pointer s to value c
*/

void*
memset_dword(void* s, int c, unsigned int n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			rep     stosl           \n\
			"
			:
			: "a"(c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/*
* void* memcpy(void* dest, const void* src, unsigned int n);
*   Inputs: void* dest = destination of copy
*			const void* src = source of copy
*			unsigned int n = number of byets to copy
*   Return Value: pointer to dest
*	Function: copy n bytes of src to dest
*/

void*
memcpy(void* dest, const void* src, unsigned int n)
{
	asm volatile("                  \n\
			.memcpy_top:            \n\
			testl   %%ecx, %%ecx    \n\
			jz      .memcpy_done    \n\
			testl   $0x3, %%edi     \n\
			jz      .memcpy_aligned \n\
			movb    (%%esi), %%al   \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			addl    $1, %%esi       \n\
			subl    $1, %%ecx       \n\
			jmp     .memcpy_top     \n\
			.memcpy_aligned:        \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			movl    %%ecx, %%edx    \n\
			shrl    $2, %%ecx       \n\
			andl    $0x3, %%edx     \n\
			cld                     \n\
			rep     movsl           \n\
			.memcpy_bottom:         \n\
			testl   %%edx, %%edx    \n\
			jz      .memcpy_done    \n\
			movb    (%%esi), %%al   \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			addl    $1, %%esi       \n\
			subl    $1, %%edx       \n\
			jmp     .memcpy_bottom  \n\
			.memcpy_done:           \n\
			"
			:
			: "S"(src), "D"(dest), "c"(n)
			: "eax", "edx", "memory", "cc"
			);

	return dest;
}

/*
* void* memmove(void* dest, const void* src, unsigned int n);
*   Inputs: void* dest = destination of move
*			const void* src = source of move
*			unsigned int n = number of byets to move
*   Return Value: pointer to dest
*	Function: move n bytes of src to dest
*/

/* Optimized memmove (used for overlapping memory areas) */
void*
memmove(void* dest, const void* src, unsigned int n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			cmp     %%edi, %%esi    \n\
			jae     .memmove_go     \n\
			leal    -1(%%esi, %%ecx), %%esi    \n\
			leal    -1(%%edi, %%ecx), %%edi    \n\
			std                     \n\
			.memmove_go:            \n\
			rep     movsb           \n\
			"
			:
			: "D"(dest), "S"(src), "c"(n)
			: "edx", "memory", "cc"
			);

	return dest;
}

/*
* int strncmp(const char* s1, const char* s2, unsigned int n)
*   Inputs: const char* s1 = first string to compare
*			const char* s2 = second string to compare
*			unsigned int n = number of bytes to compare
*	Return Value: A zero value indicates that the characters compared
*					in both strings form the same string.
*				A value greater than zero indicates that the first
*					character that does not match has a greater value
*					in str1 than in str2; And a value less than zero
*					indicates the opposite.
*	Function: compares string 1 and string 2 for equality
*/

int
strncmp(const char* s1, const char* s2, unsigned int n)
{
	int i;
	for(i=0; i<n; i++) {
		if( (s1[i] != s2[i]) ||
				(s1[i] == '\0') /* || s2[i] == '\0' */ ) {

			/* The s2[i] == '\0' is unnecessary because of the short-circuit
			 * semantics of 'if' expressions in C.  If the first expression
			 * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
			 * s2[i], then we only need to test either s1[i] or s2[i] for
			 * '\0', since we know they are equal. */

			return s1[i] - s2[i];
		}
	}
	return 0;
}

/*
* char* strcpy(char* dest, const char* src)
*   Inputs: char* dest = destination string of copy
*			const char* src = source string of copy
*   Return Value: pointer to dest
*	Function: copy the source string into the destination string
*/

char*
strcpy(char* dest, const char* src)
{
	int i=0;
	while(src[i] != '\0') {
		dest[i] = src[i];
		i++;
	}

	dest[i] = '\0';
	return dest;
}

/*
* char* strcpy(char* dest, const char* src, unsigned int n)
*   Inputs: char* dest = destination string of copy
*			const char* src = source string of copy
*			unsigned int n = number of bytes to copy
*   Return Value: pointer to dest
*	Function: copy n bytes of the source string into the destination string
*/

char*
strncpy(char* dest, const char* src, unsigned int n)
{
	int i=0;
	while(src[i] != '\0' && i < n) {
		dest[i] = src[i];
		i++;
	}

	while(i < n) {
		dest[i] = '\0';
		i++;
	}

	return dest;
}

/*
* void test_interrupts(void)
*   Inputs: void
*   Return Value: void
*	Function: increments video memory. To be used to test rtc
*/

void
test_interrupts(void)
{
	int i;
	for (i=0; i < NUM_ROWS*NUM_COLS; i++) {
		video_mem[i<<1]++;
	}
	// printf("test");
}

