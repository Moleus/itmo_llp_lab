#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "public/document_db/node.h"
#include "public/document_db/document.h"

long get_mem_usage() {
    FILE *file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128] = "";

    while (fscanf(file, "%127s", line) == 1) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            if (fscanf(file, "%d", &result) != 1) {
                result = -1;
            }
            break;
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

#define BATCH_SIZE 100
//1. Сделать тест, где вставки и удаления происходят вперемешку (500 вставок, 200 удалений)
//- каждая вставка в случайное место
//- после каждой вставки мерить время и размер файла
//измерение вставок:
//t = time()
//write * 100
//dt = time() - t
double insert_test() {
    clock_t start = clock(), end;
    double insert_time;
    for (int i = 0; i < BATCH_SIZE; i++) {
        do_insert();
    }
    end = clock();
    insert_time = (double) (end - start) / CLOCKS_PER_SEC;
    return insert_time;
}

#define MAX_MEASUREMENTS 100
#define MEASURE_EVERY 30
#define DB_FILE "benchmark-data.llp"
#define PAGE_SIZE 4096

node_id_t g_used_ids[MAX_MEASUREMENTS * BATCH_SIZE];

// page_id, item_id
long g_indirect_index_in_used_ids[MAX_MEASUREMENTS * BATCH_SIZE][PAGE_SIZE / sizeof(ItemMetadata)];
long g_used_ids_count = 0;
NodeValue g_node_variants[4];

Node generate_random_node() {
    NodeValue node_value = g_node_variants[rand() % 4];
    node_id_t parent_id = g_used_ids[rand() % g_used_ids_count];
    Node node = (Node) {.parent_id = parent_id, .value = node_value};
    return node;
}

Node gen_root_node() {
    NodeValue node_value = g_node_variants[rand() % 4];
    Node node = (Node) {.parent_id = NULL_NODE_ID, .value = node_value};
    return node;
}

CreateNodeRequest wrap_node(Node node) {
    CreateNodeRequest request = (CreateNodeRequest) {.parent = node.parent_id, .value = node.value};
    return request;
}

void insert_node(Document *doc, Node node) {
    Node result;
    CreateNodeRequest req = wrap_node(node);
    document_add_node(doc, &req, &result);
    g_used_ids[g_used_ids_count++] = result.id;
}

void delete_node(Document *doc, Node node) {
    DeleteNodeRequest req = (DeleteNodeRequest) {.node = &node};
    document_delete_node(doc, &req);
}

Document *prepare() {
    g_node_variants[0] = (NodeValue) {.type = INT_32, .int_value = 42};
    g_node_variants[1] = (NodeValue) {.type = DOUBLE, .double_value = 5.555};
    g_node_variants[2] = (NodeValue) {.type = STRING, .string_value = "Hey!!!"};
    g_node_variants[3] = (NodeValue) {.type = BOOL, .bool_value = true};

    Document *doc = document_new();
    document_init(doc, DB_FILE, PAGE_SIZE);
    insert_node(doc, gen_root_node());
    return doc;
}
/*
 * Collects data about the memory usage and file size over loop and writes it to a csv file.
 * Metrics to collect: insert_time, delete_time, mem_usage, file_size
 */
int main() {
    FILE *fp;
    const char* filename = "benchmark.csv";
    fp = fopen(filename, "w+");
    fprintf(fp, "insert_time,delete_time,mem_usage,file_size\n");

    Document *doc = prepare();

    for (int i = 0; i < MAX_MEASUREMENTS; i++) {
        clock_t start = clock(), end;
        double insert_time;
        double delete_time;

        insert_time = insert_test();
        for (int j = 0; j < MEASURE_EVERY; j++) {
            do_insert();
        }
        long mem_usage = get_mem_usage();
        long file_size = get_file_size("benchmark.csv");
        end = clock();
        insert_time = (double) (end - start) / CLOCKS_PER_SEC;
        fprintf(fp, "%f,%f,%ld,%ld\n", insert_time, delete_time, mem_usage, file_size);

    }

    long mem_usage = get_mem_usage();
    long file_size = get_file_size("benchmark.csv");
    fprintf(fp, "%d,%d,%ld,%ld\n", 0, 0, mem_usage, file_size);

    fclose(fp);
    return 0;
}