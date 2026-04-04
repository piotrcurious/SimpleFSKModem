#ifndef MOCK_SD_H
#define MOCK_SD_H

#define SdFat_h

#include "Arduino.h"
#include <map>
#include <cstring>
#include <string>
#include <vector>

#define FILE_READ 0x0
#define FILE_WRITE 0x1

#define O_RDONLY 0x0
#define O_READ 0x0

class SDClass;
typedef class File SdFile;
typedef class SDClass SdFat;
#define SD_SCK_MHZ(x) (x)

class File {
public:
    File() : _pos(0), _open(false), _sd(nullptr) {}
    File(std::string name, std::string content) : _name(name), _content(content), _pos(0), _open(true), _sd(nullptr) {}

    int read();
    int available();
    int peek();
    int read(byte* buffer, size_t len);
    void close() { _open = false; }
    String name() const { return _name; }
    bool isDirectory() const { return _name == "" || _name == "/"; }
    bool isDir() const { return isDirectory(); }

    File openNextFile();

    bool openNext(File* dir, int mode) {
        *this = dir->openNextFile();
        return (bool)*this;
    }
    void rewindDirectory();
    void rewind() { rewindDirectory(); }

    bool open(const char* name, int mode = 0);
    void getName(char* buf, size_t len) {
        strncpy(buf, _name.c_str(), len);
    }

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
    bool begin(int cs_pin, int speed) { return true; }
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
        if (name.length() > 0 && name[0] == '/') name = name.substr(1);

        if (name == "" || name == "/") {
            File f("/", "");
            f._set_sd(this);
            return f;
        }

        if (_files.count(name)) {
            File f(name, _files[name]);
            f._set_sd(this);
            return f;
        }
        return File();
    }

    void mock_add_file(std::string name, std::string content) {
        std::string n = name;
        if (n.length() > 0 && n[0] == '/') n = n.substr(1);
        _files[n] = content;
        _file_names.push_back(n);
    }

private:
    std::map<std::string, std::string> _files;
    std::vector<std::string> _file_names;
    int _dir_pos = 0;
};

extern SDClass SD;

inline int File::read() {
    if (_sd && _sd->mock_slow && (_pos % 512 == 0)) delay(5);
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
        buffer[count++] = (byte)read();
    }
    return (int)count;
}

inline int File::available() {
    return (int)_content.length() - _pos;
}

inline void File::rewindDirectory() {
    if (_sd) _sd->rewindDirectory();
}

inline bool File::open(const char* name, int mode) {
    *this = SD.open(name, mode);
    return (bool)*this;
}

inline File File::openNextFile() {
    if (_sd) return _sd->openNextFile();
    return File();
}

#endif
