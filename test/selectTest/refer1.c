#include "mysql/mysql.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define STRING_SIZE 50
#define SAMPLE "select name from zild_t where id = ?"

int main(void)
{
    MYSQL mysql;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[3];
    int id;
    char name[STRING_SIZE];
    unsigned long name_length;
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
    if(mysql_stmt_prepare(stmt,SAMPLE,strlen(SAMPLE)))
    {
        fprintf(stderr,"mysql_stmt_prepare failed:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    int paramcount=mysql_stmt_param_count(stmt);
    //printf("param count is:%d\n",paramcount);
    if(1!=paramcount)
    {
        fprintf(stderr,"param count error\n");
        exit(0);
    }
    memset(bind,0,sizeof(bind));
    bind[0].buffer_type=MYSQL_TYPE_LONG;
    bind[0].buffer=&id;
    //bind[0].buffer_length=STRING_SIZE;
    bind[0].is_null=0;
    //bind[0].length=&id_length;
    bind[1].buffer_type=MYSQL_TYPE_STRING;
    bind[1].buffer=(char*)name;
    bind[1].buffer_length=STRING_SIZE;
    bind[1].is_null=0;
    bind[1].length=&name_length;
    bind[2].buffer_type=MYSQL_TYPE_STRING;
    bind[2].buffer=(char*)name;
    bind[2].buffer_length=STRING_SIZE;
    bind[2].is_null=0;
    bind[2].length=&name_length;

    if(mysql_stmt_bind_param(stmt,&bind[0]))
    {
        fprintf(stderr,"mysql bind param error:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    id = 1;
    //id_length=strlen(id);
    //strncpy(name,"wuyu",STRING_SIZE);
    //name_length=strlen(name);
    if(mysql_stmt_execute(stmt))
    {
        fprintf(stderr,"sql execute failed:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    printf("excute success\n");
    if(mysql_stmt_bind_result(stmt, &bind[1]))
    {
        fprintf(stderr,"sql bind result failed:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    if(mysql_stmt_fetch(stmt))
    {
        fprintf(stderr,"sql fetch result failed:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    printf("%s\n" , name);
    id = 2;
    bind[0].buffer_type=MYSQL_TYPE_LONG;
    bind[0].buffer=&id;
    //bind[0].buffer_length=STRING_SIZE;
    bind[0].is_null=0;
    if(mysql_stmt_execute(stmt))
    {
        fprintf(stderr,"sql execute failed:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    printf("excute success\n");
    if(mysql_stmt_bind_result(stmt, &bind[2]))
    {
        fprintf(stderr,"sql bind result failed:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    if(mysql_stmt_fetch(stmt))
    {
        fprintf(stderr,"sql fetch result failed:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    printf("%s\n" , name);

    /*
    affected_rows=mysql_stmt_affected_rows(stmt);
    if(1!=affected_rows)
    {
        fprintf(stderr,"invalid affected rows by mysql\n");
        exit(0);
    }
    */
    if(mysql_stmt_close(stmt))
    {
        fprintf(stderr,"stmt close error:%s\n",mysql_stmt_error(stmt));
        exit(0);
    }
    return 0;
}
