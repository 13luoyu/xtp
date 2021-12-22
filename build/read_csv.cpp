#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <sys/stat.h>
using namespace std;

unordered_map<string, FILE *> map;
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
    for(auto it=map.begin();it!=map.end();it++){
        FILE *f=it->second;
        fclose(f);
    }
    map.clear();
    in.close();
    cout<<"Write depth complete."<<endl;

    ifstream in2("entrust.csv");
    in2.getline(buffer, 1024);
    while(!in2.eof()){
        in2.getline(buffer, 1024);
        split(buffer, "entrust");
    }
    for(auto it=map.begin();it!=map.end();it++){
        FILE *f=it->second;
        fclose(f);
    }
    map.clear();
    in2.close();
    cout<<"Write entrust complete."<<endl;

    ifstream in3("trade.csv");
    in3.getline(buffer, 1024);
    while(!in3.eof()){
        in3.getline(buffer, 1024);
        split(buffer, "trade");
    }
    for(auto it=map.begin();it!=map.end();it++){
        FILE *f=it->second;
        fclose(f);
    }
    map.clear();
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
    FILE *out = map[ticker];
    if(out == NULL){
        char filename[30];
        sprintf(filename, "data/%s/%s.txt", file.c_str(), ticker.c_str());
        out = fopen(filename, "w+");
        map[ticker] = out;
    }
    data_time+="\n";
    fputs(data_time.c_str(), out);
}