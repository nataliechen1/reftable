// Copyright 2019 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "pq.h"

#include <assert.h>
#include <stdlib.h>

int pq_less(struct pq_entry a, struct pq_entry b) {
  struct slice ak = {};
  struct slice bk = {};

  record_key(a.rec, &ak);
  record_key(b.rec, &bk);

  int cmp = slice_compare(ak, bk);

  free(slice_yield(&ak));
  free(slice_yield(&bk));

  if (cmp == 0) {
    return a.index > b.index;
  }

  return cmp < 0;
}

struct pq_entry merged_iter_pqueue_top(struct merged_iter_pqueue pq) {
  return pq.heap[0];
}

bool merged_iter_pqueue_is_empty(struct merged_iter_pqueue pq) {
  return pq.len == 0;
}

void merged_iter_pqueue_check(struct merged_iter_pqueue pq) {
  for (int i = 1; i < pq.len; i++) {
    int parent = (i - 1) / 2;

    assert(pq_less(pq.heap[parent], pq.heap[i]));
  }
}

struct pq_entry merged_iter_pqueue_remove(struct merged_iter_pqueue *pq) {
  struct pq_entry e = pq->heap[0];
  pq->heap[0] = pq->heap[pq->len - 1];
  pq->len--;

  int i = 0;
  while (i < pq->len) {
    int min = i;
    int j = 2 * i + 1;
    int k = 2 * i + 2;
    if (j < pq->len && pq_less(pq->heap[j], pq->heap[i])) {
      min = j;
    }
    if (k < pq->len && pq_less(pq->heap[k], pq->heap[min])) {
      min = k;
    }

    if (min == i) {
      break;
    }

    struct pq_entry tmp = pq->heap[min];
    pq->heap[min] = pq->heap[i];
    pq->heap[i] = tmp;

    i = min;
  }

  return e;
}

void merged_iter_pqueue_add(struct merged_iter_pqueue *pq, struct pq_entry e) {
  if (pq->len == pq->cap) {
    pq->cap = 2 * pq->cap + 1;
    pq->heap = realloc(pq->heap, pq->cap * sizeof(struct pq_entry));
  }

  pq->heap[pq->len++] = e;
  int i = pq->len - 1;
  while (i > 0) {
    int j = (i - 1) / 2;
    if (pq_less(pq->heap[j], pq->heap[i])) {
      break;
    }

    struct pq_entry tmp = pq->heap[j];
    pq->heap[j] = pq->heap[i];
    pq->heap[i] = tmp;
    i = j;
  }
}

void merged_iter_pqueue_clear(struct merged_iter_pqueue *pq) {
  for (int i = 0; i < pq->len; i++) {
    record_clear(pq->heap[i].rec);
  }
  free(pq->heap);
  pq->heap = NULL;
  pq->len = pq->cap = 0;
}
