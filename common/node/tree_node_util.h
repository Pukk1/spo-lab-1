//
// Created by Ivan on 14.06.2025.
//

#ifndef SPO_LAB1_TREE_NODE_UTIL_H
#define SPO_LAB1_TREE_NODE_UTIL_H

#include <malloc.h>
#include "../../node/node_types.h"
#include "../list/list_util.h"
#include "../execution/execution_util.h"
#include "../string/string_util.h"

TreeNode *mallocTreeNode(char *type, char *value, int nodeNumber);

List findListItemsUtil(TreeNode *treeNode);

#endif //SPO_LAB1_TREE_NODE_UTIL_H
