#include <gzip/compress.hpp>
#include <re2/re2.h>
#include <simdjson.h>

#include "core/arg_parser.hh"
#include "core/file.hh"
#include "core/hash.hh"
#include "core/stack.hh"

template<typename... ArgsT>
constexpr usize format_into(c8 *begin, const fmt::format_string<ArgsT...> fmt, ArgsT &&...args) {
    c8 *end = fmt::vformat_to(begin, fmt.get(), fmt::make_format_args(args...));
    *end = '\0';

    return iptr(end - begin);
}

i32 main(i32 argc, c8 **argv) {
    auto args = ArgParser({ argv, static_cast<usize>(argc) });
    if (!args["--src"]) {
        fmt::println("'src' argument is expected.");
        return EXIT_FAILURE;
    }

    if (!args["--dst"]) {
        fmt::println("'dst' argument is expected.");
        return EXIT_FAILURE;
    }

    fs::path raw_logs = args["--src"].value();
    fs::path parsed_logs = args["--dst"].value();
    std::vector<fs::path> log_file_paths;

    for (auto &year : fs::directory_iterator(raw_logs)) {
        for (auto &month : fs::directory_iterator(year)) {
            for (auto &day : fs::directory_iterator(month)) {
                for (auto &round : fs::directory_iterator(day)) {
                    auto p_dir = parsed_logs / fs::relative(round, raw_logs);
                    // skip if file already exists
                    if (fs::exists(p_dir)) {
                        continue;
                    }

                    // create dirs on parsed tree
                    fs::create_directories(p_dir);

                    const fs::path &cur_dir = round;

                    auto add_if_exists = [&](std::string_view file) {
                        const fs::path &full_file_path = cur_dir / file;
                        if (fs::exists(full_file_path)) {
                            log_file_paths.push_back(full_file_path);
                        }
                    };

                    add_if_exists("admin.log.json");
                    add_if_exists("attack.log.json");
                    add_if_exists("game.log.json");
                    add_if_exists("shuttle.log.json");

                    add_if_exists("round_end_data.html");
                }
            }
        }
    }

    fmt::println("{} log files to be parsed.", log_file_paths.size());

    for (auto &p : log_file_paths) {
        ScopedStack stack;
        auto p_dir = fs::relative(p, raw_logs);
        auto file_path = parsed_logs / p_dir;
        if (file_path.extension() == ".html") {
            fs::copy_file(p, file_path);
            continue;
        }

        file_path.replace_extension("gz");
        std::string_view path_sv(p.native().data(), p.native().size());
        auto json = simdjson::padded_string::load(path_sv);
        if (json.error() != simdjson::error_code::SUCCESS) {
            continue;
        }
        simdjson::ondemand::parser parser;
        auto doc_stream = parser.iterate_many(json.value());
        if (doc_stream.error() != simdjson::error_code::SUCCESS) {
            continue;
        }

        auto parsed_logs_alloc = stack.alloc<c8>(json.value().length());
        c8 *parsed_logs_ptr = parsed_logs_alloc.data();

        for (auto doc : doc_stream.value()) {
            auto ts = doc["ts"].get_string();
            auto secret = doc["secret"].get_bool();
            auto msg = doc["msg"].get_string();
            auto cat = doc["cat"].get_string();

            // do not read this file
            if (secret.value_unsafe() == true) {
                continue;
            }

            // initial stage, log file informs us about categories exist in the file
            // TODO: nothing to do right now

            // if there is no message, don't write into file
            if (msg.error() != simdjson::error_code::SUCCESS) {
                continue;
            }

            auto msg_sv = msg.value();

            // skip secret stuff even before parsing

            std::string_view upper_cat = "UNKNOWN";
            if (cat.error() == simdjson::error_code::SUCCESS) {
                upper_cat = stack.to_upper(cat.value_unsafe());
            }

            switch (fnv64_str(upper_cat)) {
                case fnv64_c("GAME-ACCESS"): {
                    auto msg_str = std::string(msg.value());
                    RE2::Replace(&msg_str, R"((\d+\s+)?(\d+\.\d+\.\d+\.\d+)(\s*-\s*\d+)?)", "<SECRET-IP>");
                    parsed_logs_ptr += format_into(parsed_logs_ptr, "[{}] {}: {}\n", ts.value(), upper_cat, msg_str);
                    break;
                }
                case fnv64_c("GAME-COMPAT"): {
                    if (msg_sv.starts_with("ADMINPRIVATE")) {
                        continue;
                    }

                    parsed_logs_ptr += format_into(parsed_logs_ptr, "[{}] {}: {}\n", ts.value(), upper_cat, msg_sv);
                    break;
                }
                case fnv64_c("ADMIN"):
                case fnv64_c("ADMIN-DSAY"):
                case fnv64_c("ATTACK"):
                case fnv64_c("GAME"):
                case fnv64_c("GAME-VOTE"):
                case fnv64_c("GAME-EMOTE"):
                case fnv64_c("GAME-RADIO-EMOTE"):
                case fnv64_c("GAME-TRAITOR"):
                case fnv64_c("GAME-SAY"):
                case fnv64_c("GAME-WHISPER"):
                case fnv64_c("GAME-OOC"):
                case fnv64_c("GAME-LOOC"):
                case fnv64_c("GAME-PRAYER"):
                case fnv64_c("SHUTTLE"): {
                    parsed_logs_ptr += format_into(parsed_logs_ptr, "[{}] {}: {}\n", ts.value(), upper_cat, msg_sv);
                    break;
                }
                default: {
                    parsed_logs_ptr += format_into(parsed_logs_ptr, "<FILTERED CATEGORY {}>", upper_cat);
                    break;
                }
            }
        }

        auto parsed_log_file_size = usize(parsed_logs_ptr - parsed_logs_alloc.data());

        File parsed_log_file(file_path, FileAccess::Write);
        auto compressed_log = gzip::compress(parsed_logs_alloc.data(), parsed_log_file_size, Z_BEST_COMPRESSION);
        parsed_log_file.write(compressed_log.data(), compressed_log.size());
        parsed_log_file.close();

        fs::permissions(file_path, fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all, fs::perm_options::add);
    }

    return 0;
}
