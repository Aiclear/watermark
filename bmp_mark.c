#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MARK_LEN 9
char *mark = "watermark"; // 水印文件的标志

char * get_watermark(FILE *fop, int len, char mark[], int location);
int get_watermark_lenght(FILE *fop);
void add_watermark_message(FILE *fip, FILE *fop, int len, char mark[]);
void add_watermark_flagInfo(FILE *fip, FILE *fop, int len);
void judge_file_type(FILE *fip);
void print_usage();

int main(int argc, char *argv[])
{
    FILE *fip, *fop;

    if (argc < 2) {
        print_usage();
    }

    if (strncmp(argv[1], "-", 1) == 0) {
        int flag = argv[1][1];

        switch(flag) {
        case 'e':
            fip = fopen(argv[2], "rb");
            if (fip == NULL) {
                printf("Can't open %s file\n", argv[2]);
                exit(1);
            }
            /* 判断文件是否是bmp文件 */
            judge_file_type(fip);
            /* 判断文件是否含有水印的标识 */
            char temp[MARK_LEN];
            get_watermark(fip, MARK_LEN, temp, 54);
            if (strncmp(mark, temp, MARK_LEN) == 0) {
                printf("the file had added watermark, so you can get message in this file\n");
                exit(0);
            }

            if (argv[4] == NULL || strlen(argv[4]) <= 0) {
                printf("Please input your message\n");
                exit(1);
            }
            fop = fopen(argv[3], "w+");
            add_watermark_message(fip, fop, strlen(argv[4]), argv[4]);
            fclose(fip);
            fclose(fop);
            break;

        case 'd':
            fop = fopen(argv[2], "rb");
            get_watermark(fop, MARK_LEN, temp, 54);
            if (strncmp(mark, temp, MARK_LEN) != 0) {
                printf("can't read info from this file, it has not a mark of \"watermark\"\n");
                exit(0);
            }
            int len = get_watermark_lenght(fop);
            char *string = (char *) malloc(sizeof(char) * len);
            get_watermark(fop, len, string, 158);
            for (int i = 0; i < len; i++) {
                printf("%c", string[i]);
            }
            fclose(fop);
            break;

        case 'h':
            print_usage();
            break;
        default:
            print_usage();
            break;
        }
    } else {
        printf("Invaild argument! use -h for help\n");
        exit(1);
    }

    return 0;
}

void add_watermark_flagInfo(FILE *fip, FILE *fop, int len)
{
    /* 在这里一定要重新设定文件指针指向文件内的位置，不然
    * 你会发现文件在写入信息之后便无法打开了，因为文件头部
    * 的信息被破会了
    */
    fseek(fip, 0, SEEK_SET);
    fseek(fop, 0, SEEK_SET);
    int c;
    for (int i = 0; i < 54; i++) {
        c = fgetc(fip);
        fputc(c, fop);
    }

    /*
     * 现在文件指针位置已经正确了，不需要再次指定位置了
     */
    // fseek(fip, 54, SEEK_SET);
    // fseek(fop, 54, SEEK_SET);
    for (int i = 0; i < MARK_LEN; i++) { /* 写入水印的标志 */
        for (int j = 0; j < 8; j++) {
            c = fgetc(fip);
            if ((mark[i] & (0x01 << j)) == 0) {
                c &= 0xFE;
            } else {
                c |= 0x01;
            }
            fputc(c, fop);
        }
    }
    for (int j = 0; j < 32; j++) { /* 保存存入字符的长度信息 */
        c = fgetc(fip);
        if ((len & (0x01 << j)) == 0) {
            c &= 0xFE;
        } else {
            c |= 0x01;
        }
        fputc(c, fop);
    }
}

/**
 * 写入需要隐藏在文件中的信息
 */
void add_watermark_message(FILE *fip, FILE *fop, int len, char message[])
{
    add_watermark_flagInfo(fip, fop, len);

    int c, i = 0;
    fseek(fip, 158, SEEK_SET);
    fseek(fop, 158, SEEK_SET);
    while ((c = fgetc(fip)) != EOF) {
        if (i < strlen(message)) {
            for (int j = 0; j < 8; j++) {
                if ((message[i] & (0x01 << j)) == 0) {
                    c &= 0xFE;
                } else {
                    c |= 0x01;
                }
                fputc(c, fop);
            }
        } else {
            fputc(c, fop);
        }
        i++;
    }
}

/**
 * 获取文件中的水印标志，以便于判断文件是否是此程序生成的文件
 */
char * get_watermark(FILE *fop, int len, char mark[], int location)
{
    fseek(fop, location, SEEK_SET);
    int tmp = 0;
    for (int j = 0; j < len; j++) {
        for (int i = 0; i < 8; i++) {
            tmp |= ((fgetc(fop) & 0x01) << i);
        }
        mark[j] = tmp;
        tmp = 0;
    }

    return mark;
}

/**
 * 获取水印文件中保存的字符长度的信息
 */
int get_watermark_lenght(FILE *fop)
{
    fseek(fop, 126, SEEK_SET);
    int length = 0;
    for (int i = 0; i < 32; i++) {
        length |= ((fgetc(fop) & 0x01) << i);
    }

    return length;
}

/*
 * 根据bmp文件的前两个字符为BM判断文件的类型
 */
void judge_file_type(FILE *fip)
{
    char s[3];
    fgets(s, 3, fip);
    if (strncmp(s, "BM", 2) != 0) {
        printf("%s\n", s);
        printf("The file can't recognition as a bmp type!");
        exit(1);
    }
}

void print_usage()
{
    printf("bmp_mark [option] [argument]\n");
    printf("option:\n\n");
    printf("-e \t add watermark for file\n");
    printf("-d \t get the content from the file that was added watermark\n");
    printf("-h \t display this help message\n\n");
    printf("Example: ./bmp_mark -e bmp_mark input_file output_file_name message\n");
    printf("\t ./bmp_mark -d marked_file\n");
    exit(1);
}
