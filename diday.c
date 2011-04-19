#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <time.h>

#define K 4
#define N0 2
#define N1 2
#define N2 4
#define N3 3
#define ITERATION 10000
#define DATA_PATH "./data.txt"
#define LINE_BUF_SIZE 128
#define TOKEN_BUF_SIZE 32
#define INIT_FEATURE_VECTOR_SIZE 1

typedef struct
{
    double x;
    double y;
    double z;
    int    cluster;
    int    core;
} FVector;

typedef struct
{

} Cluster_info;

void   readData(FILE *fp, FVector **pFVectors, int *fVectorNdx);
void   didayDynamicClusterMethod(FVector *pFVectors, int fVectorTotal);
double twoNorm(FVector a, FVector b);

int main(int argc, char **argv)
{
    int  i;
    FILE *fp;

    fp = fopen(DATA_PATH, "r");
    if (NULL == fp) {
        fprintf(stderr, "error opening file %s\n", DATA_PATH);
        exit(EXIT_FAILURE);
    }

    FVector *pFVectors = calloc(INIT_FEATURE_VECTOR_SIZE, sizeof(FVector));
    int   fVectorNdx = 0;
    readData(fp, &pFVectors, &fVectorNdx);

    didayDynamicClusterMethod(pFVectors, fVectorNdx);
    return 0;
}

void readData(FILE *fp, FVector **pFVectors, int *fVectorNdx)
{
    char  lineBuf[LINE_BUF_SIZE];
    char  *token;
    char  *delim     = ",";
    int   fVectorTotal = INIT_FEATURE_VECTOR_SIZE;
    char  compo      = 'x';

    while (fgets(lineBuf, LINE_BUF_SIZE, fp)) {
        token = strtok(lineBuf, delim);
        while (NULL != token) {
            if ('x' == compo) {
                compo = 'y';
                (*pFVectors)[*fVectorNdx].x = atof(token);
            } else if ('y' == compo) {
                compo = 'z';
                (*pFVectors)[*fVectorNdx].y = atof(token);
            } else if ('z' == compo) {
                compo = 'x';
                (*pFVectors)[*fVectorNdx].cluster = -1;
                (*pFVectors)[*fVectorNdx].core  = -1;
                (*pFVectors)[(*fVectorNdx)++].z = atof(token);
                if (fVectorTotal <= *fVectorNdx) {
                    fVectorTotal += 2 * INIT_FEATURE_VECTOR_SIZE;
                    (*pFVectors) = realloc(*pFVectors, sizeof(FVector) * fVectorTotal);
                }
            }
            token = strtok(NULL, delim);
        }
    }
}

void didayDynamicClusterMethod(FVector *pFVectors, int fVectorTotal)
{
    srand(time(NULL));
    int i, j;
    FVector  centers[K];
    FVector  *pCores[K];
    int      coresNdx[K];

    for (i = 0; i < K; i++) {
        coresNdx[i] = 0;
        if(0 == i)
            pCores[i] = calloc(N0, sizeof(FVector));
        else if (1 == i)
            pCores[i] = calloc(N1, sizeof(FVector));
        else if (2 == i)
            pCores[i] = calloc(N2, sizeof(FVector));
        else if (3 == i)
            pCores[i] = calloc(N3, sizeof(FVector));
    }

    // assign the first K feature vectors as centers
    for (i = 0; i < K; i++) {
        centers[i] = pFVectors[i];
        centers[i].cluster = i;
    }

    // assign feature vectors to clusters by min. distance
    double minDist;
    double tmp;
    FVector *pCandidate;
    for (i = 0; i < fVectorTotal; i++) {
        minDist = DBL_MAX;
        for (j = 0; j < K; j++) {
            tmp = twoNorm(centers[j], pFVectors[i]);
            if (tmp < minDist) {
                pCandidate = &(centers[j]);
                minDist = tmp;
            }
        }
        pFVectors[i].cluster = pCandidate->cluster;
//        if (0 == pCandidate->cluster) {
//            cluster0[cluster0Ndx++] = pFVectors[i];
//        } else if (1 == pCandidate->cluster) {
//            cluster1[cluster1Ndx++] = pFVectors[i];
//        } else if (2 == pCandidate->cluster) {
//            cluster2[cluster2Ndx++] = pFVectors[i];
//        } else if (3 == pCandidate->cluster) {
//            cluster3[cluster3Ndx++] = pFVectors[i];
//        }
    }
    for (i = 0; i < fVectorTotal; i++) {
        printf("cluster of (%f, %f, %f): %d\n", pFVectors[i].x, pFVectors[i].y, pFVectors[i].z, pFVectors[i].cluster);
    }


    // randomly select multicenter cores for all clusters
    int randNdx;
    for (i = 0; i < K; i++) {
        randNdx = rand() % fVectorTotal;


    }
}

double twoNorm(FVector a, FVector b)
{
    return (pow(pow(a.x - b.x, 2) +
                pow(a.y - b.y, 2) +
                pow(a.z - b.z, 2), 0.5));
}

