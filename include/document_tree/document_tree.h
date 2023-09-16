#include <stdlib.h>

// structures for requests
typedef struct {
    char *name;
    char *content;
} CreateDocumentRequest;

typedef struct {
    size_t id;
    char *name;
    char *content;
} UpdateDocumentRequest;

typedef struct {
    size_t id;
} DeleteDocumentRequest;

typedef struct {
    size_t id;
} GetDocumentRequest;

typedef struct {
    char *name;
} GetDocumentByNameRequest;

Result document_tree_new(DocumentTree *self);

Result document_tree_destroy(DocumentTree *self);

Result document_tree_add_document(DocumentTree *self, Document *document);

Result document_tree_get_document_by_id(DocumentTree *self, size_t id, Document *document);

Result document_tree_get_document_by_name(DocumentTree *self, char *name, Document *document);

Result document_tree_get_document_by_parent_id(DocumentTree *self, size_t parent_id, Document *document);

Result document_tree_update_document(DocumentTree *self, Document *document);

Result document_tree_delete_document_by_id(DocumentTree *self, size_t id);

Result document_tree_delete_document_by_name(DocumentTree *self, char *name);
