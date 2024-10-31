# hash_map_c
Naive and portable implementation of a hash map (view) written in C99, using the FNV1a hash function.</br>
Made as an exercise and for personal use in hobby projects.

This implementation only holds pointers to key and value pairs; no copying occurs, therefore it is the user's responsibility to maintain the data.</br>
Collisions are handled with linear probing, for the sake of simplicity and locality(?).</br>
No removal implemented, to be considered.</br>

*Opaque macro version for ease-of-use*
```c
#include <stdio.h>
#include <string.h>
#include "hash_map.h"

int main(void)
{
	struct hash_map map;
	struct hash_map_iter iter;
	/* Data */
	const char *strings[] = {"alpha", "beta", "gamma", "epsilon", "delta"};
	int nums[] = {83, 2, 3, 4, 5};
	/* Data accessors */
	char *key;
	void *val;

	HASH_MAP_INIT(&map, HASH_MAP_KEY_TYPE_STRING);
	HASH_MAP_INSERT(&map, strings[0], strlen(strings[0]) + 1, &nums[0],
		sizeof(nums[0]));
	HASH_MAP_INSERT(&map, strings[0], strlen(strings[0]) + 1, &nums[0],
		sizeof(nums[0]));
	HASH_MAP_INSERT(&map, strings[1], strlen(strings[1]) + 1, &nums[1],
		sizeof(nums[1]));
	HASH_MAP_INSERT(&map, strings[2], strlen(strings[2]) + 1, &nums[2],
		sizeof(nums[2]));
	HASH_MAP_INSERT(&map, strings[3], strlen(strings[3]) + 1, &nums[3],
		sizeof(nums[3]));
	HASH_MAP_INSERT(&map, strings[4], strlen(strings[4]) + 1, &nums[4],
		sizeof(nums[4]));
	hash_map_iter_init(&iter, &map);
	HASH_MAP_ITER_FOR_EACH(&iter, key, val) {
		printf("%s : %d\n", key, *(int *)val);
	}
	hash_map_iter_finish(&iter);
	hash_map_finish(&map);
	return 0;
}
```

*Explicit function version*
```c
#include <stdio.h>
#include <string.h>
#include "hash_map.h"

int main(void)
{
	/* TODO */
}
```

