// Example how to use override and final

#include <memory>
#include <iostream>

class FileIo
{
public:
    virtual ssize_t write(const char* data, size_t size) = 0;
    void write_string(const std::string& str) { write(str.data(), str.size()); }
};

class StdioFile final : public FileIo
{
public:
    ssize_t write(const char* data, size_t size) final /* or: override */ {
        std::cout.write(data, size);
        return size;
    }
};

int main()
{
    std::unique_ptr<StdioFile> p = std::make_unique<StdioFile>();
    p->write_string("hello");

    return 0;
}

/******************************************************************************/
