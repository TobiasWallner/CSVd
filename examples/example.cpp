#include <csvd/csvd.hpp>

#include <iostream>
#include <fstream>

int main(int argc, char* argv[]){

    if(argc <= 1){
        std::cout << "No file given" << std::endl;
        return -1;
    }

    const char* filename = argv[1];

    std::ifstream file(filename);
    if(file.is_open() == false){
        std::cout << "Cannot open file: " << filename << std::endl;
        return -1;
    }else{
        std::cout << "Opened file: " << filename << std::endl;
    }

    csvd::CSVd csv;
    {
        tl::expected<void, csvd::ReadError> result = csv.read(file);
        if(result.has_value() == false){
            std::cout << "Error reading file: " << filename << std::endl;
            std::cout << result.error() << std::endl;
            return -1;
        }
    }

    std::cout << "found the following names: " << std::endl;
    for(const csvd::Column& elem : csv){
        std::cout << "  " << elem.name << std::endl;
    }

    if(csv.empty()){
        std::cout << "No data read from file: " << filename << std::endl;
        return -1;
    }

    for(int i = 0; i < csv.size(); ++i){
        const csvd::Column& col = csv[i];
        std::cout << "Column " << i << std::endl;
        std::cout << "  column name: " << col.name << std::endl;
        std::cout << "  column size: " << col.data.size() << std::endl;
        std::cout << "  first 3 values: ";
        for(int i = 0; i < 3 && i < col.data.size(); ++i){
            if(i != 0){
                std::cout << ", ";
            }
            std::cout << col.data[i];
        }
        std::cout << std::endl;
    }
    {
        auto column = csv.find("Frequencies (Hz)");
        if(column != csv.end()){
            std::cout << "Found column: Frequencies (Hz)" << std::endl;
            std::cout << "  column name: " << column->name << std::endl;
            std::cout << "  column size: " << column->data.size() << std::endl;
            std::cout << "  first 3 values: ";
        }

        std::cout << "Erase column 'Frequencies (Hz)'" << std::endl;
        csv.erase(column);
    }
    {
        auto column = csv.find("Home Prices");
        if(column != csv.end()){
            std::cout << "Found column: Home Prices" << std::endl;
            std::cout << "  column name: " << column->name << std::endl;
            std::cout << "  column size: " << column->data.size() << std::endl;
            std::cout << "  first 3 values: ";
        }
    }
    {
        auto column = csv.find_if([](std::string_view name){
            return name.starts_with("Mag") || name.starts_with("mag");
        });
        if(column != csv.end()){
            std::cout << "find_if: Found column with name: " << column->name << std::endl;
            std::cout << "  column size: " << column->data.size() << std::endl;
            std::cout << "  first 3 values: ";
        }else{
            std::cout << "find_if: Did not found a column" << std::endl;
        }
    }

    std::cout << "Write new csv" << std::endl;

    {
        std::ofstream file("new_data.csv");
        csv.write(file);
    }

    // delete real and imag
    return 0;
}

