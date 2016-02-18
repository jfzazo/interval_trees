/**
 * @file avl_tree.h
 * Implementation of AVL trees over an array.
 *
 * @author Willemt (https://github.com/willemt). Modified by Jose Fernando Zazo (www.github.com/jfzazo)
 * @date 18/02/2016
 */
#ifndef AVL_TREE_H
#define AVL_TREE_H

typedef struct {
  void* key;
  void* val;
} node_t;

typedef struct {
  /* size of array */
  int size;
  int count;
  long (*cmp)(
    const void *e1,
    const void *e2);
  void (*shift_down_callback)(int new_root, int last_root, void *user);
  void *shift_down_callback_user;
  void (*shift_up_callback)(int new_root, int last_root, void *user);
  void *shift_up_callback_user;
  node_t *nodes;
} avltree_t;

typedef struct {
  int current_node;
} avltree_iterator_t;

avltree_t* avltree_new(int initial_size, long (*cmp)(const void *e1, const void *e2));

void avltree_free(avltree_t* me);


void* avltree_remove(avltree_t* me, void* k);

int avltree_count(avltree_t* me);

int avltree_size(avltree_t* me);

int avltree_height(avltree_t* me);

void avltree_empty(avltree_t* me);

//Return the position where the node was inserted
int avltree_insert(avltree_t* me, void* k, void* v);

void* avltree_get(avltree_t* me, const void* k);

void* avltree_get_from_idx(avltree_t* me, int idx);

/**
 * Rotate on X:
 * Y = X's parent
 * Step A: Y becomes left child of X
 * Step B: X's left child's becomes Y's right child */
void avltree_rotate_left(avltree_t* me, int idx);

/**
 * Rotate on X:
 * Y = X's left child
 * Step A: X becomes right child of X's left child
 * Step B: X's left child's right child becomes X's left child */
void avltree_rotate_right(avltree_t* me, int idx);


/**
 * Initialise a new hash iterator over this hash
 * It is NOT safe to remove items while iterating.  */
void avltree_iterator(avltree_t * h, avltree_iterator_t * iter);

/**
 * Iterate to the next item on an iterator
 * @return next item key from iterator */
void *avltree_iterator_next(avltree_t * h, avltree_iterator_t * iter);

/**
 * Iterate to the next item on an iterator
 * @return next item value from iterator */
void *avltree_iterator_next_value(avltree_t * h, avltree_iterator_t * iter);

int avltree_iterator_has_next(avltree_t * h, avltree_iterator_t * iter);

void* avltree_iterator_peek_value(avltree_t * h, avltree_iterator_t * iter);

void* avltree_iterator_peek(avltree_t * h, avltree_iterator_t * iter);

/**
 * @brief Set the callback function that will be invoked every time that  the shift of a node is produced.
 * It applies to the right rotation.
 *
 * @param me An AVL tree that has been previously allocated.
 * @param shift_up_callback The pointer to the function that will receive the previous position of the current node to change,
 * the next position where it will be stored and a user pointer.
 * @param user The pointer that will be passed as a third argument to the callback function
 */
void set_shift_up_callback(avltree_t* me, void (*shift_up_callback)(int idx, int towards, void *user), void *user);


/**
 * @brief Set the callback function that will be invoked every time that the shift of a node is produced.
 * It applies to the left rotation.
 *
 * @param me An AVL tree that has been previously allocated.
 * @param shift_up_callback The pointer to the function that will receive the previous position of the current node to change,
 * the next position where it will be stored and a user pointer.
 * @param user The pointer that will be passed as a third argument to the callback function
 */
void set_shift_down_callback(avltree_t* me, void (*shift_up_callback)(int idx, int towards, void *user), void *user);

/**
 * @brief This function forces a rebalance of the tree. It must be invoked each time that a node is inserted
 * and  avltree_insert return differs from 0.
 *
 * @param me An AVL tree that has been previously allocated.
 * @param idx The returned value by avltree_insert
 */
void rebalance(avltree_t* me, int idx);
#endif /* AVL_TREE_H */
