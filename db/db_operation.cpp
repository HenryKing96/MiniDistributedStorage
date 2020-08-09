/* using command below to compile:

g++ -o conn_test.o conn_test.c 
-L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lssl -lcrypto -ldl

*/
#include <iostream>
#include <mysql.h>
#include <string>
#include <vector>
using namespace std;

typedef struct key_info{
    string name;
    string type;
    string describe;
    bool pri_key;
    key_info(string name, string type, string describe, bool pri_key):
            name(name), type(type), describe(describe),pri_key(pri_key){}
}KEY;

// 与mysql相关的全局变量，在本任务中是不需要动的
string host = "9.134.44.12";
string user = "root";
string passwd = "jingxiangyi";
unsigned int port = 3306;
char *unix_socket = NULL;
unsigned long clientflag = 0;


int STATUS=0; //预留的执行状态标志，0为正常，其他对应可能存在的不同错误
int initilize(MYSQL &mysql); //对mysql变量进行初始化初始化
int conn_db(MYSQL &mysql, string db); //连接对应的db，中间使用到了全局变量
void close_connection(MYSQL &mysql); //关闭连接，释放资源
int db_add_row(MYSQL &mysql, string db, string table, vector<string> value); //在表中添加行,value中包含id、name、2个stat
int db_rm_row(MYSQL &mysql, string db, string table, string name); //删除table中名为name行
int db_add_table(MYSQL &mysql, string db, string name, vector<KEY> keys); //添加表，表名为name,四个信息为id、name和2个stat
int db_rm_table(MYSQL &mysql, string db, string name); //删除名字为name的表
int db_update_upload(MYSQL &mysql, string db, string table, string name, int upload_stat); //更新table表中name行的上传状态
int db_new_download(MYSQL &mysql, string db, string table, string name); //更新table表中文件的下载请求数
int db_update_download(MYSQL &mysql, string db, string table, string name, int download_stat);
bool check_upload(MYSQL &mysql, string db, string file); //检查文件是否上传完成,上传完成返回true
bool check_download(MYSQL &mysql, string db, string file);
int db_finish_download(MYSQL &mysql, string db, string table, string name);

MYSQL_RES *db_query(MYSQL &mysql, string db, string table, string items); //查询功能，指定需要查询的db、table、item
void view_query_result(MYSQL_RES *res, MYSQL &mysql); // 展示查询结果


int main(){
    MYSQL mysql;
    string db_name = "FILE";
    MYSQL_RES *result = NULL;
    string table = "files";
    string items = "*";
    if((STATUS=initilize(mysql))!=0) STATUS = -1;
    if((STATUS=conn_db(mysql, db_name))!=0) STATUS = -2;
    //if((result=db_query(mysql, db_name, table, items))==NULL) STATUS = -3;
    //view_query_result(result, mysql);

    vector<KEY> keys;
    keys.push_back(KEY("id","int","NOT NULL",true));
    keys.push_back(KEY("name","char(32)","NOT NULL",false));
    keys.push_back(KEY("upload_stat","int","NOT NULL",false));

    
    /* 
    在files表里面建立一个没有上传过的文件AAAAA，该文件有BBB、CCC、DDD三个block,
    建立名为AAAAA的表，里面记录BBB、CCC、DDD三个block，初始化stat均为0,
    随后模拟上传过程, 并实时查询状态, 上传成功之后files表中AAAAA的upload_stat变为1,
    随后模拟下载过程,
    */

    vector<string> file_info(4);
    string file_name = "AAAAA";
    static const int BLOCK_NUM = 3;
    string block_list[BLOCK_NUM] = {"BBB", "CCC", "DDD"};
    
    // 在file中添加文件AAAAA，并建立名为AAAAA的表
    if((STATUS=db_add_table(mysql, db_name, file_name, keys))!=0) STATUS = -4;
    file_info[0] = "0"; // file_ID
    file_info[1] = "\'" + file_name + "\'"; // file_name
    file_info[2] = "0"; // file_upload_stat
    file_info[3] = "3"; // block_num
    if((STATUS=db_add_row(mysql, db_name, "files", file_info))!=0) STATUS = -5;
    cout << "TABLE \"files\":" << endl;
    if((result=db_query(mysql, db_name, "files", items))==NULL) STATUS = -3;
    view_query_result(result, mysql);

    // 在表AAAAA中初始化每个block
    vector<string> block_info(3);
    for(int i=0;i<BLOCK_NUM;i++){
        block_info[0] = to_string(i);
        block_info[1] = "\'"+block_list[i]+"\'";
        block_info[2] = "0";
        if((STATUS=db_add_row(mysql, db_name, file_name, block_info))!=0) STATUS = -5;
        cout << "TABLE \"AAAAA\", added block " << (i+1) << endl;
        if((result=db_query(mysql, db_name, file_name, items))==NULL) STATUS = -3;
        view_query_result(result, mysql);
    }

    // （模拟）上传block成功之后更新
    for(int i=0;i<BLOCK_NUM;i++){
        string block_name = "\'" + block_list[i]+"\'";
        if((STATUS=db_update_upload(mysql, db_name, file_name, block_name, 1))!=0) STATUS = -6;
        cout << "TABLE \"AAAAA\", updated upload stat of block " << (i+1) << endl;
        if((result=db_query(mysql, db_name, file_name, items))==NULL) STATUS = -3;
        view_query_result(result, mysql);
        if(check_upload(mysql, db_name, file_name)){
            if((STATUS=db_update_upload(mysql, db_name, "files", "\'"+file_name+"\'", 1))!=0) STATUS = -6;
            cout << "\"AAAAA\" Upload completed!" << endl;
            if((result=db_query(mysql, db_name, "files", items))==NULL) STATUS = -3;
            view_query_result(result, mysql);
        }
    }

    // 收到一个下载请求
    //if((STATUS=db_new_download(mysql, db_name, "files", file_name))!=0) STATUS = -7;
    // 开始模拟下载过程
    /*
    for(int i=0;i<BLOCK_NUM;i++){
        string block_name = "\'" + block_list[i]+"\'";
        if((STATUS=db_update_download(mysql, db_name, file_name, block_name, 0))!=0) STATUS = -6;
        cout << "TABLE \"AAAAA\", updated download stat of block " << (i+1) << endl;
        if((result=db_query(mysql, db_name, file_name, items))==NULL) STATUS = -3;
        view_query_result(result, mysql);
        if(check_download(mysql, db_name, file_name)){
            if((STATUS=db_finish_download(mysql, db_name, "files", "\'"+file_name+"\'"))!=0) STATUS = -6;
            cout << "\"AAAAA\" Upload completed!" << endl;
            if((result=db_query(mysql, db_name, "files", items))==NULL) STATUS = -3;
            view_query_result(result, mysql);
        }
    }
    */

    //cout << "STATUS = " << STATUS << endl;
    close_connection(mysql);
    return STATUS;
}

