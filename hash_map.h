#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* NOTE: Address-of operators in macros might lead to unexpected behaviour. */
#define HASH_MAP_INIT(map, key_type) \
	hash_map_init(map, key_type, NULL, NULL)
#define HASH_MAP_INSERT(map, key, key_size, value, value_size) \
	hash_map_insert(map, (void *) key, key_size, (void *) value, value_size)
#define HASH_MAP_AT(map, key, key_size, value) \
	hash_map_at(map, key, key_size, (void *) &(value))
#define HASH_MAP_ITER_FOR_EACH(iter, key, value) \
	for (; hash_map_iter_next(iter, &(key), (void **) &(value)); )

enum hash_map_key_type {
	HASH_MAP_KEY_TYPE_UNDEFINED,
	HASH_MAP_KEY_TYPE_INT,
	HASH_MAP_KEY_TYPE_UINT,
	HASH_MAP_KEY_TYPE_FLOAT,
	HASH_MAP_KEY_TYPE_DOUBLE,
	HASH_MAP_KEY_TYPE_DOUBLE_LONG,
	HASH_MAP_KEY_TYPE_CHAR,
	HASH_MAP_KEY_TYPE_STRING,
	HASH_MAP_KEY_TYPE_PTR,
	HASH_MAP_KEY_TYPE_CUSTOM,
	HASH_MAP_KEY_TYPE_NUM_OF
};

struct hash_map_trait_desc {
	int (*compare)(const void *, const void *, size_t);
	uint64_t (*hash)(void *, size_t);
};

struct hash_map_alloc_desc {
	void *(*malloc_cb)(size_t);
	void (*free_cb)(void *);
	void *(*alloc_ctx_cb)(void *, size_t);
	void (*dealloc_ctx_cb)(void *, void *);
	void *ctx;
};

struct hash_map_entry {
	void *key;
	void *value;
	size_t key_size;
	size_t value_size;
};

struct hash_map_set {
	struct hash_map_entry *entries;
	size_t size;
	size_t capacity;
};

struct hash_map {
	struct hash_map_trait_desc trait_desc;
	struct hash_map_alloc_desc alloc_desc;
	struct hash_map_set set;
	enum hash_map_key_type key_type;
};

struct hash_map_iter {
	struct hash_map *map;
	size_t index;
};

void hash_map_init(struct hash_map *map, enum hash_map_key_type key_type,
	struct hash_map_trait_desc *trait_desc,
	struct hash_map_alloc_desc *alloc_desc);
void hash_map_finish(struct hash_map *map);
void hash_map_insert(struct hash_map *map, void *key, size_t key_size,
	void *value, size_t value_size);
int hash_map_at(const struct hash_map *map, void *key, size_t key_size,
	void **value);
size_t hash_map_size(const struct hash_map *map);
size_t hash_map_capacity(const struct hash_map *map);
void hash_map_iter_init(struct hash_map_iter *iter, struct hash_map *map);
void hash_map_iter_finish(struct hash_map_iter *iter);
int hash_map_iter_next(struct hash_map_iter *iter, char **key, void **value);

#endif
