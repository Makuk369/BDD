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

void BDD_print(Node* root, char* varOrder, int depth);

int main(){
    char* boolfunc = "A*B+!A*B";
    char* varOrder = "ABC";

    bdd = BDD_create(boolfunc, varOrder);
    
    // BDD_print(bdd->root, varOrder, 0);

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
        newBDD->root = BDD_createNode(newBDD->root, boolFunc, varOrder, 0);
    }
    else{
        #if PRINT_ERROR == 1
        printf("BDD_create malloc error!\n");
        #endif
    }
    return newBDD;
}

Node* BDD_createNode(Node* root, char* boolFunc, char* varOrder, unsigned int index){
    Node* newNode = malloc(sizeof(Node));
    if(newNode != NULL){
        newNode->index = index;
        newNode->falseCh = BDD_createNode(newNode->falseCh, boolFunc, varOrder, index+1);
        newNode->trueCh = BDD_createNode(newNode->trueCh, boolFunc, varOrder, index+1);
    }
    else{
        #if PRINT_ERROR == 1
        printf("BDD_createNode malloc error!\n");
        #endif
    }
    return root;
}

void indent(int tabCount){
    for (size_t i = 0; i < tabCount; i++)
    {
        printf("|  ");
    }
}
void BDD_print(Node* root, char* varOrder, int depth){
    if(root == NULL){
        // indent(depth);
        // printf("NULL\n");
        return;
    }

    indent(depth);
    printf("node [%p] = %c\n", root, varOrder[depth]);

    indent(depth);
    printf("falseCh\n");
    BDD_print(root->falseCh, varOrder, depth+1);

    indent(depth);
    printf("trueCh\n");
    BDD_print(root->trueCh, varOrder, depth+1);
}