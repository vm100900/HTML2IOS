#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>   // for strcasestr
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <mach-o/dyld.h>

#define PATH_MAX_LEN 4096

int copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return -1;

    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return -1;
    }

    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        fwrite(buf, 1, n, out);
    }

    fclose(in);
    fclose(out);
    return 0;
}

int copy_dir(const char *src, const char *dst) {
    struct stat st;
    if (stat(dst, &st) != 0) {
        if (mkdir(dst, 0755) != 0) {
            perror("mkdir");
            return -1;
        }
    }

    DIR *dir = opendir(src);
    if (!dir) return -1;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char src_path[PATH_MAX_LEN];
        char dst_path[PATH_MAX_LEN];
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, entry->d_name);

        if (entry->d_type == DT_DIR) {
            if (copy_dir(src_path, dst_path) != 0) {
                closedir(dir);
                return -1;
            }
        } else if (entry->d_type == DT_REG) {
            if (copy_file(src_path, dst_path) != 0) {
                closedir(dir);
                return -1;
            }
        }
    }

    closedir(dir);
    return 0;
}

void get_executable_dir(char *buf, size_t len) {
    char path[PATH_MAX_LEN];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        realpath(path, buf);
        char *dir = dirname(buf);
        strncpy(buf, dir, len - 1);
        buf[len - 1] = '\0';
    } else {
        fprintf(stderr, "Buffer too small; need size %u\n", size);
        exit(1);
    }
}

int is_HTML2IOS(const char *name) {
    return strcmp(name, "HTML2IOS") == 0;
}

char *read_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    fread(buffer, 1, size, f);
    buffer[size] = '\0';
    fclose(f);
    return buffer;
}

int write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "wb");
    if (!f) return -1;
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    return 0;
}

void inject_viewport_meta(char *filepath) {
    char *content = read_file(filepath);
    if (!content) return;

    const char *no_zoom_meta = "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no\">";
    
    char *search_pos = content;

    while (1) {
        char *meta_pos = strcasestr(search_pos, "<meta");
        if (!meta_pos) break;

        char *name_pos = strcasestr(meta_pos, "name=");
        if (!name_pos) {
            search_pos = meta_pos + 5;
            continue;
        }

        if (strcasestr(name_pos, "viewport")) {
            char *tag_end = strchr(meta_pos, '>');
            if (!tag_end) break;

            size_t old_tag_len = tag_end - meta_pos + 1;

            char *content_attr_pos = strcasestr(meta_pos, "content=");
            if (!content_attr_pos || content_attr_pos > tag_end) break;

            char *quote_start = strchr(content_attr_pos, '"');
            if (!quote_start || quote_start > tag_end) break;
            quote_start++;

            char *quote_end = strchr(quote_start, '"');
            if (!quote_end || quote_end > tag_end) break;

            size_t content_val_len = quote_end - quote_start;
            char *content_val = malloc(content_val_len + 1);
            if (!content_val) break;
            strncpy(content_val, quote_start, content_val_len);
            content_val[content_val_len] = '\0';

            int has_user_scalable = strstr(content_val, "user-scalable=no") != NULL;
            int has_maximum_scale = strstr(content_val, "maximum-scale=1") != NULL;

            free(content_val);

            if (has_user_scalable && has_maximum_scale) {
                free(content);
                return;
            }

            size_t content_len = strlen(content);
            size_t new_len = content_len - old_tag_len + strlen(no_zoom_meta) + 1;

            char *new_content = malloc(new_len);
            if (!new_content) break;

            size_t prefix_len = meta_pos - content;
            memcpy(new_content, content, prefix_len);
            strcpy(new_content + prefix_len, no_zoom_meta);
            strcpy(new_content + prefix_len + strlen(no_zoom_meta), tag_end + 1);

            write_file(filepath, new_content);

            free(new_content);
            free(content);
            return;
        }
        search_pos = meta_pos + 5;
    }

    char *new_content = NULL;
    char *head_start = strcasestr(content, "<head");
    if (head_start) {
        char *head_tag_end = strchr(head_start, '>');
        if (head_tag_end) {
            size_t offset = head_tag_end - content + 1;
            size_t new_size = strlen(content) + strlen(no_zoom_meta) + 10;
            new_content = malloc(new_size);
            if (!new_content) {
                free(content);
                return;
            }
            snprintf(new_content, new_size, "%.*s\n%s\n%s", (int)offset, content, no_zoom_meta, content + offset);
        }
    } else {
        char *html_start = strcasestr(content, "<html");
        if (html_start) {
            char *html_tag_end = strchr(html_start, '>');
            if (html_tag_end) {
                size_t offset = html_tag_end - content + 1;
                const char *head_open = "<head>\n";
                const char *head_close = "\n</head>\n";

                size_t new_size = strlen(content) + strlen(head_open) + strlen(head_close) + strlen(no_zoom_meta) + 10;
                new_content = malloc(new_size);
                if (!new_content) {
                    free(content);
                    return;
                }

                snprintf(new_content, new_size, "%.*s%s%s%s%s",
                         (int)offset, content,
                         head_open,
                         no_zoom_meta,
                         head_close,
                         content + offset);
            }
        } else {
            const char *html_open = "<html>\n<head>\n";
            const char *html_close = "\n</head>\n</html>";
            size_t new_size = strlen(content) + strlen(no_zoom_meta) + strlen(html_open) + strlen(html_close) + 10;
            new_content = malloc(new_size);
            if (!new_content) {
                free(content);
                return;
            }
            snprintf(new_content, new_size, "%s%s%s%s", html_open, no_zoom_meta, html_close, content);
        }
    }

    if (new_content) {
        write_file(filepath, new_content);
        free(new_content);
    }

    free(content);
}

