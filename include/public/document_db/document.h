#pragma once

#include "public/util/result.h"
#include "public/storage/page_manager.h"
#include "node.h"

typedef struct {
    Node *root_node;
    PageManager *page_manager;
    bool init_done;
} Document;

typedef struct {
    NodeValue value;
    node_id_t parent;
} CreateNodeRequest;

typedef struct {
    Node *node;
    NodeValue new_value;
} UpdateNodeRequest;

typedef struct {
    Node *node;
} DeleteNodeRequest;

typedef struct {
    node_id_t parent;
    size_t count;
    NodeValue values[];
} CreateMultipleNodesRequest;

typedef struct {
    size_t count;
    Node node[];
} CreateMultipleNodesResult;

typedef struct {
    Node *node;
} GetAllChildrenRequest;

typedef struct {
    size_t count;
    Node nodes[];
} GetAllChildrenResult;

Document *document_new();

Result document_init(Document *self, const char *file_path, size_t page_size);

void document_destroy(Document *self);

Result document_add_node(Document *self, CreateNodeRequest *request, Node *result);

Result document_add_bulk_nodes(Document *self, CreateMultipleNodesRequest *request, CreateMultipleNodesResult *result);

Result document_delete_node(Document *self, DeleteNodeRequest *request);

Result document_update_node(Document *self, UpdateNodeRequest *request);

Result document_get_all_children(Document *self, GetAllChildrenRequest *request, GetAllChildrenResult *result);