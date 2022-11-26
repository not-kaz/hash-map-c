#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdint.h>

struct hash_map *hash_map_create(void);
void hash_map_destroy(struct hash_map *map);
void hash_map_insert(struct hash_map *map, char *key, void *value);
void *hash_map_at(struct hash_map *map, char *key);
uint64_t hash_map_length(struct hash_map *map);
uint64_t hash_map_capacity(struct hash_map *map);

#endif