void inject_nozoom_js(char *filepath) {
    const char *js_code =
        "let lastTouchEnd = 0;\n"
        "document.addEventListener('touchstart', function(event) {\n"
        "  if (event.touches.length > 1) {\n"
        "    event.preventDefault();\n"
        "  }\n"
        "}, { passive: false });\n"
        "document.addEventListener('touchend', function(event) {\n"
        "  let now = (new Date()).getTime();\n"
        "  if (now - lastTouchEnd <= 300) {\n"
        "    event.preventDefault();\n"
        "  }\n"
        "  lastTouchEnd = now;\n"
        "}, false);\n"
        "document.addEventListener('gesturestart', function(event) {\n"
        "  event.preventDefault();\n"
        "});\n";

    char *content = read_file(filepath);
    if (!content) return;

    char *script_start = strcasestr(content, "<script");
    if (script_start) {
        char *tag_end = strchr(script_start, '>');
        if (!tag_end) {
            goto fallback_inject;
        }
        tag_end++;

        size_t content_len = strlen(content);
        size_t js_len = strlen(js_code);

        size_t new_size = content_len + js_len + 1;

        char *new_content = malloc(new_size);
        if (!new_content) {
            free(content);
            return;
        }

        size_t prefix_len = tag_end - content;
        memcpy(new_content, content, prefix_len);
        memcpy(new_content + prefix_len, js_code, js_len);
        strcpy(new_content + prefix_len + js_len, tag_end);

        write_file(filepath, new_content);

        free(new_content);
        free(content);
        return;
    }

fallback_inject:
    const char *full_js_code =
        "<script>\n"
        "let lastTouchEnd = 0;\n"
        "document.addEventListener('touchstart', function(event) {\n"
        "  if (event.touches.length > 1) {\n"
        "    event.preventDefault();\n"
        "  }\n"
        "}, { passive: false });\n"
        "document.addEventListener('touchend', function(event) {\n"
        "  let now = (new Date()).getTime();\n"
        "  if (now - lastTouchEnd <= 300) {\n"
        "    event.preventDefault();\n"
        "  }\n"
        "  lastTouchEnd = now;\n"
        "}, false);\n"
        "document.addEventListener('gesturestart', function(event) {\n"
        "  event.preventDefault();\n"
        "});\n"
        "</script>\n";

    char *body_end = strcasestr(content, "</body>");
    char *new_content = NULL;
    if (body_end) {
        size_t prefix_len = body_end - content;
        size_t new_size = strlen(content) + strlen(full_js_code) + 1;

        new_content = malloc(new_size);
        if (!new_content) {
            free(content);
            return;
        }

        memcpy(new_content, content, prefix_len);
        strcpy(new_content + prefix_len, full_js_code);
        strcpy(new_content + prefix_len + strlen(full_js_code), body_end);
    } else {
        size_t new_size = strlen(content) + strlen(full_js_code) + 1;
        new_content = malloc(new_size);
        if (!new_content) {
            free(content);
            return;
        }
        strcpy(new_content, content);
        strcat(new_content, full_js_code);
    }

    write_file(filepath, new_content);
    free(new_content);
    free(content);
}

void scan_and_fix_html(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char fullpath[PATH_MAX_LEN];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(fullpath, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            scan_and_fix_html(fullpath);
        } else if (S_ISREG(st.st_mode)) {
            const char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".html") == 0) {
                inject_viewport_meta(fullpath);
                inject_nozoom_js(fullpath);
            }
        }
    }

    closedir(dir);
}

int main() {
    char exe_dir[PATH_MAX_LEN];
    get_executable_dir(exe_dir, sizeof(exe_dir));

    char src_HTML2IOS[PATH_MAX_LEN];
    snprintf(src_HTML2IOS, sizeof(src_HTML2IOS), "%s/HTML2IOS", exe_dir);

    char cwd[PATH_MAX_LEN];
    getcwd(cwd, sizeof(cwd));

    char dst_HTML2IOS[PATH_MAX_LEN];
    snprintf(dst_HTML2IOS, sizeof(dst_HTML2IOS), "%s/HTML2IOS", cwd);

    printf("Copying HTML2IOS from %s to %s\n", src_HTML2IOS, dst_HTML2IOS);
    if (copy_dir(src_HTML2IOS, dst_HTML2IOS) != 0) {
        fprintf(stderr, "Failed to copy HTML2IOS\n");
        return 1;
    }

    char dst_website[PATH_MAX_LEN];
    snprintf(dst_website, sizeof(dst_website), "%s/HTML2IOS/website", dst_HTML2IOS);

    mkdir(dst_website, 0755);

    DIR *dir = opendir(cwd);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || is_HTML2IOS(entry->d_name))
            continue;

        char src_path[PATH_MAX_LEN];
        snprintf(src_path, sizeof(src_path), "%s/%s", cwd, entry->d_name);

        struct stat st;
        stat(src_path, &st);

        char dst_path[PATH_MAX_LEN];
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dst_website, entry->d_name);

        if (S_ISDIR(st.st_mode)) {
            copy_dir(src_path, dst_path);
        } else if (S_ISREG(st.st_mode)) {
            copy_file(src_path, dst_path);
        }
    }

    closedir(dir);

    printf("Fixing HTML files in %s...\n", dst_website);
    scan_and_fix_html(dst_website);

    printf("All done.\n");
    return 0;
}
