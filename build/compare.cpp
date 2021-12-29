#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>
using namespace std;

ofstream out("result.txt");
void compare(string filename);

int main()
{
    DIR *dir = opendir("depth");
    dirent *p;
    char base[1024];
    while((p = readdir(dir)) != NULL){
        string filename = p->d_name;
        compare(filename);
    }
    out.close();
}

void compare(string filename)
{
    ifstream in("depth/"+filename);
    ifstream in2("depth_market/"+filename);
    
    if(!in2){
        out<<"depth_market/"<<filename<<" not exist."<<endl;
        return;
    }
    int cnt1=0, cnt2=0;
    string tmp;
    while(in && !in.eof()){
        in>>tmp;
        tmp=tmp.substr(8);
        if(tmp <= "093000000" || tmp >= "150000000")
            continue;
        cnt1++;
    }
    while(in2 && !in2.eof()){
        in2>>tmp;
        if(tmp[0]!='1'){
            tmp = "0" + tmp;
        }
        if(tmp <= "093000000" || tmp >= "150000000")
        cnt2++;
    }
    if(cnt1!=cnt2)
        out<<filename<<": "<<cnt1<<" "<<cnt2<<endl;
}