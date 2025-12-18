#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <filesystem>
#include <fstream>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#else
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#endif
using namespace std;
namespace fs = filesystem;
unordered_set<string> builtins = {"echo", "exit", "type", "pwd"};

#ifdef _WIN32
const char os_pathsep = ';';
#else
const char os_pathsep = ':';
#endif

void splitByDelimeter(string s, vector<string> &args, char del)
{
  int last = s.size();
  while (!s.empty())
  {
    if (s.find(del) == string::npos)
    {
      args.push_back(s);
      break;
    }
    else
    {
      string tempStr = s.substr(0, s.find(del));
      args.push_back(tempStr);
      s = s.substr(s.find(del) + 1, s.size() - s.find(del));
    }
  }
}

string getExecFile(string cmd)
{
  string pathtoexec = "";
  string paths = getenv("PATH");
  vector<string> searchPaths;
  splitByDelimeter(paths, searchPaths, os_pathsep);
  for (string dirStr : searchPaths)
  {
    if (pathtoexec != "")
      break;
    fs::directory_entry dirEntry(dirStr);
    if (!exists(dirEntry) || !dirEntry.is_directory())
      continue;
    for (auto const &dir_entry : fs::directory_iterator(dirStr))
    {
      fs::path filepath = dir_entry;
      fs::path filestem = filepath.stem();
      if (cmd == filestem.string())
      {
        // file is found
        fs::perms p = fs::status(filepath).permissions();
        if (((fs::perms::group_exec & p) != fs::perms::none) || ((fs::perms::others_exec & p) != fs::perms::none))
        {
          pathtoexec = filepath.string();
          return pathtoexec;
        }
      }
    }
  }
  return pathtoexec;
}

void getArgv(string &input, vector<string> &argv)
{
  int i = 0;
  while (i < input.size())
  {
    string parsed = "";
    if (isspace(input[i]))
    {
      while (i < input.size() && isspace(input[i]))
        i++;
    }
    else if (input[i] == '\'')
    {
      i++;
      string insideq = "";
      while (i < input.size() && input[i] != '\'')
      {
        insideq += input[i];
        i++;
      }
      i++;
      parsed = insideq;
      argv.push_back(parsed);
    }
    else if (input[i] == '\"')
    {
      i++;
      string insideq = "";
      while (i < input.size() && input[i] != '\"')
      {
        if (input[i] == '\\')
        {
          if (i + 1 < input.size() && input[i + 1] == '\"' || input[i + 1] == '\\')
          {
            insideq += input[i + 1];
            i += 2;
            continue;
          }
        }
        insideq += input[i];
        i++;
      }
      i++;
      parsed = insideq;
      argv.push_back(parsed);
    }
    else
    {
      string nonempt = "";
      while (i < input.size() && !isspace(input[i]) && input[i] != '\'' && input[i] != '\"')
      {
        if (input[i] == '\\')
        {
          i++;
        }
        if (i >= input.size())
          break;
        nonempt += input[i];
        i++;
      }
      parsed = nonempt;
      argv.push_back(parsed);
    }
  }
  return;
}

void handleEcho(vector<string> &argv)
{
  for (int i = 1; i < argv.size(); i++)
  {
    cout << argv[i] << " ";
  }
  cout << "\n";
}

void handleType(vector<string> &argv)
{
  string command;
  if (argv.size() == 1)
    command = "";
  else
    command = argv[1];
  if (builtins.find(command) != builtins.end())
    cout << command << " is a shell builtin\n";
  else
  {
    string commandfile = getExecFile(command);
    if (commandfile != "")
      cout << command << " is " << commandfile << "\n";
    else
      cout << command << ": not found\n";
  }
}

void handleCd(vector<string> &argv)
{
  string path;
  if (argv.size() == 1)
    path = "";
  else
    path = argv[1];
  fs::directory_entry changedDir(path);
  if (path != "~" && exists(changedDir) && changedDir.is_directory())
  {
    fs::current_path(changedDir);
  }
  else if (path == "~")
  {
    string homePath = getenv("HOME");
    fs::directory_entry homeDir(homePath);
    fs::current_path(homeDir);
  }
  else
  {
    cout << "cd: " << path << ": No such file or directory\n";
  }
}

void handleCustom(vector<string> &argv,string input)
{
  string exeFile = getExecFile(argv[0]);
  #ifndef _WIN32
  if(exeFile==""){
    cout<<argv[0]<<": Command not found\n";
  }
  cout<<exeFile<<"\n";
  pid_t pid = fork();
  if(pid == 0){
    //child process it is
    char *argvc[argv.size()+1];
    for(int i=0;i<argv.size();i++){
      string& s = argv[i];
      char *cs =const_cast<char*>(s.c_str());
      argvc[i] = cs;
    }
    argvc[argv.size()]=NULL;
    execv(const_cast<char*>(exeFile.c_str()),argvc); 
  }else if(pid > 0){
    //its parent
    wait(NULL);
  }else{
    cout<<"Error occured\n";
  }
  #else
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char cmd[] = "C:\\Windows\\System32\\ping.exe -n 2 127.0.0.1";
    bool ok = CreateProcessA(
        "C:\\Windows\\System32\\ping.exe", // full path (optional but recommended)
        cmd,       // command line (argv as string)
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );
    if (!ok) {
        std::cerr << "CreateProcess failed: " << GetLastError() << "\n";
        return;
    }

    // Wait like wait()
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  #endif
}

int main()
{
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  // TODO: Uncomment the code below to pass the first stage

  while (true)
  {
    cout << "$ ";
    string input;
    getline(cin, input);
    vector<string> argv;
    getArgv(input, argv);
    if (argv.size() == 0)
      continue;

    if (argv[0] == "exit")
      break;
    else if (argv[0] == "type")
      handleType(argv);
    else if (argv[0] == "pwd"){
        fs::path currpath = fs::current_path();
      cout << currpath.string() << "\n";
    }else if (argv[0] == "cd")
      handleCd(argv);
    else if (argv[0] == "echo")
      handleEcho(argv);
    else
      handleCustom(argv,input);
  }
}