// 分配和初始化mysql对象
int initilize(MYSQL &mysql){
    if (NULL == mysql_init(&mysql)) {
        cout << "mysql_init(): " << mysql_error(&mysql) << endl;
        return -1;
    }
    return 0;
}

// 连接到一个数据库
int conn_db(MYSQL &mysql, string db){
    if (NULL == mysql_real_connect(&mysql,
                host.c_str(),
                user.c_str(),
                passwd.c_str(),
                db.c_str(),
                port,
                unix_socket,
                clientflag)) {
        cout << "mysql_real_connect(): " 
        << mysql_error(&mysql) << endl;
        return -1;
    }
 
    cout << "Connected MySQL successful!" << endl;
    return 0;
}

// 关闭连接，释放资源
void close_connection(MYSQL &mysql){
    mysql_close(&mysql);
    cout << "Connection closed!" << endl;
}

// 通过mysql语句查询表中的信息
MYSQL_RES *db_query(MYSQL &mysql, string db, string table, string items){
    if(STATUS!=0){
        cout << "Please check connection" << endl;
        return NULL;
    }

    MYSQL_RES *mysql_result;
    string query_str = "SELECT " + items + " from " + table;

    if(mysql_real_query(&mysql, query_str.c_str(), 
                       query_str.length())!=0){
        cout << "db_query(): " 
        << mysql_error(&mysql) << endl;
        return NULL;
    }
    return mysql_store_result(&mysql);
}

// 查看查询的结果
void view_query_result(MYSQL_RES *res, MYSQL &mysql){
    if (NULL == res) {
         cout << "view_query_result(): " 
                << mysql_error(&mysql) << endl;
         return;
    }
    MYSQL_ROW row;
    int rows = mysql_num_rows(res);
    cout << "The total rows is: " << rows << endl;
    int fields = mysql_num_fields(res);
    cout << "The total fields is: " << fields << endl;
    while (row = mysql_fetch_row(res)) {
        for (int i = 0; i < fields; i++) {
            cout << row[i] << '\t';
        }
        cout << endl;
    }
    cout << endl;
}

// 在数据库中添加新的表
int db_add_table(MYSQL &mysql, string db, string name, vector<KEY> keys){
    if(STATUS!=0){
        cout << "Please check connection" << endl;
        return STATUS;
    }
    string primary_key;
    string query_str = "CREATE TABLE " + name + " (";
    for(int i=0;i<keys.size();i++){
        query_str += keys[i].name + " " + keys[i].type 
                     + " " + keys[i].describe + ",";
        if(keys[i].pri_key) primary_key = keys[i].name;
    }
    query_str += " PRIMARY KEY (" + primary_key + ")) ENGINE=InnoDB";
    //cout << query_str << endl;
    if(mysql_real_query(&mysql, query_str.c_str(), 
                       query_str.length())!=0){
        cout << "db_add_table(): " 
        << mysql_error(&mysql) << endl;
        return -1;
    }
    return 0;
}

