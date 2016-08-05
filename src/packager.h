#ifndef INC_PKGR
#define INC_PKGR

/* defines for nftw() */
#define _XOPEN_SOURCE 1
#define _XOPEN_SOURCE_EXTENDED 1

/* used to calculate how many FDs we allow nftw to use */
#define NUM_FDS_TO_RESERVE (3)

extern void initRuntime();
extern int parse_args(int argc, char **argv);
extern int parse_metadata();
extern void writeModel(DIR *root, char *rootName);
extern void printMetadata(FILE *inputPackage);

#endif
