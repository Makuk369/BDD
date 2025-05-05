#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> // only used for testing

#define PRINT_ERROR 1

typedef struct bddNode{
    union{
        unsigned int index; // depth-1
        bool val; // used only in leaf (1 or 0)
    };
    bool isSkipped; // true when is useless and has been already skipped once
    struct bddNode* falseCh;
    struct bddNode* trueCh;
} BddNode;

typedef struct BDD{
    unsigned int varShifts;
    unsigned int size; // number of nodes
    BddNode* root;
} BDD;

typedef struct strNode{
    char* str;
    BddNode* owner; // pointer back to BddNode (used for reduction)
    struct strNode* next;
} StrNode;

BddNode* gTrueLeaf = NULL;
BddNode* gFalseLeaf = NULL;

BDD* BDD_create(char* boolFunc, char* varOrder);
BDD* BDD_create_with_best_order(char* boolFunc);
int BDD_use(BDD *bdd, char* input);

BddNode* BDD_createNode(BDD* bdd, BddNode* root, char* boolFunc, char* varOrder, StrNode** existingNodes, unsigned int index);
void boolFuncStrip(char* dest, char* boolFunc, char* rmVar);

// returns NULL if StrNode is added, pointer to the original if duplicate is found
BddNode* StrNode_add(StrNode** existingNodes, char* strToAdd, int index, BddNode** root);
void StrNode_print(StrNode** existingNodes, int size);

// for reading input of unknown length
char* readInput();

// shifts string to left ("ABC" -> "BCA")
void shiftToLeft(char* str);

void BDD_print(BddNode* root, char* varOrder, int depth);

int main(){
    // "A*!B+!A*B" -> 01 10 (normal, size 3)
    // "A*B+!A*B" -> 01 01 (one child is double, size 2)
    // "A*B+A*!B" -> 0 1 (left child is NULL, size 1)
    // "!A*B+!A*!B" -> 1 0 (right child is NULL, size 1)
    // "A*!B*!C+A*B*C+!A*B*!C+!A*!B*C" -> 01 10 10 01 (3 var, size 5)
    // "A*B*!C*D+A*!B*C*!D+!A*B*!C*D+!A*B*!C*!D" -> 0 1 0 0 10 01 0 (4 var, size 8)
    // "A*B*C*D+A*B*C*!D+A*B*!C*D+A*B*!C*!D+A*!B*C*D+!A*B*C*D+!A*!B*C*D" -> 0010011 (ABCD = size 4, ACBD = size 6)
    // "A*!B*C*!D*E+!A*B*!C*D*E+A*B*!C*D*!E+A*B*!C*D*E" -> 000100010010 (5 var, size 10)
    // "!A*B*!C*D*!E*F+A*!B*C*!D*E*!F+A*B*!C*D*!E*F+A*B*C*!D*E*!F" -> 0001000010000100100 (6 var, size 12)

    int testing = 1; // 1 for testing
    int printAns = 1; // 1 if checking correctness
    scanf("%d", &testing);
    scanf("%d", &printAns);
    fgetc(stdin); // to remove any trailing if present

    if (testing){
        struct timespec start, end;
        double cpuTimeUsed;

        FILE *outInfoFile;
        outInfoFile = fopen("Testing/outputInfo.txt", "w");
        if (outInfoFile == NULL) {
            printf("Error opening file!\n");
        }

        char* boolfunc = readInput();
        char* input = readInput();
    
        BDD* bdd = NULL;
        clock_gettime(CLOCK_MONOTONIC, &start);  // Start time
        bdd = BDD_create_with_best_order(boolfunc);
        clock_gettime(CLOCK_MONOTONIC, &end);    // End time
        cpuTimeUsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1E9;
        fprintf(outInfoFile, "time taken: %f\n", cpuTimeUsed);

        unsigned int maxSize = (1<<(strlen(input) + 2)) - 1; // 2^(varCount + 1) - 1
        unsigned int numOfReducedNodes = maxSize - (bdd->size + 2);
        double reductionPercentage = ((double)numOfReducedNodes / maxSize) * 100;
        fprintf(outInfoFile, "reduction percentage: %.2lf%%\n", reductionPercentage);
    
        if (printAns){
            int ans = 0;
            size_t repeats = (1<<strlen(input)) - 1;
            // printf("repeats = %llu, strlen = %llu\n", repeats, strlen(input));
            for (size_t i = 0; i < repeats; i++){
                for (unsigned int shifts = 0; shifts < bdd->varShifts; shifts++){
                    shiftToLeft(input);
                }
                ans = BDD_use(bdd, input);
                printf("%d\n", ans);
                free(input);
                input = readInput();
            }
            ans = BDD_use(bdd, input);
            printf("%d\n", ans);
        }
        free(boolfunc);
        free(input);
    }
    else{
        char* boolfunc = "!A*!B*!C+!A*!B*C+!A*B*C+A*B*C";
        char* varOrder = "BCA";
    
        BDD* bdd = NULL;
        bdd = BDD_create_with_best_order(boolfunc);
        
        bdd = BDD_create(boolfunc, varOrder);
        printf("----- BDD_print -----\n");
        BDD_print(bdd->root, varOrder, 0);
        printf("bdd size = %u\n", bdd->size);
    }

    return 0;
}

