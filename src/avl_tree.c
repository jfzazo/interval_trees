/**
 * @file avl_tree.c
 * Implementation of AVL trees over an array.
 *
 * @author Willemt (https://github.com/willemt). Modified by Jose Fernando Zazo (www.github.com/jfzazo)
 * @date 18/02/2016
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "avl_tree.h"

#define max(x,y) ((x) < (y) ? (y) : (x))

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
#if 0
  if (idx == 0) return *(int*)NULL;
#endif
  assert(idx != 0);
  return (idx - 1) / 2;
}

static void __print(avltree_t* me, int idx, int d)
{
  int i;

  for (i = 0; i < d; i++)
    printf(" ");
  printf("%c ", idx % 2 == 1 ? 'l' : 'r');

  if (me->size <= idx || !me->nodes[idx].key) {
    printf("\n");
    return;
  }

  printf("%lx\n", (unsigned long int)me->nodes[idx].key);
  __print(me, __child_l(idx), d + 1);
  __print(me, __child_r(idx), d + 1);
}

void avltree_print(avltree_t* me)
{
  printf("AVL Tree:\n");
  __print(me, 0, 0);
}

void avltree_print2(avltree_t* me)
{
  int i;

  for (i = 0; i < me->size; i++)
    printf("%lx%c", (unsigned long int)me->nodes[i].key, i == me->size ? '|' : ' ');
  printf("\n");
}

static void __enlarge(avltree_t* me)
{
  int ii, end;
  node_t *array_n;

  /* double capacity */
  array_n = calloc(me->size * 2, sizeof(node_t));

  /* copy old data across to new array */
  for (ii = 0, end = me->size /*avltree_count(me)*/; ii < end; ii++) {
    if (me->nodes[ii].key)
      memcpy(&array_n[ii], &me->nodes[ii], sizeof(node_t));
  }

  /* swap arrays */
  free(me->nodes);
  me->nodes = array_n;
  me->size *= 2;
}

avltree_t* avltree_new(int size, long (*cmp)(
                         const void *e1,
                         const void *e2))
{
  avltree_t* me;

  assert(cmp);

  me        = calloc(1, sizeof(avltree_t));
  me->size  = size;
  me->nodes = calloc(me->size, sizeof(node_t));
  me->cmp   = cmp;
  return me;
}

void avltree_free(avltree_t* me)
{
  if (me) { // Ensure that we are not accesing me->size if me is null.
    free(me->nodes);
    free(me);
  }
}

static int __count(avltree_t* me, int idx)
{
  if (me->size <= idx || !me->nodes[idx].key)
    return 0;
  return __count(me, __child_l(idx)) + __count(me, __child_r(idx)) + 1;
}

int avltree_count(avltree_t* me)
{
//    return __count(me,0);
  return me->count;
}

int avltree_size(avltree_t* me)
{
  return me->size;
}

static int __height(avltree_t* me, int idx)
{
  if (idx >= me->size) return 0;
  if (!me->nodes[idx].key) return 0;
  return max(
           __height(me, __child_l(idx)) + 1,
           __height(me, __child_r(idx)) + 1);
}

int avltree_height(avltree_t* me)
{
  return __height(me, 0);
}

static void __shift_up(avltree_t* me, int idx, int towards)
{
  if (idx >= me->size || towards >= me->size) return;
  if (!me->nodes[idx].key)    return;

  memcpy(&me->nodes[towards], &me->nodes[idx], sizeof(node_t));
  if (me->shift_up_callback) {
    me->shift_up_callback( idx, towards, me->shift_up_callback_user);
  }
  me->nodes[idx].key = NULL;
  __shift_up(me, __child_l(idx), __child_l(towards));
  __shift_up(me, __child_r(idx), __child_r(towards));
}

