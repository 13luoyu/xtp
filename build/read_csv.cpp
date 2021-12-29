#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
using namespace std;

void split_depth(const string &line);
void split_entrust(const string &line);
void split_trade(const string &line);
int buffersize = 102400;

int main()
{
    umask(0);
    mkdir("time_csv", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("time_csv/depth", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("time_csv/trade", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("time_csv/entrust", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    ifstream in("depth.csv");
    char buffer[buffersize];
    in.getline(buffer, buffersize);
    while(!in.eof()){
        in.getline(buffer, buffersize);
        split_depth(buffer);
    }
    in.close();
    cout<<"Write depth complete."<<endl;

    ifstream in2("entrust.csv");
    in2.getline(buffer, buffersize);
    while(!in2.eof()){
        in2.getline(buffer, buffersize);
        split_entrust(buffer);
    }
    in2.close();
    cout<<"Write entrust complete."<<endl;

    ifstream in3("trade.csv");
    in3.getline(buffer, buffersize);
    while(!in3.eof()){
        in3.getline(buffer, buffersize);
        split_trade(buffer);
    }
    in3.close();
    cout<<"Write trade complete."<<endl;

    return 0;
}

void split_depth(const string &line)
{
    if(line.size()==0)
        return;
    int count = 0;
    string ticker;
    string data_time;
    for(int i=0;i<line.size();i++){
        if(line[i]==','){
            count++;
            if(count >= 14)
                break;
            continue;
        }
        if(line[i]==' ')
            continue;

        if(count == 2){
            ticker += line[i];
        }
        else if(count == 13){
            data_time += line[i];
        }
    }
    char filename[30];
    sprintf(filename, "time_csv/depth/%s.txt", ticker.c_str());
    ofstream out(filename, ios::app);
    out<<data_time<<endl;
    out.close();
}

void split_entrust(const string &line)
{
    if(line.size()==0)
        return;
    int count = 0;
    string ticker;
    string data_time;
    for(int i=0;i<line.size();i++){
        if(line[i]==','){
            count++;
            if(count >= 4)
                break;
            continue;
        }
        if(line[i]==' ')
            continue;

        if(count == 2){
            ticker += line[i];
        }
        else if(count == 3){
            data_time += line[i];
        }
    }
    char filename[30];
    sprintf(filename, "time_csv/entrust/%s.txt", ticker.c_str());
    ofstream out(filename, ios::app);
    out<<data_time<<endl;
    out.close();
}

void split_trade(const string &line)
{
    if(line.size()==0)
        return;
    int count = 0;
    string ticker;
    string data_time;
    for(int i=0;i<line.size();i++){
        if(line[i]==','){
            count++;
            if(count >= 4)
                break;
            continue;
        }
        if(line[i]==' ')
            continue;

        if(count == 2){
            ticker += line[i];
        }
        else if(count == 3){
            data_time += line[i];
        }
    }
    char filename[30];
    sprintf(filename, "time_csv/trade/%s.txt", ticker.c_str());
    ofstream out(filename, ios::app);
    out<<data_time<<endl;
    out.close();
}