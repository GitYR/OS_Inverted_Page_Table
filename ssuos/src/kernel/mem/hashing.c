#include <ssulib.h>
#include <interrupt.h>
#include <proc/proc.h>
#include <device/io.h>
#include <device/console.h>
#include <mem/mm.h>
#include <mem/palloc.h>
#include <mem/paging.h>
#include <mem/hashing.h>

// Get first index to use at table.
// HASH1 함수
uint32_t F_IDX(uint32_t addr, uint32_t capacity) 
{
    return addr % ((capacity / 2) - 1);
}

// Get second index to use at table.
// HASH2 함수
uint32_t S_IDX(uint32_t addr, uint32_t capacity)
{
    return (addr * 7) % ((capacity / 2) - 1) + capacity / 2;
}

/* Level hash 테이블의 초기화 함수 */
void init_hash_table(void)
{
	// hash_table은 level_hash구조체 변수
	memset((level_hash *)&hash_table,0,sizeof(level_hash));
}

void hash_value_delete(uint32_t * addr,size_t page_idx)
{
	uint32_t idx,value,res;
	_Bool check = 0;
	size_t key; // key -> page_idx
	int i;

	idx = F_IDX((uint32_t)addr,CAPACITY);
	for(i = 0; i < SLOT_NUM; i++) // first top level
	{
		if(hash_table.top_buckets[idx].token[i]) // 1 : FULL
		{
			value = (uint32_t)RH_TO_VH((uint32_t *)hash_table.top_buckets[idx].slot[i].value);
			if(page_idx == hash_table.top_buckets[idx].slot[i].key)
			{
				check = 1;
				res = (uint32_t)VH_TO_RH(addr); 
				key = hash_table.top_buckets[idx].slot[i].key;
				hash_table.top_buckets[idx].token[i] = 0;
				memset((entry *)&hash_table.top_buckets[idx].slot[i],0,sizeof(entry));
				break;
			}
		}
	}
	if(!check)
	{
		idx = S_IDX((uint32_t)addr,CAPACITY);
		for(i = 0; i < SLOT_NUM; i++) // second top level
		{
			if(hash_table.top_buckets[idx].token[i])
			{
				value = (uint32_t)RH_TO_VH((uint32_t *)hash_table.top_buckets[idx].slot[i].value);
				if(page_idx == hash_table.top_buckets[idx].slot[i].key)
				{
					check = 1;
					res = (uint32_t)VH_TO_RH(addr); 
					key = hash_table.top_buckets[idx].slot[i].key;
					hash_table.top_buckets[idx].token[i] = 0;
					memset((entry *)&hash_table.top_buckets[idx].slot[i],0,sizeof(entry));
					break;
				}
			}
		}
	}
	if(!check) // first bottom level
	{
		idx = F_IDX((uint32_t)addr,CAPACITY);
		idx /= 2;
		for(i = 0; i < SLOT_NUM; i++)
		{
			if(hash_table.bottom_buckets[idx].token[i])
			{
				value = (uint32_t)RH_TO_VH((uint32_t *)hash_table.bottom_buckets[idx].slot[i].value);
				if(page_idx == hash_table.bottom_buckets[idx].slot[i].key)
				{
					check = 1;
					res = (uint32_t)VH_TO_RH(addr); 
					key = hash_table.bottom_buckets[idx].slot[i].key;
					hash_table.bottom_buckets[idx].token[i] = 0;
					memset((entry *)&hash_table.bottom_buckets[idx].slot[i],0,sizeof(entry));
					break;
				}
			}
		}
	}
	if(!check) // second bottom level
	{
		idx = S_IDX((uint32_t)addr,CAPACITY);
		idx /= 2;
		for(i = 0; i < SLOT_NUM; i++)
		{
			if(hash_table.bottom_buckets[idx].token[i])
			{
				value = (uint32_t)RH_TO_VH((uint32_t *)hash_table.bottom_buckets[idx].slot[i].value);
				if(page_idx == hash_table.bottom_buckets[idx].slot[i].key)
				{
					check = 1;
					res = (uint32_t)VH_TO_RH(addr); 
					key = hash_table.bottom_buckets[idx].slot[i].key;
					hash_table.bottom_buckets[idx].token[i] = 0;
					memset((entry *)&hash_table.bottom_buckets[idx].slot[i],0,sizeof(entry));
					break;
				}
			}
		}
	}
	if(check)
		printk("hash value deleted : idx : %d, key : %d, value : %x\n", idx,key,res);
}

