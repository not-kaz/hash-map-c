# hash_map_c
Naive and portable implementation of a hash map written in ANSI C99, using the FNV1a hash function.</br>
Made as an exercise and for personal use in hobby projects.

Collisions are handled with linear probing, for the sake of simplicity and locality(?).</br>
No removal implemented, yet to be decided if it's necessary.

```c
#include "hash_map.h"

int main(void)
{
	/* Hash map structs */
	struct hash_map map;
	struct hash_map_iter iter;
	/* Data variables */
	int passcode;
	int age;
	/* Pointers for data access */
	char *key;
	int *value;

	passcode = 2928;
	age = 76;
	/* If we do not provide an allocator descriptor, the map will use the standard library. */
	hash_map_init(&map, NULL);
	hash_map_iter_init(&iter, &map);
	hash_map_insert(&map, "passcode", &passcode);
	hash_map_insert(&map, "age", &age);
	HASH_MAP_ITER_FOR_EACH(&iter, key, value) {
		printf("%s : %d\n", key, (*value));
	}
	HASH_MAP_AT(&map, "passcode", value);
	hash_map_iter_finish(&iter);
	hash_map_finish(&map);
	return 0;
}
```
TODO: Actually store value data into the hash_map, not just pointers to the data.
