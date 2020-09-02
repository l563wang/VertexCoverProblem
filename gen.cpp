#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <cstring>

using namespace std;

string exec(const char* cmd) {
    array<char, 1000> buffer{};
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main(int argc, char **argv) {
    string s = "./graphGen " + (string) argv[1];
    char *arg = (char *) s.c_str();
    //strcpy(arg, argv[1]);
    //ofstream out("output.txt");
    for (int i = 0; i < 10; i++) {
        string result = exec(arg);
        for (int j = 0; j < 10; j++) {
            cout << result;
        }
    }
    return 0;
}