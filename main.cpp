//
//  main.cpp
//  Micro1_2
//
//  Created by Исыпов Илья on 12.11.2020.
//

#include <iostream>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <deque>
#include <vector>
#include <fstream>

#include <sys/types.h>      /* needed to use pid_t, etc. */
#include <sys/wait.h>       /* needed to use wait() */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>         /* LINUX constants and functions (fork(), etc.) */
#include <fcntl.h>
using namespace std;

void ProcessTime() {

    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    
    timeval user_tv, sys_tv;
    user_tv = usage.ru_utime;
    sys_tv = usage.ru_stime;
    
    cerr << "user " << user_tv.tv_sec <<"."<<user_tv.tv_usec<<" sec"<<endl;
    cerr << "system " << sys_tv.tv_sec <<"."<<sys_tv.tv_usec<<" sec"<<endl;
}


string CurDir () {
    char dir[FILENAME_MAX];
    getcwd(dir, FILENAME_MAX);
    //for (int i = 0; i < 10; i++) cout << dir[i];
    string CD = string(dir);
    return CD;
}

void ExecPwd(string a) {
    //close(1);//descriptor вывода
    //const char *name1 = a.c_str();
    //int t = open(name1, O_RDWR | O_CREAT |O_TRUNC, 0666);
    
    ofstream fout;
    fout.open(a);
    string S;
    fout << CurDir() << "\n";
    fout.close();
    //close(t);
}

int ExecCd(vector<string> &command) {
    if(command.size() != 2) {
        cout << "cd size error"<<endl;
        return 0;
    }
    else {
        if (chdir(command[1].c_str()) != 0) {
            cout << "cd exec error";
        }
        return 1;
    }
    
}


int Acception(char *word, char *rv) {
    if(*word == 0) return *rv == 0 | *rv == '/'| (*rv == '*' && strlen(rv) == 1 );
    switch(*rv) {
        case '*':{
            char *w = word;
            do {
                if(Acception(w, rv+1))
                    return 1;
            } while(*w++);
    
            return 0;
        }
        case '?':{
            return Acception(word+1, rv+1);
        }
        default: {
            return *word == *rv && Acception(word+1, rv+1);
        }
    }
}

int StarSubs(string word1, vector<string> &variants) {
    //возвращаем подходящие варианты
    string rv = word1;
    variants.clear();
    //type * = 1, ? = 0
    pid_t pid;
    int fd[2];
    pipe(fd);
    pid = fork();
    
    if (pid == 0)
     {
         close(fd[0]);
         close(1);
         dup2(fd[1], 1);
         
      execlp("/bin/ls","ls",(char *) NULL);
      exit(127);
     }
    else {
        int info;
        waitpid(pid, &info, 0);
        
          
        auto *buff = (char *) malloc(sizeof(char) * 1000);
        close(fd[1]);
        read(fd[0], buff, 1000);
        string buffs(buff);
        //free(buff);
         //cout <<"BEGIN LIST\n"<< buffs<<"END";
        //бьем на строки buff
        vector<string> dir;
        
        while (buffs.find('\n') != -1) {
            dir.push_back(buffs.substr(0, buffs.find('\n')));
            string word = buffs.substr(0, buffs.find('\n'));
            //cout << " look " << word ;
            //подходит ли по РВ
            char *crv = new char[rv.length() + 1];
            strcpy(crv, rv.c_str());
            
            char *cword = new char[word.length() + 1];
            strcpy(cword, word.c_str());
            
            //cout << " "<<Acception(cword, crv) << endl;
            if(Acception(cword, crv) == 1) variants.push_back(word);
            buffs = buffs.substr(buffs.find('\n') + 1, buffs.size());
        }
        
        delete[] buff;
        
        //exit(0);
    }
    return 1;
}

