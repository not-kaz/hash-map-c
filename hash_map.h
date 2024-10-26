#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stddef.h>

#define HASH_MAP_INSERT(map, key, val) hash_map_insert(map, key, (void *)(val))
#define HASH_MAP_AT(map, key, val) hash_map_at(map, key, (void *)&(val))
#define HASH_MAP_FOR_EACH(iter, key, value) \
	for (; hash_map_iter_next(iter, &(key), (void **)&(value)); )
/* NOTE: Address-of operators in macro might lead to unexpected behaviour. */

struct hash_map_entry {
	char *key;
	void *value;
};

struct hash_map_alloc_desc {
	void *(*alloc_cb)(size_t, void *);
	void *(*realloc_cb)(void *, size_t, void *);
	void (*dealloc_cb)(void *, void *);
	void *allocator_ctx;
};

struct hash_map {
	struct hash_map_entry *set;
	size_t length;
	size_t capacity;
	struct hash_map_alloc_desc alloc_desc;
};

struct hash_map_iter {
	struct hash_map *map;
	size_t index;
};

void hash_map_init(struct hash_map *map,
	const struct hash_map_alloc_desc *alloc_desc);
void hash_map_finish(struct hash_map *map);
void hash_map_insert(struct hash_map *map, char *key, void *val);
int hash_map_at(const struct hash_map *map, char *key, void **val);
size_t hash_map_length(const struct hash_map *map);
size_t hash_map_capacity(const struct hash_map *map);
void hash_map_iter_init(struct hash_map_iter *iter, struct hash_map *map);
void hash_map_iter_finish(struct hash_map_iter *iter);
int hash_map_iter_next(struct hash_map_iter *iter, char **key, void **val);

#endif
