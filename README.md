# interval_trees
Implementation of interval trees in C language. An array is used instead of "malloc'ing" a new structure for every node.


## Yet in development

This is a basic version that I coded in order to find IPs in ranges. I did not have a need to remove elements of the tree or perform any sophisticate operation. I will probably add them in the future.


## Credits

This implementation is sustained on AVL trees. The most efficient library I was able to find is located at the project
https://github.com/willemt/array-avl-tree. However, I found  a pair of nasty bugs that are solved in this version.
