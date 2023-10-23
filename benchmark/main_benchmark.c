#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "public/document_db/node.h"
#include "public/document_db/document.h"
#include <unistd.h>

unsigned char log_level = WARN;

long get_mem_usage(void) {
    FILE *file = fopen("/proc/self/status", "r");
    int result = -1;
    char buffer[1024] = "";

    while (fscanf(file, " %1023s", buffer) == 1) {
        if (strcmp(buffer, "VmSize:") == 0) {
            fscanf(file, " %d", &result);
        }
    }

    fclose(file);
    return result;
}

long get_file_size(const char *filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

#define BATCH_SIZE 20
#define MAX_MEASUREMENTS 300
#define DB_FILE "benchmark-data.llp"
#define PAGE_SIZE 512

node_id_t g_used_ids[BATCH_SIZE + 1];

// page_id, item_id
long g_used_ids_count = 0;
NodeValue g_node_variants[4];

Node generate_random_node(void) {
    NodeValue node_value = g_node_variants[(rand() / 3) % 4];
//    node_id_t parent_id = g_used_ids[(rand() / 3) % g_used_ids_count];
    node_id_t parent_id = ROOT_NODE_ID;

    Node node = (Node) {.parent_id = parent_id, .value = node_value};
    return node;
}

Node gen_root_node(void) {
    NodeValue node_value = g_node_variants[(rand() / 3) % 4];
    Node node = (Node) {.parent_id = NULL_NODE_ID, .value = node_value};
    return node;
}

CreateNodeRequest wrap_node(Node node) {
    CreateNodeRequest request = (CreateNodeRequest) {.parent = node.parent_id, .value = node.value};
    return request;
}

node_id_t insert_node(Document *doc, Node node) {
    Node result;
    CreateNodeRequest req = wrap_node(node);
    document_add_node(doc, &req, &result);
    return result.id;
}

void delete_node(Document *doc, Node node) {
    DeleteNodeRequest req = (DeleteNodeRequest) {.node = &node};
    document_delete_node(doc, &req);
}

struct TimeResults {
    double insert_time;
    double delete_time;
};

//1. Сделать тест, где вставки и удаления происходят вперемешку (500 вставок, 200 удалений)
//- каждая вставка в случайное место
//- после каждой вставки мерить время и размер файла
//измерение вставок:
//t = time()
//write * 100
//dt = time() - t
struct TimeResults insert_delete_test(Document *doc) {
    double avg_insert_time = 0;
    double avg_delete_time = 0;
    g_used_ids_count = 0;
    for (int i = 0; i < BATCH_SIZE; i++) {
        node_id_t id = insert_node(doc, generate_random_node());
        g_used_ids[g_used_ids_count++] = id;
        double insert_time = document_get_insertion_time_ms();
        avg_insert_time = (avg_insert_time * i + insert_time) / (i + 1);
    }

    for (int i = 0; i < BATCH_SIZE / 2; ++i) {
        node_id_t id = g_used_ids[i];
        delete_node(doc, (Node) {.id = id});
        double delete_time = document_get_deletion_time_ms();
        avg_delete_time = (avg_delete_time * i + delete_time) / (i + 1);
    }

    return (struct TimeResults) {.insert_time = avg_insert_time, .delete_time = avg_delete_time};
}


Document *prepare(void) {
    g_node_variants[0] = (NodeValue) {.type = INT_32, .int_value = 42};
    g_node_variants[1] = (NodeValue) {.type = DOUBLE, .double_value = 5.555};
    g_node_variants[2] = (NodeValue) {.type = STRING, .string_value = {"Hey!!!", .length = strlen("Hey!!!")}};
    g_node_variants[3] = (NodeValue) {.type = BOOL, .bool_value = true};

    bool file_exists = access(DB_FILE, F_OK) == 0;
    if (file_exists && remove(DB_FILE) != 0) {
        LOG_ERR("Failed to remove file %s", DB_FILE);
        exit(1);
    }
    Document *doc = document_new();
    document_init(doc, DB_FILE, PAGE_SIZE);
    insert_node(doc, gen_root_node());
    return doc;
}

/*
 * Collects data about the memory usage and file size over loop and writes it to a csv file.
 * Metrics to collect: insert_time, delete_time, mem_usage, file_size
 */
int main(void) {
    FILE *fp;
    const char *filename = "benchmark.csv";
    fp = fopen(filename, "w");
    fprintf(fp, "row,insert_time,delete_time,mem_usage,file_size\n");
    long file_size;
    long mem_usage;

    Document *doc = prepare();

    for (int i = 0; i < MAX_MEASUREMENTS; i++) {
        struct TimeResults time_results = insert_delete_test(doc);
        double insert_time = time_results.insert_time;
        double delete_time = time_results.delete_time;
        mem_usage = get_mem_usage();
        file_size = get_file_size(DB_FILE);
        fprintf(fp, "%d,%f,%f,%ld,%ld\n", i, insert_time, delete_time, mem_usage, file_size);
    }
    fclose(fp);
    return 0;
}
