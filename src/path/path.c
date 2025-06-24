#include "path.h"

#include <cwalk/include/cwalk.h>
#include <sys/stat.h>

#define PATH_BUF_LEN 4096

struct Path {
    m_string_t str;
};

Path* path_init(const char* init) {
    Path* path = malloc(sizeof(Path));
    m_string_init_set(path->str, init);
    path_normalize(path);
    return path;
}

Path* path_init_data_dir(void) {
    // TODO: other OS's
    Path* path = path_init(getenv("HOME"));
    path_join(path, ".config/f95checker");
    return path;
}

Path* path_dup(Path* path) {
    return path_init(path_cstr(path));
}

const char* path_cstr(Path* path) {
    return m_string_get_cstr(path->str);
}

const char* path_name(Path* path) {
    const char* basename = NULL;
    size_t basename_len = 0;
    cwk_path_get_basename(path_cstr(path), &basename, &basename_len);
    if(basename == NULL) {
        return "";
    }
    assert(strlen(basename) == basename_len);
    return basename;
}

const char* path_extension(Path* path) {
    const char* extension = NULL;
    size_t extension_len = 0;
    cwk_path_get_extension(path_cstr(path), &extension, &extension_len);
    if(extension == NULL) {
        return "";
    }
    assert(strlen(extension) == extension_len);
    return extension;
}

void path_set(Path* path, const char* set) {
    m_string_set(path->str, set);
    path_normalize(path);
}

void path_set_name(Path* path, const char* name) {
    char path_buf[PATH_BUF_LEN];
    cwk_path_change_basename(path_cstr(path), name, path_buf, sizeof(path_buf));
    m_string_set(path->str, path_buf);
}

void path_set_extension(Path* path, const char* extension) {
    char path_buf[PATH_BUF_LEN];
    cwk_path_change_extension(path_cstr(path), extension, path_buf, sizeof(path_buf));
    m_string_set(path->str, path_buf);
}

void path_normalize(Path* path) {
    char path_buf[PATH_BUF_LEN];
    cwk_path_normalize(path_cstr(path), path_buf, sizeof(path_buf));
    m_string_set(path->str, path_buf);
}

void path_parent(Path* path) {
    path_join(path, "..");
}

void path_join(Path* path, const char* join) {
    char path_buf[PATH_BUF_LEN];
    cwk_path_join(path_cstr(path), join, path_buf, sizeof(path_buf));
    m_string_set(path->str, path_buf);
}

bool path_is_empty(Path* path) {
    return m_string_empty_p(path->str);
}

bool path_is_file(Path* path) {
    struct stat st;
    i32 res = stat(path_cstr(path), &st);
    if(res != 0) {
        assert(res == -1);
        if(errno != ENOENT) {
            perror(path_cstr(path));
        }
        return false;
    }
    return !S_ISDIR(st.st_mode);
}

bool path_is_dir(Path* path) {
    struct stat st;
    i32 res = stat(path_cstr(path), &st);
    if(res != 0) {
        assert(res == -1);
        if(errno != ENOENT) {
            perror(path_cstr(path));
        }
        return false;
    }
    return S_ISDIR(st.st_mode);
}

bool path_mkdir(Path* path, bool recursive) {
    if(path_is_dir(path)) {
        return true;
    }
    bool ret = true;

    Path* parent = path_dup(path);
    path_parent(parent);
    if(!path_is_dir(parent)) {
        if(recursive) {
            ret = path_mkdir(parent, recursive);
        } else {
            ret = false;
        }
    }
    path_free(parent);

    if(ret) {
        i32 res = mkdir(path_cstr(path), 0755);
        if(res != 0) {
            assert(res == -1);
            perror(path_cstr(path));
            ret = false;
        }
    }

    return ret;
}

bool path_read_lines(Path* path, m_string_list_ptr lines) {
    FILE* file = fopen(path_cstr(path), "r");
    if(file == NULL) {
        perror(path_cstr(path));
        return false;
    }

    m_string_t line;
    m_string_init(line);
    while(m_string_fgets(line, file, M_STRING_READ_PURE_LINE)) {
        m_string_list_push_back(lines, line);
    }
    m_string_clear(line);

    fclose(file);

    return true;
}

bool path_read_text(Path* path, m_string_ptr string) {
    FILE* file = fopen(path_cstr(path), "r");
    if(file == NULL) {
        perror(path_cstr(path));
        return false;
    }

    bool read = m_string_fgets(string, file, M_STRING_READ_FILE);
    fclose(file);

    return read;
}

bool path_read_bytes(Path* path, m_bstring_ptr bytes) {
    struct stat st;
    i32 res = stat(path_cstr(path), &st);
    if(res != 0) {
        assert(res == -1);
        perror(path_cstr(path));
        return false;
    }
    if(S_ISDIR(st.st_mode)) {
        return false;
    }

    FILE* file = fopen(path_cstr(path), "r");
    if(file == NULL) {
        perror(path_cstr(path));
        return false;
    }

    bool read = m_bstring_fread(bytes, file, st.st_size);
    fclose(file);

    return read;
}

bool path_write_raw(Path* path, const void* data, size_t size) {
    FILE* file = fopen(path_cstr(path), "w");
    if(file == NULL) {
        perror(path_cstr(path));
        return false;
    }

    size_t wrote = fwrite(data, 1, size, file);
    fclose(file);

    return wrote == size;
}

bool path_write_text(Path* path, m_string_ptr string) {
    FILE* file = fopen(path_cstr(path), "w");
    if(file == NULL) {
        perror(path_cstr(path));
        return false;
    }

    bool wrote = m_string_fputs(file, string);
    fclose(file);

    return wrote;
}

bool path_write_bytes(Path* path, m_bstring_ptr bytes) {
    return path_write_raw(
        path,
        m_bstring_view(bytes, 0, m_bstring_size(bytes)),
        m_bstring_size(bytes));
}

void path_free(Path* path) {
    m_string_clear(path->str);
    free(path);
}
