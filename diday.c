#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define K 4
#define N1 2
#define N2 2
#define N3 4
#define N4 3
#define ITERATION 10000
#define DATA_PATH "./data.txt"
#define LINE_BUF_SIZE 128
#define TOKEN_BUF_SIZE 32
#define INIT_POINT_SIZE 1

typedef struct
{
    double x;
    double y;
    double z;
} Point;

void readData(FILE *fp, Point *pPoints, int *pointNdx);

int main(int argc, char **argv)
{
    int  i;
    FILE *fp;

    fp = fopen(DATA_PATH, "r");
    if (NULL == fp) {
        fprintf(stderr, "error opening file %s\n", DATA_PATH);
        exit(EXIT_FAILURE);
    }

    Point *pPoints = calloc(INIT_POINT_SIZE, sizeof(Point));
    int   pointNdx = 0;
    readData(fp, pPoints, &pointNdx);

    for (i = 0; i < pointNdx; i++)
        printf("(%f, %f, %f)\n", pPoints[i].x, pPoints[i].y, pPoints[i].z);

    return 0;
}

void readData(FILE *fp, Point *pPoints, int *pointNdx)
{
    char  lineBuf[LINE_BUF_SIZE];
    char  *token;
    char  *delim     = ",";
    int   pointTotal = INIT_POINT_SIZE;
    char  compo      = 'x';
    while (fgets(lineBuf, LINE_BUF_SIZE, fp)) {
        token = strtok(lineBuf, delim);
        while (NULL != token) {
            if ('x' == compo) {
                compo = 'y';
                pPoints[*pointNdx].x = atof(token);
            } else if ('y' == compo) {
                compo = 'z';
                pPoints[*pointNdx].y = atof(token);
            } else if ('z' == compo) {
                compo = 'x';
                pPoints[(*pointNdx)++].z = atof(token);
                if (pointTotal <= *pointNdx) {
                    pointTotal += 2 * INIT_POINT_SIZE;
                    pPoints = realloc(pPoints, sizeof(Point) * pointTotal);
                }
            }
            token = strtok(NULL, delim);
        }
    }
}

