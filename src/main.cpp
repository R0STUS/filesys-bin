#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <locale>
#include <codecvt>
#include <functional>
#include <map>
#include <iomanip>

#define dirst '\x00'
#define dirend '\x01'
#define filest '\x02'
#define filenx '\x03'
#define fileend '\x04'
#define error "\x0F"

std::string command;
char dir = '\x00';
std::string filesystempath = "filesystem.bin";
std::string filesys = "";
std::string cmdList = "TODO: CREATE CMDLIST GETTER FUNCTION.";
std::vector<std::string> words;

void writefilebin(const std::string& filePath, const std::string& hexValues) {
    std::ofstream file; file.open(filePath);
    file << hexValues;
    file.close();
}

std::string readfilebin(const std::string& filePath) {
    std::string hexString;
    std::stringstream buffer;
    std::ifstream file; file.open(filePath);
    if (file.is_open()) {
        buffer << file.rdbuf();
        hexString = buffer.str();
        return hexString;
    }
    else
        return error;
}

std::vector<std::string> parseToWords(std::string input) {
    std::vector<std::string> output;
    std::string tmp = "";
    for (char c : input) {
        if (c == '_') { output.push_back(tmp); tmp = ""; continue; }
        tmp += c;
    }
    output.push_back(tmp);
    return output;
}

char getMaxDirId() {
    char out = 0;
    int i;
    for (i = 0; i < filesys.size(); i++) {
        if (filesys[i] == dirst) {
            if (out < filesys[i + 1]) {
                out = filesys[i + 1];
                i++;
            }
        }
        if (filesys[i] == filest) i++;
    }
    return out + 1;
}

char getMaxFileId() {
    char out = -1;
    int i;
    for (i = 0; i < filesys.size(); i++) {
        if (filesys[i] == filest) {
            if (out < filesys[i + 1]) {
                out = filesys[i + 1];
                i++;
            }
        }
        if (filesys[i] == dirst) i++;
    }
    return out + 1;
}

std::string numToHex(const std::string& input) {
    std::string output;
    for (size_t i = 0; i < input.length(); i += 2) {
        std::string byteString = input.substr(i, 2);
        unsigned char byte = (unsigned char)strtol(byteString.c_str(), nullptr, 16);
        output += static_cast<char>(byte);
    }
    return output;
}

std::string toHex(unsigned char value) {
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << std::hex << std::nouppercase << static_cast<int>(value);
    return oss.str();
}

std::string binToStr(std::string fs) {
    std::string output;
    char skip = 0;
    char skipF = 0;
    std::map<char, std::string> mapa = {{dirst, "{"}, {dirend, "}"}, {filest, "["}, {filenx, "-"}, {fileend, "]"}};
    for (auto c : fs) {
        if (skip == 1) { output += "("; output += toHex(static_cast<unsigned char>(c)) + ")"; skip = 0; continue; }
        if (skipF == 2) { output += "("; output += toHex(static_cast<unsigned char>(c)) + ")"; skipF = 1; continue; } else if (skipF == 1) { output += toHex(static_cast<unsigned char>(c)); skipF = 0; continue; }
        if (c == dirst) skip = 1;
        if (c == filest) skipF = 2; if (c == filenx) skipF = 1;
        output += mapa[c];
    }
    return output;
}

struct Command {
    std::string commandName;
    std::function<void()> execute;
};