static void __shift_down(avltree_t* me, int idx, int towards)
{
  if (idx >= me->size || towards >= me->size) return;
  if (!me->nodes[idx].key)    return;

  __shift_down(me, __child_l(idx), __child_l(towards));
  __shift_down(me, __child_r(idx), __child_r(towards));
  memcpy(&me->nodes[towards], &me->nodes[idx], sizeof(node_t));

  if (me->shift_down_callback) {
    me->shift_down_callback( idx, towards, me->shift_down_callback_user);
  }
}

void avltree_rotate_right(avltree_t* me, int idx)
{
  /* A Partial
   * Move X out of the way so that Y can take its spot */
  __shift_down(me, __child_r(idx), __child_r(__child_r(idx)));
  memcpy(&me->nodes[__child_r(idx)], &me->nodes[idx], sizeof(node_t));
  if (me->shift_up_callback)
    me->shift_up_callback( idx, __child_r(idx), me->shift_up_callback_user);
  /* B */
  if ( __child_r(__child_l(idx)) < me->size) {
    __shift_down(me, __child_r(__child_l(idx)), __child_l(__child_r(idx)));
    me->nodes[__child_r(__child_l(idx))].key = NULL;
  }
  /* A Final
   * Move Y into X's old spot */
  __shift_up(me, __child_l(idx), idx);
}

void* avltree_get(avltree_t* me, const void* k)
{
  int i;

  for (i = 0; i < me->size; ) {
    int r;
    node_t *n;

    n = &me->nodes[i];

    /* couldn't find it */
    if (!n->key)
      return NULL;

    r = me->cmp(n->key, k);

    if (r == 0) {
      return n->val;
    } else if (r < 0) {
      i = __child_l(i);
    } else if (r > 0) {
      i = __child_r(i);
    } else {
      assert(0);
    }
  }

  /* couldn't find it */
  return NULL;
}

void* avltree_get_from_idx(avltree_t* me, int idx)
{
  if (idx < me->size) {
    return me->nodes[idx].key;
  } else {
    return NULL;
  }
}

void set_shift_up_callback(avltree_t* me, void (*shift_up_callback)(int new_root, int last_root, void *user), void *user)
{
  me->shift_up_callback = shift_up_callback;
  me->shift_up_callback_user = user;
}
void set_shift_down_callback(avltree_t* me, void (*shift_down_callback)(int new_root, int last_root, void *user), void *user)
{
  me->shift_down_callback = shift_down_callback;
  me->shift_down_callback_user = user;
}

void avltree_rotate_left(avltree_t* me, int idx)
{
  int p;

  p = __parent(idx);

  /* A Partial
   * Move Y out of the way so that X can take its spot */
  __shift_down(me, __child_l(p), __child_l(__child_l(p)));
  memcpy(&me->nodes[__child_l(p)], &me->nodes[p], sizeof(node_t));

  if (me->shift_down_callback)
    me->shift_down_callback( p, __child_l(p), me->shift_up_callback_user);
  /* B */
  __shift_down(me, __child_l(idx), __child_r(__child_l(p)));
  me->nodes[__child_l(idx)].key = NULL;

  /* A Final
   * Move Y into X's old spot */
  __shift_up(me, idx, p);
}

static void __rebalance(avltree_t* me, int idx)
{

  while (1) {
    if (2 <= __height(me, __child_l(idx)) - __height(me, __child_r(idx))) {
      int bf_l;

      bf_l = __height(me, __child_l(__child_l(idx))) -
             __height(me, __child_r(__child_l(idx)));

      if (bf_l == -1) {
        avltree_rotate_left(me, __child_l(idx));
        avltree_rotate_right(me, __child_l(idx));
      } else {
        avltree_rotate_right(me, __child_l(idx));
      }
    }  else if (-2 >= __height(me, __child_l(idx)) - __height(me, __child_r(idx))) {
      int bf_r;

      bf_r = __height(me, __child_l(__child_r(idx))) -
             __height(me, __child_r(__child_r(idx)));

      if (bf_r == -1) {
        avltree_rotate_left(me, __child_r(idx));
      } else {
        avltree_rotate_right(me, __child_r(idx));
        avltree_rotate_left(me, __child_r(idx));
      }
    }
    if (0 == idx) break;
    idx = __parent(idx);
  }
}