void BDD_initLeafs(){
    gTrueLeaf = malloc(sizeof(BddNode));
    if(gTrueLeaf != NULL){
        gTrueLeaf->val = true;
        gTrueLeaf->isSkipped = false;
        gTrueLeaf->falseCh = NULL;
        gTrueLeaf->trueCh = NULL;
    }
    else{
        #if PRINT_ERROR == 1
        printf("BDD_initLeafs malloc error!\n");
        #endif
    }

    gFalseLeaf = malloc(sizeof(BddNode));
    if(gFalseLeaf != NULL){
        gFalseLeaf->val = false;
        gFalseLeaf->isSkipped = false;
        gFalseLeaf->falseCh = NULL;
        gFalseLeaf->trueCh = NULL;
    }
    else{
        #if PRINT_ERROR == 1
        printf("BDD_initLeafs malloc error!\n");
        #endif
    }
}
BDD* BDD_create(char* boolFunc, char* varOrder){
    if((gFalseLeaf == NULL) || (gTrueLeaf == NULL)){
        BDD_initLeafs();
    }
    BDD* newBDD = malloc(sizeof(BDD));
    if(newBDD != NULL){
        // init BDD
        newBDD->varShifts = 0;
        newBDD->size = 0;
        newBDD->root = NULL;

        // init existing nodes (array of linked lists)
        int ensize = strlen(varOrder);
        StrNode* existingNodes[ensize];
        for (int i = 0; i < ensize; i++)
        {
            existingNodes[i] = NULL;
        }

        newBDD->root = BDD_createNode(newBDD, newBDD->root, boolFunc, varOrder, existingNodes, 0);

        // printf("----- StrNode_print -----\n");
        // printf("varOrder = %s\n", varOrder);
        // StrNode_print(existingNodes, ensize);
    }
    else{
        #if PRINT_ERROR == 1
        printf("BDD_create malloc error!\n");
        #endif
    }
    return newBDD;
}

void shiftToLeft(char* str){
    int len = strlen(str);
    if (len <= 1) return;

    char first = str[0];
    for (int i = 0; i < len - 1; i++) {
        str[i] = str[i + 1];
    }
    str[len - 1] = first;
}

int isUnique(char c, const char* str) {
    while (*str) {
        if (*str == c) return 0; // not unique
        str++;
    }
    return 1; // unique
}
// gets each variable present in boolFunc
void extractVariables(const char* boolFunc, char* varsOutput){
    int outIndex = 0;
    for (int i = 0; boolFunc[i] != '\0'; i++) {
        if((boolFunc[i] != '!') && (boolFunc[i] != '+') && (boolFunc[i] != '*')){
            if (isUnique(boolFunc[i], varsOutput)) {
                varsOutput[outIndex++] = boolFunc[i];
            }
        }
    }
    varsOutput[outIndex] = '\0';
}
BDD* BDD_create_with_best_order(char* boolFunc){
    char varOrder[27] = "\0"; // Max 26 uppercase letters + null terminator
    extractVariables(boolFunc, varOrder);

    unsigned int shifts = 0;

    unsigned int bestSize = 0;
    BDD* newBdd = NULL;
    BDD* bestBdd = NULL;

    for (size_t i = 0; i < strlen(varOrder); i++){
        newBdd = BDD_create(boolFunc, varOrder);
        newBdd->varShifts = shifts;
        // printf("----- BDD_print -----\n");
        // printf("varOrder = %s\n", varOrder);
        // BDD_print(newBdd->root, varOrder, 0);
        // printf("bdd size = %u\n", newBdd->size);

        shiftToLeft(varOrder);
        shifts++;
        
        if((bestSize > newBdd->size) || (bestSize == 0)){ // new best found or is not set yet
            // printf("New best size %d -> %d\n", bestSize, newBdd->size);
            bestSize = newBdd->size;
            bestBdd = newBdd;
        }
        else{
            // printf("Worse size %d < %d\n", bestSize, newBdd->size);
            free(newBdd);
            newBdd = NULL;
        }
    }
    
    // printf("----- BDD_print -----\n");
    // printf("varOrder = %s\n", bestVarOrder);
    // BDD_print(bestBdd->root, bestVarOrder, 0);
    // printf("bdd size = %u\n", bestSize);

    return bestBdd;
}

