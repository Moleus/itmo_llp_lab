#include <stdlib.h>
#include <stdbool.h>

// Структуры для представления информации в оперативной памяти.
// Типы значений int32_t, float, char*, bool
// создать типы для узла, документа, дерева документов. В узле могут быть значения, а могут быть ссылки на другие узлы
typedef struct {
    const char *value;
    size_t length;
} String;

typedef enum {
    INT,
    FLOAT,
    STRING,
    BOOL
} ValueType;

typedef struct {
    ValueType type;
    union {
        int32_t int_value;
        double double_value;
        String string_value;
        bool bool_value;
    };
} NodeValue;

typedef struct {
    size_t id;
    char *name;
    struct Node *parent_node;
    struct Node *children;
    size_t children_count;
    NodeValue value;
} Node;

typedef struct {
    size_t id;
    char *name;
    Node *root_node;
} Document;

typedef struct {
    Document *documents;
    size_t documents_count;
    size_t capacity;
} DocumentTree;

// Структуры для представления информации в оперативной памяти о запросе:
// создать типы для запросов на создание, редактирование и удаление ноды
typedef struct {
    NodeValue value;
    Node* parent;
} CreateNodeRequest;

typedef struct {
    Node* node;
    NodeValue new_value;
} UpdateNodeRequest;

typedef struct {
    Node* node;
} DeleteNodeRequest;

// Структура для поиска ноды подходящей под условие
typedef struct {
    Node* node;
    bool (*condition)(Node*);
} FindNodeRequest;