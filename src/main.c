#include "public/document_db/document.h"

// main
unsigned char log_level = DEBUG;

int main(int argc, char const *argv[])
{
    // create document.

    CreateNodeRequest string_req = {
        .value = (NodeValue) {
            .type = STRING,
            .string_value = (String) {
                .value = "hello",
                .length = 5
            }
        },
    };
    // double value request
    CreateNodeRequest double_req = {
        .value = (NodeValue) {
            .type = DOUBLE,
            .double_value = 3.14
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
    document_add_node(doc, &string_req, &string_node);
    document_add_node(doc, &double_req, &double_node);
    // remove 1 node
    document_delete_node(doc, &(DeleteNodeRequest) {.node = &string_node});

    document_destroy(doc);
    return 0;
}