int BDD_use(BDD *bdd, char* input){
    BddNode *curNode = bdd->root;

    while(1){
        // check if is leaf
        if((curNode->falseCh == NULL) && (curNode->trueCh == NULL)){
            return curNode->val;
        }

        // go to false child
        if(input[curNode->index] == '0'){
            curNode = curNode->falseCh;
        }
        else if(input[curNode->index] == '1'){ // go to true child
            curNode = curNode->trueCh;
        }
        else{
            #if PRINT_ERROR == 1
            printf("BDD_use wrong input error!\n");
            #endif
            return -1;
        }
    }
}

BddNode* BDD_initNode(unsigned int index){
    BddNode* newNode = malloc(sizeof(BddNode));
    if(newNode != NULL){
        newNode->index = index;
        newNode->isSkipped = false;
        newNode->falseCh = NULL;
        newNode->trueCh = NULL;
    }
    else{
        #if PRINT_ERROR == 1
        printf("BDD_initNode malloc error!\n");
        #endif
    }
    return newNode;
}
BddNode* BDD_createNode(BDD* bdd, BddNode* root, char* boolFunc, char* varOrder, StrNode** existingNodes, unsigned int index){
    // root is allways NULL
    root = BDD_initNode(index);
    bdd->size++;

    // update existingNodes list and get duplicate
    BddNode* duplicateNode = StrNode_add(existingNodes, boolFunc, index, &root);
    if(duplicateNode != NULL){ // if duplicate is found return it
        free(root);
        bdd->size--;
        return duplicateNode;
    }

    // boolFunc is A or !A
    if(strlen(boolFunc) <= 2){
        if(boolFunc[0] == varOrder[index]){ // boolFunc = A
            root->falseCh = gFalseLeaf;
            root->trueCh = gTrueLeaf;
        }
        else{ // boolFunc = !A
            root->falseCh = gTrueLeaf;
            root->trueCh = gFalseLeaf;
        }
    }
    else{ // boolFunc len >= 3 (not one var)
        char newBoolFunc[strlen(boolFunc)];
        char rmVar[3] = "\0";

        // first strip for false child
        rmVar[0] = '!';
        rmVar[1] = varOrder[index];
        boolFuncStrip(newBoolFunc, boolFunc, rmVar);
        if(strlen(newBoolFunc) != 0){ // newBoolFunc is "normal"
            root->falseCh = BDD_createNode(bdd, root->falseCh, newBoolFunc, varOrder, existingNodes, index+1);
        }

        // second strip for true child
        rmVar[1] = '\0';
        rmVar[0] = varOrder[index];
        boolFuncStrip(newBoolFunc, boolFunc, rmVar);
        if(strlen(newBoolFunc) != 0){ // newBoolFunc is "normal"
            root->trueCh = BDD_createNode(bdd, root->trueCh, newBoolFunc, varOrder, existingNodes, index+1);
        }

        if((root->falseCh == NULL) && (root->trueCh != NULL)){ // only falseCh left null
            root->falseCh = gFalseLeaf;
        }
        else if((root->falseCh != NULL) && (root->trueCh == NULL)){ // only trueCh left null
            root->trueCh = gFalseLeaf;
        }
        else if((root->falseCh == NULL) && (root->trueCh == NULL)){ // both are null (A+!A)
            root->falseCh = gTrueLeaf;
            root->trueCh = gTrueLeaf;
        }

        // if child is not leaf, then try to skip useless node
        if((root->falseCh != gFalseLeaf) && (root->falseCh != gTrueLeaf)){
            if(root->falseCh->trueCh == root->falseCh->falseCh){
                if(root->falseCh->isSkipped == false){
                    bdd->size--;
                    root->falseCh->isSkipped = true;
                }
                root->falseCh = root->falseCh->falseCh;
            }
        }
        if((root->trueCh != gFalseLeaf) && (root->trueCh != gTrueLeaf)){
            if(root->trueCh->trueCh == root->trueCh->falseCh){
                if(root->trueCh->isSkipped == false){
                    bdd->size--;
                    root->trueCh->isSkipped = true;
                }
                root->trueCh = root->trueCh->trueCh;
            }
        }
    }
    return root;
}

