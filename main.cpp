#include <iostream>
#include <boost/thread.hpp>
#include <memory>
#include <array>
#include <cstdio>

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main(int argc, char** argv) {
    std::cout << argv[0] << std::endl;
    int pid = std::stoi(exec("/Users/edward/Sync/projects/cpp/httb/tests/mock/run-server.sh"));

    bool exit = false;
    std::string input;
    std::cout << "type 'x' for terminating:" << std::endl;
    do {
        std::cin >> input;

        if(input == "x") {
            exit = true;
        }

    } while(!exit);

    std::stringstream ss;
    ss << "kill -9 " << pid;
    std::cout << ss.str() << std::endl;
    std::cout << exec(ss.str().c_str()) << std::endl;

    return 0;
}