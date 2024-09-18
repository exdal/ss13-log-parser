#include "core/file.hh"

#include "core/os.hh"

File::File(const fs::path &path, FileAccess access) {
    auto r = os::file_open(path, access);
    if (!r) {
        this->result = r.error();
        return;
    }

    this->handle = r.get();
    this->size = os::file_size(this->handle.value());
}

Result<usize, FileResult> File::write(this File &self, const void *data, usize size) {
    return os::file_write(self.handle.value(), data, size);
}

Result<usize, FileResult> File::read(this File &self, void *data, usize size) {
    return os::file_read(self.handle.value(), data, size);
}

void File::seek(this File &self, u64 offset) {
    os::file_seek(self.handle.value(), offset);
}

void File::close(this File &self) {
    if (self.handle) {
        os::file_close(self.handle.value());
        self.handle.reset();
    }
}

Result<std::string, FileResult> File::read_string(this File &self, usize size) {
    std::string str;
    str.resize(size);
    if (auto r = self.read(str.data(), str.length()); !r) {
        return r.error();
    }

    return str;
}
