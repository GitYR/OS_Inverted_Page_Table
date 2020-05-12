#ifndef __PALLOC_H__
#define __PALLOC_H__

#include <ssulib.h>

void init_palloc (void);

uint32_t *palloc_get_page (void);

/* 연속적인 비어있는 페이지들의 페이지 개수 묶음 반환 */
uint32_t *palloc_get_multiple (size_t page_cnt);

void palloc_free_page (void *);

void palloc_free_multiple (void *, size_t page_cnt);

void palloc_pf_test(void);

#endif /* palloc.h */

