#ifndef MYSQL_OPERATION_H
#define MYSQL_OPERATION_H

#include <iostream>
#include <mysql.h>
#include <string>
#include <vector>

/*

0. 编译的时候实用以下指令,其中main为执行(.h)的主文件

g++ -o main.o main.cpp -L /usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lssl -lcrypto -ldl -std=c++11

为方便理解，下面给出demo中的三个概念，此时的场景是，files表为总表，已经上传了一个名为"AAAAA"的文件，其中分为了三个块:

1. 数据库中的表有两个，分别是files(总表)和AAAAA(每个文件对应的表)

MySQL [FILE]> SHOW TABLES;
+----------------+
| Tables_in_FILE |
+----------------+
| AAAAA          |
| files          |
+----------------+

2. files表中记录了AAAAA的文件总览信息

MySQL [FILE]> SELECT * FROM files;
+----+-------+----------------+-----------+
| id | name  | file_md5       | block_num |
+----+-------+----------------+-----------+
|  0 | AAAAA | A1B2C3D4E5F6G7 |         3 |
+----+-------+----------------+-----------+

3. AAAAA表中记录了每个块的状态，此时显示的是0号和1号上传成功，2号还未上传

MySQL [FILE]> SELECT * FROM AAAAA;
+----+----------------------------------+-------------+
| id | block_md5                        | upload_stat |
+----+----------------------------------+-------------+
|  0 | 0AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA1 |           1 |
|  1 | 0BBBBBBBBBBBBBBBBBBBBBBBBBBBBBB1 |           1 |
|  2 | 0CCCCCCCCCCCCCCCCCCCCCCCCCCCCCC1 |           0 |
+----+----------------------------------+-------------+

*/

class DBhelper
{
public:
    // 默认构造函数，完成配置并启动连接，配置需要运行代码前在mysql里面操作，完成后就不需要改动
    DBhelper():host("localhost"),
                user("root"),
                passwd("jingxiangyi"), 
                port(3306), 
                db("FILE"),
                query_result(nullptr)
    {
        mysql_init(&mysql);
        conn_db();
    }

    // 析构函数关闭连接，释放资源
    ~DBhelper(){
        mysql_close(&mysql);
        std::cout << "Connection closed!" 
                    << std::endl;
    }
    /* 标准的查询和展示方式，但只作为通用的辅助工具，后面有基于该项目具体的实现
    // 查询table中的items，将结果放到query_result变量中
    void db_query(std::string table, std::string items = "*"){
        std::string query_str = "SELECT " + items + " from " + table;
        if(mysql_real_query(&mysql, query_str.c_str(), 
                        query_str.length())!=0){
            std::cout << "db_query(): " 
            << mysql_error(&mysql) << std::endl;
            return;
        }
        query_result = mysql_store_result(&mysql);
    }

    
    // 查看查询的结果
    void view_query_result(){
        if (nullptr == query_result) {
            std::cout << "view_query_result(): " 
                    << mysql_error(&mysql) << std::endl;
            return;
        }
        MYSQL_ROW row;
        int rows = mysql_num_rows(query_result);
        std::cout << "The total rows is: " << rows << std::endl;
        int fields = mysql_num_fields(query_result);
        std::cout << "The total fields is: " << fields << std::endl;
        while (row = mysql_fetch_row(query_result)) {
            for (int i = 0; i < fields; i++) {
                std::cout << row[i] << '\t';
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    */

    // 查看文件是否已经上传
    bool check_file_exist(const std::string &file_name){
        std::string query_str = "SELECT * FROM files WHERE name=\'"+file_name+"\'";
        if(mysql_real_query(&mysql, query_str.c_str(), query_str.length())!=0){
            std::cout << "check_file_exist(): " 
            << mysql_error(&mysql) << std::endl;
            return false;
        }
        return mysql_num_rows(mysql_store_result(&mysql))==1;
    }

