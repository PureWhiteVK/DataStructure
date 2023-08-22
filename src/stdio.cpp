#include <iostream>
#include <sstream>

using namespace std;

// assume input is one line but does not limit input numbers
// like 
/*
[1,4,2,3,2,3,1,2]
[1,4,2,3,2,3]
[1,4,2,3,2,3,1,2,2,3,1,2,3]
*/
// how to correct parse input
int main() {
    std::string line;
    std::string str_value;
    std::vector<int> value;
    while(std::getline(std::cin,line)) {
        std::cout << "current line: " << line << std::endl;
        auto start_pos = line.find_first_of('[') + 1;
        auto end_pos = line.find_last_of(']');
        std::stringstream s{line.substr(start_pos,end_pos - start_pos)};
        value.clear();
        while(std::getline(s,str_value,',')) {
            value.emplace_back(std::stoi(str_value));
        }
        std::cout << "current input vector: " << std::endl;
        std::cout << "[ ";
        for(auto i : value) {
            std::cout << i << " ";
        }
        std::cout << "]" << std::endl;
    }
}