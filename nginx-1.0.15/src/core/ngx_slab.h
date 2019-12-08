
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_SLAB_H_INCLUDED_
#define _NGX_SLAB_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_slab_page_s  ngx_slab_page_t;

/* 共享内存描述信息 */
struct ngx_slab_page_s {
    uintptr_t         slab; /* 多用途 */
    ngx_slab_page_t  *next; /* 指向双向链表的下一页 */
    uintptr_t         prev; /* 多用途,同时指向双向链表的上一页 */
};


typedef struct {
    /* 为下面的互斥锁成员ngx_shmtx_t mutex服务,使用信号量作为进程同步工具时会使用它 */
    ngx_atomic_t      lock;
    /* 设定的最小内存块长度 */
    size_t            min_size;
    /* min_size对应的位偏移,因为slab的算法大量采用位操作 */
    size_t            min_shift;
    /* 每一页对应一个ngx_slab_page_t页描述结构体,所有的ngx_slab_page_t存放在连续的内存中构成
     * 数组,而pages就是数组首地址 */
    ngx_slab_page_t  *pages;
    /* 所有的空闲页组成一个链表挂在free成员上 */
    ngx_slab_page_t   free;

    /* 共享内存的范围 */
    u_char           *start;
    u_char           *end;

    ngx_shmtx_t       mutex;

    u_char           *log_ctx;
    u_char            zero;

    void             *data;
    /* 指向所属的ngx_shm_t成员的addr成员 */
    void             *addr;
} ngx_slab_pool_t;


void ngx_slab_init(ngx_slab_pool_t *pool);
void *ngx_slab_alloc(ngx_slab_pool_t *pool, size_t size);
void *ngx_slab_alloc_locked(ngx_slab_pool_t *pool, size_t size);
void ngx_slab_free(ngx_slab_pool_t *pool, void *p);
void ngx_slab_free_locked(ngx_slab_pool_t *pool, void *p);


#endif /* _NGX_SLAB_H_INCLUDED_ */
