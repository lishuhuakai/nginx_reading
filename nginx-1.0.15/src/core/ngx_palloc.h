
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
    ngx_pool_large_t     *next; /* ���еĴ���ڴ�ͨ��nextָ������һ�� */
    void                 *alloc; /* ʵ��ͨ��ngx_alloc������Ĵ���ڴ� */
};


typedef struct {
    /* ָ��δ����Ŀ����ڴ���׵�ַ */
    u_char               *last;
    /* ָ��ǰС���ڴ�ص�β�� */
    u_char               *end;
    /* ͬ����һ��pool�Ķ��С���ڴ�ؼ�,ͨ��next���� */
    ngx_pool_t           *next;
    ngx_uint_t            failed;
} ngx_pool_data_t;

/* nginx�ڴ��
 */
struct ngx_pool_s {
    /* ����С���ڴ��.������С���ڴ�ʱ,ʣ���Ԥ����ռ䲻��ʱ,���ٷ���1��ngx_poot_t
     * ���ǻ�ͨ��d�е�next��Ա���ɵ�����
     */
    ngx_pool_data_t       d;
    /* ���������ڴ�����С�黹�Ǵ��ı�׼ */
    size_t                max;
    /* ���С���ڴ�ع�������ʱ,currentָ������ڴ�ʱ�����ĵ�1��С���ڴ�� */
    ngx_pool_t           *current;
    ngx_chain_t          *chain;
    /* ����ڴ涼ֱ�Ӵӽ��̵Ķ��з���,Ϊ���ܹ��������ڴ��ʱͬʱ�ͷŴ���ڴ�
     * �ͽ�ÿһ�η���Ĵ���ڴ�ͨ��ngx_pool_large_t��ɵ��������large��Ա��
     */
    ngx_pool_large_t     *large;
    /* ���д�������Դ,��ngx_pool_cleanup_t���󹹳ɵ��������cleanup��Ա�� */
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
