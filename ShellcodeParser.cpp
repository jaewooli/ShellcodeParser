#include <iostream>
#include <string>
#include <fstream>
#include <getopt.h>
#include <sstream>
using namespace std;

static int MAX_FILENAMELENGTH = 15;
static const char* const short_options = "hA";
static const struct option long_options[] = {
        {"att",no_argument,0,'A'},
        {"help",no_argument,0,'h'},
        {NULL,0,NULL,0}
    };

static const char* const Help = 
"Usage:\n\tShellcodeParser TARGETFILENAME [options]\nOptions:\n\t-h\tprint this summary\n\t-A att \tconvert att syntax (default: intel)\n";
static bool isATT = false;

int main(int argc, char* argv[]){
    if (argc == 1){
        cerr<<"Usage: ShellcodeParser TARGETFILENAME [options]"<<endl<<"TARGETFILENAME should be next to ShellcodeParser"<<endl; 
        exit(EXIT_FAILURE);
    }

    string tfile = argv[1];
    int position = tfile.find_last_of(".");

    string targetfilename = tfile;
    string filetype = tfile.substr(position+1);
    string outputfilename = tfile.substr(0,position);

    if (targetfilename.length() > MAX_FILENAMELENGTH){
        cerr<<"File name should be shorter than "<<MAX_FILENAMELENGTH<<endl;
        exit(EXIT_FAILURE);
    }
    if (filetype !="s" && filetype !="asm" ){
        cerr<<"Only .s file can be inverted to shellcode"<<endl;
        exit(EXIT_FAILURE);
    }
    int next_option = 0;

    while(next_option != -1){
        next_option = getopt_long(argc, argv, short_options, long_options,NULL);
        switch(next_option){
            case 'h':
                cout << Help<<endl;
                exit(EXIT_FAILURE);
            case 'A':
                isATT = true;
                break;
            case -1:
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    char command1[120];
    char command2[120];
    char command3[120];
    char command4[120];
    char command5[120];

    if (isATT){
        sprintf(command1,"as -o %s.o %s",outputfilename.c_str(), targetfilename.c_str());
    }else{
        sprintf(command1,"nasm -f elf64 -o %s.o %s",outputfilename.c_str(), targetfilename.c_str());
    }
    
    sprintf(command2,"ld -o %s %s.o",outputfilename.c_str(),outputfilename.c_str());
    sprintf(command3, "objdump -d %s | grep \\<_start\\>: -A 100",outputfilename.c_str());
    sprintf(command4,"objdump -d %s | grep \\<_start\\>: -A 100 > %s_tmp",outputfilename.c_str(), outputfilename.c_str());
    sprintf(command5, "rm %s_tmp %s.o %s",outputfilename.c_str(),outputfilename.c_str(),outputfilename.c_str());

    try{
    system(command1);
    system(command2);
    system(command3);
    system(command4);

    stringstream shellcode;
    string bin;
    string tmp_filename = outputfilename + "_tmp";
    char tmp;
    bool isWriting = false;
    bool needNextLine = false;
    int nextInsert = 0;

    ifstream FileReader(tmp_filename,ios_base::in);

    getline(FileReader,bin);
    while(!FileReader.eof()){
        if (needNextLine){
            getline(FileReader,bin);
            isWriting = false;
            needNextLine = false;
        }else{
            FileReader.get(tmp);
            if (tmp == '\x20'){
                if (isWriting){
                    nextInsert = 2;
                }
            }else if (tmp == '\x09'){
                if (isWriting){
                    needNextLine = true;
                    continue;
                }
                isWriting = true;
                nextInsert = 2;
            }else{
                if(isWriting){
                    if (nextInsert == 2 ){
                        shellcode<<"\\x"<<tmp;
                    }else{
                        shellcode<<tmp;
                    }
                    nextInsert -= 1;
                }
            }
        }
    }

    system(command5);

    cout <<"\nShellcode:\t"<<shellcode.str()<<endl;
    }
    catch(const char* e){
        string e1 = e;
        if (e1.find("_start")){
            cerr<<"Need below string in "<<targetfilename<<"\n.sectopm .text\n.global _start\n_start:"<<endl;
        }else{
            cerr <<e<<endl;
        }  
    }



}