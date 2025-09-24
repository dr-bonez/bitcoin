
extern "C" {
#include <quickjs/quickjs-libc.h>
#include <quickjs/quickjs.h>
}

#include <filesystem>
#include <map>
#include <string>
#include <vector>

bool g_bytecodeLoaded = false;
std::map<std::string, std::vector<uint8_t>> g_preJsBytecodeMap;
std::map<std::string, std::vector<uint8_t>> g_postJsBytecodeMap;

std::optional<std::vector<uint8_t>> loadFile(JSContext* ctx, const std::filesystem::path& path, int module)
{
    auto filename = path.filename().string();
    size_t len = 0;
    uint8_t* buf = js_load_file(ctx, &len, path.c_str());
    if (buf && len > 0) {
        auto ext = path.extension();
        int eval_flags = JS_EVAL_FLAG_COMPILE_ONLY;
        if (module < 0) {
            module = (ext == ".mjs" ||
                      JS_DetectModule((const char*)buf, len));
        }
        if (module)
            eval_flags |= JS_EVAL_TYPE_MODULE;
        else
            eval_flags |= JS_EVAL_TYPE_GLOBAL;

        JSValue obj = JS_Eval(ctx, (const char*)buf, len, filename.c_str(), eval_flags);
        if (JS_IsException(obj)) {
            // TODO: handle error
            js_free(ctx, buf);
            return std::nullopt;
        }
        js_free(ctx, buf);
        int flags;
        if (ext == ".js") {
            flags = JS_WRITE_OBJ_BYTECODE;
        } else if (ext == ".json") {
            flags = 0;
        } else {
            return std::nullopt;
        }
        size_t bytecode_len;
        uint8_t* bytecode = JS_WriteObject(ctx, &bytecode_len, obj, flags);
        JS_FreeValue(ctx, obj);

        return std::vector<uint8_t>(bytecode, bytecode + bytecode_len);
    }
    return std::nullopt;
}

void scanAndLoadFiles(JSContext* ctx, const std::string& directory)
{
    if (g_bytecodeLoaded) return;
    g_preJsBytecodeMap = {};
    g_postJsBytecodeMap = {};
    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            const auto& path = entry.path();
            auto filename = path.filename().string();
            if (path.extension() == ".js" &&
                !filename.empty() &&
                std::isdigit(static_cast<unsigned char>(filename[0]))) {
                auto bytecode = loadFile(ctx, path, 0);
                if (bytecode.has_value()) {
                    if (filename[0] == '0') {
                        g_preJsBytecodeMap[filename] = *bytecode;
                    } else {
                        g_postJsBytecodeMap[filename] = *bytecode;
                    }
                }
            }
        }
    }
    g_bytecodeLoaded = true;
}