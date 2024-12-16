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
        {"line",required_argument,0,'l'},
        {NULL,0,NULL,0}
    };

static const char* const Help = 
"Usage:\n\tShellcodeParser TARGETFILENAME [options]\nOptions:\n\t-h\t\tprint this summary\n\t-A --att \tconvert att syntax (default: intel)\n\t--line=[LINES] \tmodify object file dump lines(default: 100)";

static bool isATT = false;
static int objPirntLine = 100;
static string fileList[] = {"","","",""}; //fileList[0]: targetfile fileList[1]: .o , fileList[2]: binary, fileLIst[3]: binary_tmp

void setFileList(string& targetFileName, string& outputfileName){
    fileList[0] = targetFileName;
    fileList[1] = outputfileName + ".o";
    fileList[2] = outputfileName;
    fileList[3] = outputfileName+"_tmp";
}
void optionManager(int argc, char* argv[], const char* const short_options, const struct option long_options[],int* __longnid ){
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
            case 'l':
                objPirntLine = stoi(optarg);
                break;
            case -1:
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }
}

void compileFile(const bool isATT, const string& targetFileName, const string& objectFileName){     //need to add compile C file
    char command1[120];

    if (isATT){
        sprintf(command1,"as -o %s %s",objectFileName.c_str(), targetFileName.c_str());
    }else{
        sprintf(command1,"nasm -f elf64 -o %s %s",objectFileName.c_str(), targetFileName.c_str());
    }

    system(command1);
}

void linkFile(const string& objectFileName, const string& binaryFileName){
    char command[120];

    sprintf(command,"ld -o %s %s",binaryFileName.c_str(),objectFileName.c_str());
    system(command);
}

void printObjDump(const string& binaryFileName, const int objPrintLine){
    char command[120];
    sprintf(command, "objdump -d %s | grep \\<_start\\> -A %d",binaryFileName.c_str(), objPrintLine);
    system(command);
}

void printObjDump(const string& binaryFileName, const int objPrintLine, const string& tmpFileName){
    char command[120];
    sprintf(command, "objdump -d %s | grep \\<_start\\> -A %d > %s",binaryFileName.c_str(), objPrintLine, tmpFileName.c_str());
    system(command);
}

void removeNewFiles(const string& objectFileName, const string& binaryFileName, const string& tmpFileName){
    char command[120];
    sprintf(command, "rm %s %s %s",objectFileName.c_str(), binaryFileName.c_str(), tmpFileName.c_str());
    system(command);
}

stringstream writeShellcode(string& tmpFileName){
    stringstream shellcode;
    string trash;
    char tmp;
    bool isWriting = false;
    bool needNextLine = false;
    int nextInsert = 0;

    ifstream FileReader(tmpFileName,ios_base::in);

    getline(FileReader,trash);
    while(!FileReader.eof()){
        if (needNextLine){
            getline(FileReader,trash);
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
    return shellcode;
}

void printShellcode(const string& shellcode){
    cout <<"\nShellcode:\t"<<shellcode<<endl;
}


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

    optionManager(argc, argv, short_options, long_options, NULL);

    if (targetfilename.length() > MAX_FILENAMELENGTH){
        cerr<<"\n\tFile name should be shorter than "<<MAX_FILENAMELENGTH<<endl<<endl;
        exit(EXIT_FAILURE);
    }else if(filetype == targetfilename){
        cerr<<"\n\tTAGETFILE should be next to "<<argv[0]<<endl<<endl;
        exit(EXIT_FAILURE);
    }else if (filetype !="s" && filetype !="asm" ){
        cerr<<"\n\tOnly .s file can be inverted to shellcode\n"<<endl;
        exit(EXIT_FAILURE);
    }

    setFileList(targetfilename, outputfilename);

    try{
        compileFile(isATT,fileList[0],fileList[1]);
        linkFile(fileList[1],fileList[2]);
        printObjDump(fileList[2],objPirntLine);
        printObjDump(fileList[2],objPirntLine,fileList[3]);

        printShellcode(writeShellcode(fileList[3]).str());

        removeNewFiles(fileList[1],fileList[2],fileList[3]);
    }catch(exception& e){
        cerr<<e.what()<<endl;
        exit(EXIT_FAILURE);
    }



}