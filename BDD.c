#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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
    unsigned int numOfVars;
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
// BDD *BDD_create_with_best_order(string boolFunc);
// char BDD_use(BDD *bdd, string vstupy);

BddNode* BDD_createNode(BDD* bdd, BddNode* root, char* boolFunc, char* varOrder, StrNode** existingNodes, unsigned int index);
void boolFuncStrip(char* dest, char* boolFunc, char* rmVar);

// returns NULL if StrNode is added, pointer to the original if duplicate is found
BddNode* StrNode_add(StrNode** existingNodes, char* strToAdd, int index, BddNode** root);
void StrNode_print(StrNode** existingNodes, int size);

void BDD_print(BddNode* root, char* varOrder, int depth);

int main(){
    // "A*!B+!A*B" -> 01 10 (normal, size 3)
    // "A*B+!A*B" -> 01 01 (one child is double, size 2)
    // "A*B+A*!B" -> 0 1 (left child is NULL, size 1)
    // "!A*B+!A*!B" -> 1 0 (right child is NULL, size 1)
    // "A*!B*!C+A*B*C+!A*B*!C+!A*!B*C" -> 01 10 10 01 (3 var, size 5)
    // "A*B*!C*D+A*!B*C*!D+!A*B*!C*D+!A*B*!C*!D" -> 0 1 0 0 10 01 0 (4 var, size 8)
    // "A*B*C*D+A*B*C*!D+A*B*!C*D+A*B*!C*!D+A*!B*C*D+!A*B*C*D+!A*!B*C*D" -> 0 01 0 01 1 (ABCD = size 4, ACBD = size 6)
    // "A*!B*C*!D*E+!A*B*!C*D*E+A*B*!C*D*!E+A*B*!C*D*E" -> 000100010010 (5 var, size 10)
    // "!A*B*!C*D*!E*F+A*!B*C*!D*E*!F+A*B*!C*D*!E*F+A*B*C*!D*E*!F" -> 0001000010000100100 (6 var, size 12)
    char* boolfunc = "!A*B*!C*D*!E*F+A*!B*C*!D*E*!F+A*B*!C*D*!E*F+A*B*C*!D*E*!F";
    char* varOrder = "ABCDEF";

    BDD* bdd = NULL;
    bdd = BDD_create(boolfunc, varOrder);
    
    printf("----- BDD_print -----\n");
    BDD_print(bdd->root, varOrder, 0);
    printf("bdd size = %u\n", bdd->size);

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
    BDD_initLeafs();
    BDD* newBDD = malloc(sizeof(BDD));
    if(newBDD != NULL){
        // init BDD
        newBDD->numOfVars = strlen(varOrder);
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

        printf("----- StrNode_print -----\n");
        StrNode_print(existingNodes, ensize);
    }
    else{
        #if PRINT_ERROR == 1
        printf("BDD_create malloc error!\n");
        #endif
    }
    return newBDD;
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
        printf("DuplicateNode not NULL [%p]\n", duplicateNode);
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

        // printf("childs [%p] - [%p] index: %d\n", root->falseCh, root->trueCh, index);
        // if child is not leaf, then try to skip useless node
        if((root->falseCh != gFalseLeaf) && (root->falseCh != gTrueLeaf)){
            // printf("falseCh not leaf [%p] != [%p]/[%p]\n", root->falseCh, gFalseLeaf, gTrueLeaf);
            if(root->falseCh->trueCh == root->falseCh->falseCh){
                printf("useless falseCh at index %d\n", index);
                if(root->falseCh->isSkipped == false){
                    bdd->size--;
                    root->falseCh->isSkipped = true;
                }
                root->falseCh = root->falseCh->falseCh;
            }
        }
        if((root->trueCh != gFalseLeaf) && (root->trueCh != gTrueLeaf)){
            // printf("trueCh not leaf [%p] != [%p]/[%p]\n", root->trueCh, gFalseLeaf, gTrueLeaf);
            if(root->trueCh->trueCh == root->trueCh->falseCh){
                printf("useless trueCh at index %d\n", index);
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
    printf("node [%p] %c\n", root, varOrder[depth]);

    indent(depth);
    printf("falseCh\n");
    BDD_print(root->falseCh, varOrder, depth+1);

    indent(depth);
    printf("trueCh\n");
    BDD_print(root->trueCh, varOrder, depth+1);
}