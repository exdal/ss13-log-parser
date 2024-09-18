#pragma once

enum class FileResult : i32 {
    Success = 0,
    NoAccess,
    Exists,
    IsDir,
    InUse,
    Unknown,

    WriteInterrupt,
    ReadInterrupt,
};
constexpr bool operator!(FileResult v) {
    return v != FileResult::Success;
}

enum class FileAccess {
    Write = 1 << 0,
    Read = 1 << 1,
};

struct File {
    ls::option<uptr> handle = ls::nullopt;
    usize size = 0;
    FileResult result = FileResult::Success;

    File() = default;
    File(const fs::path &path, FileAccess access);
    File(const File &) = default;
    File(File &&) = default;
    ~File() { close(); }

    Result<usize, FileResult> write(this File &, const void *data, usize size);
    Result<usize, FileResult> read(this File &, void *data, usize size);
    void seek(this File &, u64 offset);
    void close(this File &);

    Result<std::string, FileResult> read_string(this File &, usize size);

    File &operator=(File &&) = default;
    bool operator==(const File &) const = default;
    explicit operator bool() { return result == FileResult::Success; }
};

template<>
struct has_bitmask<FileAccess> : ls::true_type {};