int StarDir(string direction, string start_dir, vector<string> &variants) {
    if(direction.size() == 1) {
        chdir("/");
        vector<string> vars;
        StarSubs("*", vars);
        for (vector<string>::iterator it = vars.begin() ; it!=vars.end() ; ++it) {
            variants.push_back("/"+*it);
            //cout <<"VARS: " <<*it <<endl;
        }
        return 1;
    }
    //string start_dir = CurDir();
    vector<string> dirs = {""};
    //разбиваем по '/'
    for(int i = 1; i < direction.size(); i++) {
        if(direction[i] !=  '/') {
            dirs[dirs.size() - 1]+=direction[i];
        }
        else dirs.push_back("");
    }
    string go_dir = "";
    int i;
    for(i = 0; i < dirs.size(); i++) {
        if((dirs[i].find('*') == -1) && (dirs[i].find('?') == -1)) {
            go_dir+="/"+dirs[i];
        } else {
            break;
        }
        //cout << dirs[i] <<"&"; cout << endl;
    }
    if (i == dirs.size()) {
        vector<string> vars;
        StarSubs(dirs[i - 1], vars);
        if(vars.size()!=0)
            variants.push_back(direction);
        return 0;
    }
    //cout <<"go dir "<<go_dir<<endl;
    string tale = "";
    for (int j = i+1; j < dirs.size(); j++) {
        if(dirs[j]!="")
            tale+=dirs[j] + "/";
    }
    //cout << "dir i " << dirs[i] << endl;
    if(chdir(go_dir.c_str()) == -1 && go_dir!="") return 0;
    //cout << "here :" << CurDir();
    vector<string> vars;
    StarSubs(dirs[i], vars);
    for (vector<string>::iterator it = vars.begin() ; it!=vars.end() ; ++it) {
        //cout <<"go_dir " << go_dir <<" it: "<<*it<<" tale: " << tale <<endl;
        //cout << "direct "<<direction<<endl;
        //cout <<"now dir "<< CurDir()<<endl;
        //cout <<"it: "<< *it << endl;
        //cout <<"tale: "<< tale << endl;
        //cout << go_dir + "/" + *it + "/" + tale << endl;
        if(tale!="")
            StarDir(go_dir + "/" + *it + "/" + tale, start_dir, variants);
        else
            StarDir(go_dir + "/" + *it, start_dir, variants);
        //else
        //cout << go_dir;
           // cout<<*it <<endl;
    }
    return 0;
}

int StarLine(vector<string> &commands) {
    //подставляем вместо *? в строке
    vector<string> commands1;
    for (int i = 0; i < commands.size(); i++) {
        if(commands[i].find('/') != -1 && (commands[i].find('*') != -1 || commands[i].find('?') != -1)) {
            string start_dir = CurDir();
            vector<string> variants = {};
            chdir("/");
            //cout << "comi: "<<commands[i]<<endl;
            StarDir(commands[i], CurDir(), variants);
            //cout <<"Answer\n";
            for (vector<string>::iterator it = variants.begin() ; it!=variants.end() ; ++it) {
                commands1.push_back(*it);
                //cout << *it<<endl;
            }
            chdir(start_dir.c_str());
            if(variants.size() == 0 && commands[i]!="/") {
                cout << "no coincedence" << endl;
                commands.clear();
                return 0;
            }
        } else
        if(commands[i].find('*') != -1 || commands[i].find('?') != -1) {
            vector<string> variants;
            StarSubs(commands[i], variants);
            for (vector<string>::iterator it = variants.begin() ; it!=variants.end() ; ++it) {
                commands1.push_back(*it);
                   // cout<<*it <<endl;
            }
            if(variants.size() == 0) {
                cout << "no coincedence" << endl;
                commands.clear();
                return 0;
            }
        } else {
            commands1.push_back(commands[i]);
        }
        
    }
    commands = commands1;
    return 0;
}

int ExecCom(vector<string> command, string a, string b, int k){
    vector<char *> arg;
    for (int i = 0; i < command.size(); ++i) {
        //cout << "comi:"<<command[i].c_str()<<endl;
        arg.push_back((char *) command[i].c_str());
    }
    arg.push_back(NULL);
    
    
    //int fd[2];
    //pipe(fd);
        pid_t pid = fork();
        if (pid == 0) {
            //cout << "ok3";
            if(k!=0) {
                close(0);//descriptor ввода
                const char *name = a.c_str();
                open(name, O_RDWR | O_CREAT, 0666);
            }
            close(1);//descriptor вывода
            const char *name1 = b.c_str();
            open(name1, O_RDWR | O_CREAT |O_TRUNC, 0666);
            //cout << "ok4"<<arg[0]<<endl;
            execvp(arg[0], &arg[0]);
            //cout << "ok6";
        } else {
            int info;
            //string line;
            //close(0);
            //const char *name = a.c_str();
            //int t = open(name, O_RDWR | O_CREAT, 0666);
            //dup2(t, 0);
            
            waitpid(pid, &info, 0);
            //close(t);
            //dup2(0,0);
        }
    return 0;
}

