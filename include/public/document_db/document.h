#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include "util/result.h"
#include "domain.h"

typedef struct {
    size_t id;
    char *name;
    Node *root_node;
} Document;

typedef struct {
    NodeValue value;
    Node *parent;
} CreateNodeRequest;

typedef struct {
    Node *node;
    NodeValue new_value;
} UpdateNodeRequest;

typedef struct {
    Node *node;
} DeleteNodeRequest;

typedef struct {
    Node *node;

    bool (*condition)(Node *);
} FindNodeRequest;

typedef struct {
    Node *nodes;
    size_t nodes_count;
} GetAllNodesResult;

Result document_new(Document *self);

Result document_destroy(Document *self);

Result document_add_node(Document *self, CreateNodeRequest *request, Node *result);

Result document_delete_node(Document *self, DeleteNodeRequest *request);

Result document_update_node(Document *self, UpdateNodeRequest *request);

Result document_find_node(Document *self, FindNodeRequest *request, Node *result);

Result document_get_all_nodes(Document *self, GetAllNodesResult *result);
