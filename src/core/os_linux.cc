#include "core/os.hh"

#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <unistd.h>

Result<uptr, FileResult> os::file_open(const fs::path &path, FileAccess access) {
    errno = 0;
    auto file_result = FileResult::Success;

    i32 flags = O_CREAT | O_WRONLY | O_RDONLY | O_TRUNC;
    if (access & FileAccess::Write)
        flags &= ~O_RDONLY;
    if (access & FileAccess::Read)
        flags &= ~(O_WRONLY | O_CREAT | O_TRUNC);

    i32 file = open64(path.c_str(), flags, S_IRUSR | S_IWUSR);
    if (file < 0) {
        switch (errno) {
            case EACCES:
                file_result = FileResult::NoAccess;
                break;
            case EEXIST:
                file_result = FileResult::Exists;
                break;
            case EISDIR:
                file_result = FileResult::IsDir;
                break;
            case EBUSY:
                file_result = FileResult::InUse;
                break;
            default:
                file_result = FileResult::Unknown;
                break;
        }

        return file_result;
    }

    return static_cast<uptr>(file);
}

usize os::file_size(uptr file) {
    struct stat st = {};
    fstat(static_cast<i32>(file), &st);

    return st.st_size;
}

Result<usize, FileResult> os::file_write(uptr file, const void *data, usize size) {
    errno = 0;

    u64 written_bytes_size = 0;
    while (written_bytes_size < size) {
        u64 remainder_size = size - written_bytes_size;
        const u8 *cur_data = reinterpret_cast<const u8 *>(data) + written_bytes_size;
        iptr cur_written_size = write(static_cast<i32>(file), cur_data, remainder_size);
        if (cur_written_size < 0) {
            return FileResult::WriteInterrupt;
        }

        written_bytes_size += cur_written_size;
    }

    return written_bytes_size;
}

Result<usize, FileResult> os::file_read(uptr file, void *data, usize size) {
    errno = 0;

    u64 read_bytes_size = 0;
    while (read_bytes_size < size) {
        u64 remainder_size = size - read_bytes_size;
        u8 *cur_data = reinterpret_cast<u8 *>(data) + read_bytes_size;
        iptr cur_read_size = read(static_cast<i32>(file), cur_data, remainder_size);
        if (cur_read_size < 0) {
            return FileResult::ReadInterrupt;
        }

        read_bytes_size += cur_read_size;
    }

    return read_bytes_size;
}

void os::file_seek(uptr file, usize offset) {
    lseek64(static_cast<i32>(file), static_cast<i64>(offset), SEEK_SET);
}

void os::file_close(uptr file) {
    close(static_cast<i32>(file));
}

u64 os::mem_page_size() {
    return 0;
}

void *os::mem_reserve(u64 size) {
    return mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
}

void os::mem_release(void *data, u64 size) {
    munmap(data, size);
}

bool os::mem_commit(void *data, u64 size) {
    return mprotect(data, size, PROT_READ | PROT_WRITE);
}

void os::mem_decommit(void *data, u64 size) {
    madvise(data, size, MADV_DONTNEED);
    mprotect(data, size, PROT_NONE);
}
