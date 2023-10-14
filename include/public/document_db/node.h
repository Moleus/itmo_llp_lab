#pragma once

#include "public/util/common.h"

typedef struct {
    const char *value;
    size_t length;
} String;

typedef enum {
    INT_32,
    DOUBLE,
    STRING,
    BOOL
} ValueType;

typedef struct {
    ValueType type;
    union {
        int32_t int_value;
        double double_value;
        String string_value;
        bool bool_value;
    };
} NodeValue;

typedef struct {
    size_t id;
    struct Node *parent_node;
    NodeValue value;
} Node;

Result node_new(size_t id, Node *parent, NodeValue value, Node *result);

Result node_add_child(Node *self, Node *child);

Result node_delete_child(Node *self, size_t child_id);

Result node_destroy(Node *self);

Result node_get_value(Node *self, NodeValue *result);