std::vector<Command> commands = { {
        "mkdir",
        [&dir, &filesys, &filesystempath, &cmdList, &words]() {
            int i;
            std::string tmp;
            for (i = 0; i < filesys.size(); i++) {
                tmp += filesys[i];
                if (filesys[i] == dirst && filesys[i + 1] == dir) {
                    tmp += filesys[i + 1]; tmp += dirst; tmp += getMaxDirId(); tmp += dirend;
                    //std::cout << binToStr(tmp) << std::endl;
                    i++;
                    //std::cout << "dir found!" << std::endl;
                }
            }
            writefilebin(filesystempath, tmp);
        }
    }, {
        "ls",
        [&dir, &filesys, &filesystempath, &cmdList, &words]() {
            std::cout << binToStr(filesys) << std::endl;
        }
    }, {
        "man",
        [&dir, &filesys, &filesystempath, &cmdList, &words]() {
            std::cout << "BFSICLI is a basic filesystem interaction command line interface. Commands:\n" << cmdList << std::endl;
        }
    }, {
        "cd",
        [&dir, &filesys, &filesystempath, &cmdList, &words]() {
            if (words.size() < 2)
                dir = '\x00';
            else {
                try {
                    dir = std::stoi(words[1]);
                }
                catch (...) {
                    std::cout << "Error!" << std::endl;
                }
            }
        }
    }, {
        "touch",
        [&dir, &filesys, &filesystempath, &cmdList, &words]() {
            int i;
            std::string tmp;
            for (i = 0; i < filesys.size(); i++) {
                tmp += filesys[i];
                if (filesys[i] == dirst && filesys[i + 1] == dir) {
                    tmp += filesys[i + 1]; tmp += filest; tmp += getMaxFileId();
                    if (words.size() > 1) {
                        try {
                            for (int i = 1; i < words.size(); i++) {
                                tmp += numToHex(words[i]);
                                if (i != words.size() - 1) tmp += filenx;
                            }
                        } catch (...) {std::cout << "Failed to make bytes." << std::endl;}
                    }
                    tmp += fileend;
                    i++;
                }
            }
            writefilebin(filesystempath, tmp);
        }
    }, {
        "rm",
        [&dir, &filesys, &filesystempath, &cmdList, &words]() {
            if (words.size() > 1) {
                int i;
                char wait = 0;
                std::string tmp;
                for (i = 0; i < filesys.size(); i++) {
                    if (filesys[i] == filest && filesys[i + 1] == numToHex(words[1])[0]) {
                        wait = 1;
                        std::cout << "Deleting file..." << std::endl;
                    }
                    if (wait == 0)
                        tmp += filesys[i];
                    if (wait == 1 && filesys[i] == fileend) wait = 0;
                }
                writefilebin(filesystempath, tmp);
            }
            else
                std::cout << "Argument required!" << std::endl;
        }
    }, {
        "rmdir",
        [&dir, &filesys, &filesystempath, &cmdList, &words]() {
            if (words.size() > 1) {
                int i;
                char wait = 0;
                std::string tmp;
                for (i = 0; i < filesys.size(); i++) {
                    if (filesys[i] == dirst && filesys[i + 1] == numToHex(words[1])[0]) {
                        wait = 1;
                        std::cout << "Deleting directory..." << std::endl;
                    }
                    if (wait == 0)
                        tmp += filesys[i];
                    if (wait == 1 && filesys[i] == dirend) wait = 0;
                }
                writefilebin(filesystempath, tmp);
            }
            else
                std::cout << "Argument required!" << std::endl;
        }
    }, {
        "exit",
        [&dir, &filesys, &filesystempath, &cmdList, &words]() {
            std::exit(0);
        }
    }
};

void getFileSys() {
    filesys = readfilebin(filesystempath);
    if (filesys == error) {
        filesys = "";
        filesys += dirst;
        filesys += '\x00';
        filesys += dirend;
        writefilebin(filesystempath, filesys);
    }
}

int main() {
    while (1) {
        getFileSys();
        command = "";
        std::cout << " '" << toHex(dir) << "' BFSICLI # ";
        std::cin >> command;
        std::cout << "Parsing command '" << command << "'..." << std::endl;
        words = parseToWords(command);
        std::cout << "Checking command..." << std::endl;
        for (auto c : commands) {
            if (c.commandName == words[0]) {
                c.execute();
                std::cout << "Command executed." << std::endl;
                break;
            }
        }
    }
    return 0;
}