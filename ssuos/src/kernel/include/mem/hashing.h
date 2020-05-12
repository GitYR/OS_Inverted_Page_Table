#ifndef __HASHING_H__
#define __HASHING_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <type.h>
#include <proc/proc.h>
#include <ssulib.h>

#define SLOT_NUM 4							// The number of slots in a bucket
#define CAPACITY 256						// level hash table capacity

typedef struct entry
{
    uint32_t key; // page의 인덱스 : palloc_get_page()로 얻어오는 값
    uint32_t value; // page의 실제주소
}entry;

typedef struct level_bucket
{
    uint8_t token[SLOT_NUM]; // slot에 아이템이 삽입되었다면 1,아니라면 0
    entry slot[SLOT_NUM]; // there are 4 slots in a level bucket.
}level_bucket;

typedef struct level_hash 
{
    level_bucket top_buckets[CAPACITY]; 
    level_bucket bottom_buckets[CAPACITY / 2]; // 2 top_bucket = bottom_bucket
}level_hash;

level_hash hash_table; // 하나의 해시 테이블

void init_hash_table(void); // for initializing the hash table

void hash_value_delete(uint32_t * addr,size_t page_idx);

void hash_value_insert(uint32_t * addr,entry node);

void hash_value_move(uint32_t * addr,entry node,_Bool isTop);

_Bool is_empty(uint32_t * addr,_Bool isTop); // 옮겨야할 위치가 비었는지 확인

uint32_t F_IDX(uint32_t addr, uint32_t capacity);	// Get first index to use at table

uint32_t S_IDX(uint32_t addr, uint32_t capacity);	// Get second index to use at table

#endif
