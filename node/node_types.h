//
// Created by Ivan on 14.06.2025.
//

#ifndef SPO_LAB1_NODE_TYPES_H
#define SPO_LAB1_NODE_TYPES_H

typedef struct TreeNode TreeNode;
typedef struct ChildNodes ChildNodes;

struct ChildNodes {
    int size;
    TreeNode **childNodes;
};

struct TreeNode {
    char *type;
    TreeNode **childNodes;
    int childrenNumber;
    char *value;
    int id;
};

#endif //SPO_LAB1_NODE_TYPES_H