void hash_value_insert(uint32_t * addr,entry node)
{
	uint32_t idx,idx1,idx2,value = node.value;
	size_t key = node.key;
	_Bool check = 0; // success(1) or fail(0)
	int i;
	char * str;
	
	idx1 = F_IDX((uint32_t)addr,CAPACITY);
	idx2 = S_IDX((uint32_t)addr,CAPACITY);
	
	for(i = 0; i < SLOT_NUM; i++) // 첫 번째 top level
	{
		if(!(hash_table.top_buckets[idx1].token[i])) // 0 : empty
		{
			check = 1;
			hash_table.top_buckets[idx1].token[i] = 1;
			hash_table.top_buckets[idx1].slot[i].key = key;
			hash_table.top_buckets[idx1].slot[i].value = value;
			str = "top level";
			idx = idx1;
			break;
		}
		if(!(hash_table.top_buckets[idx2].token[i])) // 0 : empty
		{
			check = 1;
			hash_table.top_buckets[idx2].token[i] = 1;
			hash_table.top_buckets[idx2].slot[i].key = key;
			hash_table.top_buckets[idx2].slot[i].value = value;
			str = "top level";
			idx = idx2;
			break;
		}
	}
	if(!check) // 이제 bottom level 검색
	{
		idx1 = F_IDX((uint32_t)addr,CAPACITY);
		idx2 = S_IDX((uint32_t)addr,CAPACITY);
		idx1 /= 2;
		idx2 /= 2;

		for(i = 0; i < SLOT_NUM; i++) // 첫 번째 bottom level
		{
			if(!(hash_table.bottom_buckets[idx1].token[i])) // 0 : empty
			{
				check = 1;
				hash_table.bottom_buckets[idx1].token[i] = 1;
				hash_table.bottom_buckets[idx1].slot[i].key = key;
				hash_table.bottom_buckets[idx1].slot[i].value = value;
				str = "bottom level";
				idx = idx1;
				break;
			}
			if(!(hash_table.bottom_buckets[idx2].token[i])) // 0 : empty
			{
				check = 1;
				hash_table.bottom_buckets[idx2].token[i] = 1;
				hash_table.bottom_buckets[idx2].slot[i].key = key;
				hash_table.bottom_buckets[idx2].slot[i].value = value;
				str = "bottom level";
				idx = idx2;
				break;
			}
		}
	}
	if(!check) // 원래 있던 top level의 원소를 옮겨야 함
	{
		entry temp;
		uint32_t * temp_addr,temp_idx;
		idx1 = F_IDX((uint32_t)addr,CAPACITY);
		idx2 = S_IDX((uint32_t)addr,CAPACITY);

		for(i = 0; i < SLOT_NUM; i++)
		{
			if(hash_table.top_buckets[idx1].token[i]) // 첫 번째 top_level 탐색
			{
				temp.key = hash_table.top_buckets[idx1].slot[i].key;
				temp.value = hash_table.top_buckets[idx1].slot[i].value;

				temp_addr = RH_TO_VH(temp.value);
				if(is_empty(temp_addr,1)) // 비어있다면
				{
					check = 1;
					memset((entry *)&hash_table.top_buckets[idx1].slot[i],0,sizeof(entry));
					hash_table.top_buckets[idx1].slot[i].key = key;
					hash_table.top_buckets[idx1].slot[i].value = value;
					hash_table.top_buckets[idx1].token[i] = 1;
					str = "top level";
					idx = idx1;
					break;
				}

			}
			if(hash_table.top_buckets[idx2].token[i]) // 두 번째 top_level 탐색
			{
				temp.key = hash_table.top_buckets[idx2].slot[i].key;
				temp.value = hash_table.top_buckets[idx2].slot[i].value;

				temp_addr = RH_TO_VH(temp.value);
				if(is_empty(temp_addr,1))
				{
					check = 1;
					memset((entry *)&hash_table.top_buckets[idx2].slot[i],0,sizeof(entry));
					hash_table.top_buckets[idx2].slot[i].key = key;
					hash_table.top_buckets[idx2].slot[i].value = value;
					hash_table.top_buckets[idx2].token[i] = 1;
					str = "top level";
					idx = idx2;
					break;
				}
			}
		}

		if(!check) // Bottom level 탐색 <- TOP이동이 불가하다면
		{
			idx1 /= 2;
			idx2 /= 2;
		
			for(i = 0; i < SLOT_NUM; i++)
			{

				if(hash_table.bottom_buckets[idx1].token[i]) 
				{
					temp.key = hash_table.bottom_buckets[idx1].slot[i].key;
					temp.value = hash_table.bottom_buckets[idx1].slot[i].value;

					temp_addr = RH_TO_VH(temp.value);
					if(is_empty(temp_addr,0))
					{
						memset((entry *)&hash_table.bottom_buckets[idx1].slot[i],0,sizeof(entry));
						hash_table.bottom_buckets[idx1].slot[i].key = key;
						hash_table.bottom_buckets[idx1].slot[i].value = value;
						hash_table.bottom_buckets[idx1].token[i] = 1;
						hash_value_move(RH_TO_VH(temp.value),temp,0);
						check = 1;
						str = "bottom level";
						idx = idx1;
						break;
					}
				}
				if(hash_table.bottom_buckets[idx2].token[i]) 
				{
					temp.key = hash_table.bottom_buckets[idx2].slot[i].key;
					temp.value = hash_table.bottom_buckets[idx2].slot[i].value;

					temp_addr = RH_TO_VH(temp.value);
					if(is_empty(temp_addr,0))
					{
						memset((entry *)&hash_table.bottom_buckets[idx2].slot[i],0,sizeof(entry));
						hash_table.bottom_buckets[idx2].slot[i].key = key;
						hash_table.bottom_buckets[idx2].slot[i].value = value;
						hash_table.bottom_buckets[idx2].token[i] = 1;
						hash_value_move(RH_TO_VH(temp.value),temp,0);
						check = 1;
						str = "bottom level";
						idx = idx2;
						break;
					}
				}
			} // end of 'for' loop
		}
		else
			hash_value_move(RH_TO_VH(temp.value),temp,1);
	}

	if(check)
		printk("hash value inserted in %s : idx : %d, key : %d, value : %x\n", str,idx,key,value);

}

