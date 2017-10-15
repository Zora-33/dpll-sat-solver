#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int DEBUG = 1;
int clauseNumber, variableNumber;

struct Literal {
  struct Literal * next;
  int index;
};

struct Clause {
  struct Literal * head;
  struct Clause * next;
};

void printClauseSet(struct Clause * root){
  struct Clause* itr = root;
  while (itr != NULL){
    struct Literal * l = itr->head;
    while (l != NULL){
      printf("%d ", l->index);
      l = l->next;
    }
    printf("\n");
    itr = itr->next;
  }
}

int findUnitClause(struct Clause * root){
  struct Clause * itr = root;
  while (itr != NULL){
    if (itr->head == NULL) {
      if (DEBUG) printf("Empty clause\n");
    }
    if(itr->head->next == NULL){
      return itr->head->index;
    }
    itr = itr->next;
  }
  return 0;
}

int sign(int num){
  return (num > 0) - (num < 0);
}

int findPureLiteral(struct Clause * root){
  int * literalLookup = (int*) calloc(variableNumber + 1, sizeof(int));
  struct Clause * itr = root;
  while (itr != NULL){
    struct Literal * l = itr->head;
    while (l != NULL){
      if (DEBUG) printf("inspecting literal %d\n", l->index);
      int seen = literalLookup[abs(l->index)];
      if (seen == 0) literalLookup[abs(l->index)] = sign(l->index);
      else if (seen == -1 && sign(l->index) == 1) literalLookup[abs(l->index)] = 2;
      else if (seen == 1 && sign(l->index) == -1) literalLookup[abs(l->index)] = 2;
      l = l->next;
    }
    itr = itr->next;
  }
  int i;
  for (i = 1; i < variableNumber + 1; i++) {
    printf("literal lookup @%d: %d\n", i, literalLookup[i]);
    if (literalLookup[i] == -1 || literalLookup[i] == 1) return i * literalLookup[i];
  }
  return 0;
}

void removeClause(struct Clause * root, struct Clause * prev, struct Clause * target){
  if (target == root) {
    *root = *target->next;
  } else {
    prev->next = target->next;
  }
}

void removeLiteral(struct Clause * clause, struct Literal * prev, struct Literal * target){
  if (target == clause->head) {
    clause->head = target->next;
  } else {
    prev->next = target->next;
  }
}

int unitPropagation(struct Clause * root){
  int unitLiteralIndex = findUnitClause(root);
  if (DEBUG) printf("unit clause found with literal: %d\n", unitLiteralIndex);
  if (unitLiteralIndex == 0) return 0;

  struct Clause * itr = root;
  struct Clause * prev;
  while (itr != NULL){
    struct Literal * currentL = itr->head;
    struct Literal * previousL;
    while (currentL != NULL){
      if (currentL->index == unitLiteralIndex) {
        // remove this clause
        if (DEBUG) printf("Removing the clause that starts with %d\n", itr->head->index);
        if (itr == root) *root = *root->next;
        else {
          prev->next = itr->next;
        }
        itr = prev; 
        break;
      } else if (currentL->index == -unitLiteralIndex) {
        // remove this literal
        if (DEBUG) printf("Removing the literal %d from the clause that starts with %d\n", currentL->index, itr->head->index);
        if (currentL == itr->head) itr->head = currentL->next;
        else {
          previousL->next = currentL->next;
        }
        currentL = previousL;
        continue;
      }
      previousL = currentL;
      currentL = currentL->next;
    }
    prev = itr;
    itr = itr->next;
  }
}

int pureLiteralElimination(struct Clause * root){
  int pureLiteralIndex = findPureLiteral(root);
  if (DEBUG) printf("pure literal found: %d\n", pureLiteralIndex);
  if (pureLiteralIndex == 0) return 0;

  struct Clause * itr = root;
  struct Clause * prev;
  while (itr != NULL){
    struct Literal * l = itr->head;
    while (l != NULL){
      if (l->index == pureLiteralIndex) {
        // remove this clause
        if (DEBUG) printf("Removing the clause that starts with %d\n", itr->head->index);
        if (itr == root) *root = *root->next;
        else {
          prev->next = itr->next;
        }
        itr = prev; 
        break;
      }
      l = l->next;
    }
    prev = itr;
    itr = itr->next;
  }
}

struct Clause * readClauseSet(char * filename){
  FILE * fp;
  char line[256];
  size_t len = 0;

  fp = fopen(filename, "r");
  if (fp == NULL) exit(1);

  char * token;
  struct Clause * root, * currentClause, * previousClause;
  struct Literal * currentLiteral, * previousLiteral;
  
  while(fgets(line, sizeof(line), fp)){
    if (line[0] == 'c') continue;
    if (line[0] == 'p') {
      sscanf(line, "p cnf %d %d", &variableNumber, &clauseNumber);
      printf("Number of variables: %d\n", variableNumber);
      printf("Number of clauses: %d\n", clauseNumber);
    } else {
      currentClause = malloc(sizeof(struct Clause));
      if (root == NULL) {
        if (DEBUG) printf("setting root\n");
        root = currentClause;
      }
      if (previousClause != NULL) {
        if (DEBUG) printf("setting current as the next of previous clause\n");
        previousClause->next = currentClause;  
      }
      
      token = strtok(line, " ");
      while(token != NULL){
        int literalIndex = atoi(token);
        currentLiteral = malloc(sizeof(struct Literal));
        currentLiteral->index = literalIndex;
        if (literalIndex != 0){
          if (currentClause->head == NULL){
            if (DEBUG) printf("setting literal %d as head of current clause\n", currentLiteral->index);
            currentClause->head = currentLiteral;
          }
          
          if (previousLiteral != NULL){
            if (DEBUG) printf("setting literal %d as the next of previous literal\n", currentLiteral->index);
            previousLiteral->next = currentLiteral;
          }
        }

        if (DEBUG) printf("current literal is now previous literal\n");
        previousLiteral = currentLiteral; 
         
        token = strtok(NULL, " ");
      }
      if (DEBUG) printf("current clause is now previous clause\n");
      previousClause = currentClause;
    }
  }
  fclose(fp);
  
  return root;
}

int main(int argc, char *argv[]){
  if (argc < 2) {
    printf("Filename should be provided\n");
    return 1;
  }

  struct Clause * root = readClauseSet(argv[1]);

  int result = unitPropagation(root);
  printClauseSet(root);
  return 0;
}