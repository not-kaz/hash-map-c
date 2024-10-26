#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "hash_map.h"

#define FNV_OFFSET 0xcbf29ce484222325
#define FNV_PRIME 0x00000100000001b3
#define INITIAL_CAPACITY 32
#define LOAD_FACTOR 0.75f

static void *malloc_and_discard_ctx(size_t size, void *ctx)
{
	(void)(ctx);
	return malloc(size);
}

static void *realloc_and_discard_ctx(void *ptr, size_t size, void *ctx)
{
	(void)(ctx);
	return realloc(ptr, size);
}

static void free_and_discard_ctx(void *ptr, void *ctx)
{
	(void)(ctx);
	free(ptr);
}

static uint64_t hash(char *key)
{
	uint64_t hash;

	hash = FNV_OFFSET;
	for (char *ch = key; *ch; ch++) {
		hash ^= (uint8_t)(*ch);
		hash *= FNV_PRIME;
	}
	return hash;
}

static uint64_t calc_index(char *key, uint64_t capacity)
{
	return (uint64_t)(hash(key) & (capacity - 1));
}

static struct hash_map_entry *init_map_set(const size_t capacity, struct
	hash_map_alloc_desc *alloc_desc)
{
	struct hash_map_entry *set = NULL;

	/* NOTE: Caller functions should have done null checking beforehand. */
	set = alloc_desc->alloc_cb(sizeof(struct hash_map_entry)
		* capacity, alloc_desc->allocator_ctx);
	if (set) {
		memset(set, 0, sizeof(struct hash_map_entry) * capacity);
	}
	return set;
}

static void set_map_entry(struct hash_map_entry *set, const size_t capacity,
	char *key, void *value, const struct hash_map_alloc_desc *alloc_desc)
{
	uint64_t index;
	size_t len;
	char *str;

	/* NOTE: We expect a valid null-terminated key, which should have been *
	 * verified by the caller function. */
	index = calc_index(key, capacity);
	while (set[index].key != NULL) {
		if (strcmp(set[index].key, key) == 0) {
			set[index].value = value;
			return;
		}
		index++;
		if (index >= capacity) {
			index = 0;
		}
	}
	len = strlen(key) + 1;
	str = (char *)(alloc_desc->alloc_cb(len, alloc_desc->allocator_ctx));
	if (!str) {
		return;
	}
	set[index].key = memcpy(str, key, len);
	set[index].value = value;
}

void hash_map_init(struct hash_map *map,
	const struct hash_map_alloc_desc *alloc_desc)
{
	if (!map) {
		return;
	}
	memset(map, 0, sizeof(struct hash_map));
	if (alloc_desc) {
		/* TODO: Verify 'alloc_desc' values. If invalid, provide *
		 * alternatives per callback. */
		map->alloc_desc = *alloc_desc;
	} else {
		map->alloc_desc.alloc_cb = malloc_and_discard_ctx;
		map->alloc_desc.realloc_cb = realloc_and_discard_ctx;
		map->alloc_desc.dealloc_cb = free_and_discard_ctx;
		map->alloc_desc.allocator_ctx = NULL;
	}
	map->set = init_map_set(INITIAL_CAPACITY, &map->alloc_desc);
	if (!map->set) {
		return;
	}
	map->capacity = INITIAL_CAPACITY;
}

void hash_map_finish(struct hash_map *map)
{
	if (!map) {
		return;
	}
	if (map->set) {
		for (size_t i = 0; i < map->capacity; i++) {
			if (map->set[i].key) {
				map->alloc_desc.dealloc_cb(map->set[i].key,
					map->alloc_desc.allocator_ctx);
			}
		}
		map->alloc_desc.dealloc_cb(map->set,
			map->alloc_desc.allocator_ctx);
	}
}

void hash_map_insert(struct hash_map *map, char *key, void *value)
{
	if (!map || !key || !map->set) {
		return;
	}
	if ((float)(map->size) / (float)(map->capacity) >= LOAD_FACTOR) {
		struct hash_map_entry *new_set;
		size_t new_cap;

		new_cap = (map->capacity) * 2;
		new_set = init_map_set(new_cap, &map->alloc_desc);
		if (!new_set) {
			return;
		}
		for (size_t i = 0; i < map->capacity; i++) {
			if (map->set[i].key == NULL) {
				continue;
			}
			set_map_entry(new_set, new_cap, map->set[i].key,
				map->set[i].value, &map->alloc_desc);
		}
		map->alloc_desc.dealloc_cb(map->set,
			map->alloc_desc.allocator_ctx);
		map->set = new_set;
		map->capacity = new_cap;
	}
	set_map_entry(map->set, map->capacity, key, value, &map->alloc_desc);
	map->size++;
}

int hash_map_at(const struct hash_map *map, char *key, void **value)
{
	uint64_t index;

	if (!map || !map->set) {
		return 0;
	}
	index = calc_index(key, map->capacity);
	while (map->set[index].key != NULL) {
		if (strcmp(key, map->set[index].key) == 0) {
			(*value) = map->set[index].value;
			return 1;
		}
		index++;
		if (index >= map->capacity) {
			index = 0;
		}
	}
	return 0;
}

size_t hash_map_size(const struct hash_map *map)
{
	return map ? map->size : 0;
}

size_t hash_map_capacity(const struct hash_map *map)
{
	return map ? map->capacity : 0;
}

void hash_map_iter_init(struct hash_map_iter *iter, struct hash_map *map)
{
	if (!iter) {
		return;
	}
	memset(iter, 0, sizeof(struct hash_map_iter));
	if (map) {
		iter->map = map;
	}
}

void hash_map_iter_finish(struct hash_map_iter *iter)
{
	if (iter) {
		memset(iter, 0, sizeof(struct hash_map_iter));
	}
}

int hash_map_iter_next(struct hash_map_iter *iter, char **key, void **value)
{
	/* NOTE: This function should NOT be used explicitly, it is intended as
	 * an internal function for macros. */
	if (!iter || !key || !value) {
		return 0;
	}
	while (iter->index < iter->map->capacity) {
		if (iter->map->set[iter->index].key != NULL) {
			*key = iter->map->set[iter->index].key;
			*value = iter->map->set[iter->index].value;
			iter->index++;
			return 1;
		}
		iter->index++;
	}
	return 0;
}
