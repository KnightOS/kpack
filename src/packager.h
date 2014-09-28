#ifndef INC_PKGR
#define INC_PKGR

extern void initRuntime();
extern int parse_args(int argc, char **argv);
extern int parse_metadata();
extern void writeModel(DIR *root, char *rootName);
extern void printMetadata(FILE *inputPackage);

#endif
