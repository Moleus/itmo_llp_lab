#include <assert.h>
#include "private/document_db/document.h"
#include "private/storage/page_manager.h"

#define FILE_SIGNATURE 0x12345678

Document *document_new() {
    Document *document = malloc(sizeof(Document));
    ASSERT_NOT_NULL(document, FAILED_TO_ALLOCATE_MEMORY)
    document->page_manager = page_manager_new();
    document->root_node = NULL;
    document->init_done = false;
    return document;
}

Result document_init(Document *self, const char *file_path, size_t page_size) {
    self->init_done = true;
    return page_manager_init(self->page_manager, file_path, page_size, FILE_SIGNATURE);
}

void document_destroy(Document *self) {
    ASSERT_ARG_NOT_NULL(self);
    assert(self->init_done);

    page_manager_destroy(self->page_manager);
    free(self);
}

// --- ADD NODE ---
// persists new node and assigns id to it
Result document_persist_new_node(Document *self, Node *node) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(node);

    ItemPayload itemPayload = {
            .data = &node->value,
            .size = sizeof(Node)
    };

    Page *page = NULL;
    Result res = page_manager_get_page_for_data(self->page_manager, itemPayload, &page);
    RETURN_IF_FAIL(res, "failed to persist new data");

    ItemAddResult item_result;
    res = page_manager_put_item(self->page_manager, page, itemPayload, &item_result);
    RETURN_IF_FAIL(res, "failed to persist new data");

    node->id = (node_id_t) {.page_id = page->page_header.page_id.id, .item_id = item_result.metadata.item_id.id};

    return OK;
}

Result document_add_root_node(Document *self, Node *root) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(root);
    // check that we have only 1 root node in file
    // if we have 1 root node, then return error
    // if we have 0 root nodes, then create new root node


    ItemIterator *items_it = page_manager_get_items(self->page_manager);
    while (item_iterator_has_next(items_it)) {
        Item *item;
        Result get_item_res = item_iterator_next(items_it, &item);
        RETURN_IF_FAIL(get_item_res, "failed to add root node");
        Node *node = item->payload.data;
        if (is_root_node(node->id)) {
            ABORT_EXIT(INTERNAL_LIB_ERROR, "Root node already exists in document tree");
        }
    }
    item_iterator_destroy(items_it);

    return document_persist_new_node(self, root);
}

Result document_add_child_node(Document *self, Node *current_node) {
    ItemIterator *items_it = page_manager_get_items(self->page_manager);
    while (item_iterator_has_next(items_it)) {
        Item *item;
        Result get_item_res = item_iterator_next(items_it, &item);
        RETURN_IF_FAIL(get_item_res, "failed to add node");
        Node *tmp_node = item->payload.data;
        if (node_id_eq(tmp_node->id, current_node->parent_id)) {
            // found parent node. Ok. Adding new
            return document_persist_new_node(self, current_node);
        }
    }
    item_iterator_destroy(items_it);
    // didn't find parent node
    return ERROR("Parent node doesn't exist in document tree");
}

Result document_add_node(Document *self, CreateNodeRequest *request, Node *result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(request);
    ASSERT_ARG_NOT_NULL(result);
    assert(self->init_done);

    // if parent node is null, then this is root node. Then check that we have only 1 root node in file
    // if parent node is not null, then check that it exists in file
    result->value = request->value;
    result->parent_id = request->parent;

    if (is_null_node(request->parent)) {
        return document_add_root_node(self, result);
    } else {
        return document_add_child_node(self, result);
    }
}