_Bool is_empty(uint32_t * addr,_Bool isTop)
{
	uint32_t idx1,idx2;
	int i;

	idx1 = F_IDX((uint32_t)addr,CAPACITY);
	idx2 = S_IDX((uint32_t)addr,CAPACITY);

	if(isTop) // Top Level
	{
		for(i = 0; i < SLOT_NUM; i++)
		{
			if(!(hash_table.top_buckets[idx1].token[i])) // empty
				return 1;
			if(!(hash_table.top_buckets[idx2].token[i])) // empty
				return 1;
		}
	}
	else // Bottom Level
	{
		idx1 /= 2;
		idx2 /= 2;
		for(i = 0; i < SLOT_NUM; i++)
		{
			if(!(hash_table.bottom_buckets[idx1].token[i])) // empty
				return 1;
			if(!(hash_table.bottom_buckets[idx2].token[i])) // empty
				return 1;
		}
	}
	return 0;
}

void hash_value_move(uint32_t * addr,entry node,_Bool isTop)
{
	uint32_t value,idx1,idx2;
	size_t key;
	int i;
	_Bool check = 0;

	idx1 = F_IDX((uint32_t)addr,CAPACITY);
	idx2 = S_IDX((uint32_t)addr,CAPACITY);
	value = node.value;
	key = node.key;

	if(isTop)
	{
		for(i = 0; i < SLOT_NUM; i++)
		{
			if(hash_table.top_buckets[idx1].token[i] == 0)
			{
				hash_table.top_buckets[idx1].token[i] = 1;
				hash_table.top_buckets[idx1].slot[i].key = key;
				hash_table.top_buckets[idx1].slot[i].value = value;
				//printk("move %d to %d\n", idx, idx1);
				break;
			}
			if(hash_table.top_buckets[idx2].token[i] == 0)
			{
				hash_table.top_buckets[idx2].token[i] = 1;
				hash_table.top_buckets[idx2].slot[i].key = key;
				hash_table.top_buckets[idx2].slot[i].value = value;
				//printk("move %d to %d\n", idx, idx2);
				break;
			}
		}
	}
	else // Bottom
	{
		idx1 /= 2;
		idx2 /= 2;

		for(i = 0; i < SLOT_NUM; i++)
		{
			if(hash_table.bottom_buckets[idx1].token[i] == 0)
			{
				hash_table.bottom_buckets[idx1].token[i] = 1;
				hash_table.bottom_buckets[idx1].slot[i].key = key;
				hash_table.bottom_buckets[idx1].slot[i].value = value;
				//printk("move %d to %d\n", idx, idx1);
				break;
			}
			if(hash_table.bottom_buckets[idx2].token[i] == 0)
			{
				hash_table.bottom_buckets[idx2].token[i] = 1;
				hash_table.bottom_buckets[idx2].slot[i].key = key;
				hash_table.bottom_buckets[idx2].slot[i].value = value;
				//printk("move %d to %d\n", idx, idx2);
				break;
			}
		}
	}
}
