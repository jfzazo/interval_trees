/**
 * @file interval_tree.h
 * Implementation of basic interval trees using an AVL implementation.
 * This initial version does not support the possibility of removing
 * nodes.
 *
 * Implemented over an array, so memory access is optimal
 *
 * @author Jose Fernando Zazo (www.github.com/jfzazo)
 * @date 18/02/2016
 */
#ifndef INTERVAL_TREE_H
#define INTERVAL_TREE_H


typedef struct _interval_tree_t interval_tree_t; /**< Opaque structure of the tree */

/**
 * @brief Structure defining a close range [a,b].
 */
struct _range_t {
  int64_t inf; /**< Lower limit */
  int64_t sup; /**< Upper limit */
};
typedef struct _range_t range_t;

/**
 * @brief Initializes a interval tree that will contain space for initial_size
 * nodes. A small value might cause a frequent reallocation of the memory when new
 * elements are added.
 *
 * @param initial_size Initial array size.
 * @return NULL if the tree could not be generated.
 */
interval_tree_t* interval_tree_new(int initial_size);

/**
 * @brief Free a previous allocated structure by the interval_tree_new function
 *
 * @param me The returned value by the interval_tree_new function.
 */
void interval_tree_free(interval_tree_t* me);

/**
 * @brief Insert a range in the tree. Optionally, it can contain a value struct.
 * The key for this kind of trees will be obtained from the range.
 *
 * @param me A interval tree that has been previously allocated by a call to interval_tree_new.
 * @param _range_t The interval of the node. It is impossible to store two nodes with the same
 * range (same key implies an update of the value field).
 *
 * @param v The value that the user will retrieve by the time that a hit is produced when
 * looking for this kind of elements.
 */
void interval_tree_insert(interval_tree_t* me, struct _range_t *r, void *v);

/**
 * @brief Given an integer, check for an occurence in a range.
 *
 * @param me  A interval tree that has been previously allocated by a call to interval_tree_new.
 * @param k The integer to search.
 *
 * @return The value associated to the matched range, NULL if no occurence has appeared.
 */
void *interval_tree_query(interval_tree_t* me, int k);

/**
 * @brief Print the current tree in a fashionable manner.
 *
 * @param me  A interval tree that has been previously allocated by a call to interval_tree_new.
 */
void interval_tree_print(interval_tree_t* me);

#endif