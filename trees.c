/*
 *      Matias Moseley     CS 450     10/16/2022     Griefer List
 *
 *      scapegoat
 *      rbt
 *
 *      Please use lower case for command line arguments 
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<math.h>

#define NAME_LEN 17
#define NUM_GRIEFERS 102000
#define TRUE 1
#define FALSE 0
#define LOG_ALPHA_MULT 5.678873587 // Alpha is 2/3, so 1/log10(3/2) is equal to this number
#define RED 1
#define BLACK 0

/* Scapegoat Tree functions */

struct ScapeNode{
        char *name;
        int servBanCount, date;
        struct ScapeNode *p, *l, *r;
};

void inOrder(struct ScapeNode *side, struct ScapeNode **stack, struct ScapeNode **list){
        int x = 0, pos = 0;
        struct ScapeNode *temp = side;
        while(temp != NULL || x){ // While node is present OR nodes in stack
                if(temp != NULL){
                        stack[x++] = temp;
                        temp = temp->l;
                } else {
                        temp = stack[--x];
                        list[pos++] = temp; // Pushing onto in order array
                        temp = temp->r;
                }
        }
}


int scapeSize(struct ScapeNode *side, struct ScapeNode **stack){
        if(side == NULL) return 0;

        int x = 0, num = 0;
        struct ScapeNode *temp = side;
        while(temp != NULL || x){ // While nodes is present OR nodes in stack
                if(temp != NULL){
                        stack[x++] = temp;
                        temp = temp->l;
                        num++; // Node counter
                } else {
                        temp = stack[--x];
                        temp = temp->r;
                }
        }
        return num;
}

int partSize(struct ScapeNode *child, struct ScapeNode *parent, struct ScapeNode **stack, int oldSize){
        if(parent == NULL) return oldSize;
        if(parent->l == child) return scapeSize(parent->r, stack) + oldSize + 1; // Know left, find right
        return scapeSize(parent->l, stack) + oldSize + 1; // Know right, find left
}

struct ScapeNode *balance(struct ScapeNode **order, int low, int high){
        if(!high) return NULL;
        int mid = high >> 1;
        int lowMedian = low+mid+1;
        int highMedian = high-mid-1;

        if((order[lowMedian-1]->l = balance(order, low, mid)) != NULL) order[lowMedian-1]->l->p = order[lowMedian-1];
        if((order[lowMedian-1]->r = balance(order, lowMedian, highMedian)) != NULL) order[lowMedian-1]->r->p = order[lowMedian-1];
        return order[lowMedian-1];
}

/* Red-Black Tree functions */

struct RBNode{
        int color; // Red: 1     Black: 0
        char *name;
        int servBanCount, date;
        struct RBNode *p, *l, *r;
};


