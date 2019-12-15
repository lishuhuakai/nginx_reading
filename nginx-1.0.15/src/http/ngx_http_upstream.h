
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_UPSTREAM_H_INCLUDED_
#define _NGX_HTTP_UPSTREAM_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <ngx_event_pipe.h>
#include <ngx_http.h>


#define NGX_HTTP_UPSTREAM_FT_ERROR           0x00000002
#define NGX_HTTP_UPSTREAM_FT_TIMEOUT         0x00000004
#define NGX_HTTP_UPSTREAM_FT_INVALID_HEADER  0x00000008
#define NGX_HTTP_UPSTREAM_FT_HTTP_500        0x00000010
#define NGX_HTTP_UPSTREAM_FT_HTTP_502        0x00000020
#define NGX_HTTP_UPSTREAM_FT_HTTP_503        0x00000040
#define NGX_HTTP_UPSTREAM_FT_HTTP_504        0x00000080
#define NGX_HTTP_UPSTREAM_FT_HTTP_404        0x00000100
#define NGX_HTTP_UPSTREAM_FT_UPDATING        0x00000200
#define NGX_HTTP_UPSTREAM_FT_BUSY_LOCK       0x00000400
#define NGX_HTTP_UPSTREAM_FT_MAX_WAITING     0x00000800
#define NGX_HTTP_UPSTREAM_FT_NOLIVE          0x40000000
#define NGX_HTTP_UPSTREAM_FT_OFF             0x80000000

#define NGX_HTTP_UPSTREAM_FT_STATUS          (NGX_HTTP_UPSTREAM_FT_HTTP_500  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_502  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_503  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_504  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_404)

#define NGX_HTTP_UPSTREAM_INVALID_HEADER     40


#define NGX_HTTP_UPSTREAM_IGN_XA_REDIRECT    0x00000002
#define NGX_HTTP_UPSTREAM_IGN_XA_EXPIRES     0x00000004
#define NGX_HTTP_UPSTREAM_IGN_EXPIRES        0x00000008
#define NGX_HTTP_UPSTREAM_IGN_CACHE_CONTROL  0x00000010
#define NGX_HTTP_UPSTREAM_IGN_SET_COOKIE     0x00000020
#define NGX_HTTP_UPSTREAM_IGN_XA_LIMIT_RATE  0x00000040
#define NGX_HTTP_UPSTREAM_IGN_XA_BUFFERING   0x00000080
#define NGX_HTTP_UPSTREAM_IGN_XA_CHARSET     0x00000100


typedef struct {
    ngx_msec_t                       bl_time;
    ngx_uint_t                       bl_state;

    ngx_uint_t                       status;
    time_t                           response_sec;
    ngx_uint_t                       response_msec;
    off_t                            response_length;

    ngx_str_t                       *peer;
} ngx_http_upstream_state_t;


typedef struct {
    ngx_hash_t                       headers_in_hash;
    ngx_array_t                      upstreams;
                                             /* ngx_http_upstream_srv_conf_t */
} ngx_http_upstream_main_conf_t;

typedef struct ngx_http_upstream_srv_conf_s  ngx_http_upstream_srv_conf_t;

typedef ngx_int_t (*ngx_http_upstream_init_pt)(ngx_conf_t *cf,
    ngx_http_upstream_srv_conf_t *us);
typedef ngx_int_t (*ngx_http_upstream_init_peer_pt)(ngx_http_request_t *r,
    ngx_http_upstream_srv_conf_t *us);


typedef struct {
    ngx_http_upstream_init_pt        init_upstream;
    ngx_http_upstream_init_peer_pt   init;
    void                            *data;
} ngx_http_upstream_peer_t;


typedef struct {
    ngx_addr_t                      *addrs;
    ngx_uint_t                       naddrs;
    ngx_uint_t                       weight;
    ngx_uint_t                       max_fails;
    time_t                           fail_timeout;

    unsigned                         down:1;
    unsigned                         backup:1;
} ngx_http_upstream_server_t;


#define NGX_HTTP_UPSTREAM_CREATE        0x0001
#define NGX_HTTP_UPSTREAM_WEIGHT        0x0002
#define NGX_HTTP_UPSTREAM_MAX_FAILS     0x0004
#define NGX_HTTP_UPSTREAM_FAIL_TIMEOUT  0x0008
#define NGX_HTTP_UPSTREAM_DOWN          0x0010
#define NGX_HTTP_UPSTREAM_BACKUP        0x0020


struct ngx_http_upstream_srv_conf_s {
    ngx_http_upstream_peer_t         peer;
    void                           **srv_conf;

