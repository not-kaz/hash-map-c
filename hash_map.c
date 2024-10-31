#include <float.h>
#include <math.h>
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
#define IS_VALID_MAP_SET(set) \
	((set).entries && (set).capacity)
#define IS_VALID_KEY_TYPE(type) \
	((type) > HASH_MAP_KEY_TYPE_UNDEFINED \
			&& (type) < HASH_MAP_KEY_TYPE_NUM_OF)


/* TODO: Change cast style from '(type)(variable)' to '(type) variable'. */

static void *alloc_w_desc(const struct hash_map_alloc_desc *alloc_desc,
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

static void free_w_desc(const struct hash_map_alloc_desc *alloc_desc, void *ptr)
{
	/* NOTE: Caller functions should have done null checking beforehand. */
	if (HAS_VALID_ALLOC_CTX(*alloc_desc)) {
		alloc_desc->dealloc_ctx_cb(alloc_desc->ctx, ptr);
	} else {
		alloc_desc->free_cb(ptr);
	}
}

static int compare_int(const void *lhs, const void *rhs, size_t size)
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

static int compare_unsigned_int(const void *lhs, const void *rhs, size_t size)
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

static int compare_float(const void *lhs, const void *rhs, size_t size)
{
	union {
		float f;
		double d;
		long double ld;
	} tmp_lhs, tmp_rhs;
	union {
		float f;
		double d;
		long double ld;
	} diff;

	/* TODO: Use isnan() for values. */
	/* TODO: Check if comparing LDBL_EPSILON with diff is still valid,
	 * even after casting up from float and double. Do we need FLT_EPSILON
	 * etc., for each specific type? */
	switch (size) {
	case sizeof(float):
		memcpy(&tmp_lhs.f, lhs, sizeof(float));
		memcpy(&tmp_rhs.f, rhs, sizeof(float));
		diff.f = tmp_lhs.f - tmp_rhs.f;
		return (fabsf(diff.f) < FLT_EPSILON)
			? 0 : ((diff.f < 0) ? -1 : 1);
	case sizeof(double):
		memcpy(&tmp_lhs.d, lhs, sizeof(double));
		memcpy(&tmp_rhs.d, rhs, sizeof(double));
		diff.d = tmp_lhs.d - tmp_rhs.d;
		return (fabs(diff.d) < DBL_EPSILON)
			? 0 : ((diff.d < 0) ? -1 : 1);
	case sizeof(long double):
		memcpy(&tmp_lhs.ld, lhs, sizeof(long double));
		memcpy(&tmp_rhs.ld, rhs, sizeof(long double));
		diff.ld = tmp_lhs.ld - tmp_rhs.ld;
		return (fabsl(diff.ld) < LDBL_EPSILON)
			? 0 : ((diff.ld < 0) ? -1 : 1);
	default:
		return 0;
	}
}

static int compare_string(const void *lhs, const void *rhs, size_t size)
{
	return strncmp((const char *) lhs, (const char *) rhs, size);
}

static int compare_ptr(const void *lhs, const void *rhs, size_t size)
{
	(void) size;
	return (lhs > rhs) ? 1 : ((lhs < rhs) ? -1 : 0);
}

static uint64_t hash(const void *key, size_t key_size)
{
	uint64_t hash = FNV_OFFSET;
	const unsigned char *data = key;

	for (size_t i = 0; i < key_size; i++) {
		hash ^= data[i];
		hash *= FNV_PRIME;
	}
	return hash;
}

static uint64_t calc_index(const void *key, size_t key_size, uint64_t capacity)
{
	return (uint64_t) (hash(key, key_size) & (capacity - 1));
}

static struct hash_map_set make_map_set(const size_t capacity,
	struct hash_map_alloc_desc *alloc_desc)
{
	struct hash_map_set set = {0};

	set.entries = alloc_w_desc(alloc_desc,
		sizeof(struct hash_map_entry) * capacity);
	if (set.entries) {
		memset(set.entries, 0,
			sizeof(struct hash_map_entry) * capacity);
		set.capacity = capacity;
	}
	return set;
}

static void set_map_entry(struct hash_map *map, const void *key,
	size_t key_size, const void *value, size_t value_size)
{
	uint64_t index;

	/* NOTE: We expect a valid key, which should have been verified by
	 * the caller function. */
	index = calc_index(key, key_size, map->set.capacity);
	while (map->set.entries[index].key != NULL) {
		if (map->trait_desc.compare(key, map->set.entries[index].key,
				key_size) == 0) {
			map->set.entries[index].value = value;
			map->set.entries[index].value_size = value_size;
			return;
		}
		index++;
		if (index >= map->set.capacity) {
			index = 0;
		}
	}
	/* NOTE: We basically just assign new address to the value field,
	 * however, we could assign new value directly to whatever the address
	 * is pointing to. Consider it. */
	map->set.entries[index].value = value;
	map->set.entries[index].value_size = value_size;
	map->set.entries[index].key = key;
	map->set.entries[index].key_size = key_size;

}

void hash_map_init(struct hash_map *map, enum hash_map_key_type key_type,
	struct hash_map_trait_desc *trait_desc,
	struct hash_map_alloc_desc *alloc_desc)
{
	if (!map) {
		return;
	}
	memset(map, 0, sizeof(struct hash_map));
	if (IS_VALID_KEY_TYPE(key_type)) {
		map->key_type = key_type;
	} else {
		return;
	}
	if (trait_desc && trait_desc->compare) { /* && trait_desc->hash) { */
		/* NOTE: Research if memcpy() is preferrable. */
		map->trait_desc.compare = trait_desc->compare;
		map->trait_desc.hash =
			trait_desc->hash ? trait_desc->hash : hash;
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
			/* fall-through */
		default:
			/* NOTE: If the key_type is not a supported default type
			 * and user has not provided a custom compare function
			 * for their type, we consider the map uninitialized. */
			return;
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
	map->set = make_map_set(INITIAL_CAPACITY, &map->alloc_desc);
}

void hash_map_finish(struct hash_map *map)
{
	if (!map) {
		return;
	}
	if (IS_VALID_MAP_SET(map->set)) {
		free_w_desc(&map->alloc_desc, map->set.entries);
	}
}

void hash_map_insert(struct hash_map *map, const void *key, size_t key_size,
	const void *value, size_t value_size)
{

	if (!map || !key || !value || !IS_VALID_MAP_SET(map->set)
			|| !IS_VALID_KEY_TYPE(map->key_type)) {
		return;
	}
	if ((float) map->set.size / (float) map->set.capacity >= LOAD_FACTOR) {
		struct hash_map new_map;

		new_map.set = make_map_set(map->set.capacity * 2,
			&map->alloc_desc);
		if (!IS_VALID_MAP_SET(new_map.set)) {
			return;
		}
		for (size_t i = 0; i < map->set.capacity; i++) {
			if (map->set.entries[i].key == NULL) {
				continue;
			}
			set_map_entry(&new_map, map->set.entries[i].key,
				map->set.entries[i].key_size,
				map->set.entries[i].value,
				map->set.entries[i].value_size);
		}
		free_w_desc(&map->alloc_desc, map->set.entries);
		new_map.set.size = map->set.size;
		map->set = new_map.set;
	}
	set_map_entry(map, key, key_size, value, value_size);
	map->set.size++;
}

int hash_map_at(const struct hash_map *map, const void *key, size_t key_size,
	const void **value)
{
	uint64_t index;

	if (!map || !IS_VALID_MAP_SET(map->set) || !key || !value) {
		return 0;
	}
	index = calc_index(key, key_size, map->set.capacity);
	while (map->set.entries[index].key != NULL) {
		if (map->trait_desc.compare(key, map->set.entries[index].key,
					map->set.entries[index].key_size)
					== 0) {
			(*value) = map->set.entries[index].value;
			return 1;
		}
		index++;
		if (index >= map->set.capacity) {
			index = 0;
		}
	}
	return 0;
}

size_t hash_map_size(const struct hash_map *map)
{
	return map ? map->set.size : 0;
}

size_t hash_map_capacity(const struct hash_map *map)
{
	return map ? map->set.capacity : 0;
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

int hash_map_iter_next(struct hash_map_iter *iter, const void **key,
	const void **value)
{
	/* NOTE: This function should NOT be used explicitly, it is intended as
	 * an internal function for helper macros. */
	if (!iter || !iter->map || !IS_VALID_MAP_SET(iter->map->set)
			|| !key || !value) {
		return 0;
	}
	/* TODO: Implement iterator invalidation, tracking 'seen' elements. */
	while (iter->index < iter->map->set.capacity) {
		if (iter->map->set.entries[iter->index].key != NULL) {
			*key = iter->map->set.entries[iter->index].key;
			*value = iter->map->set.entries[iter->index].value;
			iter->index++;
			return 1;
		}
		iter->index++;
	}
	return 0;
}
