// Basic program skeleton for a Sketch File (.sk) Viewer
#include "displayfull.h"
#include "sketch.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Allocate memory for a drawing state and initialise it
state *newState() {
  state *new = malloc(sizeof(state));
  new->x = 0;
  new->y = 0;
  new->tx = 0;
  new->ty = 0;
  new->tool = 1;
  new->start = 0;
  new->data = 0;
  new->end = false;
  return new; 
}

// Release all memory associated with the drawing state
void freeState(state *s) {
  free(s);
}

// Extract an opcode from a byte (two most significant bits).
int getOpcode(byte b) {
  int n = b >> 6; 
  return n;
}

// Extract an operand (-32..31) from the rightmost 6 bits of a byte.
int getOperand(byte b) { 
  int n = b & 63; // sets most significant 2 bits to 0
  int check = n >> 5; // gets first 3 bits
  if(check == 1){ 
    n = n | 192; // sets the 2 most significant bits to 1
    return (int)(signed char)(n);
  } 
  return n;
}

// Execute the next byte of the command sequence.
void obey(display *d, state *s, byte op) {
  int opcode = getOpcode(op);
  int operand = getOperand(op);
  if(opcode == 2) {
    if(operand < 3) s->tool = operand;
    else{
      if(operand == 3) colour(d, s->data);
      else if(operand == 4) s->tx = s->data;
      else if(operand == 5) s->ty = s->data;
      s->data = 0;
    }
  } // tool
  else if(opcode == 0) { // update x
    s->tx += operand;
  }
  else if(opcode == 3) { // data
    s->data = (s->data << 6) | (operand & 63);
  }
  else if(opcode == 1) { // update y
    s->ty += operand;
    if(s->tool == 1) {
      line(d, s->x, s->y, s->tx, s->ty);
      s->data = 0;
    }
    else if(s->tool == 2) {
      block(d, s->x, s->y, s->tx - s->x, s->ty - s->y);
      s->data = 0;
    }
    s->x = s->tx;
    s->y = s->ty;
  }
}

// Draw a frame of the sketch file. For basic and intermediate sketch files
// this means drawing the full sketch whenever this function is called.
// For advanced sketch files this means drawing the current frame whenever
// this function is called.
bool processSketch(display *d, const char pressedKey, void *data) {
  if (data == NULL) return (pressedKey == 27);
  char *filename = getName(d);
  state *s = (state*) data;
  FILE *in = fopen(filename, "rb");
  byte b = fgetc(in);
  obey(d, s, b);
  while(! feof(in)){
    b = fgetc(in);
    obey(d, s, b);
  }
  fclose(in);
  show(d);
  s->x = 0;
  s->y = 0;
  s->tx = 0;
  s->ty = 0;
  s->tool = 1;
  return true;
    //TO DO: OPEN, PROCESS/DRAW A SKETCH FILE BYTE BY BYTE, THEN CLOSE IT
    //NOTE: CHECK DATA HAS BEEN INITIALISED... if (data == NULL) return (pressedKey == 27);
    //NOTE: TO GET ACCESS TO THE DRAWING STATE USE... state *s = (state*) data;
    //NOTE: TO GET THE FILENAME... char *filename = getName(d);
    //NOTE: DO NOT FORGET TO CALL show(d); AND TO RESET THE DRAWING STATE APART FROM
    //      THE 'START' FIELD AFTER CLOSING THE FILE
}

// View a sketch file in a 200x200 pixel window given the filename
void view(char *filename) {
  display *d = newDisplay(filename, 200, 200);
  state *s = newState();
  run(d, s, processSketch);
  freeState(s);
  freeDisplay(d);
}

// Include a main function only if we are not testing (make sketch),
// otherwise use the main function of the test.c file (make test).
#ifndef TESTING
int main(int n, char *args[n]) {
  if (n != 2) { // return usage hint if not exactly one argument
    printf("Use ./sketch file\n");
    exit(1);
  } else view(args[1]); // otherwise view sketch file in argument
  return 0;
}
#endif
