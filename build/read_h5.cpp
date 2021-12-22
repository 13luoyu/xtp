#include "hdf5.h"
#include <iostream>
#include <string.h>
#include <fstream>
using namespace std;

//FeatureDB回调函数
herr_t FeaturesDB_func(hid_t group, const char *name, void *op_data);
//每只股票回调函数
herr_t Stock_func(hid_t group, const char *name, void *op_data);
//op_data的信息
struct Op_Data{
    char stock_id[7];   //股票ID
    char filename[15];  //文件名
};

int main()
{
    hid_t file;  //HDF5对象
    herr_t status;  //状态
    //打开文件
    file = H5Fopen("depth_market.h5", H5F_ACC_RDWR, H5P_DEFAULT);
    Op_Data data;
    strcpy(data.filename, "depth_market");    
    status = H5Giterate(file, "FeaturesDB", NULL,
                        FeaturesDB_func, &data);
    //关闭文件
    status = H5Fclose(file);

    file = H5Fopen("order.h5", H5F_ACC_RDWR, H5P_DEFAULT);
    strcpy(data.filename, "order");    
    status = H5Giterate(file, "FeaturesDB", NULL,
                        FeaturesDB_func, &data);
    status = H5Fclose(file);

    file = H5Fopen("transaction.h5", H5F_ACC_RDWR, H5P_DEFAULT);
    strcpy(data.filename, "transaction");    
    status = H5Giterate(file, "FeaturesDB", NULL,
                        FeaturesDB_func, &data);
    status = H5Fclose(file);

    return 0;
}

herr_t FeaturesDB_func(hid_t group, const char *name, void *op_data)
{
    herr_t status;
    Op_Data * data = (Op_Data *) op_data;
    strcpy(data->stock_id, name);
    status = H5Giterate(group, name, NULL,
                        Stock_func, data);
    return 0;
}

herr_t Stock_func(hid_t group, const char *name, void *op_data)
{
    int buffer_size = 6000;
    int buffer[buffer_size]={0};
    Op_Data * data = (Op_Data *)op_data;
    //对T进行处理
    if(strcmp(name, "T") == 0){
        hid_t dataset_t;
        dataset_t = H5Dopen(group, name, H5P_DEFAULT);
        herr_t status;
        status = H5Dread(dataset_t, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                        H5P_DEFAULT, buffer);
        
        //写文件，将每个股票的时间写下来。
        char filename[35];
        sprintf(filename, "time_h5/%s/%s.txt", data->filename, data->stock_id);
        ofstream out(filename);
        for(int i=0; i<buffer_size; i++){
            if(buffer[i] == 0)
                break;
            out<<buffer[i]<<endl;
        }
        out.close();
        status = H5Dclose(dataset_t);
    }
    return 0;
}