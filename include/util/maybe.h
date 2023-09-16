// Macro which represents maybe structure with value and error bool flag and creates function with name
// <function_name>_result which returns maybe structure
#define MAYBE(name, value_type) \
    typedef struct { \
        value_type value; \
        bool is_present; \
    } maybe_##name; \
    maybe_##name maybe_##name##_nothing() { \
       return (maybe_##name) {.is_present = false}; \
    } \
    maybe_##name maybe_##name##_set(value_type value) { \
       return (maybe_##name) {.value = value, .is_present = true}; \
    }

