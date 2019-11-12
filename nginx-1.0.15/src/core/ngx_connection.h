
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONNECTION_H_INCLUDED_
#define _NGX_CONNECTION_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_listening_s  ngx_listening_t;

/*
 * 每个ngx_listening_t结构体代表nginx服务器监听的一个端口
 */
struct ngx_listening_s {
    ngx_socket_t        fd; /* socket套接字句柄 */

    struct sockaddr    *sockaddr; /* 监听的sockaddr地址 */
    socklen_t           socklen;    /* size of sockaddr */
    size_t              addr_text_max_len; /* 存储IP地址字符串的最大长度,也就是addr_text的长度 */
    ngx_str_t           addr_text; /* 以字符串形式存储IP地址 */

    int                 type; /* 套接字类型,当type为SOCK_STREAM时,表示TCP */

    int                 backlog;
    int                 rcvbuf; /* 接收缓冲区大小 */
    int                 sndbuf; /* 发送缓冲区大小 */

    /* handler of accepted connection */
    ngx_connection_handler_pt   handler; /* tcp连接成功之后的处理方法 */

    void               *servers;  /* array of ngx_http_in_addr_t, for example */

    ngx_log_t           log;
    ngx_log_t          *logp;

    /* 如果为新的tcp连接创建内存池,则内存池的初始大小应该是pool_size */
    size_t              pool_size;
    /* should be here because of the AcceptEx() preread */
    size_t              post_accept_buffer_size;
    /* should be here because of the deferred accept */
    ngx_msec_t          post_accept_timeout;

    ngx_listening_t    *previous;
    /* 当前监听句柄对应着的ngx_connection_t结构体 */
    ngx_connection_t   *connection;
    /* open为1表示当前监听句柄有效,执行ngx_init_cycle时不关闭监听端口,为0表示正常关闭 */
    unsigned            open:1;
    /* remain */
    unsigned            remain:1;
    unsigned            ignore:1;

    unsigned            bound:1;       /* already bound */
    unsigned            inherited:1;   /* inherited from previous process */
    unsigned            nonblocking_accept:1;
    unsigned            listen:1;
    unsigned            nonblocking:1;
    unsigned            shared:1;    /* shared between threads or processes */
    unsigned            addr_ntop:1;

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned            ipv6only:2;
#endif

#if (NGX_HAVE_DEFERRED_ACCEPT)
    unsigned            deferred_accept:1;
    unsigned            delete_deferred:1;
    unsigned            add_deferred:1;
#ifdef SO_ACCEPTFILTER
    char               *accept_filter;
#endif
#endif
#if (NGX_HAVE_SETFIB)
    int                 setfib;
#endif

};


typedef enum {
     NGX_ERROR_ALERT = 0,
     NGX_ERROR_ERR,
     NGX_ERROR_INFO,
     NGX_ERROR_IGNORE_ECONNRESET,
     NGX_ERROR_IGNORE_EINVAL
} ngx_connection_log_error_e;


typedef enum {
     NGX_TCP_NODELAY_UNSET = 0,
     NGX_TCP_NODELAY_SET,
     NGX_TCP_NODELAY_DISABLED
} ngx_connection_tcp_nodelay_e;


typedef enum {
     NGX_TCP_NOPUSH_UNSET = 0,
     NGX_TCP_NOPUSH_SET,
     NGX_TCP_NOPUSH_DISABLED
} ngx_connection_tcp_nopush_e;


#define NGX_LOWLEVEL_BUFFERED  0x0f
#define NGX_SSL_BUFFERED       0x01