    ngx_array_t                     *servers;  /* ngx_http_upstream_server_t */

    ngx_uint_t                       flags;
    ngx_str_t                        host;
    u_char                          *file_name;
    ngx_uint_t                       line;
    in_port_t                        port;
    in_port_t                        default_port;
};


typedef struct {
	/* 当ngx_http_upstream_t结构体中没有实现resolved成员时,upstream这个结构体才会生效
	 * 它会定义上游服务器的配置 */
    ngx_http_upstream_srv_conf_t    *upstream;
    /* 连接上游服务器的超时时间,单位为毫秒 */
    ngx_msec_t                       connect_timeout;
    /* 发送tcp包到上游服务器的超时时间,单位为毫秒 */
    ngx_msec_t                       send_timeout;
    /* 接收tcp包到上游服务器的超时时间,单位为毫秒 */
    ngx_msec_t                       read_timeout;
    ngx_msec_t                       timeout;
	/* TCP的SO_SNOLOWAT选项,表示发送缓冲区的下限 */
    size_t                           send_lowat;
	/* 定义了接收头部的缓冲区分配的内存大小(ngx_http_upstream_t的buffer缓冲区) */
    size_t                           buffer_size;
	
    size_t                           busy_buffers_size;
	/* 仅当buffering标志位为1,如果上游速度快于下游速度,将有可能把来自上游的响应存储
	 * 到临时文件中,而max_temp_file_size指定了临时文件的最大长度,实际上,它将限制
	 * ngx_event_pipe_t的结构题中的temp_file */
    size_t                           max_temp_file_size;
    size_t                           temp_file_write_size;

    size_t                           busy_buffers_size_conf;
    size_t                           max_temp_file_size_conf;
    size_t                           temp_file_write_size_conf;

    ngx_bufs_t                       bufs;
	/* 针对ngx_http_upstream_t结构体中保存解析完的包头的hearder_in成员,ignore_headers可以按照
	 * 二进制位使得upstream在转发包头时,跳过对某些头部的处理 */
    ngx_uint_t                       ignore_headers;
    ngx_uint_t                       next_upstream;
    ngx_uint_t                       store_access;
    ngx_flag_t                       buffering;
    ngx_flag_t                       pass_request_headers;
    ngx_flag_t                       pass_request_body;
	/* 标志位,它为1时,表示与上游服务器交互将不检查Nginx与下游客户端间的连接是否断开,也就是说,即使下游客户端
	 * 主动关闭了连接,也不会中断与上游服务器之间的交互 */
    ngx_flag_t                       ignore_client_abort;
    ngx_flag_t                       intercept_errors;
    ngx_flag_t                       cyclic_temp_file;

    ngx_path_t                      *temp_path;

    ngx_hash_t                       hide_headers_hash;
    ngx_array_t                     *hide_headers;
    ngx_array_t                     *pass_headers;

    ngx_addr_t                      *local;

#if (NGX_HTTP_CACHE)
    ngx_shm_zone_t                  *cache;

    ngx_uint_t                       cache_min_uses;
    ngx_uint_t                       cache_use_stale;
    ngx_uint_t                       cache_methods;

    ngx_array_t                     *cache_valid;
    ngx_array_t                     *cache_bypass;
    ngx_array_t                     *no_cache;
#endif

    ngx_array_t                     *store_lengths;
    ngx_array_t                     *store_values;

    signed                           store:2;
    unsigned                         intercept_404:1;
    unsigned                         change_buffering:1;

#if (NGX_HTTP_SSL)
    ngx_ssl_t                       *ssl;
    ngx_flag_t                       ssl_session_reuse;
#endif

    ngx_str_t                        module;
} ngx_http_upstream_conf_t;


typedef struct {
    ngx_str_t                        name;
    ngx_http_header_handler_pt       handler;
    ngx_uint_t                       offset;
    ngx_http_header_handler_pt       copy_handler;
    ngx_uint_t                       conf;
    ngx_uint_t                       redirect;  /* unsigned   redirect:1; */
} ngx_http_upstream_header_t;


typedef struct {
    ngx_list_t                       headers;

    ngx_uint_t                       status_n;
    ngx_str_t                        status_line;

    ngx_table_elt_t                 *status;
    ngx_table_elt_t                 *date;
    ngx_table_elt_t                 *server;
    ngx_table_elt_t                 *connection;

    ngx_table_elt_t                 *expires;
    ngx_table_elt_t                 *etag;
    ngx_table_elt_t                 *x_accel_expires;
    ngx_table_elt_t                 *x_accel_redirect;
    ngx_table_elt_t                 *x_accel_limit_rate;

    ngx_table_elt_t                 *content_type;
    ngx_table_elt_t                 *content_length;

    ngx_table_elt_t                 *last_modified;
    ngx_table_elt_t                 *location;
    ngx_table_elt_t                 *accept_ranges;
    ngx_table_elt_t                 *www_authenticate;

#if (NGX_HTTP_GZIP)
    ngx_table_elt_t                 *content_encoding;
#endif

    off_t                            content_length_n;

    ngx_array_t                      cache_control;
} ngx_http_upstream_headers_in_t;


