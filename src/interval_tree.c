/**
 * @file interval_tree.c
 * Implementation of interval trees over an array.
 *
 * @author Jose Fernando Zazo (www.github.com/jfzazo)
 * @date 18/02/2016
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "avl_tree.h"
#include "interval_tree.h"


#define max(x,y) ((x) < (y) ? (y) : (x))
#define min(x,y) ((x) > (y) ? (y) : (x))


struct _interval_node_t {
  int64_t max;
  int64_t min;
  range_t range;
  void *v;
};
typedef struct _interval_node_t interval_node_t;

struct _interval_tree_t {
  avltree_t *tree;
  interval_node_t *nodes;
  void **multiple_query_return;
  int *nodes_perm;
  int size;
  int count;
};


static int __child_l(const int idx)
{
  return idx * 2 + 1;
}

static int __child_r(const int idx)
{
  return idx * 2 + 2;
}

static int __parent(const int idx)
{
  assert(idx != 0);
  return (idx - 1) / 2;
}

static long cmp_range(const void *e1, const void *e2)
{
  range_t *a, *b;
  a = ((range_t *)e1);
  b = ((range_t *)e2);
  if (a->inf < b->inf) { // e2>e1
    return 1;
  } else if (a->inf == b->inf && a->sup == b->sup) {
    return 0;
  } else { // e2<e1
    return -1;
  }
}


static void __enlarge(interval_tree_t* me)
{
  int ii, end;
  interval_node_t *array_nodes;
  int *array_perms;
  int prev_node;
  avltree_iterator_t iter;
  node_t *n;

  /* double capacity */
  array_nodes = calloc(me->size * 2, sizeof(interval_node_t));
  array_perms = calloc(me->size * 4, sizeof(int));
  memset(array_perms, -1, me->size * 2);
  /* copy old data across to new array */
  for (ii = 0, end = me->size; ii < end; ii++) {
    memcpy(&array_nodes[ii], &me->nodes[ii], sizeof(interval_node_t));
  }
  for (ii = 0, end = 2 * me->size; ii < end; ii++) {
    memcpy(&array_perms[ii], &me->nodes_perm[ii], sizeof(int));
  }
  /* Update the references  to the key in this module ... Not the most efficient aspect */
  avltree_iterator(me->tree, &iter);
  prev_node = iter.current_node;
  while ( (n = avltree_iterator_next(me->tree, &iter)) && prev_node != iter.current_node) {
    n->key = &(array_nodes[array_perms[prev_node]].range);
    prev_node = iter.current_node;
  }

  /* swap arrays */
  free(me->nodes);
  free(me->nodes_perm);
  me->nodes      = array_nodes;
  me->nodes_perm = array_perms;
  me->size *= 2;
  free(me->multiple_query_return);
  me->multiple_query_return = calloc(me->size + 1, sizeof(void *));
}


static void up_rebalance(int idx, int towards, void *user)
{
  interval_tree_t* me = (interval_tree_t*)user;

  if (towards !=  __child_r(idx) && avltree_get_from_idx(me->tree, __child_r(idx)) && avltree_get_from_idx(me->tree, __child_l(idx)) && avltree_get_from_idx(me->tree, __child_r(__child_l(idx))) ) {
    me->nodes[me->nodes_perm[idx]].max =  max(max(me->nodes[me->nodes_perm[__child_r(__child_l(idx))]].max, me->nodes[me->nodes_perm[__child_l(idx)]].max), me->nodes[me->nodes_perm[idx]].range.sup );
    me->nodes[me->nodes_perm[idx]].min =  min(min(me->nodes[me->nodes_perm[__child_r(__child_l(idx))]].min, me->nodes[me->nodes_perm[__child_l(idx)]].min), me->nodes[me->nodes_perm[idx]].range.inf );
  } else if (avltree_get_from_idx(me->tree, __child_l(idx)) && avltree_get_from_idx(me->tree, __child_r(__child_l(idx)))) {
    me->nodes[me->nodes_perm[idx]].max =    max(me->nodes[me->nodes_perm[__child_r(__child_l(idx))]].max,  me->nodes[me->nodes_perm[idx]].range.sup );
    me->nodes[me->nodes_perm[idx]].min =    min(me->nodes[me->nodes_perm[__child_r(__child_l(idx))]].min,  me->nodes[me->nodes_perm[idx]].range.inf );
  } else if (towards !=  __child_r(idx) && avltree_get_from_idx(me->tree, __child_r(idx))) {
    me->nodes[me->nodes_perm[idx]].max =  max(me->nodes[me->nodes_perm[__child_r(idx)]].max,  me->nodes[me->nodes_perm[idx]].range.sup );
    me->nodes[me->nodes_perm[idx]].min =  min(me->nodes[me->nodes_perm[__child_r(idx)]].min,  me->nodes[me->nodes_perm[idx]].range.inf );
  } else {
    me->nodes[me->nodes_perm[idx]].max = me->nodes[me->nodes_perm[idx]].range.sup;
    me->nodes[me->nodes_perm[idx]].min = me->nodes[me->nodes_perm[idx]].range.inf;
  }
  me->nodes_perm[towards] = me->nodes_perm[idx];
  me->nodes_perm[idx] = -1;

  if (towards) { // If we do change the parent, propagate the maximum
    int current_node = __parent(towards);
    while (current_node != 0 && me->nodes_perm[current_node] != -1) {
      me->nodes[me->nodes_perm[current_node]].max = max(me->nodes[me->nodes_perm[current_node]].max, me->nodes[me->nodes_perm[towards]].max);
      me->nodes[me->nodes_perm[current_node]].min = min(me->nodes[me->nodes_perm[current_node]].min, me->nodes[me->nodes_perm[towards]].min);
      current_node = __parent(current_node);
    }

    if (me->nodes_perm[current_node] != -1) {
      me->nodes[me->nodes_perm[current_node]].max = max(me->nodes[me->nodes_perm[current_node]].max, me->nodes[me->nodes_perm[towards]].max);
      me->nodes[me->nodes_perm[current_node]].min = min(me->nodes[me->nodes_perm[current_node]].min, me->nodes[me->nodes_perm[towards]].min);
    }
  }
}