/* Main */
void main(int argc, char *argv[]){
        clock_t start = clock();

        FILE *ptr = fopen(argv[2], "r");
        char name[NAME_LEN];
        int unixTime;
        int numNodes = 0;

        switch(argv[1][0]){
                case 's':{
                        struct ScapeNode **tree = malloc(NUM_GRIEFERS * sizeof(struct ScapeNode*)); // Scapegoat tree array
                        struct ScapeNode *root = NULL;

                        while(fscanf(ptr, "%s %*d %d", name, &unixTime) != EOF){ // Read from file
                                struct ScapeNode *newNode = (struct ScapeNode*)(malloc(sizeof(struct ScapeNode))); // Build node

                                int depth = 0;
                                if(root == NULL){ // Insert nodes to make the tree
                                        newNode->name = strdup(name); newNode->servBanCount = 1; newNode->date = unixTime; newNode->p = NULL; newNode->l = NULL; newNode->r = NULL;
                                        root = newNode;
                                        tree[numNodes++] = newNode;
                                } else {
                                        struct ScapeNode *cur = root;
                                        for(;;){ // Binary Search
                                                int dir = strcmp(name, cur->name);

                                                if(dir > 0){
                                                        if(cur->r == NULL){
                                                                newNode->name = strdup(name); newNode->servBanCount = 1; newNode->date = unixTime; newNode->p = cur; newNode->l = NULL; newNode->r = NULL;
                                                                cur->r = newNode; tree[numNodes++] = newNode; break;
                                                        } else cur = cur->r;
                                                }
                                                else if(dir < 0){
                                                        if(cur->l == NULL){
                                                                newNode->name = strdup(name); newNode->servBanCount = 1; newNode->date = unixTime; newNode->p = cur; newNode->l = NULL; newNode->r = NULL;
                                                                cur->l = newNode; tree[numNodes++] = newNode; break;
                                                        } else cur = cur->l;
                                                }
                                                else{
                                                        if(cur->date < unixTime) cur->date = unixTime;
                                                        depth = -2;
                                                        cur->servBanCount++; free(newNode); break;
                                                }
                                                depth++;
                                        }
                                }
                                if(depth > (log10(numNodes) * LOG_ALPHA_MULT) + 1 ){ // If depth is greater than log base 3/2 (numNodes) + 1 then find a scapegoat
                                        struct ScapeNode *parent = newNode;
                                        struct ScapeNode **stack = malloc(numNodes * sizeof(struct ScapeNode*));
                                        int i = 1;

                                        while(i < 0.666*(i = partSize(parent, parent->p, stack, i))){ // While one side of node is less than alpha (2/3)
                                                parent = parent->p;
                                        }

                                        parent = parent->p;

                                        struct ScapeNode *gp = parent->p;
                                        struct ScapeNode **order = malloc(i * sizeof(struct ScapeNode*));
                                        inOrder(parent, stack, order);

                                        if(gp == NULL){ // Root is unbalanced
                                                root = balance(order, 0, i);
                                                root->p = NULL;
                                        }
                                        else if(gp->l == parent){ // Left unbalanced
                                                gp->l = balance(order, 0, i);
                                                gp->l->p = gp;
                                        }
                                        else{ // Right unbalanced
                                                gp->r = balance(order, 0, i);
                                                gp->r->p = gp;
                                        }
                                        free(order); free(stack);
                                }
                        }
                        fclose(ptr);

                        struct ScapeNode *temp;
                        while(fscanf(stdin, "%s", name) != EOF){ // Read from STDIN
                                temp = root;
                                for(;;){ // Binary Search
                                        int dir = strcmp(name, temp->name);

                                        if(dir > 0){
                                                if(temp->r != NULL) temp = temp->r;
                                                else{
                                                        fprintf(stdout, "%s is not currently banned from any servers.\n", name);
                                                        break;
                                                }
                                        }
                                        else if(dir < 0){
                                                if(temp->l != NULL) temp = temp->l;
                                                else{
                                                        fprintf(stdout, "%s is not currently banned from any servers.\n", name);
                                                        break;
                                                }
                                        }
                                        else{
                                                fprintf(stdout, "%s was banned from %d servers. most recently on: %d\n", name, temp->servBanCount, temp->date);
                                                break;
                                        }
                                }
                        }

                        for(int i = 0; i < numNodes; i++){ // Free memory
                                free(tree[i]->name);
                                free(tree[i]);
                        }
                        free(tree);

                        break;}

                case 'b':
                        fprintf(stdout, "B-tree\n"); break;
                case 'a':
                        fprintf(stdout, "AVL\n"); break;
                case 'r':{
                        struct RBNode **tree = malloc(NUM_GRIEFERS * sizeof(struct RBNode*)); // Red-Black tree array
                        struct RBNode *root = NULL;

                        while(fscanf(ptr, "%s %*d %d", name, &unixTime) != EOF){ // Read from file
                                struct RBNode *newNode = (struct RBNode*)(malloc(sizeof(struct RBNode))); // Build node | Nodes start as RED unless root
                                int checkProperties = TRUE;

                                if(root == NULL){
                                        newNode->name = strdup(name); newNode->servBanCount = 1; newNode->date = unixTime; newNode->color = BLACK; newNode->p = NULL; newNode->l = NULL; newNode->r = NULL;
                                        root = newNode;
                                        tree[numNodes++] = newNode;
                                        checkProperties = FALSE;
                                } else {
                                        struct RBNode *cur = root;
                                        for(;;){ // Binary Search
                                                int dir = strcmp(name, cur->name);

                                                if(dir > 0){
                                                        if(cur->r == NULL){
                                                                newNode->name = strdup(name); newNode->servBanCount = 1; newNode->date = unixTime; newNode->color = RED; newNode->p = cur; newNode->l = NULL; newNode->r = NULL;
                                                                cur->r = newNode; tree[numNodes++] = newNode; break;
                                                        } else cur = cur->r;
                                                }
                                                else if(dir < 0){
                                                        if(cur->l == NULL){
                                                                newNode->name = strdup(name); newNode->servBanCount = 1; newNode->date = unixTime; newNode->color = RED; newNode->p = cur; newNode->l = NULL; newNode->r = NULL;
                                                                cur->l = newNode; tree[numNodes++] = newNode; break;
                                                        } else cur = cur->l;
                                                }
                                                else{
                                                        if(cur->date < unixTime) cur->date = unixTime;
                                                        cur->servBanCount++; free(newNode); checkProperties = FALSE; break;
                                                }
                                        }
                                }
                                if(checkProperties){
                                        struct RBNode *parent = newNode->p;
                                        struct RBNode *gp;
                                        int c;

                                        while(newNode != root && newNode->color == RED && parent->color == RED){ // While not at root and RED Violation | If there isn't a RED violation there can't be a BLACK violation as all nodes will start as RED
                                                gp = parent->p;

                                                if(parent == gp->r){ // Right side
                                                        struct RBNode *uncle = gp->l; // Uncle

                                                        if(uncle == NULL || uncle->color == BLACK){ // Uncle BLACK
                                                                if(newNode == parent->l){ // Left side | So Right-Left Case | If FALSE this is a Right-Right Case

                                                                        if((parent->l = newNode->r) != NULL) newNode->r->p = parent; // Rotate right about the parent
                                                                        if((newNode->p = gp) == NULL) root = newNode;
                                                                        else if(parent == gp->l) gp->l = newNode;
                                                                        else gp->r = newNode;
                                                                        newNode->r = parent;
                                                                        parent->p = newNode; // End rotate

                                                                        newNode = parent; // Traverse up the tree
                                                                        parent = newNode->p;
                                                                }
                                                                if((gp->r = parent->l) != NULL) parent->l->p = gp; // Rotate left about the GrandParent
                                                                if((parent->p = gp->p) == NULL) root = parent;
                                                                else if(gp == parent->p->l) parent->p->l = parent;
                                                                else parent->p->r = parent;
                                                                parent->l = gp;
                                                                gp->p = parent; // End rotate

                                                                c = gp->color; // Color swap
                                                                gp->color = parent->color;
                                                                parent->color = c;

                                                                newNode = parent;
                                                                root->color = BLACK; // May stop RED violation because we may have changed ROOT's color

                                                        } else { // Uncle RED
                                                                gp->color = RED; // Color swap to avoid BLACK violations
                                                                parent->color = BLACK; uncle->color = BLACK;

                                                                newNode = gp;
                                                                root->color = BLACK; // May stop RED violation because we may have changed ROOT's color
                                                        }
                                                } else { // Left side
                                                        struct RBNode *uncle = gp->r; // Uncle

                                                        if(uncle == NULL || uncle->color == BLACK){ // Uncle BLACK
                                                                if(newNode == parent->r){ // Right side | So Left-Right Case | If FALSE this is a Left-Left Case

                                                                        if((parent->r = newNode->l) != NULL) newNode->l->p = parent; // Rotate left about the parent
                                                                        if((newNode->p = gp) == NULL) root = newNode;
                                                                        else if(parent == gp->l) gp->l = newNode;
                                                                        else gp->r = newNode;
                                                                        newNode->l = parent;
                                                                        parent->p = newNode; // End rotate

                                                                        newNode = parent; // Traverse up the tree
                                                                        parent = newNode->p;
                                                                }

                                                               if((gp->l = parent->r) != NULL) parent->r->p = gp; // Rotate right about the GrandParent
                                                                if((parent->p = gp->p) == NULL) root = parent;
                                                                else if(gp == parent->p->l) parent->p->l = parent;
                                                                else parent->p->r = parent;
                                                                parent->r = gp;
                                                                gp->p = parent; // End rotate

                                                                c = gp->color; // Color swap
                                                                gp->color = parent->color;
                                                                parent->color = c;

                                                                newNode = parent;
                                                                root->color = BLACK; // May stop RED violation because we may have changed ROOT's color

                                                        } else { // Uncle RED
                                                                gp->color = RED; // Color swap to avoid BLACK violations
                                                                parent->color = BLACK; uncle->color = BLACK;

                                                                newNode = gp;
                                                                root->color = BLACK; // May stop RED violation because we may have changed ROOT's color
                                                        }
                                                }
                                        }
                                }
                        }
                        fclose(ptr);

                        struct RBNode *temp;
                        while(fscanf(stdin, "%s", name) != EOF){ // Read from STDIN
                                temp = root;
                                for(;;){ // Binary Search
                                        int dir = strcmp(name, temp->name);

                                        if(dir > 0){
                                                if(temp->r != NULL) temp = temp->r;
                                                else{
                                                        fprintf(stdout, "%s is not currently banned from any servers.\n", name);
                                                        break;
                                                }
                                        }
                                        else if(dir < 0){
                                                if(temp->l != NULL) temp = temp->l;
                                                else{
                                                        fprintf(stdout, "%s is not currently banned from any servers.\n", name);
                                                        break;
                                                }
                                        }
                                        else{
                                                fprintf(stdout, "%s was banned from %d servers. most recently on: %d\n", name, temp->servBanCount, temp->date);
                                                break;
                                        }
                                }
                        }

                        for(int i = 0; i < numNodes; i++){ // Free memory
                                free(tree[i]->name);
                                free(tree[i]);
                        }
                        free(tree);

                        break;}
                default:
                        break;
        }

        double time_taken_in_seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
        double time_taken_in_microseconds = time_taken_in_seconds * 1000000.0;
        fprintf(stdout, "total time in microseconds: %f\n", time_taken_in_microseconds);
}