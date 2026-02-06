#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define CLEAR_SCREEN() printf("\033[2J\033[H")
#define SLEEP_MS(ms) usleep((ms) * 1000)
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#define MAX_OUTPUT 8192

int decompress_line(const char *input, size_t input_len, char *output, size_t *output_len) {
    size_t out_pos = 0;
    size_t i = 0;
    
    while (i < input_len) {
        unsigned char ch = input[i++];
        int count = 0;
        int digits = 0;
        while (i < input_len && isdigit((unsigned char)input[i]) && digits < 4) {
            count = count * 10 + (input[i] - '0');
            i++;
            digits++;
        }
        
        if (digits == 0) {
            count = 1;
        }
        
        if (out_pos + count >= MAX_OUTPUT) {
            count = MAX_OUTPUT - out_pos - 1;
        }

        memset(output + out_pos, ch, count);
        out_pos += count;
    }
    
    output[out_pos] = '\0' ;
    *output_len = out_pos;
    return 0;
}

char* read_file(const char *filename, size_t *size) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }
    
    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *data = malloc(*size + 1);
    if (!data) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(fp);
        return NULL;
    }
    
    size_t read_size = fread(data, 1, *size, fp);
    data[read_size] = '\0';
    *size = read_size;
    fclose(fp);
    
    return data;
}

int main(int argc, char *argv[]) {
    const char *filename = (argc > 1) ? argv[1] : "data.txt";
    int fps = (argc > 2) ? atoi(argv[2]) : 30;
    
    printf("Bad Apple Terminal Player\n");
    printf("File: %s | FPS: %d\n", filename, fps);
    
    size_t file_size;
    char *file_data = read_file(filename, &file_size);
    if (!file_data) return 1;
    
    getchar();
    
    printf(HIDE_CURSOR);
    
    char *decompressed = malloc(MAX_OUTPUT);
    if (!decompressed) {
        fprintf(stderr, "Memory allocation failed\n");
        free(file_data);
        return 1;
    }
    
    int frame_delay_ms = 1000 / fps;
    int frame_count = 0;
    
    char *ptr = file_data;
    char *line_start = file_data;
    
    CLEAR_SCREEN();
    
    while (*ptr) {
        while (*ptr && *ptr != '\n') ptr++;
        
        size_t line_len = ptr - line_start;
        
        if (line_len > 0 && line_start[line_len - 1] == '\r') {
            line_len--;
        }
        
        if (line_len > 0) {
            size_t decompressed_len;
            
            if (decompress_line(line_start, line_len, decompressed, &decompressed_len) == 0) {
                fwrite(decompressed, 1, decompressed_len, stdout);
                printf("\n");
            }
        } else {
            fflush(stdout);
            SLEEP_MS(frame_delay_ms);
            CLEAR_SCREEN();
            frame_count++;
        }
        
        if (*ptr == '\n') {
            ptr++;
            line_start = ptr;
        }
    }
    
    fflush(stdout);
    SLEEP_MS(frame_delay_ms);
    
    printf(SHOW_CURSOR);
    
    free(file_data);
    free(decompressed);
    
    return 0;
}
