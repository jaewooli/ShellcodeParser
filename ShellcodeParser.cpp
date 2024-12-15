#include <iostream>
#include <string>
#include <fstream>
#include <getopt.h>
using namespace std;

static int MAX_FILENAMELENGTH = 15;
static const char* const short_options = "n:h:";
static const struct option long_options[] = {
        {"name",required_argument,0,'n'},
        {"help",no_argument,0,'h'},
        {NULL,0,NULL,0}
    };

static const char* const Help = 
"Usage:\n\tShellcodeParser TARGETFILENAME [options]\nOptions:\n\t-h\t\t\tprint this summary\n\t-n name\t\tname the output file";
int main(int argc, char* argv[]){
    if (argc == 1){
        cerr<<"Usage: ShellcodeParser TARGETFILENAME [options]"<<endl<<"TARGETFILENAME should be next to ShellcodeParser"<<endl; //Is this good for security
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

    

    ifstream FileReader(targetfilename, ios_base::in |ios_base::binary);

    while(next_option != -1){
        next_option = getopt_long(argc, argv, short_options, long_options,NULL);
        switch(next_option){
            case 'n':
                outputfilename = optarg;
                break;
            case -1:
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    char command1[120]; //Should be bigger than MAX_FILENAME*2 + "gcc -o  "
    char command2[120];
    char command3[120];
    sprintf(command1,"nasm -f elf64 -o %s.o %s",outputfilename.c_str(), targetfilename.c_str());
    sprintf(command2,"ld -o %s %s.o",outputfilename.c_str(),outputfilename.c_str());
    sprintf(command3, "objdump -d %s | grep \\<_start\\>: -A 100",outputfilename.c_str());
    try{
    system(command1);
    system(command2);
    system(command3);
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