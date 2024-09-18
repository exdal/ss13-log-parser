#pragma once

#include "core/file.hh"
#include "ls/result.hh"

namespace os {
/// FILE ///
Result<uptr, FileResult> file_open(const fs::path &path, FileAccess access);
usize file_size(uptr file);
Result<usize, FileResult> file_write(uptr file, const void *data, usize size);
Result<usize, FileResult> file_read(uptr file, void *data, usize size);
void file_seek(uptr file, usize offset);
void file_close(uptr file);

/// MEMORY ///
u64 mem_page_size();
void *mem_reserve(u64 size);
void mem_release(void *data, u64 size = 0);
bool mem_commit(void *data, u64 size);
void mem_decommit(void *data, u64 size);
}  // namespace os