/* 代表一个被动连接 */
struct ngx_connection_s {
    void               *data;
    /* 连接对应的读事件 */
    ngx_event_t        *read;
    /* 连接对应的写事件 */
    ngx_event_t        *write;
    /* 套接字句柄 */
    ngx_socket_t        fd;
    /* 直接接收网络字符流的方法 */
    ngx_recv_pt         recv;
    /* 直接发送网络字符流的方法 */
    ngx_send_pt         send;
    /* 以ngx_recv_chain_t为参数来接收网路字符流的方法 */
    ngx_recv_chain_pt   recv_chain;
    /* 以ngx_send_chain_t为参数来发送网络字符流的方法 */
    ngx_send_chain_pt   send_chain;
    /* 这个连接对应的ngx_listening_t监听对象,此连接由listening监听端口的事件建立 */
    ngx_listening_t    *listening;
    /* 连接上已经发送出去的字节数目 */
    off_t               sent;

    ngx_log_t          *log;
    /* 内存池,一般在accept一个新连接时,会创建一个内存池,而在这个连接结束的时候销毁内存池
     * 注意,这里所说的连接是指成功建立的TCP连接,所有的ngx_connection_t结构都是预分配的
     * 这个内存池的大小将由上面的listening监听对象中的pool_size成员决定 */
    ngx_pool_t         *pool;
    /* 连接客户端的sockaddr结构体 */
    struct sockaddr    *sockaddr;
    socklen_t           socklen;
    /* 连接客户端字符串形式的IP地址 */
    ngx_str_t           addr_text;

#if (NGX_SSL)
    ngx_ssl_connection_t  *ssl;
#endif
    /* 本机的监听端口对应的sockaddr结构体,也就是listening监听对象中的sockaddr成员 */
    struct sockaddr    *local_sockaddr;
    /* 用于接收,缓存客户端发来的字符流,每个事件消费模块可自由决定从连接池中分配多大的空间给
     * buffer这个连接缓存字段,例如在HTTP模块中,它的大小决定于client_header_buffer_size配置项 */
    ngx_buf_t          *buffer;

    ngx_queue_t         queue;
    /* 连接使用的次数.ngx_connection_t结构体 */
    ngx_atomic_uint_t   number;
    /* 处理的请求次数 */
    ngx_uint_t          requests;

    unsigned            buffered:8;

    unsigned            log_error:3;     /* ngx_connection_log_error_e */

    unsigned            single_connection:1;
    unsigned            unexpected_eof:1;
    /* 标志位,为1时表示连接已经超时 */
    unsigned            timedout:1;
    unsigned            error:1;
    unsigned            destroyed:1;

    unsigned            idle:1;
    unsigned            reusable:1;
    unsigned            close:1;

    unsigned            sendfile:1;
    unsigned            sndlowat:1;
    unsigned            tcp_nodelay:2;   /* ngx_connection_tcp_nodelay_e */
    unsigned            tcp_nopush:2;    /* ngx_connection_tcp_nopush_e */

#if (NGX_HAVE_IOCP)
    unsigned            accept_context_updated:1;
#endif

#if (NGX_HAVE_AIO_SENDFILE)
    unsigned            aio_sendfile:1;
    ngx_buf_t          *busy_sendfile;
#endif

#if (NGX_THREADS)
    ngx_atomic_t        lock;
#endif
};


ngx_listening_t *ngx_create_listening(ngx_conf_t *cf, void *sockaddr,
    socklen_t socklen);
ngx_int_t ngx_set_inherited_sockets(ngx_cycle_t *cycle);
ngx_int_t ngx_open_listening_sockets(ngx_cycle_t *cycle);
void ngx_configure_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_connection(ngx_connection_t *c);
ngx_int_t ngx_connection_local_sockaddr(ngx_connection_t *c, ngx_str_t *s,
    ngx_uint_t port);
ngx_int_t ngx_connection_error(ngx_connection_t *c, ngx_err_t err, char *text);

ngx_connection_t *ngx_get_connection(ngx_socket_t s, ngx_log_t *log);
void ngx_free_connection(ngx_connection_t *c);

void ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable);

#endif /* _NGX_CONNECTION_H_INCLUDED_ */
