#include <type.h>
#include <round.h>
#include <synch.h>
#include <bitmap.h>
#include <mem/mm.h>
#include <mem/paging.h>
#include <mem/palloc.h>
#include <mem/hashing.h>
#include <device/console.h>

//  Page allocator.  Hands out memory in page-size (or
//  page-multiple) chunks.  

/* kernel heap page struct */
struct khpage
{
	uint16_t page_type; 
	uint16_t nalloc; // the number of allocated pages
	uint32_t used_bit[4]; 
	struct khpage *next; 
}; // 24 Bytes

/* free list */
struct freelist
{
	struct khpage *list;
	int nfree; // the number of free lists
};

static struct khpage * khpage_list; // 커널 힙 페이지 리스트
static struct freelist freelist;
static uint32_t page_alloc_index; // 페이지 할당 인덱스

/* Initializes the page allocator. */
void init_palloc (void) 
{
	/* calculate the space needed for the kernel heap page list */
	// sizeof(struct khpage) : 24, 1024(2^10) -> 1KB
	size_t bm_size = sizeof(struct khpage) * 1024;

	/* khpage list alloc */
	/* KERNEL_ADDR : 1MB */
	/* Initialize a starting address point */
	khpage_list = (struct khpage *)(KERNEL_ADDR); // 1MB에서 시작

	/* initialize */
	memset((void*)khpage_list, 0, bm_size); // 24KB의 크기만큼 1MB에서부터 초기화
	page_alloc_index = 0; // index는 0부터 시작
	freelist.list = NULL;
	freelist.nfree = 0;
}

/* Obtains and returns a group of PAGE_CNT contiguous free pages. */
uint32_t * palloc_get_multiple (size_t page_cnt)
{
	void *pages = NULL;
	struct khpage *khpage = freelist.list;
	struct khpage *prepage = freelist.list;
	size_t page_idx;
	entry node;

	if(page_cnt == 0)
		return NULL;

	while(khpage != NULL) // freelist가 존재한다면 실행
	{
		if(khpage->nalloc == page_cnt) // khpage에 있는 할당된 공간의 수가 page_cnt와 일치한다면
		{
			// khpage_list의 시작위치는 1MB로 초기화되어 있음.
			// 밑의 page_idx 공식은 해당 페이지로 찾아가기 위함.
			// VKERNEL_HEAP_START : 3GB부터 시작, 가상주소 공간의 HEAP영역의 인덱스 
			// pages는 HEAP 영역에 관한 인덱스 변수
			page_idx = ((uint32_t)khpage - (uint32_t)khpage_list) / sizeof(struct khpage); 
			pages = (void*)(VKERNEL_HEAP_START + page_idx * PAGE_SIZE); 

			if(prepage == khpage) // freelist 주소
			{
				freelist.list = khpage->next;
				freelist.nfree--;
				break;
			}
			else // prepage != khpage
			{
				prepage->next = khpage->next;
				freelist.nfree--;
				break;
			}
		}
		prepage = khpage;
		khpage = khpage->next; // for loop operation
	} // the end of while

	if(pages == NULL) // if pages are null :
	{
		pages = (void*)(VKERNEL_HEAP_START + page_alloc_index * PAGE_SIZE);
		page_idx = page_alloc_index;
		page_alloc_index += page_cnt;
	}

	if(pages != NULL) // if pages are not null :
	{
		memset(pages, 0, PAGE_SIZE * page_cnt);
	}
	
	node.key = page_idx;
	node.value = (uint32_t )VH_TO_RH((uint32_t *)pages);
	hash_value_insert((uint32_t *)pages,node); // hash insert

	return (uint32_t*)pages; 
}

/* Obtains a single free page and returns its address.   */
uint32_t * palloc_get_page (void) 
{
	return palloc_get_multiple(1); // 하나의 페이지만 반환
}

/* Frees the PAGE_CNT pages starting at PAGES. */
void palloc_free_multiple (void *pages, size_t page_cnt) 
{
	struct khpage *khpage = freelist.list;
	// 가상주소 공간에서의 페이지 인덱스 구하기 : page_idx
	size_t page_idx = (((uint32_t)pages - VKERNEL_HEAP_START) / PAGE_SIZE);

	// pages의 주소가 VKERNEL_HEAP_START보다 작거나 page_cnt가 0이라면 -> 할당된 페이지가 없다면
	if(pages == NULL || page_cnt == 0)
		return;

	// freelist가 존재하지 않는다면 (init_palloc 이후 사용되지 않거나 전부 다 써버린 상태)
	if(khpage == NULL)
	{
		freelist.list = khpage_list + page_idx; // VKERNEL_HEAP_START부터 할당
		freelist.list->nalloc = page_cnt;
		freelist.list->next = NULL;
	}
	else // freelist가 존재함.
	{
		while(khpage->next != NULL)
			khpage = khpage->next;

		// 할당되지 않은 공간까지 찾아가고 밑 연산들은 khpage == NULL일 때와 동일
		khpage->next = khpage_list + page_idx;
		khpage->next->nalloc = page_cnt;
		khpage->next->next = NULL;
	}
	freelist.nfree++; // freelist 개수 추가.
	hash_value_delete((uint32_t *)pages,page_idx); // hash delete
}

/* Frees the page at PAGE. */
void palloc_free_page (void *page) 
{
	palloc_free_multiple(page, 1);
}

void palloc_pf_test(void)
{
	printk("------------------------------------\n");

	uint32_t *one_page1 = palloc_get_page();
	uint32_t *one_page2 = palloc_get_page();
	uint32_t *two_page1 = palloc_get_multiple(2);
	uint32_t *three_page;

	printk("one_page1 = %x\n", one_page1); 
	printk("one_page2 = %x\n", one_page2); 
	printk("two_page1 = %x\n", two_page1);

	printk("=----------------------------------=\n"); 
	
	palloc_free_page(one_page1);
	palloc_free_page(one_page2);
	palloc_free_multiple(two_page1,2);
	
	one_page1 = palloc_get_page();
	two_page1 = palloc_get_multiple(2);
	one_page2 = palloc_get_page();

	printk("one_page1 = %x\n", one_page1);
	printk("one_page2 = %x\n", one_page2);
	printk("two_page1 = %x\n", two_page1);
	
	printk("=----------------------------------=\n");
	palloc_free_multiple(one_page2, 3);
	one_page2 = palloc_get_page();
	three_page = palloc_get_multiple(3);

	printk("one_page1 = %x\n", one_page1);
	printk("one_page2 = %x\n", one_page2);
	printk("three_page = %x\n", three_page);

	palloc_free_page(one_page1);
	palloc_free_page(three_page);
	three_page = (uint32_t*)((uint32_t)three_page + 0x1000);
	palloc_free_page(three_page);
	three_page = (uint32_t*)((uint32_t)three_page + 0x1000);
	palloc_free_page(three_page);
	palloc_free_page(one_page2);
}
