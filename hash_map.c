#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "hash_map.h"

#define FNV_OFFSET 0xcbf29ce484222325
#define FNV_PRIME 0x00000100000001b3
#define INITIAL_CAPACITY 32
#define LOAD_FACTOR 0.75f
#define HAS_VALID_ALLOC_CTX(desc) \
	((desc).alloc_ctx_cb && (desc).dealloc_ctx_cb && (desc).ctx)
#define HAS_VALID_MALLOC_AND_FREE(desc) \
	((desc).malloc_cb && (desc).free_cb)


/* TODO: Change cast style from '(type)(variable)' to '(type) variable'. */

static void *alloc_w_desc(struct hash_map_alloc_desc *alloc_desc,
	size_t size)
{
	void *buf =  NULL;

	/* NOTE: Caller functions should have done null checking beforehand. */
	if (HAS_VALID_ALLOC_CTX(*alloc_desc)) {
		buf = alloc_desc->alloc_ctx_cb(alloc_desc->ctx, size);
	} else if (HAS_VALID_MALLOC_AND_FREE(*alloc_desc)) {
		buf = alloc_desc->malloc_cb(size);
	}
	/* NOTE: Check for NULL at function call site. */
	return buf;
}

static void free_w_desc(struct hash_map_alloc_desc *alloc_desc, void *ptr)
{
	/* NOTE: Caller functions should have done null checking beforehand. */
	if (HAS_VALID_ALLOC_CTX(*alloc_desc)) {
		alloc_desc->dealloc_ctx_cb(alloc_desc->ctx, ptr);
	} else {
		buf = alloc_desc->free_cb(ptr);
	}
}

static int compare_unsigned_int(void *lhs, void *rhs, size_t size)
{
	union {
		int8_t i8;
		int16_t i16;
		int32_t i32;
		int64_t i64;
	} tmp_lhs, tmp_rhs;
	int cmp;

	switch (size) {
	case sizeof(int8_t):
		memcpy(&tmp_lhs.i8, lhs, sizeof(int8_t));
		memcpy(&tmp_rhs.i8, rhs, sizeof(int8_t));
		cmp = (tmp_lhs.i8 > tmp_rhs.i8) - (tmp_lhs.i8 < tmp_rhs.i8);
		break;
	case sizeof(int16_t):
		memcpy(&tmp_lhs.i16, lhs, sizeof(int16_t));
		memcpy(&tmp_rhs.i16, rhs, sizeof(int16_t));
		cmp = (tmp_lhs.i16 > tmp_rhs.i16) - (tmp_lhs.i16 < tmp_rhs.i16);
		break;
	case sizeof(int32_t):
		memcpy(&tmp_lhs.i32, lhs, sizeof(int32_t));
		memcpy(&tmp_rhs.i32, rhs, sizeof(int32_t));
		cmp = (tmp_lhs.i32 > tmp_rhs.i32) - (tmp_lhs.i32 < tmp_rhs.i32);
		break;
	case sizeof(int64_t):
		memcpy(&tmp_lhs.i64, lhs, sizeof(int64_t));
		memcpy(&tmp_rhs.i64, rhs, sizeof(int64_t));
		cmp = (tmp_lhs.i64 > tmp_rhs.i64) - (tmp_lhs.i64 < tmp_rhs.i64);
		break;
	default:
		cmp = 0;
	}
	return cmp;
}

static int compare_unsigned_int(void *lhs, void *rhs, size_t size)
{
	union {
		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
	} tmp_lhs, tmp_rhs;
	int cmp;

	switch (size) {
	case sizeof(uint8_t):
		memcpy(&tmp_lhs.u8, lhs, sizeof(uint8_t));
		memcpy(&tmp_rhs.u8, rhs, sizeof(uint8_t));
		cmp = (tmp_lhs.u8 > tmp_rhs.u8) - (tmp_lhs.u8 < tmp_rhs.u8);
		break;
	case sizeof(uint16_t):
		memcpy(&tmp_lhs.u16, lhs, sizeof(uint16_t));
		memcpy(&tmp_rhs.u16, rhs, sizeof(uint16_t));
		cmp = (tmp_lhs.u16 > tmp_rhs.u16) - (tmp_lhs.u16 < tmp_rhs.u16);
		break;
	case sizeof(uint32_t):
		memcpy(&tmp_lhs.u32, lhs, sizeof(uint32_t));
		memcpy(&tmp_rhs.u32, rhs, sizeof(uint32_t));
		cmp = (tmp_lhs.u32 > tmp_rhs.u32) - (tmp_lhs.u32 < tmp_rhs.u32);
		break;
	case sizeof(uint64_t):
		memcpy(&tmp_lhs.u64, lhs, sizeof(uint64_t));
		memcpy(&tmp_rhs.u64, rhs, sizeof(uint64_t));
		cmp = (tmp_lhs.u64 > tmp_rhs.u64) - (tmp_lhs.u64 < tmp_rhs.u64);
		break;
	default:
		cmp = 0;
	}
	return cmp;
}

