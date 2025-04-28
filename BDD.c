#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define PRINT_ERROR 1

typedef struct node{
    union{
        unsigned int index; // depth-1
        bool val; // used only in leaf (1 or 0)
    };
    struct node* falseCh;
    struct node* trueCh;
} Node;

typedef struct BDD{
    unsigned int numOfVars;
    unsigned int size; // number of nodes
    Node* root;
} BDD;

BDD* bdd = NULL;
Node* trueLeaf = NULL;
Node* falseLeaf = NULL;

BDD* BDD_create(char* boolFunc, char* varOrder);
// BDD *BDD_create_with_best_order(string boolFunc);
// char BDD_use(BDD *bdd, string vstupy);

Node* BDD_createNode(Node* root, char* boolFunc, char* varOrder, unsigned int index);
void boolFuncStrip(char* dest, char* boolFunc, char* rmVar);

void BDD_print(Node* root, char* varOrder, int depth);

int main(){
    char* boolfunc = "!A";
    char* varOrder = "ABC";

    bdd = BDD_create(boolfunc, varOrder);
    
    printf("----- BDD_print -----\n");
    BDD_print(bdd->root, varOrder, 0);


    return 0;
}

void BDD_initLeafs(){
    trueLeaf = malloc(sizeof(Node));
    if(trueLeaf != NULL){
        trueLeaf->falseCh = NULL;
        trueLeaf->trueCh = NULL;
        trueLeaf->val = true;
    }
    else{
        #if PRINT_ERROR == 1
        printf("BDD_initLeafs malloc error!\n");
        #endif
    }

    falseLeaf = malloc(sizeof(Node));
    if(falseLeaf != NULL){
        falseLeaf->falseCh = NULL;
        falseLeaf->trueCh = NULL;
        falseLeaf->val = false;
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
        newBDD->numOfVars = strlen(varOrder);
        newBDD->size = 0;
        newBDD->root = NULL;
        newBDD->root = BDD_createNode(newBDD->root, boolFunc, varOrder, 0);
    }
    else{
        #if PRINT_ERROR == 1
        printf("BDD_create malloc error!\n");
        #endif
    }
    return newBDD;
}

Node* BDD_initNode(unsigned int index){
    Node* newNode = malloc(sizeof(Node));
    if(newNode != NULL){
        newNode->index = index;
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
Node* BDD_createNode(Node* root, char* boolFunc, char* varOrder, unsigned int index){
    // root is allways NULL
    root = BDD_initNode(index);

    printf("created node [%p] %c\n", root, varOrder[index]);
    printf("falseLeaf val [%p] %d\n", falseLeaf, falseLeaf->val);
    printf("trueLeaf val [%p] %d\n", trueLeaf, trueLeaf->val);

    // boolFunc is A or !A
    if(strlen(boolFunc) <= 2){
        if(boolFunc[0] == varOrder[index]){ // boolFunc = A
            root->falseCh = falseLeaf;
            printf("added falseLeaf [%p] %d\n", root->falseCh, root->falseCh->val);
            root->trueCh = trueLeaf;
            printf("added trueLeaf [%p] %d\n", root->trueCh, root->trueCh->val);
        }
        else{ // boolFunc = !A
            root->falseCh = trueLeaf;
            root->trueCh = falseLeaf;
        }
    }
    else{ // boolFunc len >= 3 (not one var)
        root->falseCh = BDD_createNode(root->falseCh, boolFunc, varOrder, index+1);
        root->trueCh = BDD_createNode(root->trueCh, boolFunc, varOrder, index+1);
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

void indent(int tabCount){
    for (int i = 0; i < tabCount; i++)
    {
        printf("|  ");
    }
}
void BDD_print(Node* root, char* varOrder, int depth){
    if(root == NULL){
        // indent(depth);
        printf("NULL\n");
        return;
    }

    // je leaf
    if(root->falseCh == NULL){
        printf("leaf = %d\n", root->val);
        return;
    }
    else if(root->trueCh == NULL){
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