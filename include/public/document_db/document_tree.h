#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include "util/result.h"
#include "domain.h"
#include "document.h"

typedef struct {
    Document *documents;
    size_t documents_count;
    size_t capacity;
} DocumentTree;

Result document_tree_new(DocumentTree *self);

void document_tree_destroy(DocumentTree *self);

Result document_tree_add_document(DocumentTree *self, Document *new_document);

Result document_tree_delete_document(DocumentTree *self, size_t id);

Result document_tree_get_document(DocumentTree *self, size_t document_id, Document *result);