void Out(vector<string> commands, string &filename, int sign, string &inoutfile, int k) {
    //для вида com <(>) a sign: 1 == >
    string cod = commands [0];
    vector<char *> args;
    for (int i = 0; i < commands.size()-2; ++i) {
        args.push_back((char *) commands[i].c_str());
    }
    args.push_back(NULL);
    
    if (sign) {
        pid_t pid = fork();
        if (pid == 0) {
            if(k != 0) {
                close(0);//descriptor ввода
                const char *name1 = inoutfile.c_str();
                open(name1, O_RDWR | O_CREAT, 0666);
            }
            close(1);//descriptor вывода
            const char *name = filename.c_str();
            open(name, O_RDWR | O_CREAT | O_TRUNC, 0666);
            //делаем а файлом вывода
            execvp(cod.c_str(), &args[0]);
        } else {
            int info;
            waitpid(pid, &info, 0);
        }
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            close(0);
            const char *name = filename.c_str();
            open(name, O_RDWR | O_CREAT, 0666);
            
            close(1);
            const char *name1 = inoutfile.c_str();
            open(name1, O_RDWR | O_CREAT | O_TRUNC, 0666);
            //делаем а файлом ввода.
            execvp(cod.c_str(), &args[0]);
        } else {
            int info;
            //cout << info;
            waitpid(pid, &info, 0);
        }
    }
}

void Out2(vector<string> commands, string &filename1, string &filename2) {
    string cod = commands [0];
    vector<char *> args;
    for (int i = 0; i < commands.size()-4; ++i) {
        args.push_back((char *) commands[i].c_str());
    }
    args.push_back(NULL);
    
    
    //cases com < a > b & com > a < b
    int fd[2];
    pipe(fd);
    // fn1 > com > fn2
        pid_t pid = fork();
        if (pid == 0) {
            close(0);//descriptor ввода
            const char *name = filename1.c_str();
            open(name, O_RDWR | O_CREAT, 0666);
            
            close(1);//descriptor вывода
            const char *name1 = filename2.c_str();
            open(name1, O_RDWR | O_CREAT | O_TRUNC, 0666);
            execvp(cod.c_str(), &args[0]);
        } else {
            int info;
            waitpid(pid, &info, 0);
            
        }
    
}

