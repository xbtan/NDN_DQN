#include "Add.h"

string Input_Prefix(string Input_Prefix_String)
{
 cout << "Please enter the prefix:";
 getline(cin, Input_Prefix_String);

 return Input_Prefix_String;
}

string Input_FileVersion(string Input_FileVersion_String)
{
 cout << "Please enter the FileVersion:";
 getline(cin, Input_FileVersion_String);

 return Input_FileVersion_String;
}

string Input_FilePath(string Input_FilePath_String)
{
 cout << "Please enter the File:";
 getline(cin, Input_FilePath_String);

 return Input_FilePath_String;
}

