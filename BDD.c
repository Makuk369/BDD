#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct node{
    unsigned int index; // hlbka
    struct node* falseCh;
    struct node* trueCh;
} Node;

typedef struct BDD{
    unsigned int numOfVars;
    unsigned int size; // number of nodes
    Node* root;
} BDD;


// BDD *BDD_create (string bfunkcia, string poradie);
// BDD *BDD_create_with_best_order (string bfunkcia);
// char BDD_use (BDD *bdd, string vstupy);

int main(){

    printf("hello\n");

    return 0;
}