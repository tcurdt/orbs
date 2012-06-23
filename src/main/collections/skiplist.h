#ifndef _SKIPLIST_H
#define _SKIPLIST_H

#define VALUE int

typedef struct SkiplistNode {
  VALUE value;
  struct SkiplistNode** forward;
} SkiplistNode;

typedef struct {
  SkiplistNode* header;
  int level;
} Skiplist;

int skiplist_init(Skiplist* list);
int skiplist_contains(Skiplist* list, VALUE needle);
int skiplist_insert(Skiplist* list, VALUE value);
int skiplist_remove(Skiplist* list, VALUE value);
int skiplist_destroy(Skiplist* list);
int skiplist_print(Skiplist* list);

#endif