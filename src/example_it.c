/**
 * @file example_it.c
 * Example of how to work with interval trees over an array.
 *
 * @author Jose Fernando Zazo (www.github.com/jfzazo)
 * @date 18/02/2016
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "interval_tree.h"

// Implementation source algorithm http://www.geeksforgeeks.org/interval-tree
// Rotation aspects. Check https://en.wikipedia.org/wiki/Tree_rotation

#define POINTER_TO_INT(p) (unsigned int)((uint64_t)(p))
#define INT_TO_POINTER(i) (void *)((uint64_t)(i))

int main()
{
  interval_tree_t *intervalt = interval_tree_new(8);
  void *id;
  range_t r;
  r.inf = 0; r.sup = 20;
  id = INT_TO_POINTER(0x69); // We could use a structure but it also can be an integer encapsulated as a pointer
  int i;
  for (i = 0; i < 8; i++) {
    printf("Inserting interval [%ld,%ld] with id %X\n", r.inf, r.sup, POINTER_TO_INT(id));
    interval_tree_insert(intervalt, &r, id);
    r.inf = r.sup + 1;
    r.sup = r.sup + 20;
    id = INT_TO_POINTER(POINTER_TO_INT(id) + 1);
  }

  for (i = 0, id = INT_TO_POINTER(3); i < 8; i++, id += 20) {
    printf("Asking for integer %d. Got id: %X\n", POINTER_TO_INT(id), POINTER_TO_INT(interval_tree_query(intervalt, POINTER_TO_INT(id))));
  }
  interval_tree_free(intervalt);
  return 0;
}