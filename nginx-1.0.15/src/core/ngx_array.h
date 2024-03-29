
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

/*
 * ngx动态数组的实现
 */

#ifndef _NGX_ARRAY_H_INCLUDED_
#define _NGX_ARRAY_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


struct ngx_array_s {
    void        *elts; /* 指向数组的首地址 */
    ngx_uint_t   nelts;  /* 数组中已经使用了元素的个数 */
    size_t       size; /* 每个数组元素占用内存空间的大小 */
    ngx_uint_t   nalloc; /* 当前数组中能够容纳元素个数的总大小 */
    ngx_pool_t  *pool; /* 内存池对象 */
};

/*
 * 构建一个动态数组
 *@p 内存池
 *@n 初始化容量大小
 *@size 每个元素所占的内存大小
 */
ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);
void ngx_array_destroy(ngx_array_t *a);
void *ngx_array_push(ngx_array_t *a);
void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);

/*
 * 初始化一个已经存在的动态数组
 */
static ngx_inline ngx_int_t
ngx_array_init(ngx_array_t *array, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    /*
     * set "array->nelts" before "array->elts", otherwise MSVC thinks
     * that "array->nelts" may be used without having been initialized
     */

    array->nelts = 0;
    array->size = size;
    array->nalloc = n;
    array->pool = pool;

    array->elts = ngx_palloc(pool, n * size);
    if (array->elts == NULL) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


#endif /* _NGX_ARRAY_H_INCLUDED_ */