int LineExec(deque<vector<string>> &commands) {
    string output = "";
    int k = 0; // p - qnty of < & >
    int proc_timer = 0;
    long start_t, end_t;
    start_t = clock();
    string a = "system1", b = "system2";
    while(commands.size() != 0) {
        int p = 0;
        vector<string> command = commands.front();
        long len = command.size();
        
        for (long i = 0; i < len; i++) {
            //cout <<"com i:" << command[i]<<endl;
            if(command[i].find('*')!=-1 || command[i].find('?')!=-1 || command[i].find('/')!=-1) {
                //удаляем *?
                //cout << "yes" << command[i]<<endl;
                StarLine(command);
                //cout << "after:";
                //for (int w = 0; w < command.size(); w++) cout << command[w] <<" ";
                //cout << endl;
                break;
                //return 1;
            }
        }
        
        for (long i = 0; i < len; i++) {
            if(command[i] == "<" || command[i] == ">") {
                //cout << "comi:" << command[i]<<endl;
                p++;
            }
        }
        
        if(command[0] == "time") {
            //cout << "time)\n";
            proc_timer = 1;
            command.erase(command.begin());
        }
        //cout << "p:"<<p<<endl;
        if(p == 2) {
            //cout << "ok";
            if(command.size() < 5) return 0;
            if(command[command.size() - 4] == "<" && command[command.size() - 2] == ">") {
                if(commands.size() == 1 && k == 0) {
                    Out2(command, command[command.size() - 3], command[command.size() - 1]);
                    return 1;
                } else {
                    cout << "< & > in conv\n";
                    return 0;
                }
            } else if(command[command.size() - 4] == ">" && command[command.size() - 2] == "<") {
                if(commands.size() == 1 && k == 0) {
                    Out2(command, command[command.size() - 1], command[command.size() - 3]);
                    return 1;
                } else {
                    cout << "< & > in conv\n";
                    return 0;
                }
            } else {
                cout << "< & > format error\n";
                return 0;
            }
        } else
            if(p == 1) {
                if(command.size() < 3) return 0;
                if(command[command.size() - 2] == "<") {
                    if(k!=0) {
                        cout <<"com < a not in start of conv\n";
                        return 0;
                    } else {
                        //cout << "ok";
                        Out(command, command[command.size() - 1], 0, b, k);
                        //return 1;
                    }
                //if(k == 0)
                }
                else if (command[command.size() - 2] == ">") {
                    if(commands.size() != 1) {
                        cout <<"com > a not in end of conv\n";
                        return 0;
                    } else {
                        //cout << "ok >\n";
                        if(k%2 == 1) {
                            Out(command, command[command.size() - 1], 1, b, k);
                        } else {
                            Out(command, command[command.size() - 1], 1, a, k);
                        }
                        //чистим вывод
                        ofstream fout;
                        fout.open(a);
                        fout.close();
                        fout.open(b);
                        fout.close();
                        //return 1;
                    }
                } else {
                    cout <<"Error in <(>) format\n";
                    return 0;
                }
            
        } else if(command[0] == "cd") {
            if(k!=0 || commands.size() > 1) {
                cout << "cd in conv error\n";
                return 0;
            }
            ExecCd(command);
        }
        else if (command[0] == "pwd") {
            //cout << "pwd case\n";
            if(k!=0) {
                cout << "pwd not in conv start\n";
                return 0;
            }
            ExecPwd(b);
        } else
        {
            if(k%2 == 0) {
                ExecCom(command, a, b, k);
            }
            else {
                ExecCom(command, b, a, k);
            }
            //ExecOuter(command);
            //cout <<"k:"<<k<<endl;
            
        }
        k++;
        //ProcessTime();
        //cout << "k:"<<k<<endl;
        commands.pop_front();
    }
    //cout <<"heelp\n";
    if(k%2 == 0) {
        //внимательно с чтением в конце
        //cout <<"case1"<<endl;
        ifstream in;
        in.open(a);
        string S;
             while ( getline(in,S)) {
                 cout << S<<endl;
             }
        in.close();
    } else {
        //cout <<"case2"<<endl;
        ifstream in;
        in.open(b);
        string S;
             while ( getline(in,S)){
                 cout << S <<endl;
             }
        in.close();
    }
    if(proc_timer == 1) {
        end_t = clock();
        cerr << "walltime " <<(double) (end_t - start_t) / 1000 <<" sec"<<endl;
        ProcessTime();
    }
    //чистим сист файлы
    remove(a.c_str());
    remove(b.c_str());
    //ofstream fout;
    //fout.open(a);
    //fout.close();
    //fout.open(b);
    //fout.close();
    return 1;
}

void LineProcessing(string line){
    //разбиваем каждую команду на string элементы
    deque<vector<string>> commands;
    commands.push_back(vector<string>());
    //long len = line.length();
    string com = "";
    
    char *word;
    char *cstr = new char[line.length() + 1];
    strcpy(cstr, line.c_str());
    word = strtok(cstr, " \t\r\n\a");
    while (word != NULL)
    {
        string s = word;
        if (s == "|"){
            //cout << "\n";
            commands.push_back(vector<string>());
            //commands содержат компоненты конвеера
        }
        else {
            //cout << s;
            commands.back().push_back(s);
        }
        word = strtok(NULL, " \t\r\n\a");
    }
    
    //CommandsExec(commands);
    LineExec(commands);
    /*
    int t = commands.size();
    cout << "com qnt:" << t <<"\n";
    for (int i = 0; i < t; i++) {
        for(int j = 0; j < commands[i].size(); j++) cout << commands[i][j] << " ";
        cout << endl;
    }
     */
}

int start(){
    string UserName = getenv("USER");
    
    string line;
    //chdir("/Users/isypov/Desktop/test");
    //
    while(1){
        cout <<CurDir();
        if (UserName == "root") {
            cout << "!";
        } else {
            cout << ">";
        }
        if(getline(cin, line)) {
            if(line.length()!= 0 ) {
                //cout << "look:" << line << endl;
                LineProcessing(line);
            }
            //else return 1;
        }
        else return 1;
        //cout << "work\n";
    }
    return 1;
   
}

int main(int argc, const char * argv[]) {
    start();
    return 0;
}