    // 创建名为file_name的表
    void db_add_file_table(const std::string &file_name){
        std::string query_str = "CREATE TABLE `" + file_name 
                                 + "` (id int NOT NULL, block_md5 char(33) NOT NULL, "
                                 + "upload_stat int NOT NULL, PRIMARY KEY (block_md5)) ENGINE=InnoDB";
        //cout << query_str << endl;
        if(mysql_real_query(&mysql, query_str.c_str(), 
                        query_str.length())!=0){
            std::cout << "db_add_file_table(): " 
            << mysql_error(&mysql) << std::endl;
            return;
        }
        //std::cout << "Table " << file_name << "added" << std::endl;
    }

    // 在file总表中添加一个名为file_name的文件，包含id号，block数量等
    void db_add_file_row(const std::string &file_name, 
                        const int &block_num, 
                        const std::string &file_md5)
    {
        std::string query_str = "INSERT INTO files VALUES(" 
                                + std::to_string(file_cnt) + ", " // 文件的id号
                                + "\'" + file_name + "\', "
                                + "\'" + file_md5 + "\', "
                                + std::to_string(block_num) + ")";// 记录block的数量
        
        if(mysql_real_query(&mysql, query_str.c_str(), 
                        query_str.length())!=0){
            std::cout << "db_add_file_row(): " 
            << mysql_error(&mysql) << std::endl;
            return;
        }
        file_cnt++;
        //std::cout << "Added file: " << file_name << std::endl;
    }

    // 创建需要上传的新文件file_name，包括在file表中添加文件信息，以及创建名为file_name的表
    void create_file(const std::string &file_name, const int &block_num, const std::string &file_md5){
        db_add_file_table(file_name);
        db_add_file_row(file_name, block_num, file_md5);
        std::cout << "File " << file_name << " created with " << std::to_string(block_num) << " blocks" << std::endl;
    }

    // 检查文件的上传状态，如果完成上传，返回true
    bool check_upload_status(const std::string &file_name){
        std::string query_str = "SELECT * FROM " + file_name + " WHERE upload_stat=0";
        if(mysql_real_query(&mysql, query_str.c_str(), query_str.length())!=0){
            std::cout << "check_upload_status(): " 
            << mysql_error(&mysql) << std::endl;
            return false;
        }
        return mysql_num_rows(mysql_store_result(&mysql))==0;
    }

    // 在file_name表中添加一个名为block_name的块，同时记录id、md5，默认此时上传状态为”未上传“(upload_stat=0)
    void db_add_block_row(const std::string &file_name, const int &block_id, const std::string &block_md5){
        std::string query_str = "INSERT INTO " + file_name + " VALUES(" 
                                + std::to_string(block_id) + ", " // 块的id号
                                + "\'" + block_md5 + "\', " // 块的md5值
                                + "0)";// 块的上传状态（未完成为0）
        
        if(mysql_real_query(&mysql, query_str.c_str(), 
                        query_str.length())!=0){
            std::cout << "db_add_block_row(): " 
            << mysql_error(&mysql) << std::endl;
            return;
        }
        //std::cout << "Added block: " << block_md5 << std::endl;
    }

    // 接受所有块的md5，解析后对每个块调用db_add_block_row，将每个块写入名为file_name的表中。
    void set_block_md5(const std::string &file_name, const std::string &md5_string){
        int block_id = 0;
        while(block_id*32 < md5_string.length()){
            std::string temp_md5 = md5_string.substr(block_id*32, 32);
            //std::cout << temp_md5 << std::endl;
            db_add_block_row(file_name, block_id, temp_md5);
            block_id++;
        }
    }

    // 上传成功后，更新block的状态upload_stat=1
    void set_block_uploaded(const std::string &file_name, const int &block_id){
        std::string query_str = "UPDATE " + file_name // 设置文件名，找到表
                                + " SET upload_stat=1 WHERE id=" 
                                + std::to_string(block_id); // 设置上传状态
        if(mysql_real_query(&mysql, query_str.c_str(), 
                        query_str.length())!=0){
            std::cout << "set_block_uploaded(): " 
            << mysql_error(&mysql) << std::endl;
            return;
        }
        //std::cout << "Updated block: " << std::to_string(block_id) << std::endl;
    }
    
