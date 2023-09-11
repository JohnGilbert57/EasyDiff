/*************************************************************
 *
 * ediff - ediff - "Easy Diff"
 *   simple text comparison using only system calls
 *
 * PUT OTHER INFORMATION ABOUT YOU AND ETC...
 * this is the version to start with...
 * Name: John Gilbert
 * Class: CS4420
 * Professor: Dr. Ostermann.
 *
 ************************************************************/

/* include all of the header files that the manual pages say we need */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
// forward declarations for local routines 
// these are just suggestions, but organizing your programing using
// these functions will save you a LOT of time
static void Usage(const char error[]);
static void Process(void);
static int ReadChar(int df, unsigned pos);
static int SameLine(int fd1, int pos1, int fd2, int pos2);
static void PrintLine(int fd, unsigned pos);
static int NextLinePos(int fd, unsigned pos);
static void printComparison(int fd1, int pos1, int fd2, int pos2, int lineNum);
static void printAddition(int fd, int pos, int lineNum, bool isLeft);
/* global variables */
static int debug = 0;
char *progname;

int fd1, fd2; /* file descriptors for the files to compare */
char *filename1 = NULL; /* file argument1 */
char *filename2 = NULL; /* file argument2 */


/*************************************************************
 **
 ** Main program, usage "ediff file1 file2"
 **
 *************************************************************/
int main(int argc, char *argv[]) {
    progname = argv[0];
    if (argc == 4) {
        if(argv[1][0] == '-' && argv[1][1] == 'd') {        
            debug = 1;
        } else {
            Usage(NULL);
        }
        filename1 = argv[2];
        filename2 = argv[3];
    } else if (argc == 3) {
        filename1 = argv[1];
        filename2 = argv[2];
    } else {
        Usage(NULL);
    }
    fd1 = open(filename1, O_RDONLY);
    fd2 = open(filename2, O_RDONLY);
    if (fd1 == -1) {
        perror(filename1);
        exit(-1);
    } else if (fd2 == -1) {
        perror(filename2);
        exit(-1);
    } else {
        Process();
    }
    
    if (debug)
        printf("Debug is set to %d\n", debug);

    // open the files specified and check for errors, etc...
    // Process(); /* do the real work */
    if(close(fd1) == -1) {
        perror(filename1);
        exit(-1);
    } else if (close(fd2) == -1) {
        perror(filename2);
        exit(-1);
    }
    exit(0); /* everything must have worked */
}

static void Process(void) {
    int currentLinePosOne = 0;
    int currentLinePosTwo = 0;
    int lineNumberOne = 1;
    int lineNumberTwo = 1;
    int diffs = 0;
    while(true) {
        if (currentLinePosOne != EOF && currentLinePosTwo != EOF) {
            if (!SameLine(fd1, currentLinePosOne, fd2, currentLinePosTwo)) {
                diffs++;
                printComparison(fd1, currentLinePosOne, fd2, currentLinePosTwo, lineNumberOne);
            }
            lineNumberOne++;
            lineNumberTwo++;
        } else if (currentLinePosOne != EOF) {
            diffs++;
            printAddition(fd1, currentLinePosOne, lineNumberOne - 1, true);
            lineNumberOne++; 
        } else if (currentLinePosTwo != EOF) {
            diffs++;
            printAddition(fd2, currentLinePosTwo, lineNumberTwo - 1, false);
            lineNumberTwo++;
        } else {
            break;
        }
        currentLinePosOne = NextLinePos(fd1, currentLinePosOne);
        currentLinePosTwo = NextLinePos(fd2, currentLinePosTwo);
    }
    if (!diffs) {
        printf("Files %s and %s are identical\n", filename1, filename2);
    }
}


static void printComparison(int fd1, int pos1, int fd2, int pos2, int lineNum) {
    printf("%dc%d\n", lineNum, lineNum);
    printf("< ");
    fflush(stdout);
    PrintLine(fd1, pos1);
    printf("\n");
    printf("---\n");
    printf("> ");
    fflush(stdout);
    PrintLine(fd2, pos2);
    printf("\n");
}

static void printAddition(int fd, int pos, int lineNum, bool isLeft) {
    printf("%dA\n", lineNum);
    if (isLeft) {
        printf("< ");
    } else {
        printf("> ");
    }
    fflush(stdout);
    PrintLine(fd, pos);
    printf("\n");
}

/*************************************************************
 **
 ** SameLine: are two lines the same??
 **
 *************************************************************/
static int SameLine(int fd1, int pos1, int fd2, int pos2) {

    if (debug) {
        printf("Comparing lines at pos %d and %d\n", pos1, pos2);
    }
    char c1 = ReadChar(fd1, pos1);
    char c2 = ReadChar(fd2, pos2);
    while (c1 == c2) {
        if ((c1 == '\n' && c2 == '\n') || (c1 == EOF && c2 == EOF)) {
            return true;
        }
        c1 = ReadChar(fd1, pos1++);
        c2 = ReadChar(fd2, pos2++);
    }
    return false;
}

/*************************************************************
 **
 ** NextLinePos: what is the file offset of the next 
 **    character beyond the newline, starting at "pos"
 ** returns EOF if there are no characters after the current line
 **
 *************************************************************/
static int NextLinePos(int fd, unsigned pos) {
    if (debug >= 1)
        printf("NextLinePos(%d, %u) called\n", fd, pos);

    char ch;
    if(read(fd, &ch, 1) != 1) {
        return EOF;
    }
    if (lseek(fd, pos, SEEK_SET) == -1) {
        perror(progname);
        exit(-1);
    } 
    do {
        pos++;
    } while(read(fd, &ch, 1) == 1 && ch != '\n');
    return pos;
}

/*************************************************************
 **
 ** PrintLine: print the line starting at pos
 **
 *************************************************************/
static void PrintLine(int fd, unsigned pos) {
    if (debug) {
        printf("Printing line from fd %d at position %u\n", fd, pos);
    }
    char ch;
    if (lseek(fd, pos, SEEK_SET) == -1) {
        perror(progname);
        exit(-1);
    }
    while (read(fd, &ch, 1) == 1 && ch != '\n') {
        if (write(1, &ch, 1) == -1) {
            if(debug) {
                printf("Caught an error while writing to the console");
            }
            perror(progname);
            exit(-1);
        }
    }   
}


/*************************************************************
 **
 ** ReadChar: read and return the char at position pos
 **    from the given descriptor given
 ** returns constant EOF on end of file
 **
 *************************************************************/
static int ReadChar(int fd, unsigned pos) {
    char ch;
    if (lseek(fd, pos, SEEK_SET) == -1) {
        perror(progname);
    }
    if(read(fd, &ch, 1) == 1) {
        return ch;
    }
    return EOF;
}


/*************************************************************
 **
 ** Usage: bad argument, tell how to call the program
 **
 *************************************************************/
static void Usage(const char error[]) {
    if (error) {
        printf("%s\n", error);
    }
    printf("usage: %s [-d] filename1 filename2\n", progname);
    exit(-1);
}
