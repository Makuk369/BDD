#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct node{
    unsigned int index; // hlbka-1
    char var; // deciding var (napr.: A, 1 - in leaf)
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

Node* BDD_createNode(Node* root, char* boolFunc, char* varOrder);

void BDD_print(Node* root, char* varOrder, int depth);

int main(){
    char* boolfunc = "A*B+!A*B";
    char* varOrder = "ABC";

    bdd = BDD_create(boolfunc, varOrder);
    

    return 0;
}

void BDD_initLeafs(char* varOrder){
    trueLeaf = malloc(sizeof(Node));
    if(trueLeaf != NULL){
        trueLeaf->falseCh = NULL;
        trueLeaf->trueCh = NULL;
        trueLeaf->index = strlen(varOrder)-1;
        trueLeaf->var = '1';
    }

    falseLeaf = malloc(sizeof(Node));
    if(falseLeaf != NULL){
        falseLeaf->falseCh = NULL;
        falseLeaf->trueCh = NULL;
        falseLeaf->index = strlen(varOrder)-1;
        falseLeaf->var = '0';
    }
}
BDD* BDD_create(char* boolFunc, char* varOrder){
    BDD_initLeafs(varOrder);
    BDD* newBDD = malloc(sizeof(BDD));
    if(newBDD != NULL){
        newBDD->numOfVars = strlen(varOrder);
        newBDD->size = 0;
        newBDD->root = BDD_createNode(newBDD->root, boolFunc, varOrder);
    }
    return newBDD;
}

Node* BDD_createNode(Node* root, char* boolFunc, char* varOrder){

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