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


struct _interval_node_t {
  int64_t max;
  range_t range;
  void *v;
};
typedef struct _interval_node_t interval_node_t;

struct _interval_tree_t {
  avltree_t *tree;
  interval_node_t *nodes;
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
  if (a->inf < b->inf || (a->sup < b->sup)) { // e2>e1
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
  array_perms = calloc(me->size * 2, sizeof(int));

  /* copy old data across to new array */
  for (ii = 0, end = me->size; ii < end; ii++) {
    memcpy(&array_nodes[ii], &me->nodes[ii], sizeof(interval_node_t));
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
}


static void up_rebalance(int idx, int towards, void *user)
{
  interval_tree_t* me = (interval_tree_t*)user;

  if (towards !=  __child_r(idx) && avltree_get_from_idx(me->tree, __child_r(idx)) && avltree_get_from_idx(me->tree, __child_l(idx)) && avltree_get_from_idx(me->tree, __child_r(__child_l(idx))) ) {
    me->nodes[me->nodes_perm[idx]].max =  max(max(me->nodes[me->nodes_perm[__child_r(__child_l(idx))]].max, me->nodes[me->nodes_perm[__child_l(idx)]].max), me->nodes[me->nodes_perm[idx]].range.sup );
  } else if (avltree_get_from_idx(me->tree, __child_l(idx)) && avltree_get_from_idx(me->tree, __child_r(__child_l(idx)))) {
    me->nodes[me->nodes_perm[idx]].max =    max(me->nodes[me->nodes_perm[__child_r(__child_l(idx))]].max,  me->nodes[me->nodes_perm[idx]].range.sup );
  } else if (towards !=  __child_r(idx) && avltree_get_from_idx(me->tree, __child_r(idx))) {
    me->nodes[me->nodes_perm[idx]].max =  max(me->nodes[me->nodes_perm[__child_r(idx)]].max,  me->nodes[me->nodes_perm[idx]].range.sup );
  } else {
    me->nodes[me->nodes_perm[idx]].max = me->nodes[me->nodes_perm[idx]].range.sup;
  }
  me->nodes_perm[towards] = me->nodes_perm[idx];

  if (towards) { // If we do change the parent, propagate the maximum
    int current_node = __parent(towards);
    while (current_node != 0) {
      me->nodes[me->nodes_perm[current_node]].max = max(me->nodes[me->nodes_perm[current_node]].max, me->nodes[me->nodes_perm[towards]].max);
      current_node = __parent(current_node);
    }
    me->nodes[me->nodes_perm[current_node]].max = max(me->nodes[me->nodes_perm[current_node]].max, me->nodes[me->nodes_perm[towards]].max);
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
}


interval_tree_t* interval_tree_new(int initial_size)
{
  interval_tree_t* me;

  me = calloc(1, sizeof(interval_tree_t));
  me->size = initial_size;
  me->nodes = calloc( initial_size, sizeof(interval_node_t));
  me->nodes_perm = calloc( initial_size, sizeof(int));
  me->tree = avltree_new(initial_size, cmp_range);
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
  if (position > me->size ) {
    __enlarge(me);
  }
  me->nodes_perm[position] = me->count;

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

void *interval_tree_query(interval_tree_t* me, int k)
{
  int i;

  for (i = 0; i < me->size; ) {
    range_t * r;

    r = &(me->nodes[me->nodes_perm[i]].range); // avltree_get_from_idx(me->tree, i);
    /* couldn't find it */
    if (r == NULL) {
      return NULL;
    }

    // 1) If x overlaps with root's interval, return the root's interval.
    if (r->inf <= k && r->sup >= k ) {
      return me->nodes[me->nodes_perm[i]].v;
    }

    //2) If left child of root is not empty and the max  in left child
    // is greater than x's low value, recur for left child
    // printf("Accedo a %d perm %d\n", __child_l(i), me->nodes_perm[__child_l(i)] );
    if (avltree_get_from_idx(me->tree, __child_l(i)) && me->nodes[me->nodes_perm[__child_l(i)]].max >= k ) {
      i = __child_l(i);
    } else { // 3) Else recur for right child.
      i = __child_r(i);
    }
  }

  /* couldn't find it */
  return NULL;
}
