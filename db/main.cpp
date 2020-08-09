#include "mysql_operation.h"
#include <iostream>
#include <string>

int main(){
    DBhelper a;
    std::string file_name = "AAAAA";
    std::string file_md5 = "A1B2C3D4E5F6G8";
    int block_nums = 3;
    std::cout << a.get_block_id_string(file_name) << std::endl;
    std::cout << a.get_block_num(file_name) << std::endl;
    std::cout << a.get_file_block_md5(file_name) << std::endl;
/*
    if(a.check_file_exist("AAAAA")){
        std::cout << "file exist" << std::endl;
    }
    else{
        std::cout << "not exist" << std::endl;
    }
    
    a.db_query("files", "*");
    a.view_query_result();

    if(a.check_file_exist(file_name)){
        std::cout << "file exist" << std::endl;
    }
    else{
        a.create_file(file_name, block_nums, file_md5);
    }
    
    //a.db_query("files", "*");
    //a.view_query_result();
    //a.db_query(file_name, "*");
    //a.view_query_result()

    if(a.check_upload_status(file_name)){
        std::cout << "upload successful" << std::endl;
    }
    else{
        std::cout << "upload not finished" << std::endl;
    }

    std::string md5_string = "0AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA1";
    md5_string += "0BBBBBBBBBBBBBBBBBBBBBBBBBBBBBB1";
    md5_string += "0CCCCCCCCCCCCCCCCCCCCCCCCCCCCCC1";
    a.set_block_md5(file_name, md5_string);
    a.set_block_uploaded(file_name, 0);
    a.set_block_uploaded(file_name, 1);
    a.set_block_uploaded(file_name, 2);

    if(a.check_upload_status(file_name)){
        std::cout << "upload successful" << std::endl;
    }
    else{
        std::cout << "upload not finished" << std::endl;
    }
    
    
    std::cout << a.get_block_md5(0, file_name) << std::endl;
    std::cout << a.get_block_id_string(file_name) << std::endl;
    std::cout << a.get_block_num(file_name) << ", and the size of value is " 
                << sizeof(a.get_block_num(file_name)) << std::endl;
*/
    //std::cout << a.get_file_block_md5(file_name) << std::endl;
    return 0;
}