void boolFuncStrip(char* dest, char* boolFunc, char* rmVar){
    char* strCopy = strdup(boolFunc);
    char* token;
    char* tokenCopy;
    char* subToken;
    char tokenCutout[strlen(boolFunc)];
    tokenCutout[0] = '\0';
    bool firstCutout = true;

    int isRmVar = 1; // 1 == false
    bool hasRmVar = false;
    char newStr[strlen(boolFunc)];
    newStr[0] = '\0'; // set to empty string
    bool firstTimeAppend = true;
    
    // split by +
    while((token = strtok_r(strCopy, "+", &strCopy)) != NULL){
        // printf("token = %s\n", token);

        // split by *
        tokenCopy = strdup(token);
        while((subToken = strtok_r(tokenCopy, "*", &tokenCopy)) != NULL){
            // printf("subToken = %s\n", subToken);

            // find if matches desiredVar and cut it out
            isRmVar = strcmp(subToken, rmVar);
            if(isRmVar != 0){ // 0 == true
                if(firstCutout){
                    strcat(tokenCutout, subToken);
                    // printf("tokenCutout first: %s\n", tokenCutout);
                    firstCutout = false;
                }
                else{
                    strcat(tokenCutout, "*");
                    strcat(tokenCutout, subToken);
                }
            }
            else{
                isRmVar = 1;
                hasRmVar = true;
            }
        }

        // append newStr
        if(hasRmVar){
            if(firstTimeAppend){
                strcat(newStr, tokenCutout);
                firstTimeAppend = false;
            }
            else{
                strcat(newStr, "+");
                strcat(newStr, tokenCutout);
            }
            hasRmVar = false;
        }

        // reset tokenCutout
        tokenCutout[0] = '\0';
        firstCutout = true;
    }
    
    strcpy(dest, newStr);
}

BddNode* StrNode_add(StrNode** existingNodes, char* strToAdd, int index, BddNode** root){
    // printf("boolFunc in add = %s at index = %d\n", strToAdd, index);
    // for first node add
    if(existingNodes[index] == NULL){
        existingNodes[index] = malloc(sizeof(StrNode));
        if(existingNodes[index] != NULL){
            existingNodes[index]->str = strdup(strToAdd);
            existingNodes[index]->owner = *root;
            existingNodes[index]->next = NULL;
            // printf("adding first node [%p] {%s}\n", existingNodes[index]->owner, existingNodes[index]->str);
        }
        return NULL;
    }

    StrNode* curNode = existingNodes[index];
    while (1){
        if(strcmp(curNode->str, strToAdd) == 0){ // found duplicade str == do nothing
            // printf("duplicate str [%p] {%s} == {%s}\n", curNode->owner, curNode->str, strToAdd);
            return curNode->owner;
        }
        else{
            if(curNode->next == NULL){ // empty space == add
                curNode->next = malloc(sizeof(StrNode));
                if(curNode->next != NULL){
                    curNode->next->str = strdup(strToAdd);
                    curNode->next->owner = *root;
                    curNode->next->next = NULL;
                    // printf("adding node [%p] {%s}\n", curNode->next->owner, curNode->next->str);
                    return NULL;
                }
            }
            // printf("going deeper\n");
            curNode = curNode->next;
        }
    }
}
void StrNode_print(StrNode** existingNodes, int size){
    for (int i = 0; i < size; i++)
    {
        printf("[%d] = ", i);
        StrNode* curNode = existingNodes[i];
        while (curNode != NULL){
            printf("%s -> ", curNode->str);
            curNode = curNode->next;
        }
        printf("\n");
    }
}

char* readInput(){
    #define CHUNK 100
    char* input = NULL;
    char tempbuf[CHUNK];
    size_t inputlen = 0, templen = 0;
    do {
        fgets(tempbuf, CHUNK, stdin);
        tempbuf[strcspn(tempbuf, "\n")] = '\0';
        templen = strlen(tempbuf);
        input = realloc(input, inputlen+templen+1);
        strcpy(input+inputlen, tempbuf);
        inputlen += templen;
    } while (templen==CHUNK-1 && tempbuf[CHUNK-2]!='\n');
    return input;
}

void indent(int tabCount){
    for (int i = 0; i < tabCount; i++)
    {
        printf("|  ");
    }
}
void BDD_print(BddNode* root, char* varOrder, int depth){
    if(root == NULL){
        // indent(depth);
        printf("NULL\n");
        return;
    }

    // je leaf
    if(root->falseCh == NULL){
        indent(depth);
        printf("leaf = %d\n", root->val);
        return;
    }
    else if(root->trueCh == NULL){
        indent(depth);
        printf("leaf = %d\n", root->val);
        return;
    }

    indent(depth);
    printf("node [%p] %c index %d\n", root, varOrder[root->index], root->index);

    indent(depth);
    printf("falseCh\n");
    BDD_print(root->falseCh, varOrder, depth+1);

    indent(depth);
    printf("trueCh\n");
    BDD_print(root->trueCh, varOrder, depth+1);
}