static void down_rebalance(int idx, int towards, void *user)
{
  interval_tree_t* me = (interval_tree_t*)user;

  if (towards !=  __child_l(idx) && avltree_get_from_idx(me->tree, __child_l(idx)) && avltree_get_from_idx(me->tree, __child_r(idx)) && avltree_get_from_idx(me->tree, __child_l(__child_r(idx))) ) {
    me->nodes[me->nodes_perm[idx]].max =  max(max(me->nodes[me->nodes_perm[__child_l(__child_r(idx))]].max, me->nodes[me->nodes_perm[__child_l(idx)]].max), me->nodes[me->nodes_perm[idx]].range.sup );
  } else if (avltree_get_from_idx(me->tree, __child_r(idx)) && avltree_get_from_idx(me->tree, __child_l(__child_r(idx)))) {
    me->nodes[me->nodes_perm[idx]].max =    max(me->nodes[me->nodes_perm[__child_l(__child_r(idx))]].max,  me->nodes[me->nodes_perm[idx]].range.sup );
  } else if (towards !=  __child_l(idx) && avltree_get_from_idx(me->tree, __child_l(idx))) {
    me->nodes[me->nodes_perm[idx]].max =  max(me->nodes[me->nodes_perm[__child_l(idx)]].max,  me->nodes[me->nodes_perm[idx]].range.sup );
  } else {
    me->nodes[me->nodes_perm[idx]].max = me->nodes[me->nodes_perm[idx]].range.sup;
  }

  me->nodes_perm[towards] = me->nodes_perm[idx];
  me->nodes_perm[idx] = -1;
}


interval_tree_t* interval_tree_new(int initial_size)
{
  interval_tree_t* me;

  me = calloc(1, sizeof(interval_tree_t));
  me->size = initial_size;
  me->nodes = calloc( initial_size, sizeof(interval_node_t));
  me->nodes_perm = calloc( 2 * initial_size, sizeof(int));
  memset(me->nodes_perm, -1, 2 * initial_size);
  me->tree = avltree_new(initial_size, cmp_range);
  me->multiple_query_return = calloc(initial_size + 1, sizeof(void *));
  set_shift_up_callback(me->tree, up_rebalance, me);
  set_shift_down_callback(me->tree, down_rebalance, me);
  return me;
}

void interval_tree_free(interval_tree_t* me)
{
  if (me) {
    avltree_free(me->tree);
    free(me->nodes);
    free(me->nodes_perm);
    free(me->multiple_query_return);
    free(me);
  }
}

