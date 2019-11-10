
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     16384
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2


typedef struct ngx_shm_zone_s  ngx_shm_zone_t;

typedef ngx_int_t (*ngx_shm_zone_init_pt) (ngx_shm_zone_t *zone, void *data);

struct ngx_shm_zone_s {
    void                     *data;
    ngx_shm_t                 shm;
    ngx_shm_zone_init_pt      init;
    void                     *tag;
};


struct ngx_cycle_s {
    /*
     * 保存着所有模块存储的配置项的结构体指针,它首先是一个数组，每个数组成员又是一个指针
     * 这个指针指向的是另一个存储着指针的数组
     */
    void                  ****conf_ctx;
    /*
     * 内存池
     */
    ngx_pool_t               *pool;

    ngx_log_t                *log;
    ngx_log_t                 new_log;

    ngx_connection_t        **files;
    /* 可用连接池,和free_connection_n配合使用 */
    ngx_connection_t         *free_connections;
    /* 可用连接池中的连接总数 */
    ngx_uint_t                free_connection_n;
    /* 双向链表容器,元素类型是ngx_connection_t结构体,表示课重复使用的连接队列 */
    ngx_queue_t               reusable_connections_queue;
    /* 动态数组,每个数组元素存储着ngx_listening_t成员,表示监听端口以及相关的参数 */
    ngx_array_t               listening;
    /* 动态数组容器,它保存着nginx所有要操作的目录,如果目录不存在,会试图创建,而创建目录
     * 失败会导致Nginx启动失败.例如上传文件的临时目录也在pathes中,如果没有权限创建,则会
     * 导致Nginx无法启动.
     */
    ngx_array_t               pathes;
    /* 单链表容器,元素类型为ngx_open_file_t结构体.它表示Nginx已经打开的所有文件
     */
    ngx_list_t                open_files;
    /* 单链表容器,元素类型为ngx_shm_zone_t结构体,每个元素表示一块共享内存 */
    ngx_list_t                shared_memory;
    /* 当前进程中,所有连接的总数 */
    ngx_uint_t                connection_n;
    /*  */
    ngx_uint_t                files_n;
    /* 当前进程中,所有的连接对象 */
    ngx_connection_t         *connections;
    ngx_event_t              *read_events;
    ngx_event_t              *write_events;

    ngx_cycle_t              *old_cycle;
    /* 配置文件相对于安装目录的路径名称 */
    ngx_str_t                 conf_file;
    ngx_str_t                 conf_param;
    /* Nginx配置文件所在的目录的路径 */
    ngx_str_t                 conf_prefix;
    /* Nginx安装目录的路径 */
    ngx_str_t                 prefix;
    ngx_str_t                 lock_file;
    /* 使用gethostname系统调用得到的主机名 */
    ngx_str_t                 hostname;
};


typedef struct {
     ngx_flag_t               daemon;
     ngx_flag_t               master;

     ngx_msec_t               timer_resolution;

     ngx_int_t                worker_processes;
     ngx_int_t                debug_points;

     ngx_int_t                rlimit_nofile;
     ngx_int_t                rlimit_sigpending;
     off_t                    rlimit_core;

     int                      priority;

     ngx_uint_t               cpu_affinity_n;
     u_long                  *cpu_affinity;

     char                    *username;
     ngx_uid_t                user;
     ngx_gid_t                group;

     ngx_str_t                working_directory;
     ngx_str_t                lock_file;

     ngx_str_t                pid;
     ngx_str_t                oldpid;

     ngx_array_t              env;
     char                   **environment;

#if (NGX_THREADS)
     ngx_int_t                worker_threads;
     size_t                   thread_stack_size;
#endif

} ngx_core_conf_t;


typedef struct {
     ngx_pool_t              *pool;   /* pcre's malloc() pool */
} ngx_core_tls_t;


#define ngx_is_init_cycle(cycle)  (cycle->conf_ctx == NULL)


ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);
ngx_int_t ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log);
void ngx_delete_pidfile(ngx_cycle_t *cycle);
ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig);
void ngx_reopen_files(ngx_cycle_t *cycle, ngx_uid_t user);
char **ngx_set_environment(ngx_cycle_t *cycle, ngx_uint_t *last);
ngx_pid_t ngx_exec_new_binary(ngx_cycle_t *cycle, char *const *argv);
u_long ngx_get_cpu_affinity(ngx_uint_t n);
ngx_shm_zone_t *ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name,
    size_t size, void *tag);


extern volatile ngx_cycle_t  *ngx_cycle;
extern ngx_array_t            ngx_old_cycles;
extern ngx_module_t           ngx_core_module;
extern ngx_uint_t             ngx_test_config;
extern ngx_uint_t             ngx_quiet_mode;
#if (NGX_THREADS)
extern ngx_tls_key_t          ngx_core_tls_key;
#endif


#endif /* _NGX_CYCLE_H_INCLUDED_ */
