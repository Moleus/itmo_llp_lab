#include "public/document_db/document.h"

// main
unsigned char log_level = DEBUG;

int main(int argc, char const *argv[])
{
    // create document.

    CreateNodeRequest string_req = {
        .parent = NULL_NODE_ID,
        .value = (NodeValue) {
            .type = STRING,
            .string_value = (String) {
                .value = "hello",
                .length = 5
            }
        },
    };
    Document *doc = document_new();
    Result res = document_init(doc, "/tmp/llp-heap-file", 512);
    if (res.status != RES_OK) {
        LOG_ERR("failed to init document: %s", res.message);
        return 1;
    }

    Node string_node;
    Node double_node;
    res = document_add_node(doc, &string_req, &string_node);
    if (res.status != RES_OK) {
        LOG_ERR("failed to add node string: %s", res.message);
        return 1;
    }
    // double value request
    CreateNodeRequest double_req = {
            .parent = string_node.id,
            .value = (NodeValue) {
                    .type = DOUBLE,
                    .double_value = 3.14
            },
    };
    res = document_add_node(doc, &double_req, &double_node);
    if (res.status != RES_OK) {
        LOG_ERR("failed to add node double: %s", res.message);
        return 1;
    }
    // remove 1 node
    LOG_INFO("string node id: %d, data: %.*s", string_node.id, string_node.value.string_value.length, string_node.value.string_value.value);
    document_delete_node(doc, &(DeleteNodeRequest) {.node = &string_node});
    LOG_INFO("double node id: %d, data: %f", double_node.id, double_node.value.double_value);

    document_destroy(doc);
    return 0;
}