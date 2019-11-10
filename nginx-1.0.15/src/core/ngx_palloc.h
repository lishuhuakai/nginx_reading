
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * NGX_MAX_ALLOC_FROM_POOL should be (ngx_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)

#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)

#define NGX_POOL_ALIGNMENT       16
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)


typedef void (*ngx_pool_cleanup_pt)(void *data);

typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;

struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;
    void                 *data;
    ngx_pool_cleanup_t   *next;
};


typedef struct ngx_pool_large_s  ngx_pool_large_t;

struct ngx_pool_large_s {
    ngx_pool_large_t     *next; /* 所有的大块内存通过next指针连在一起 */
    void                 *alloc; /* 实际通过ngx_alloc分配出的大块内存 */
};


typedef struct {
    /* 指向未分配的空闲内存的首地址 */
    u_char               *last;
    /* 指向当前小块内存池的尾部 */
    u_char               *end;
    /* 同属于一个pool的多个小块内存池间,通过next相连 */
    ngx_pool_t           *next;
    ngx_uint_t            failed;
} ngx_pool_data_t;

/* nginx内存池
 */
struct ngx_pool_s {
    /* 描述小块内存池.当分配小块内存时,剩余的预分配空间不足时,会再分配1个ngx_poot_t
     * 它们会通过d中的next成员构成单链表
     */
    ngx_pool_data_t       d;
    /* 评估申请内存属于小块还是大块的标准 */
    size_t                max;
    /* 多个小块内存池构成链表时,current指向分配内存时遍历的第1个小块内存池 */
    ngx_pool_t           *current;
    ngx_chain_t          *chain;
    /* 大块内存都直接从进程的堆中分配,为了能够在销毁内存池时同时释放大块内存
     * 就将每一次分配的大块内存通过ngx_pool_large_t组成单链表挂在large成员上
     */
    ngx_pool_large_t     *large;
    /* 所有待清理资源,以ngx_pool_cleanup_t对象构成单链表挂在cleanup成员上 */
    ngx_pool_cleanup_t   *cleanup;
    ngx_log_t            *log;
};


typedef struct {
    ngx_fd_t              fd;
    u_char               *name;
    ngx_log_t            *log;
} ngx_pool_cleanup_file_t;


void *ngx_alloc(size_t size, ngx_log_t *log);
void *ngx_calloc(size_t size, ngx_log_t *log);

ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);
void ngx_destroy_pool(ngx_pool_t *pool);
void ngx_reset_pool(ngx_pool_t *pool);

void *ngx_palloc(ngx_pool_t *pool, size_t size);
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);


ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);
void ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd);
void ngx_pool_cleanup_file(void *data);
void ngx_pool_delete_file(void *data);


#endif /* _NGX_PALLOC_H_INCLUDED_ */
