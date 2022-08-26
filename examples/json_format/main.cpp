#include "third_party/json/cjson-interface.h"
using neb::CJsonObject;
int main(int argc,char *argv[])
{
    (void)argc;
    (void)argv;
    std::string input_file_name;
    std::string output_file_name="./format.json";
    if(argc<2)
    {
        fprintf(stderr,"%s <input_file_path> [output_file_path->default:./format.json]",argv[0]);
    }
    input_file_name=argv[1];
    if(argc>=3)
    {
        output_file_name=argv[2];
    }
    auto object=CJsonObject::CreateInstance(input_file_name);
    object->SetSavePath(output_file_name);
    object->SaveToFile();
    return 0;
}