// 删除名字为name的表
int db_rm_table(MYSQL &mysql, string db, string name){
    if(STATUS!=0){
        cout << "Please check connection" << endl;
        return STATUS;
    }
    
    string query_str = "DROP TABLE " + name;
    //cout << query_str << endl;
    if(mysql_real_query(&mysql, query_str.c_str(), 
                       query_str.length())!=0){
        cout << "db_rm_table(): " 
        << mysql_error(&mysql) << endl;
        return -1;
    }
    return 0;
}

// 在表中添加新的行
int db_add_row(MYSQL &mysql, string db, string table, vector<string> value){
    if(STATUS!=0){
        cout << "Please check connection" << endl;
        return STATUS;
    }

    string query_str = "INSERT INTO " + table + " VALUES(";
    for(int i=0;i<value.size();i++){
        query_str += value[i];
        if(i==value.size()-1) query_str += ")";
        else query_str += ",";
    }

    if(mysql_real_query(&mysql, query_str.c_str(), 
                       query_str.length())!=0){
        cout << "db_add_row(): " 
        << mysql_error(&mysql) << endl;
        return -1;
    }
    return 0;
}

// 在表中删除条件为name的行
int db_rm_row(MYSQL &mysql, string db, string table, string name){
    if(STATUS!=0){
        cout << "Please check connection" << endl;
        return STATUS;
    }

    string query_str = "DELETE FROM " + table + " WHERE NAME = "+ name;

    if(mysql_real_query(&mysql, query_str.c_str(), 
                       query_str.length())!=0){
        cout << "db_rm_row(): " 
        << mysql_error(&mysql) << endl;
        return -1;
    }
    return 0;
}

// 更新表中数据
int db_update_upload(MYSQL &mysql, string db, string table, string name, int upload_stat){
    if(STATUS!=0){
        cout << "Please check connection" << endl;
        return STATUS;
    }
    string query_str = "UPDATE "+table+" SET upload_stat="+to_string(upload_stat)+" WHERE name="+name;
    if(mysql_real_query(&mysql, query_str.c_str(), 
                       query_str.length())!=0){
        cout << "db_update_upload(): " 
        << mysql_error(&mysql) << endl;
        return -1;
    }
    return 0;
}

bool check_upload(MYSQL &mysql, string db, string file){
    if(STATUS!=0){
        cout << "Please check connection" << endl;
        return false;
    }

    //MYSQL_RES *mysql_result;
    string query_str = "SELECT * FROM " + file + " WHERE upload_stat=0";
    if(mysql_real_query(&mysql, query_str.c_str(), 
                       query_str.length())!=0){
        cout << "db_query(): " 
        << mysql_error(&mysql) << endl;
        return false;
    }
    //mysql_result = mysql_store_result(&mysql);
    if(mysql_num_rows(mysql_store_result(&mysql))==0) return true;
    return false;
}

int db_new_download(MYSQL &mysql, string db, string table, string name){
    if(STATUS!=0){
        cout << "Please check connection" << endl;
        return STATUS;
    }
    string query_str = "UPDATE "+table+" SET download_stat=download_stat+1 WHERE name=\'"+name+"\'";
    if(mysql_real_query(&mysql, query_str.c_str(), 
                       query_str.length())!=0){
        cout << "db_new_download(): " 
        << mysql_error(&mysql) << endl;
        return -1;
    }
    query_str = "UPDATE "+name+" SET download_stat=1";
    if(mysql_real_query(&mysql, query_str.c_str(), 
                       query_str.length())!=0){
        cout << "db_new_download(): " 
        << mysql_error(&mysql) << endl;
        return -1;
    }
    return 0;
}

int db_update_download(MYSQL &mysql, string db, string table, string name, int download_stat){
    if(STATUS!=0){
        cout << "Please check connection" << endl;
        return STATUS;
    }
    string query_str = "UPDATE "+table+" SET download_stat="+to_string(download_stat)+" WHERE name="+name;
    if(mysql_real_query(&mysql, query_str.c_str(), 
                       query_str.length())!=0){
        cout << "db_update_upload(): " 
        << mysql_error(&mysql) << endl;
        return -1;
    }
    return 0;
}

bool check_download(MYSQL &mysql, string db, string file){
    if(STATUS!=0){
        cout << "Please check connection" << endl;
        return false;
    }

    //MYSQL_RES *mysql_result;
    string query_str = "SELECT * FROM " + file + " WHERE download_stat=1";
    if(mysql_real_query(&mysql, query_str.c_str(), 
                       query_str.length())!=0){
        cout << "db_query(): " 
        << mysql_error(&mysql) << endl;
        return false;
    }
    //mysql_result = mysql_store_result(&mysql);
    if(mysql_num_rows(mysql_store_result(&mysql))==0) return true;
    return false;
}

int db_finish_download(MYSQL &mysql, string db, string table, string name){
    if(STATUS!=0){
        cout << "Please check connection" << endl;
        return STATUS;
    }
    string query_str = "UPDATE "+table+" SET download_stat=download_stat-1 WHERE name="+name;
    if(mysql_real_query(&mysql, query_str.c_str(), 
                       query_str.length())!=0){
        cout << "db_update_upload(): " 
        << mysql_error(&mysql) << endl;
        return -1;
    }
    return 0;
}