    // 返回名为file_name文件的第block_id个块的block_md5字符串
    std::string get_block_md5(const int &block_id, const std::string &file_name){
        std::string query_str = "SELECT block_md5 FROM " + file_name + " WHERE id=" + std::to_string(block_id);
        //std::string query_str = "SELECT * FROM AAAAA ";//WHERE id=0";
        if(mysql_real_query(&mysql, query_str.c_str(), 
                        query_str.length())!=0){
            std::cout << "get_block_md5(): " 
            << mysql_error(&mysql) << std::endl;
            return "";
        }
        //query_result = mysql_store_result(&mysql);
        MYSQL_ROW row = mysql_fetch_row(mysql_store_result(&mysql));
        return row[0];
    }

    // 返回上传成功的块组成的id字符串
    std::string get_block_id_string(const std::string &file_name){
        std::string temp_string;
        std::string query_str = "SELECT id FROM " + file_name + " WHERE upload_stat=1";
        if(mysql_real_query(&mysql, query_str.c_str(), 
                        query_str.length())!=0){
            std::cout << "get_block_id_string(): " 
            << mysql_error(&mysql) << std::endl;
            return "";
        }
        query_result = mysql_store_result(&mysql);
        MYSQL_ROW row;
        int rows = mysql_num_rows(query_result);
        //cout << "The total rows is: " << rows << endl;
        int fields = mysql_num_fields(query_result);
        //cout << "The total fields is: " << fields << endl;
        while (row = mysql_fetch_row(query_result)) {
                temp_string += std::string(row[0]) + ":";
        }
        if(temp_string.length()>0){
            temp_string.pop_back();
        }
        return temp_string;
    }

    // 获取名为file_name的文件的块数，返回值为int
    int get_block_num(const std::string &file_name){
        std::string query_str = "SELECT block_num FROM files WHERE name =\'" 
                                + file_name + "\'";
        if(mysql_real_query(&mysql, query_str.c_str(), 
                        query_str.length())!=0){
            std::cout << "get_block_nums(): " 
            << mysql_error(&mysql) << std::endl;
            return -1;
        }
        //query_result = mysql_store_result(&mysql);
        MYSQL_ROW row = mysql_fetch_row(mysql_store_result(&mysql));
        return atoi(row[0]);
    }

    // 返回名为file_name文件所有block拼接成的block_md5字符串，排序为id升序
    std::string get_file_block_md5(const std::string &file_name){
        std::string query_str = "SELECT block_md5 FROM `" + file_name + "` ORDER BY id ASC" ;
        //std::string query_str = "SELECT * FROM AAAAA ";//WHERE id=0";
        if(mysql_real_query(&mysql, query_str.c_str(), 
                        query_str.length())!=0){
            std::cout << "get_file_block_md5(): " 
            << mysql_error(&mysql) << std::endl;
            return "";
        }

        std::string temp_string;
        query_result = mysql_store_result(&mysql);
        MYSQL_ROW row;
        int rows = mysql_num_rows(query_result);
        //cout << "The total rows is: " << rows << endl;
        int fields = mysql_num_fields(query_result);
        //cout << "The total fields is: " << fields << endl;
        while (row = mysql_fetch_row(query_result)) {
                temp_string += std::string(row[0]);
        }

        return temp_string;
    }
private:
    MYSQL mysql;
    std::string host;
    std::string user;
    std::string passwd;
    std::string db;
    unsigned int port;
    MYSQL_RES *query_result;
    int file_cnt=0;
    
    //连接数据库
    void conn_db(){
        if (NULL == mysql_real_connect(&mysql,
                    host.c_str(),
                    user.c_str(),
                    passwd.c_str(),
                    db.c_str(),
                    port,
                    nullptr,
                    0)) {
            std::cout << "mysql_real_connect(): " 
            << mysql_error(&mysql) << std::endl;
            return;
        }
        std::cout << "Connected MySQL successful!" << std::endl;
    }
};

#endif