typedef struct {
    ngx_str_t                        host;
    in_port_t                        port;
    ngx_uint_t                       no_port; /* unsigned no_port:1 */
    /* 地址的个数 */
    ngx_uint_t                       naddrs;
    in_addr_t                       *addrs;
    /* 上游服务器的地址 */
    struct sockaddr                 *sockaddr;
    socklen_t                        socklen;

    ngx_resolver_ctx_t              *ctx;
} ngx_http_upstream_resolved_t;


typedef void (*ngx_http_upstream_handler_pt)(ngx_http_request_t *r,
    ngx_http_upstream_t *u);


struct ngx_http_upstream_s {
	/* 处理读事件的回调方法,每一个阶段都有一个不同的read_event_handler */
    ngx_http_upstream_handler_pt     read_event_handler;
	/* 处理写事件的回调方法,每一个阶段都有一个不同的write_event_handler */
    ngx_http_upstream_handler_pt     write_event_handler;
	/* 表示主动向上游服务器发起的连接 */
    ngx_peer_connection_t            peer;
	/* 向下游服务器转发响应时,如果打开了缓存且认为上游服务器网速更快,这是会用pipe成员来转发响应
	 * 在使用这种方式来转发响应时,必须由HTTP模块在使用upstream机制前构造pipe结构体,否则会出现coredump */
    ngx_event_pipe_t                *pipe;
    /* request_bufs决定发送什么样的请求给上游服务器,在实现create_request方法时需要设置 */
    ngx_chain_t                     *request_bufs;
	/* 定义了向下游发送响应的方式 */
    ngx_output_chain_ctx_t           output;
    ngx_chain_writer_ctx_t           writer;
    /* upstream访问时的所有限制性参数 */
    ngx_http_upstream_conf_t        *conf;
	/* HTTP模块在实现process_header方法时,如果希望upstream直接转发响应,就需要把解析出的响应头部适配为HTTP的
	 * 响应头部,同时需要将包头中的信息设置到headers_in结构体中,这样,在转发的时候,会将headers_in中设置的头部
	 * 添加到要发送到下游客户端的响应头部headers_out中 */
    ngx_http_upstream_headers_in_t   headers_in;
    /* 通过resolved可以直接指定上游服务器的地址 */
    ngx_http_upstream_resolved_t    *resolved;
    /* buffer成员存储了接收自上游服务器发来的响应内容,由于它会被复用,所以具有下列多种意义:
     * 1.在使用process_header方法解析上游响应的包头时,buffer中将会保存完整的响应头
     * 2.当下面的buffering成员为1,而且此时upstream是向下游转发上游的包体时,buffer没有意义
     * 3.当buffering标志为0时,buffer缓冲区会被用于反复地接收上游的包体,进而向下游转发
     * 4.当upstream并不用于转发上游包体时,buffer会被用于反复接收上游的包体 */
    ngx_buf_t                        buffer;
	/* 表示来自上游服务器的响应包体的长度 */
    size_t                           length;
	/* out_bufs在两种场景下有不同的意义,1.当不需要转发包体,且使用默认的input_filter方法处理包体时
	 * out_bufs会指向响应包体,事实上,out_bufs链表中会产生多个ngx_buf_t缓冲区,每个缓冲区都指向buffer
	 * 缓存中的一部分,而这里的一部分就是每次调用recv方法接收到的一段TCP流.
	 * 2.当需要转发响应包体到下游时,这个链表指向上一次想下游转发响应到现在这段时间内接收自上有的缓存响应
	 */
    ngx_chain_t                     *out_bufs;
	/* 当需要转发响应包体到下游时,它表示上一次向下游转发响应时没有发送完的内容 */
    ngx_chain_t                     *busy_bufs;
	/* 这个链表将用户回收out_bufs中已经发送给下游的ngx_buf_t结构体 */
    ngx_chain_t                     *free_bufs;
	/* 处理包体前的初始化方法,其中data参数用于传递用户数据结构,它实际上就是下面的input_filter_ctx指针 */
    ngx_int_t                      (*input_filter_init)(void *data);
	/* 处理包体的方法,bytes表示本次接收到的包体长度,返回NGX_ERROR时表示处理包体错误,请求需要结束 */
    ngx_int_t                      (*input_filter)(void *data, ssize_t bytes);
    void                            *input_filter_ctx;

#if (NGX_HTTP_CACHE)
    ngx_int_t                      (*create_key)(ngx_http_request_t *r);
#endif
    /* 构造发往上游服务器的请求内容 */
    ngx_int_t                      (*create_request)(ngx_http_request_t *r);
	/* 与上游服务器的通信失败后,如果按照充实规则还需要再次向上游服务器发起请求,则会调用reinit
	_request方法 */
    ngx_int_t                      (*reinit_request)(ngx_http_request_t *r);
    /* 收到上游服务器的响应后就会回调process_header方法,如果process_header返回NGX_AGAIN
     * 那么是在告诉upstream还没有接收到完整的响应包头,此时,对于本次upstream请求来说,再次
     * 接收到上游服务器发来的TCP流时,还会调用process_header方法处理,直到process_header函数
     * 返回非NGX_AGAIN值 */
    ngx_int_t                      (*process_header)(ngx_http_request_t *r);
    /* 销毁upstream请求时调用 */
    void                           (*abort_request)(ngx_http_request_t *r);
	/* 请求结束的时候会调用 */
    void                           (*finalize_request)(ngx_http_request_t *r,
                                         ngx_int_t rc);
    ngx_int_t                      (*rewrite_redirect)(ngx_http_request_t *r,
                                         ngx_table_elt_t *h, size_t prefix);

