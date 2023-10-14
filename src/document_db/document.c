#include "private/document_db/document.h"

Result document_add_node(Document *self, CreateNodeRequest *request, Node *result) {
    ASSERT_ARG_NOT_NULL(self);

    self->page_manager.
}