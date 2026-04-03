#ifndef MOCK_SD_H
#define MOCK_SD_H

#include "Arduino.h"
#include <map>
#include <string>

#define FILE_READ 0x0
#define FILE_WRITE 0x1

class File {
public:
    File() : _content(""), _pos(0), _open(false) {}
    File(std::string name, std::string content) : _name(name), _content(content), _pos(0), _open(true) {}

    int read() {
        if (_pos < (int)_content.length()) {
            return (unsigned char)_content[_pos++];
        }
        return -1;
    }

    bool available() {
        return _pos < (int)_content.length();
    }

    void close() {
        _open = false;
    }

    operator bool() const {
        return _open;
    }

    String readStringUntil(char terminator) {
        String res = "";
        while (available()) {
            char c = (char)read();
            if (c == terminator) break;
            res += c;
        }
        return res;
    }

private:
    std::string _name;
    std::string _content;
    int _pos;
    bool _open;
};

class SDClass {
public:
    bool begin(int cs_pin) { return true; }

    File open(const char* filename, int mode = FILE_READ) {
        std::string name(filename);
        if (_files.count(name)) {
            return File(name, _files[name]);
        }
        return File();
    }

    void mock_add_file(std::string name, std::string content) {
        _files[name] = content;
    }

private:
    std::map<std::string, std::string> _files;
};

extern SDClass SD;

#endif