    ngx_msec_t                       timeout;
	/* 用于表示上游响应的错误码,包体长度等信息 */
    ngx_http_upstream_state_t       *state;

    ngx_str_t                        method;
    ngx_str_t                        schema;
    ngx_str_t                        uri;

    ngx_http_cleanup_pt             *cleanup;

    unsigned                         store:1;
    unsigned                         cacheable:1;
    unsigned                         accel:1;
    unsigned                         ssl:1;
#if (NGX_HTTP_CACHE)
    unsigned                         cache_status:3;
#endif
    /* 在向客户端转发上游服务器的包体时才有用,当buffering为1时,表示使用多个缓冲区以及磁盘文件
     * 来转发上游的响应包体,当Nginx与上游间的网速远大于Nginx与下游客户端之间的网速时,让Nginx开辟
     * 更多的内存甚至使用磁盘文件来缓存上游的响应包体,这是有意义的,它可以减轻上游服务器的并发压力.
     * 当buffering为0时,表示只使用上面的这一个bufer缓冲区来向下游转发响应包体 */
    unsigned                         buffering:1;
	/* request_sent表示是否已经向上游服务器发送了请求,当request_sent为1时,表示upstream机制已经向上
	 * 上游服务器发送了部分或者全部的请求, 事实上,这个标志位更多的是为了使用ngx_output_chain方法发送
	 * 请求,必须有request_sent标志位记录是否调用过ngx_output_chain方法 */
    unsigned                         request_sent:1;
	/* 将上游服务器的响应划分为包头和包尾,如果把响应直接转发给客户端,header_sent标志为表示包头是否发送
	 * header_sent为1表示已经将包头转发给客户端了,如果不转发响应到客户端,则header_sent没有意义 */
    unsigned                         header_sent:1;
};


typedef struct {
    ngx_uint_t                      status;
    ngx_uint_t                      mask;
} ngx_http_upstream_next_t;


ngx_int_t ngx_http_upstream_header_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);

ngx_int_t ngx_http_upstream_create(ngx_http_request_t *r);
void ngx_http_upstream_init(ngx_http_request_t *r);
ngx_http_upstream_srv_conf_t *ngx_http_upstream_add(ngx_conf_t *cf,
    ngx_url_t *u, ngx_uint_t flags);
char *ngx_http_upstream_bind_set_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
ngx_int_t ngx_http_upstream_hide_headers_hash(ngx_conf_t *cf,
    ngx_http_upstream_conf_t *conf, ngx_http_upstream_conf_t *prev,
    ngx_str_t *default_hide_headers, ngx_hash_init_t *hash);


#define ngx_http_conf_upstream_srv_conf(uscf, module)                         \
    uscf->srv_conf[module.ctx_index]


extern ngx_module_t        ngx_http_upstream_module;
extern ngx_conf_bitmask_t  ngx_http_upstream_cache_method_mask[];
extern ngx_conf_bitmask_t  ngx_http_upstream_ignore_headers_masks[];


#endif /* _NGX_HTTP_UPSTREAM_H_INCLUDED_ */
