/* Generic linked list routine.
   sky 2018/10/10
 */
#include <stdlib.h>
#include <assert.h>
#include "linklist.h"

/* Allocate new list. */
struct list *
list_new (void)
{
  return malloc (sizeof (struct list));
}

/* Free list. */
void
list_free (struct list *l)
{
  free(l);
  l = NULL;
}

/* Allocate new listnode.  Internal use only. */
static struct listnode *
listnode_new (void)
{
  return malloc (sizeof (struct listnode));
}

/* Free listnode. */
static void
listnode_free (struct listnode *node)
{
  free (node);
  node = NULL;
}

/* Add new data to the list. */
void
listnode_add (struct list *list, void *val)
{
  struct listnode *node;
  
  assert (val != NULL);
  
  node = listnode_new ();
  memset(node,0,sizeof(struct listnode));

  node->prev = list->tail;
  node->data = val;

  if (list->head == NULL)
    list->head = node;
  else
    list->tail->next = node;
  list->tail = node;

  list->count++;
}

/*
 * Add a node to the list.  If the list was sorted according to the
 * cmp function, insert a new node with the given val such that the
 * list remains sorted.  The new node is always inserted; there is no
 * notion of omitting duplicates.
 */
void
listnode_add_sort (struct list *list, void *val)
{
  struct listnode *n;
  struct listnode *new;
  
  assert (val != NULL);
  
  new = listnode_new ();
  memset(new,0,sizeof(struct listnode));
  
  new->data = val;

  if (list->cmp)
    {
      for (n = list->head; n; n = n->next)
	{
	  if ((*list->cmp) (val, n->data) < 0)
	    {	    
	      new->next = n;
	      new->prev = n->prev;

	      if (n->prev)
		n->prev->next = new;
	      else
		list->head = new;
	      n->prev = new;
	      list->count++;
	      return;
	    }
	}
    }

  new->prev = list->tail;

  if (list->tail)
    list->tail->next = new;
  else
    list->head = new;

  list->tail = new;
  list->count++;
}

/* Move given listnode to tail of the list */
void
listnode_move_to_tail (struct list *l, struct listnode *n)
{
  LISTNODE_DETACH(l,n);
  LISTNODE_ATTACH(l,n);
}

/* Delete specific date pointer from the list. */
void
listnode_delete (struct list *list, void *val)
{
  struct listnode *node;

  assert(list);
  for (node = list->head; node; node = node->next)
    {
      if (node->data == val)
	{
	  if (node->prev)
	    node->prev->next = node->next;
	  else
	    list->head = node->next;

	  if (node->next)
	    node->next->prev = node->prev;
	  else
	    list->tail = node->prev;

	  list->count--;
	  listnode_free (node);
	  return;
	}
    }
}

/* Return first node's data if it is there.  */
void *
listnode_head (struct list *list)
{
  struct listnode *node;

  assert(list);
  node = list->head;

  if (node)
    return node->data;
  return NULL;
}

/* Delete all listnode from the list. */
void
list_delete_all_node (struct list *list)
{
  struct listnode *node;
  struct listnode *next;

  assert(list);
  for (node = list->head; node; node = next)
    {
      next = node->next;
      if (list->del)
	(*list->del) (node->data);
      listnode_free (node);
    }
  list->head = list->tail = NULL;
  list->count = 0;
}

/* Delete all listnode then free list itself. */
void
list_delete (struct list *list)
{
  assert(list);
  list_delete_all_node (list);
  list_free (list);
}

/* Lookup the node which has given data. */
struct listnode *
listnode_lookup (struct list *list, void *data)
{
  struct listnode *node;

  assert(list);
  for (node = listhead(list); node; node = listnextnode (node))
    if (data == listgetdata (node))
      return node;
  return NULL;
}


