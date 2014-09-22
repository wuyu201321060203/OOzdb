#include "mysql/mysql.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define STRING_SIZE 50
#define SELECT_SAMPLE "select name from zild_t where id = 1"

int main(void)
{
    MYSQL mysql;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind;
    MYSQL_RES *meta_result;
    unsigned long length;
    int paramcount,columncount,rowcount;
    char name[STRING_SIZE];
    char status[STRING_SIZE];
    my_bool is_null;
    mysql_init(&mysql);
    if(!mysql_real_connect(&mysql,"localhost","root","123","test",0,NULL,0))
    {
        fprintf(stderr,"connect error:%s\n",mysql_error(&mysql));
        exit(0);
    }
    stmt=mysql_stmt_init(&mysql);
    if(!stmt)
    {
        fprintf(stderr,"mysql_stmt_init failed:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    if(mysql_stmt_prepare(stmt,SELECT_SAMPLE,strlen(SELECT_SAMPLE)))
    {
        fprintf(stderr,"mysql_stmt_prepare failed:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    paramcount=mysql_stmt_param_count(stmt);
    printf("param count is:%d\n",paramcount);
    if(0!=paramcount)
    {
        fprintf(stderr,"param count error\n");
        exit(0);
    }
    /*
    meta_result=mysql_stmt_result_metadata(stmt);
    if(!meta_result)
    {
        fprintf(stderr,"mysql return metadata error:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    */
    if(mysql_stmt_execute(stmt))
    {
        fprintf(stderr,"sql excute failed:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    printf("excute success\n");
    memset(&bind,0,sizeof(bind));

    bind.buffer_type=MYSQL_TYPE_STRING;
    bind.buffer=(char*)name;
    bind.buffer_length=STRING_SIZE;
    bind.is_null=&is_null;
    bind.length=&length;

    if(mysql_stmt_bind_result(stmt,&bind))
    {
        fprintf(stderr,"bind result error:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    printf("bind result success\n");
    /*
    if(mysql_stmt_store_result(stmt))
    {
        fprintf(stderr,"store result error:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    printf("store result success\n");
    */
    while(!mysql_stmt_fetch(stmt))
    {
        printf("name:%s\n",name);
    }
//    mysql_free_result(meta_result);
    if(mysql_stmt_close(stmt))
    {
        fprintf(stderr,"stmt close error:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    return 0;
}