void interval_tree_insert(interval_tree_t* me, range_t *r, void *v)
{
  void *tv;
  void *tk;
  int position;

  if (me->count >= me->size ) {
    __enlarge(me);
  }
  memcpy(&(me->nodes[me->count].range), r, sizeof(range_t));
  me->nodes[me->count].max = me->nodes[me->count].range.sup;
  tv = (void *) me->nodes[me->count].max;
  tk = &me->nodes[me->count].range;
  me->nodes[me->count].v = v;  // Value  of the node (id of the network...)

  position = avltree_insert(me->tree, tk, tv);
  me->nodes_perm[position] = me->count;

  if (position > me->size ) {
    __enlarge(me);
  }

  if (position) { // If we do not change the parent, propagate the maximum
    int current_node = __parent(position);
    while (current_node != 0) {
      me->nodes[me->nodes_perm[current_node]].max = max(me->nodes[me->nodes_perm[current_node]].max, me->nodes[me->nodes_perm[position]].max);
      current_node = __parent(current_node);
    }
    me->nodes[me->nodes_perm[current_node]].max = max(me->nodes[me->nodes_perm[current_node]].max, me->nodes[me->nodes_perm[position]].max);
  }

  if (position) {

    rebalance(me->tree, position);

  }

  me->count++;
}

static void * __interval_tree_query(interval_tree_t* me, int idx, int k)
{
  range_t * r;
  void *ret_value;

  r =  avltree_get_from_idx(me->tree, idx);
  if (r == NULL) {
    return NULL;
  }

  if (me->nodes[me->nodes_perm[idx]].max < k || me->nodes[me->nodes_perm[idx]].min > k ) {
    return NULL;
  }

  // 1) If x overlaps with root's interval, return the root's interval.
  if (r->inf <= k && r->sup >= k ) {
    return me->nodes[me->nodes_perm[idx]].v;
  }

  //2) If left child of root is not empty and the [min, max] range
  // contains the value of k, recur for the left child.
  // The condition is a base case in the recursive function.
  if (!(ret_value = __interval_tree_query(me, __child_l(idx), k))) {

    //3) If right child of root is not empty and the [min, max] range
    // contains the value of k, recur for the right child.
    // The condition is a base case in the recursive function.
    ret_value = __interval_tree_query(me, __child_r(idx), k);
  }

  return ret_value;
}

void *interval_tree_query(interval_tree_t* me, int k)
{
  return __interval_tree_query(me, 0, k);
}


static void __interval_tree_multiple_query(interval_tree_t* me, int idx, int k, int *ncoincidences)
{
  range_t * r;

  r =  avltree_get_from_idx(me->tree, idx);
  if (r == NULL) {
    me->multiple_query_return[*ncoincidences] = NULL;
    return; // The output is unused
  }

  if (me->nodes[me->nodes_perm[idx]].max < k || me->nodes[me->nodes_perm[idx]].min > k ) {
    me->multiple_query_return[*ncoincidences] = NULL;
    return;
  }

  // 1) If x overlaps with root's interval, return the root's interval.
  if (r->inf <= k && r->sup >= k ) {
    me->multiple_query_return[*ncoincidences] = me->nodes[me->nodes_perm[idx]].v;
    (*ncoincidences)++;
  }

  //2) If left child of root is not empty and the [min, max] range
  // contains the value of k, recur for the left child.
  // The condition is a base case in the recursive function.
  __interval_tree_multiple_query(me, __child_l(idx), k, ncoincidences);

  //3) If right child of root is not empty and the [min, max] range
  // contains the value of k, recur for the right child.
  // The condition is a base case in the recursive function.
  __interval_tree_multiple_query(me, __child_r(idx), k, ncoincidences);

  return;
}

void **interval_tree_multiple_query(interval_tree_t* me, int k)
{
  int ncoincidences = 0;
  me->multiple_query_return[ncoincidences] = NULL;
  __interval_tree_multiple_query(me, 0, k, &ncoincidences);
  return me->multiple_query_return;
}

static void __print(interval_tree_t* me, int idx, int d)
{
  int i;

  for (i = 0; i < d; i++)
    printf(" ");
  printf("%c: ", idx % 2 == 1 ? 'l' : 'r');

  if (me->size <= idx || !avltree_get_from_idx(me->tree, idx) || !(&(me->nodes[me->nodes_perm[i]].range))) {
    printf("-\n");
    return;
  }

  printf("Range [%ld-%ld]. Max %ld Min %ld\n", me->nodes[me->nodes_perm[idx]].range.inf, me->nodes[me->nodes_perm[idx]].range.sup, me->nodes[me->nodes_perm[idx]].max, me->nodes[me->nodes_perm[idx]].min);
  __print(me, __child_l(idx), d + 1);
  __print(me, __child_r(idx), d + 1);
}

void interval_tree_print(interval_tree_t* me)
{
  __print(me, 0, 0);
}
