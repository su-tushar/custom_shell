#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <filesystem>
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
      if (cmd == string(filestem))
      {
        // file is found
        fs::perms p = fs::status(filepath).permissions();
        if (((fs::perms::group_exec & p) != fs::perms::none) || ((fs::perms::others_exec & p) != fs::perms::none))
        {
          pathtoexec = string(filepath);
          return pathtoexec;
        }
      }
    }
  }
  return pathtoexec;
}

void handleEcho(string input)
{
  if(input.find("echo ")==string::npos) cout<<"Invalid format\n";
  else{
    string echoout = input.substr(input.find("echo ")+5,input.size()-input.find("echo "));
    cout<<echoout<<"\n";
  }
}

void handleType(string input)
{
  if(input.find("type ")==string::npos) cout<<"Invalid format\n";
  string arg2 = input.substr(input.find("type ")+5,input.size()-input.find("type "));
  if (builtins.find(arg2) != builtins.end())
    cout << arg2 << " is a shell builtin\n";
  else
  {
    string commandfile = getExecFile(arg2);
    if (commandfile != "")
      cout << arg2 << " is " << commandfile << "\n";
    else
      cout << arg2 << ": not found\n";
  }
}

void handleCd(string input)
{
  if(input.find("cd ")==string::npos) cout<<"Invalid format\n";
  string arg2 = input.substr(input.find("cd ")+3,input.size()-input.find("cd "));
  fs::directory_entry changedDir(arg2);
  if (arg2 != "~" && exists(changedDir) && changedDir.is_directory())
  {
    fs::current_path(changedDir);
  }
  else if (arg2 == "~")
  {
    string homePath = getenv("HOME");
    fs::directory_entry homeDir(homePath);
    fs::current_path(homeDir);
  }
  else
  {
    cout << "cd: " << arg2 << ": No such file or directory\n";
  }
}

void handleCustom(string fullInput,string command)
{
  string commandfile = getExecFile(command);
  if (commandfile != "")
  {
    int rstatus = system(fullInput.c_str());
  }
  else
    cout << command << ": command not found \n";
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

    if (input.size() == 0)
      continue;
    string command;
    if (input.find(' ') == string::npos)
      command = input;
    else
      command = input.substr(0, input.find(' '));
    if (command == "exit")
      break;
    else if (command == "echo")
      handleEcho(input);
    else if (command == "type")
      handleType(input);
    else if (command == "pwd")
      cout << string(fs::current_path()) << "\n";
    else if (command == "cd")
      handleCd(input);
    else
      handleCustom(input,command);
  }
}