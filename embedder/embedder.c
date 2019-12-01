#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static void free_cptr(char **ptr)
{
    if (*ptr == NULL)
    {
        return;
    }
    free(*ptr);
}
#define autostring_t __attribute__((cleanup(free_cptr))) char *
#define autostring(VARNAME, FMT, ...) autostring_t VARNAME; asprintf(&VARNAME, FMT, ##__VA_ARGS__)

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: %s <libname> <filename> <symname>\n", argv[0]);
        return 1;
    }
    char *libname = argv[1];
    char *filename = argv[2];
    char *symname = argv[3];

    FILE *in = fopen(filename, "rb");
    if (NULL == in)
    {
        printf("ERROR: Unable to open %s\n", filename);
        return 1;
    }

    size_t file_size;
    fseek(in, 0, SEEK_END);
    file_size = ftell(in);
    fseek(in, 0, SEEK_SET);
    autostring_t filedata = malloc(file_size);
    assert(NULL != filedata);
    size_t bytes_read = fread(filedata, 1, file_size, in);
    assert(file_size == bytes_read);
    fclose(in);
    printf("Embedding %s (%zu bytes) as %s in %s\n", filename, file_size, symname, libname);

    char gaurd_name[256] = {0};
    for(size_t i = 0; i < strlen(libname); i++)
    {
        char c = libname[i];
        switch (c)
        {
        case ' ':
        case '.':
        case '/':
            c = '_';
        break;
        default:
            c = toupper(c);
        break;
        }
        gaurd_name[i] = c;
    }
    autostring(tmp_header_name, "%s.h.tmp", libname);
    autostring(tmp_source_name, "%s.c.tmp", libname);
    autostring(fin_header_name, "%s.h", libname);
    autostring(fin_source_name, "%s.c", libname);
    FILE *out = fopen(tmp_header_name, "wb");
    if (NULL == out)
    {
        printf("ERROR: Unable to open %s for writing\n", tmp_header_name);
        return 1;
    }
    fprintf(out, "#ifndef %s_H_\n", gaurd_name);
    fprintf(out, "#define %s_H_\n", gaurd_name);
    fprintf(out, "extern char *%s;\n", symname);
    fprintf(out, "#define %s_size %zu\n", symname, file_size);
    fprintf(out, "#endif\n");
    fclose(out);

    out = fopen(tmp_source_name, "wb");
    if (NULL == out)
    {
        printf("ERROR: Unable to open %s for writing\n", tmp_source_name);
        unlink(tmp_header_name);
        return 1;
    }
    fprintf(out, "#include <%s>\n", fin_header_name);
    fprintf(out, "static char _%s[] = {", symname);
    for(size_t i = 0; i < file_size; i++)
    {
        if (0 != i)
        {
            fprintf(out, ", ");
        }
        if (i % 16 == 0)
        {
            fprintf(out, "\n");
        }
        fprintf(out, "0x%02x", (unsigned char) filedata[i]);
    }
    fprintf(out, "\n};\n");
    fprintf(out, "char *%s = _%s;\n", symname,symname);
    fclose(out);
    // TODO: check these
    rename(tmp_header_name, fin_header_name);
    rename(tmp_source_name, fin_source_name);

    return 0;
}