
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

typedef struct print_array ppa;

#define LARGE  106
#define MEDIUM 76
#define SMALL 36


struct print_array{
    char* type;
    int curr;
    int full_length;
    int white_space_left;
    int white_space_right;
    int middle;
};


ppa* ppa_create(char*);
void ppa_print_start(ppa*, char*);
void ppa_add_line_left(ppa*, char*);
void ppa_add_line_right(ppa*, char*, char*);
void ppa_print_end(ppa*, char*);
void ppa_destroy(ppa*);
void ppa_put_at_mid(ppa*, int, char*);

void ppa_add_blanks(int);
void ppa_add_full_line(ppa*);