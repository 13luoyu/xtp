#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
using namespace std;

void split(const string &line, const string &file);

int main()
{
    umask(0);
    mkdir("data", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("data/depth", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("data/trade", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("data/entrust", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    ifstream in("depth.csv");
    char buffer[1024];
    in.getline(buffer, 1024);
    while(!in.eof()){
        in.getline(buffer, 1024);
        split(buffer, "depth");
    }
    in.close();
    cout<<"Write depth complete."<<endl;

    ifstream in2("entrust.csv");
    in2.getline(buffer, 1024);
    while(!in2.eof()){
        in2.getline(buffer, 1024);
        split(buffer, "entrust");
    }
    in2.close();
    cout<<"Write entrust complete."<<endl;

    ifstream in3("trade.csv");
    in3.getline(buffer, 1024);
    while(!in3.eof()){
        in3.getline(buffer, 1024);
        split(buffer, "trade");
    }
    in3.close();
    cout<<"Write trade complete."<<endl;

    return 0;
}

void split(const string &line, const string &file)
{
    if(line.size()==0)
        return;
    int count = 0;
    string ticker;
    string data_time;
    for(int i=0;i<line.size();i++){
        if(line[i]==','){
            count++;
            continue;
        }
        if(line[i]==' ')
            continue;

        if(count == 1){
            ticker += line[i];
        }
        else if(count == 12){
            data_time += line[i];
        }
    }
    char filename[30];
    sprintf(filename, "data/%s/%s.txt", file.c_str(), ticker.c_str());
    ofstream out(filename);
    out<<data_time<<endl;
    out.close();
}