static int compare_float(void *lhs, void *rhs, size_t size)
{
	union {
		float f;
		double d;
		long double ld;
	} tmp_lhs, tmp_rhs;
	long double diff;

	/* TODO: Use isnan() for values. */
	/* TODO: Check if comparing LDBL_EPSILON with diff is still valid,
	 * even after casting up from float and double. Do we need FLT_EPSILON
	 * etc., for each specific type? */
	switch (size) {
	case sizeof(float):
		memcpy(&tmp_lhs.f, lhs, sizeof(float));
		memcpy(&tmp_rhs.f, rhs, sizeof(float));
		diff =  (long double) (tmp_lhs.f - tmp_rhs.f);
		break;
	case sizeof(double):
		memcpy(&tmp_lhs.d, lhs, sizeof(double));
		memcpy(&tmp_rhs.d, rhs, sizeof(double));
		diff =  (long double) (tmp_lhs.d - tmp_rhs.d);
		break;
	case sizeof(long double):
		memcpy(&tmp_lhs.ld, lhs, sizeof(long double));
		memcpy(&tmp_rhs.ld, rhs, sizeof(long double));
		diff =  (long double) (tmp_lhs.ld - tmp_rhs.ld);
		break;
	default:
		diff = 0;
	}
	return (fabsl(diff) < LDBL_EPSILON) ? 0 : ((diff < 0) ? -1 : 1);
}

static int compare_string(void *lhs, void *rhs, size_t size)
{
	(void) size;
	return strncmp((const char *) lhs, (const char *) rhs, size);
}

static int compare_ptr(void *lhs, void *rhs, size_t size)
{
	return (lhs > rhs) ? 1 : ((lhs < rhs) ? -1 : 0);
}

static unsigned long long hash(char *key)
{
	unsigned long long  hash;

	hash = FNV_OFFSET;
	for (char *ch = key; *ch; ch++) {
		hash ^= (unsigned char)(*ch);
		hash *= FNV_PRIME;
	}
	return hash;
}

static unsigned long long calc_index(char *key, unsigned long long capacity)
{
	return (unsigned long long) (hash(key) & (capacity - 1));
}

static struct hash_map_entry *init_map_set(const size_t capacity,
	struct hash_map_alloc_desc *alloc_desc)
{
	struct hash_map_entry *set = NULL;

	/* NOTE: Caller functions should have done null checking beforehand. */
	set = alloc_w_desc(alloc_desc,
		sizeof(struct hash_map_entry) * capacity);
	if (set) {
		memset(set, 0, sizeof(struct hash_map_entry) * capacity);
	}
	return set;
}

static void set_map_entry(struct hash_map_entry *set, const size_t capacity,
	char *key, void *value, const struct hash_map_alloc_desc *alloc_desc)
{
	unsigned long long index;
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
	str = (char *) alloc_w_desc(alloc_desc, len);
	if (!str) {
		return;
	}
	set[index].key = memcpy(str, key, len);
	set[index].value = value;
}

void hash_map_init(struct hash_map *map, enum hash_map_key_type key_type,
	struct hash_map_trait_desc *trait_desc,
	struct hash_map_alloc_desc *alloc_desc)
{
	if (!map) {
		return;
	}
	memset(map, 0, sizeof(struct hash_map));
	if (trait_desc && trait_desc.compare && trait_desc.hash) {
		/* NOTE: Research if memcpy() is preferrable. */
		map->trait_desc = *trait_desc;

	} else {
		switch (key_type) {
		case HASH_MAP_KEY_TYPE_CHAR:
			/* TODO: Find out if we can treat chars as int. */
			/* fall-through */
		case HASH_MAP_KEY_TYPE_INT:
			map->trait_desc.compare = compare_int;
			break;
		case HASH_MAP_KEY_TYPE_UINT:
			map->trait_desc.compare = compare_unsigned_int;
			break;
		case HASH_MAP_KEY_TYPE_FLOAT:
			/* fall-through */
		case HASH_MAP_KEY_TYPE_DOUBLE:
			/* fall-through */
		case HASH_MAP_KEY_TYPE_DOUBLE_LONG:
			map->trait_desc.compare = compare_float;
			break;
		case HASH_MAP_KEY_TYPE_STRING:
			map->trait_desc.compare = compare_string;
			break;
		case HASH_MAP_KEY_TYPE_PTR:
			map->trait_desc.compare = compare_ptr;
		     break;
		case HASH_MAP_KEY_TYPE_CUSTOM:
		     /* fall-through */
		case HASH_MAP_KEY_TYPE_UNDEFINED:
		     /* NOTE: Not necessary. */
		     /* fall-through */
		default:
		     /* NOTE: We expliclity set it to undefined in case we
		      * passed in a type we do not recognize or if we specified
		      * type to be custom but did not provide a trait
		      * descriptor for it.*/
		     key_type = HASH_MAP_KEY_TYPE_UNDEFINED;
		     map->trait_desc.compare = memcmp;
		     break;
		};
		map->trait_desc.hash = hash;
	}
	if (alloc_desc) {
		if (HAS_VALID_MALLOC_AND_FREE(*alloc_desc)
				|| HAS_VALID_ALLOC_CTX(*alloc_desc)) {
			map->alloc_desc = *alloc_desc;
		}
	} else {
		map->alloc_desc.malloc_cb = malloc;
		map->alloc_desc.free_cb = free;
	}
	map->set = init_map_set(INITIAL_CAPACITY, &map->alloc_desc);
	map->key_type = key_type;
}

void hash_map_finish(struct hash_map *map)
{
	if (!map) {
		return;
	}
	if (map->set && map->alloc_desc) {
		for (size_t i = 0; i < map->capacity; i++) {
			if (map->set[i].key) {
				free_w_desc(map->alloc_desc, map->set[i].key);
			}
		}
		free_w_desc(map->alloc_desc, map->set);
	}
}

void hash_map_insert(struct hash_map *map, char *key, void *value)
{
	if (!map || !key) {
		return;
	}
	if (!map->set) {
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
		free_w_desc(map->alloc_desc, map->set);
		map->set = new_set;
		map->capacity = new_cap;
	}
	set_map_entry(map->set, map->capacity, key, value, &map->alloc_desc);
	map->size++;
}

int hash_map_at(const struct hash_map *map, char *key, void **value)
{
	unsigned long long index;

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
