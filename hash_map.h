#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdint.h>

struct hash_map *hash_map_create(void);
void hash_map_destroy(struct hash_map *map);
void hash_map_insert(struct hash_map *map, char *key, void *value);
int hash_map_at(struct hash_map *map, char *key, void **value);
uint64_t hash_map_length(struct hash_map *map);
uint64_t hash_map_capacity(struct hash_map *map);
struct hash_map_iter *hash_map_iter_create(struct hash_map *map);
void hash_map_iter_destroy(struct hash_map_iter *iter);
int hash_map_iter_next(struct hash_map_iter *iter, char **key, void **value);

#endif