// --- DELETE NODE ---
Result document_delete_node(Document *self, DeleteNodeRequest *request) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(request);
    assert(self->init_done);

    // if node contains children - raise error
    ItemIterator *items_it = page_manager_get_items(self->page_manager);
    while (item_iterator_has_next(items_it)) {
        Item *item;
        Result get_item_res = item_iterator_next(items_it, &item);
        RETURN_IF_FAIL(get_item_res, "failed to delete node");
        Node *tmp_node = item->payload.data;

        if (node_id_eq(tmp_node->parent_id, request->node->id)) {
            // We are trying to delete node with children
            ABORT_EXIT(INTERNAL_LIB_ERROR, "Node contains children");
        }
    }
    item_iterator_destroy(items_it);
    items_it = page_manager_get_items(self->page_manager);
    // find our node
    while (item_iterator_has_next(items_it)) {
        Item *item;
        Result get_item_res = item_iterator_next(items_it, &item);
        RETURN_IF_FAIL(get_item_res, "failed to delete node");
        Node *tmp_node = item->payload.data;
        if (node_id_eq(tmp_node->id, request->node->id)) {
            // We found our node
            Page *page = items_it->page_iterator->current_page;
            return page_manager_delete_item(self->page_manager, page, item);
        }
    }
    item_iterator_destroy(items_it);

    // didn't find node
    return ERROR("Node doesn't exist in document tree");
}

// --- UPDATE NODE ---
Result document_update_node(Document *self, UpdateNodeRequest *request) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(request);
    assert(self->init_done);
    // delete + add

    DeleteNodeRequest delete_request = {
            .node = request->node
    };
    CreateNodeRequest create_request = {
            .parent = request->node->parent_id,
            .value = request->new_value
    };
    Result res = document_delete_node(self, &delete_request);
    RETURN_IF_FAIL(res, "failed to delete node while updating");
    res = document_add_node(self, &create_request, request->node);
    RETURN_IF_FAIL(res, "failed to add node while updating");
    return OK;
}

// --- ADD MULTIPLE NODES ---
Result document_add_bulk_nodes(Document *self, CreateMultipleNodesRequest *request, CreateMultipleNodesResult *result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(request);
    ASSERT_ARG_NOT_NULL(result);
    assert(self->init_done);

    if (request->count == 0) {
        return ERROR("Bulk add, empty request");
    }

    ItemIterator *items_it = page_manager_get_items(self->page_manager);
    while (item_iterator_has_next(items_it)) {
        Item *item;
        Result get_item_res = item_iterator_next(items_it, &item);
        RETURN_IF_FAIL(get_item_res, "failed to bulk add node");
        Node *tmp_node = item->payload.data;
        if (node_id_eq(tmp_node->id, request->parent)) {
            // found parent node. Ok. Adding new
            result->count = request->count;
            for (size_t i = 0; i < request->count; i++) {
                Node *node = &result->node[i];
                node->value = request->values[i];
                node->parent_id = request->parent;
                Result res = document_persist_new_node(self, node);
                item_iterator_destroy(items_it);
                RETURN_IF_FAIL(res, "failed to bulk add node");
            }
            item_iterator_destroy(items_it);
            return OK;
        }
    }
    item_iterator_destroy(items_it);
    // didn't find parent node
    return ERROR("Parent node doesn't exist in document tree");
}

// --- GET ALL CHILDREN ---
Result document_get_all_children(Document *self, GetAllChildrenRequest *request, GetAllChildrenResult *result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(request);
    ASSERT_ARG_NOT_NULL(result);
    assert(self->init_done);

    ItemIterator *items_it = page_manager_get_items(self->page_manager);

    size_t count = 0;
    while (item_iterator_has_next(items_it)) {
        Item *item;
        Result get_item_res = item_iterator_next(items_it, &item);
        RETURN_IF_FAIL(get_item_res, "failed to bulk add node");
        Node *tmp_node = item->payload.data;
        if (node_id_eq(tmp_node->parent_id, request->node->id)) {
            count++;
        }
    }
    item_iterator_destroy(items_it);
    result->count = count;
    if (count == 0) {
        return OK;
    }
    Node nodes[count];
    size_t i = 0;
    items_it = page_manager_get_items(self->page_manager);
    while (item_iterator_has_next(items_it)) {
        Item *item;
        Result get_item_res = item_iterator_next(items_it, &item);
        RETURN_IF_FAIL(get_item_res, "failed to bulk add node");
        Node *tmp_node = item->payload.data;
        if (node_id_eq(tmp_node->parent_id, request->node->id)) {
            nodes[i++] = *tmp_node;
        }
    }
    item_iterator_destroy(items_it);
    // didn't find parent node
    return OK;
}