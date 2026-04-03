#ifndef MOCK_SD_H
#define MOCK_SD_H

#include "Arduino.h"
#include <map>
#include <string>
#include <vector>

#define FILE_READ 0x0
#define FILE_WRITE 0x1

class SDClass;

class File {
public:
    File() : _content(""), _pos(0), _open(false), _sd(nullptr) {}
    File(std::string name, std::string content) : _name(name), _content(content), _pos(0), _open(true), _sd(nullptr) {}

    int read();
    int peek();
    int read(byte* buffer, size_t len);
    int available();
    void close();
    String name() const { return _name; }
    bool isDirectory() const;
    File openNextFile();
    void rewindDirectory();

    operator bool() const { return _open; }

    String readStringUntil(char terminator) {
        String res = "";
        while (available()) {
            char c = (char)read();
            if (c == terminator) break;
            res += c;
        }
        return res;
    }

    void _set_sd(SDClass* sd) { _sd = sd; }

private:
    std::string _name;
    std::string _content;
    int _pos;
    bool _open;
    SDClass* _sd;
};

class SDClass {
public:
    bool begin(int cs_pin) { return true; }
    bool mock_slow = false;

    void rewindDirectory() { _dir_pos = 0; }
    File openNextFile() {
        if (_dir_pos < (int)_file_names.size()) {
            return open(_file_names[_dir_pos++].c_str());
        }
        return File();
    }

    File open(const char* filename, int mode = FILE_READ) {
        std::string name(filename);
        if (name == "/") {
            File f("/", "");
            f._set_sd(this);
            return f;
        }
        if (_files.count(name)) {
            File f(name, _files[name]);
            f._set_sd(this);
            return f;
        }
        // Try without leading slash if present
        if (name[0] == '/') {
            std::string name2 = name.substr(1);
            if (_files.count(name2)) {
                File f(name2, _files[name2]);
                f._set_sd(this);
                return f;
            }
        }
        return File();
    }

    void mock_add_file(std::string name, std::string content) {
        _files[name] = content;
        _file_names.push_back(name);
    }

private:
    std::map<std::string, std::string> _files;
    std::vector<std::string> _file_names;
    int _dir_pos = 0;
};

inline int File::read() {
    if (_sd && _sd->mock_slow && (_pos % 512 == 0)) delay(5); // Simulate 5ms latency per sector
    if (_pos < (int)_content.length()) {
        return (unsigned char)_content[_pos++];
    }
    return -1;
}

inline int File::peek() {
    return (_pos < (int)_content.length()) ? (unsigned char)_content[_pos] : -1;
}

inline int File::read(byte* buffer, size_t len) {
    size_t count = 0;
    while (count < len && available()) {
        // We use the internal read() which handles latency simulation
        buffer[count++] = (byte)read();
    }
    return (int)count;
}

inline int File::available() {
    return (int)_content.length() - _pos;
}

inline void File::close() {
    _open = false;
}

inline bool File::isDirectory() const {
    return _name == "/";
}

inline File File::openNextFile() {
    if (_sd) return _sd->openNextFile();
    return File();
}

inline void File::rewindDirectory() {
    if (_sd) _sd->rewindDirectory();
}

extern SDClass SD;

#endif
