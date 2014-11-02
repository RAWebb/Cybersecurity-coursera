#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 

char greeting[] = "Hello there\n1. Receive wisdom\n2. Add wisdom\nSelection >";
char prompt[] = "Enter some wisdom\n";
char pat[] = "Achievement unlocked!\n";
char secret[] = "secret key";

int infd = 0; /* stdin */            \\ Not sure why these are stdin & stdout, but oh well
int outfd = 1; /* stdout */

#define DATA_SIZE 128

typedef struct _WisdomList {         \\ Define the structure.  Don't quite get this, a nested structure with a wisdomlist in a wisdomlist?
  struct  _WisdomList *next;         \\  *next??? maybe points to where the next line of wisdom is in memory???
  char    data[DATA_SIZE];           \\  data = 128 char, probably a hole here if we enter more than 128 char.
} WisdomList; 

struct _WisdomList  *head = NULL;          \\ Create the structure.  ahhh, head is not a C keyword, defined here.  This defines a structure called _WisdomList 
                                            \\ which contains only the pointer head

typedef void (*fptr)(void);                  \\ pointer to a function.  Function returns void (1st void) and funciton takes no arguements (2nd void) 
                                            \\ ahhh, wondering what fptr was below, here it is.  So far just a typedef though

void write_secret(void) {
  write(outfd, secret, sizeof(secret));    \\ will write secret if I can access this function
  return;
}

void pat_on_back(void) {
  write(outfd, pat, sizeof(pat));          \\ will write "Achi ..."  if I access this function
  return;
}

void get_wisdom(void) {
  char buf[] = "no wisdom\n";           \\ store message in case of no wisdom
  if(head == NULL) {
    write(outfd, buf, sizeof(buf)-sizeof(char));     \\don't know what head is refernencing, but somehow it's the wisdom list.  If null, print message
  } else {
    WisdomList  *l = head;          \\ putting data at the *l pointer into head
    while(l != NULL) {                \\ operates while l is not NULL
      write(outfd, l->data, strlen(l->data));        \\don't know what the ->data  means yet.  But somehow this is outputting the stored data (wisdom)
      write(outfd, "\n", 1);                          \\ to screen
      l = l->next;                      \\don't know what the ->next  means yet.
    }
  }
  return;
}

void put_wisdom(void) {
  char  wis[DATA_SIZE] = {0};       \\ DATA_SIZE previously defined as 128 above.
  int   r;

  r = write(outfd, prompt, sizeof(prompt)-sizeof(char));
  if(r < 0) {
    return;
  }
 
  r = (int)gets(wis);            \\ not sure why int, might be to make attack posible?
  if (r == 0)
    return;

  WisdomList  *l = malloc(sizeof(WisdomList));

  if(l != NULL) {
    memset(l, 0, sizeof(WisdomList));
    strcpy(l->data, wis);                \\ I think this basically reads existing wisdom into data using l pointer which is where malloc has stored previous wisdom ???
    if(head == NULL) {
      head = l;
    } else {
      WisdomList  *v = head;             \\ don't quite follow this whole part, but somehow storing each wisdom entry
      while(v->next != NULL) {
        v = v->next;
      }
      v->next = l;
    }
  }

  return;
}

fptr  ptrs[3] = { NULL, get_wisdom, put_wisdom };       \\ function pointers to get & put, but why NULL?, maybe 1st index in array is ptrs[0]
                                                        \\ and want to correspond to options 1 & 2 but could also add 1 to the read in value
														\\ this variable not part of any sub-functions, it's defined for main.  It could have
														\\ been defined at top w/ all chars, not sure why it wasn't.  May have been to get it in 
														\\ memory above the funct definitions so they wouldn't get overwritten by the overflow
														\\ nope - checked this, all vars are above functions

int main(int argc, char *argv[]) {                   \\ here's the start, returns int takes int input into argc

  while(1) {                                    \\ hmm, why a while??  and (1) always true, weird.
      char  buf[1024] = {0};                     \\ buf has 1024 char
      int r;
      fptr p &= pat_on_back;                       \\this is a slightly hokey injection.  puts a function pointer to hidden function into stack memory
	                                               \\ one of the exploits is to index the ptrs array (which stores pointers to get & put functions) 
												   \\ and find the (very big) index number which accesses memory that doesn't belong to actually belong to fptr
												   \\ and instead uses the pointer at p to run the hidden pat-on-back function
												   
												   
      r = write(outfd, greeting, sizeof(greeting)-sizeof(char));
      if(r < 0) {
        break;
      }
      r = read(infd, buf, sizeof(buf)-sizeof(char));     \\r seems to be a temp variable which is used all over for misc stuff.
	                                                     \\ I think they formatted like this (r = read()), to conserve space so they didn't have to 
														 \\ write the read function serveral times
      if(r > 0) {                      \\tests for negative (maybe this just determines if it exists? maybe read just returns 1 or 0 and this goes into r?
	                                   \\but not other out-of-range values, it is supposed to be 1 or 2
        buf[r] = '\0';                  \\ why is this necessary?
        int s = atoi(buf);           \\turn buf (array of char, but supposed to be just 1 or 2) into an int
        fptr tmp = ptrs[s];           \\ set function pointer tmp ptrs[1] or ptrs[2] or, if hacked, ptrs[big#] which points to hidden funct 
        tmp();                        \\ run the funct, either put or get_wisdom ...  or hidden pat_on_back
      } else {
        break;
      }
  }

  return 0;
}
