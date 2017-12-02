//
// Created by Sean on 2017/12/1.
//
#include <cstdio>
#include <cstdlib>
#include <iostream>

typedef struct _node_t {
    unsigned int key;
    struct _node_t *next;
} node_t;

int main() {
    node_t *newNode = malloc(sizeof(node_t));
    newNode->key = 1551;
    newNode->next = NULL;

    printf("&newNode is: %d\n", &newNode);

    node_t *testNode1 = newNode;
    printf("testNode1 is: %d\n", testNode1);
    printf("&testNode1 is: %d\n", &testNode1);
    cout << testNode1->key << endl;
    cout << testNode1->next << endl;

    node_t testNode2 = &newNode;
    printf("testNode1 is: %d\n", testNode1);
    printf("&testNode1 is: %d\n", &testNode1);
    cout << testNode1->key << endl;
    cout << testNode1->next << endl;

    return 0;
}

