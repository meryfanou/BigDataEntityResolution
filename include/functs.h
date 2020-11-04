typedef struct hashTable hashTable;
typedef struct specInfo specInfo;

void readDataset(DIR*, char*, hashTable**);
specInfo** readFile(FILE*, int*, specInfo**);