void rebalance(avltree_t* me, int position)
{
  return  __rebalance(me, __parent(position));
}

static int __previous_ordered_node(avltree_t* me, int idx)
{
  int prev, i;

  for (prev = -1, i = __child_l(idx);
       /* array isn't that big, or key is null -> we don't have this child */
       i < me->size && me->nodes[i].key;
       prev = i, i = __child_r(i)
      );

  return prev;
}

void* avltree_remove(avltree_t* me, void* k)
{
  int i;

  for (i = 0; i < me->size; ) {
    long r;
    node_t *n;

    n = &me->nodes[i];

    /* couldn't find it */
    if (!n->key)
      return NULL;

    r = me->cmp(n->key, k);

    if (r == 0) {
      /* replacement */
      int rep;

      me->count -= 1;

      k = n->key;

      rep = __previous_ordered_node(me, i);
      if (-1 == rep) {
        /* make sure the node is now blank */
        n->key = NULL;
      } else {
        /* have r's left node become right node of r's parent.
         * NOTE: r by definition shouldn't have a right child */
        __shift_up(me, __child_l(rep), __child_r(__parent(rep)));

        /* have r replace deleted node */
        __shift_up(me, rep, i);
      }

      if (i != 0)
        __rebalance(me, __parent(i));

      return k;
    } else if (r < 0) {
      i = __child_l(i);
    } else if (r > 0) {
      i = __child_r(i);
    } else {
      assert(0);
    }
  }

  /* couldn't find it */
  return NULL;
}

void avltree_empty(avltree_t* me)
{
  int i;

  for (i = 0; i < me->size; i++) {
    me->nodes[i].key = NULL;
  }
}

int avltree_insert(avltree_t* me, void* k, void* v)
{
  int i;
  node_t* n;

  for (i = 0; i < me->size; ) {
    n = &me->nodes[i];

    /* found an empty slot */
    if (!n->key) {
      n->key = k;
      n->val = v;
      me->count += 1;

      return i;
    }

    long r = me->cmp(n->key, k);

    if (r == 0) {
      /*  we don't need to rebalance because we just overwrite this slot */
      n->val = v;
      return i;
    } else if (r < 0) {
      i = __child_l(i);
    } else if (r > 0) {
      i = __child_r(i);
    } else {
      assert(0);
    }
  }

  /* we're outside of the loop because we need to enlarge */
  __enlarge(me);
  n = &me->nodes[i];
  n->key = k;
  n->val = v;
  me->count += 1;
  return i;
}

void* avltree_iterator_peek(avltree_t * h, avltree_iterator_t * iter)
{
  if (iter->current_node < h->size) {
    node_t *next;

    next = &h->nodes[++iter->current_node];
    if (next->key)
      return next;
  }

  return NULL;
}

void* avltree_iterator_peek_value(avltree_t * h, avltree_iterator_t * iter)
{
  return avltree_get(h, avltree_iterator_peek(h, iter));
}

int avltree_iterator_has_next(avltree_t * h, avltree_iterator_t * iter)
{
  return NULL != avltree_iterator_peek(h, iter);
}

void *avltree_iterator_next_value(avltree_t * h, avltree_iterator_t * iter)
{
  void* k;

  k = avltree_iterator_next(h, iter);
  if (!k) return NULL;
  return avltree_get(h, k);
}

void *avltree_iterator_next(avltree_t * h, avltree_iterator_t * iter)
{
  node_t *n;
  node_t *next;

  assert(iter);

  n = &h->nodes[iter->current_node];

  while (iter->current_node < h->size - 1) {
    next = &h->nodes[++iter->current_node];
    if (next->key)
      break;
  }

  return n;
}

void avltree_iterator(avltree_t * h __attribute__((unused)),
                      avltree_iterator_t * iter)
{
  iter->current_node = 0;
}

