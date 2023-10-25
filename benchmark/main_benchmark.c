#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "public/document_db/node.h"
#include "public/document_db/document.h"
#include "private/storage/file.h"
#include <time.h>

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

#define SEED 42
#define BATCH_SIZE 500
#define MAX_MEASUREMENTS 100
#define DB_FILE "benchmark-data.llp"
#define PAGE_SIZE 80

// растет на 122
// page_id, item_id

Node generate_random_node(NodeValue node_variants[4]) {
    NodeValue node_value = node_variants[(rand() / 3) % 4];
    node_id_t parent_id = ROOT_NODE_ID;

    Node node = (Node) {.parent_id = parent_id, .value = node_value, .id = NULL_NODE_ID};
    return node;
}

Node gen_root_node(NodeValue node_variants[4]) {
    NodeValue node_value = node_variants[(rand() / 3) % 4];
    Node node = (Node) {.parent_id = NULL_NODE_ID, .value = node_value, .id = ROOT_NODE_ID};
    return node;
}

CreateNodeRequest wrap_node(Node node) {
    CreateNodeRequest request = (CreateNodeRequest) {.parent = node.parent_id, .value = node.value};
    return request;
}

node_id_t insert_node(Document *doc, Node node) {
    Node result = {0};
    CreateNodeRequest req = wrap_node(node);
    Result res = document_add_node(doc, &req, &result);
    ABORT_IF_FAIL(res, "Failed to insert node");
    return result.id;
}

void delete_node(Document *doc, Node *node) {
    DeleteNodeRequest req = (DeleteNodeRequest) {.node = node};
    Result res = document_delete_node(doc, &req);
    ABORT_IF_FAIL(res, "Failed to delete node");
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
struct TimeResults insert_delete_test(Document *doc, NodeValue node_variants[4]) {
    static int counter = 0;
    double avg_insert_time = 0;
    double avg_delete_time = 0;
    long used_ids_count = 0;
    node_id_t used_ids[BATCH_SIZE + 1] = {0};
    clock_t t = clock();
    counter++;
    for (int i = 0; i < BATCH_SIZE; i++) {
        node_id_t id = insert_node(doc, generate_random_node(node_variants));
        used_ids[used_ids_count++] = id;
        if (i % 100 == 0) {
            clock_t t2 = clock();
            double insert_time = ((double) (t2 - t)) / CLOCKS_PER_SEC * 1000;
            t = t2;
            printf("%d;%f\n", counter, insert_time);
        }
    }

    clock_t t_del = clock();
    for (int i = 0; i < BATCH_SIZE - 100; ++i) {
        node_id_t id = used_ids[i];
        Node node = {0};
        node.id = id;
        delete_node(doc, &node);
        double delete_time = document_get_deletion_time_ms();
        avg_delete_time = (avg_delete_time * i + delete_time) / (i + 1);
        if (i % 100 == 0) {
            clock_t t2 = clock();
            double d_time = ((double) (t2 - t_del)) / CLOCKS_PER_SEC * 1000;
            t_del = t2;
            printf("%d;;%f\n", counter, d_time);
        }
    }
    fflush(stdout);

    return (struct TimeResults) {.insert_time = avg_insert_time, .delete_time = avg_delete_time};
}


Document *prepare(NodeValue node_variants[4]) {
    bool file_exists = access(DB_FILE, F_OK) == 0;
    if (file_exists && remove(DB_FILE) != 0) {
        LOG_ERR("Failed to remove file %s", DB_FILE);
        exit(1);
    }
    Document *doc = document_new();
    document_init(doc, DB_FILE, PAGE_SIZE);
    insert_node(doc, gen_root_node(node_variants));
    return doc;
}

/*
 * Collects data about the memory usage and file size over loop and writes it to a csv file.
 * Metrics to collect: insert_time, delete_time, mem_usage, file_size
 */
int main(void) {
    FILE *fp = NULL;
    const char *filename = "benchmark.csv";
    fp = fopen(filename, "w");
    fprintf(fp, "row,insert_time,delete_time,mem_usage,file_size\n");
    long file_size;
    long mem_usage;
    srand(SEED);

    NodeValue node_variants[4] = {0};
    String str = {"Hey!!!", .length = strlen("Hey!!!")};
    node_variants[0] = (NodeValue) {.type = INT_32, .int_value = 42};
    node_variants[1] = (NodeValue) {.type = DOUBLE, .double_value = 5.555};
    node_variants[2] = (NodeValue) {.type = STRING, .string_value = str};
    node_variants[3] = (NodeValue) {.type = BOOL, .bool_value = true};
    Document *doc = prepare(node_variants);

    for (int i = 0; i < MAX_MEASUREMENTS; i++) {
        struct TimeResults time_results = insert_delete_test(doc, node_variants);
        double insert_time = time_results.insert_time;
        double delete_time = time_results.delete_time;
        mem_usage = get_mem_usage();
        file_size = get_file_size(DB_FILE);
        fprintf(fp, "%d,%f,%f,%ld,%ld\n", i, insert_time, delete_time, mem_usage, file_size);
    }
    fflush(fp);
    fclose(fp);
    document_destroy(doc);
    return 0;
}
