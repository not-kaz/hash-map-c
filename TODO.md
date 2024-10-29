# TODO

1. More macros to reduce argument boilerplate code for the user.
2. Explicit functions that do checks against previously established sizes etc.
	1. *hash_map_insert_ex(struct hash_map *map, void *key, void *value, size_t key_size, size_t value_size)*
		- This will compare passed sizes with those defined at hash map creation.
		- We can wrap them in macros HASH_MAP_INSERT(map, key, value).
