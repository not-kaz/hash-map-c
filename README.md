# hash_map_c
Naive implementation of a hash map written in C, using the FNV1a hash function.</br>
Made as an exercise and for personal use in hobby projects, probably not wise to use for production.

Collisions are handled with linear probing, for the sake of simplicity and locality(?).</br>
No removal implemented yet, will see if it's necessary.

```
#include "hash_map.h"

int main(void)
{
	struct hash_map *map;

	map = hash_map_create();
	hash_map_insert(map, "passcode", (void *)(intptr_t)(12345));
	hash_map_destroy(map);
	return 0;
}
```
