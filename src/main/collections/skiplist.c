#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "skiplist.h"

#define P 0.5
#define MAX_LEVEL 6

static SkiplistNode* make_node(int level, VALUE value) {
  SkiplistNode* node = (SkiplistNode*)malloc(sizeof(SkiplistNode));
  node->forward = (SkiplistNode**)calloc(level + 1, sizeof(SkiplistNode*));
  node->value = value;
  return node;
}

static float frand() {
  return (float) rand() / RAND_MAX;
}

static int random_level() {
  static int first = 1;
  int lvl = 0;

  if(first) {
    srand( (unsigned)time( NULL ) );
    first = 0;
  }

  while(frand() < P && lvl < MAX_LEVEL) {
    lvl++;
  }

  return lvl;
} 


int skiplist_init(Skiplist* list) {
  list->header = make_node(MAX_LEVEL, 0);
  list->level = 0;
  return 0;
}

int skiplist_contains(Skiplist* list, VALUE needle) {
  SkiplistNode* x = list->header;
  int i;
  for(i = list->level; i >= 0; i--) {
    while(x->forward[i] != NULL && x->forward[i]->value < needle) {
      x = x->forward[i];
    }
  }
  x = x->forward[0];

  if(x != NULL && x->value == needle) {
    return 1;
  }

  return 0;
}

int skiplist_insert(Skiplist* list, VALUE value) {

  SkiplistNode* x = list->header;	
  SkiplistNode* update[MAX_LEVEL + 1];
  memset(update, 0, MAX_LEVEL + 1);

  int i;
  for(i = list->level; i >= 0; i--) {
    while(x->forward[i] != NULL && x->forward[i]->value < value) {
      x = x->forward[i];
    }
    update[i] = x;
  }
  x = x->forward[0];

  if(x == NULL || x->value != value) {
    int lvl = random_level();

    if(lvl > list->level) {
      for(i = list->level + 1; i <= lvl; i++) {
        update[i] = list->header;
      }
      list->level = lvl;
    }
    x = make_node(lvl, value);
    for(i = 0; i <= lvl; i++) {
      x->forward[i] = update[i]->forward[i];
      update[i]->forward[i] = x;
    }
  }
  return 0;
}

int skiplist_remove(Skiplist* list, VALUE value) {
  SkiplistNode* x = list->header;	
  SkiplistNode* update[MAX_LEVEL + 1];
  memset(update, 0, MAX_LEVEL + 1);

  int i;
  for(i = list->level; i >= 0; i--) {
    while(x->forward[i] != NULL && x->forward[i]->value < value) {
      x = x->forward[i];
    }
    update[i] = x;
  }
  x = x->forward[0];

  if(x->value == value) {
    for(i = 0; i <= list->level; i++) {
      if(update[i]->forward[i] != x) {
        break;
      }
      update[i]->forward[i] = x->forward[i];
    }
    printf("free %d\n", x->value);
    free(x);
    while(list->level > 0 && list->header->forward[list->level] == NULL) {
      list->level--;
    }
  }
  return 0;
}

int skiplist_destroy(Skiplist* list) {
  SkiplistNode* n;
  SkiplistNode* x = list->header->forward[0];
  while(x != NULL) {
    n = x->forward[0];
    printf("free %d\n", x->value);
    free(x);
    x = n;
  }
  return 0;
}

int skiplist_print(Skiplist* list) {
  SkiplistNode* n;
  SkiplistNode* x = list->header->forward[0];
  while(x != NULL) {
    printf("%d\n", x->value);
    x = x->forward[0];
  }
  printf("--\n");
  return 0;
}
