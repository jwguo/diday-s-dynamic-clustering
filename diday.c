/* 9913525
 *
 *
 *
 * ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <time.h>

// ==========need to change to command line options============================
#define K 4
#define N0 2
#define N1 2
#define N2 4
#define N3 3
#define ITERATION 10000
#define DATA_PATH "./data.txt"
// ============================================================================

#define LINE_BUF_SIZE 128
#define TOKEN_BUF_SIZE 32
#define INIT_FEATURE_VECTOR_SIZE 16

typedef struct
{
    double x;
    double y;
    double z;
    int    cluster;
    int    core;
} FVector;

void   readData(FILE *fp, FVector **pFVectors, int *fVectorNdx);
void   didayDynamicClusterMethod(FVector *pFVectors, int fVectorTotal, int clusterCnt, int *pCoreCnt, int iteration);
double twoNorm(FVector a, FVector b);
double distCE(FVector *pFVectors, int fVectorTotal, int clusterCnt, FVector **pCores, int *coresNdx);
void   randCore(FVector *pFVectors, int fVectorTotal, int clusterCnt, int *pCoreCnt, int **pCluster, int *clusterNdx, FVector **pCores, int *coresNdx);

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
    int fVectorNdx = 0;
    readData(fp, &pFVectors, &fVectorNdx);

    int coreCnt[K];
    for (i = 0; i < K; i++) {
        if (0 == i)
            coreCnt[i] = N0;
        if (1 == i)
            coreCnt[i] = N1;
        if (2 == i)
            coreCnt[i] = N2;
        if (3 == i)
            coreCnt[i] = N3;
    }
    didayDynamicClusterMethod(pFVectors, fVectorNdx, K, coreCnt, ITERATION);

    return 0;
}

void readData(FILE *fp, FVector **pFVectors, int *fVectorNdx)
{
    char  lineBuf[LINE_BUF_SIZE];
    char  *token;
    char  *delim = ",";
    int   fVectorTotal = INIT_FEATURE_VECTOR_SIZE;
    char  compo = 'x';

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

void didayDynamicClusterMethod(FVector *pFVectors, int fVectorTotal, int clusterCnt, int *pCoreCnt, int iteration)
{
    srand(time(NULL));
    int i, j, k, m;
    FVector  initCenters[clusterCnt];
    FVector *pCores[clusterCnt];
    int      coresNdx[clusterCnt];
    int     *pCluster[clusterCnt];
    int      clusterNdx[clusterCnt];
    double   minDist;
    double   tmp;
    FVector *pCandidate;

    for (i = 0; i < clusterCnt; i++) {
        pCores[i]     = calloc(pCoreCnt[i], sizeof(FVector));
        pCluster[i]   = calloc(fVectorTotal, sizeof(int));
        coresNdx[i]   = 0;
        clusterNdx[i] = 0;
    }

    // assign the first clusterCnt feature vectors as initCenters
    for (i = 0; i < clusterCnt; i++) {
        initCenters[i] = pFVectors[i];
        initCenters[i].cluster = i;
    }

    // assign feature vectors to clusters by min. distance
    for (i = 0; i < fVectorTotal; i++) {
        minDist = DBL_MAX;
        for (j = 0; j < clusterCnt; j++) {
            tmp = twoNorm(initCenters[j], pFVectors[i]);
            if (tmp < minDist) {
                pCandidate = &(initCenters[j]);
                minDist = tmp;
            }
        }
        pFVectors[i].cluster = pCandidate->cluster;
        pCluster[pCandidate->cluster][clusterNdx[pCandidate->cluster]++] = i;
    }

    // randomly select multicenter cores for all clusters
    randCore(pFVectors, fVectorTotal, clusterCnt, pCoreCnt, pCluster, clusterNdx, pCores, coresNdx);

    // find the best cluster sets and core sets iteratively
    int bestCoreSet[fVectorTotal];
    int bestClusterSet[fVectorTotal];
    double minDistCE = DBL_MAX;
    for (i = 0; i < iteration; i++) {
        for (j = 0; j < clusterCnt; j++)
            clusterNdx[j] = 0;

        for (j = 0; j < fVectorTotal; j++) {
            // assign feature vectors to clusters via min. dist. to cores
            minDist = DBL_MAX;
            for (k = 0; k < clusterCnt; k++) {
                for (m = 0; m < coresNdx[k]; m++) {
                    tmp = twoNorm(pCores[k][m], pFVectors[j]);
                    if (tmp < minDist) {
                        pCandidate = &(pCores[k][m]);
                        minDist = tmp;
                    }
                }
            }
            pFVectors[j].cluster = pCandidate->cluster;
            pCluster[pCandidate->cluster][clusterNdx[pCandidate->cluster]++] = j;
        }
        // compute D(k) and D(C, E)
        tmp = distCE(pFVectors, fVectorTotal, clusterCnt, pCores, coresNdx);
        if (tmp < minDistCE) {
            // candidate
            minDistCE = tmp;
            for (j = 0; j < fVectorTotal; j++) {
                bestCoreSet[j] = pFVectors[j].core;
                bestClusterSet[j] = pFVectors[j].cluster;
            }
        }
        // reselect random new core sets
        randCore(pFVectors, fVectorTotal, clusterCnt, pCoreCnt, pCluster, clusterNdx, pCores, coresNdx);
    }

    // print final results to standard output
    int memberCnts[clusterCnt];
    for (i = 0; i < clusterCnt; i++)
        memberCnts[i] = 0;
    for (i = 0; i < clusterCnt; i++) {
        printf("\ncluster #%d:\n", i);
        for (j = 0; j < fVectorTotal; j++) {
            if (pFVectors[j].cluster == i) {
                memberCnts[i]++;
                printf("(%d, %d, %d)", (int)pFVectors[j].x, (int)pFVectors[j].y, (int)pFVectors[j].z);
                if(pFVectors[j].core == i)
                    printf(" <- core of cluster #%d", i);
                printf("\n");
            }
        }
    }
    printf("\n");
    for (i = 0; i < clusterCnt; i++)
        printf("cluster %d: #%d\n", i, memberCnts[i]);

    printf("\nmin( D(C,E) ): %f\n", minDistCE);
}

double twoNorm(FVector a, FVector b)
{
    return (pow(pow(a.x - b.x, 2) +
                pow(a.y - b.y, 2) +
                pow(a.z - b.z, 2), 0.5));
}

double distCE(FVector *pFVectors, int fVectorTotal, int clusterCnt, FVector **pCores, int *coresNdx)
{
    int i, j;
    int clusterNum;
    double result = 0;

    for (i = 0; i < fVectorTotal; i++) {
        // traverse feature vectors
        clusterNum = pFVectors[i].cluster;
        for (j = 0; j < coresNdx[clusterNum]; j++) {
            result += twoNorm(pCores[clusterNum][j], pFVectors[i]);
        }
    }

    return result;
}

void randCore(FVector *pFVectors, int fVectorTotal, int clusterCnt, int *pCoreCnt, int **pCluster, int *clusterNdx, FVector **pCores, int *coresNdx)
{
    int i, j;
    int randNdx;

    for (i = 0; i < clusterCnt; i++)
        coresNdx[i] = 0;

    for (i = 0; i < fVectorTotal; i++)
        pFVectors[i].core = -1;

    for (i = 0; i < clusterCnt; i++) {
        // traverse each cluster
        if (clusterNdx[i] <= pCoreCnt[i]) {
            // cluster member count is smaller than default core count
            for (j = 0; j < clusterNdx[i]; j++) {
                // each cluster member is core
                pFVectors[pCluster[i][j]].core = i;
                // !!test
                if (pFVectors[pCluster[i][j]].core != pFVectors[pCluster[i][j]].cluster)
                    printf("\n\nERROR!!ERROR!!ERROR!!\n\n");
                pCores[i][coresNdx[i]++] = pFVectors[pCluster[i][j]];
            }
        } else {
            // randomly select cores
            for (j = 0; j < pCoreCnt[i]; ) {
                randNdx = rand() % clusterNdx[i];
                if (pFVectors[pCluster[i][randNdx]].core != i) {
                    pFVectors[pCluster[i][randNdx]].core = i;
                    pCores[i][coresNdx[i]++] = pFVectors[pCluster[i][randNdx]];
                    j++;
                }
            }
        }
    }
}

