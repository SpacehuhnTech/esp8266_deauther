#include "SimpleList.h"

// Initialize LinkedList with default values
template<typename T>
SimpleList<T>::SimpleList(){
  listBegin = NULL;
  listEnd = NULL;
  listSize = 0;

  lastNodeGot = NULL;
  lastIndexGot = -1;
  isCached = false;
}

// Clear Nodes and free Memory
template<typename T>
SimpleList<T>::~SimpleList(){
  clear();
  listBegin = NULL;
  listEnd = NULL;
  listSize = 0;
  lastNodeGot = NULL;
  lastIndexGot = -1;
  isCached = false;
}

template<typename T>
Node<T>* SimpleList<T>::getNode(int index){
  if(index < 0 || index >= listSize)
    return NULL;

  Node<T>* hNode = listBegin;
  int c = 0;

  if(isCached && index >= lastIndexGot){
    c = lastIndexGot;
    hNode = lastNodeGot;
  }

  while(hNode != NULL && c < index){
    hNode = hNode->next;
    c++;
  }

  if(hNode){
    isCached = true;
    lastIndexGot = c;
    lastNodeGot = hNode;
  }
  
  return hNode;
}

template<typename T>
int SimpleList<T>::size(){
  return listSize;
}

template<typename T>
void SimpleList<T>::add(T obj){
  Node<T> *node = new Node<T>();
  node->data = obj;
  node->next = NULL;
  
  if(!listBegin)
    listBegin = node;
    
  if(!listEnd){
    listEnd = node;
  } else {
    listEnd->next = node;
    listEnd = node;
  }

  isCached = true;
  lastIndexGot = listSize;
  lastNodeGot = listEnd;
    
  listSize++;
}

template<typename T>
void SimpleList<T>::add(int index, T obj){
  if(index < 0 || index >= listSize){
    add(obj);
    return;
  }
  
  // Example: a -> c
  // we want to add b between a and c
  
  Node<T>* nodeA = getNode(index);
  Node<T>* nodeC = nodeA ? nodeA->next : NULL;

  Node<T> *nodeB = new Node<T>();
  nodeB->data = obj;
  nodeB->next = nodeC;
  
  // a -> b -> c
  if(nodeA) 
    nodeA->next = nodeB;
  
  if(!nodeC)
    listEnd = nodeB;
  
  if(!listBegin)
    listBegin = nodeB;

  isCached = true;
  lastIndexGot = index;
  lastNodeGot = listEnd;
  
  listSize++;
}

template<typename T>
void SimpleList<T>::remove(int index){
  if (index < 0 || index >= listSize)
    return;

  Node<T>* nodeA = getNode(index - 1);                       // prev
  Node<T>* nodeB = nodeA ? nodeA->next : getNode(index);     // node that shall be deleted
  Node<T>* nodeC = nodeB ? nodeB->next : getNode(index + 1); // next
  
  // a -> b -> c
  if(nodeA && nodeC){
    // a -> c
    nodeA->next = nodeC;
  }
  
  // a -> b
  else if(nodeA){
    // a
    nodeA->next = NULL;
    listEnd = nodeA;
  }
  
  // b -> c
  else if(nodeC){
    // c
    listBegin = nodeC;
  }
  
  // b
  else{
    listBegin = NULL;
    listEnd = NULL;
  }

  if(listBegin == nodeB){
    listBegin = nodeB->next;
  }

  if(listEnd == nodeB){
    listEnd = getNode(listSize - 2);
  }
  
  delete(nodeB);
  
  isCached = false;
  
  listSize--;
}

template<typename T>
T SimpleList<T>::get(int index){
  Node<T> *hNode = getNode(index);
  return (hNode ? hNode->data : T());
}


template<typename T>
T SimpleList<T>::pop(){
  if(listSize <= 0)
    return T();
  
  T data = listEnd->data;
  remove(listSize-1);
  return data;
}

template<typename T>
T SimpleList<T>::shift(){
  if(listSize <= 0)
    return T();
  
  T data = listBegin->data;
  remove(0);
  return data;
}

template<typename T>
void SimpleList<T>::clear(){
  while(listSize > 0)
    remove(0);
}

template<typename T>
void SimpleList<T>::swap(int x, int y){
  // only continue when the index numbers are unequal and at least 0
  if(x != y && x >= 0 && y >= 0){
    if(y > x){ // the first index should be bigger than the second. If not, swap them!
      int h = x;
      x = y;
      y = h;
    }
    
    // Example: a -> b -> c -> ... -> g -> h -> i
    //          we want to swap b with h
    
    Node<T>* nodeA = getNode(x - 1);                           // x.prev
    Node<T>* nodeB = nodeA ? nodeA->next : getNode(x);         // x
    Node<T>* nodeC = nodeB ? nodeB->next : getNode(x + 1);     // x.next
    Node<T>* nodeG =  x == y - 1 ? nodeB : getNode(y - 1);     // y.prev
    Node<T>* nodeH = nodeG ? nodeG->next : getNode(y);         // y
    Node<T>* nodeI = nodeH ? nodeH->next : getNode(y + 1);     // y.next
    
    // a -> h -> i      b -> c -> ... -> g -> h -> i
    if(nodeA)
      nodeA->next = nodeH;
    else
      listBegin = nodeH;
    
    // a -> h -> i      b -> i
    nodeB->next = nodeI;
    if(!nodeI)
      listEnd = nodeB;
    
    // a -> h -> c -> ... -> g -> h -> i    b -> i
    if(nodeH != nodeC) // when there are other nodes between x (nodeB) and y (nodeH)
      nodeH->next = nodeC; // let nodeH.next point at nodeC
    else // otherwise let nodeH.next point at nodeB
      nodeH->next = nodeB; 
    
    // a -> h -> c -> ... -> g -> b -> i
    if(nodeG != nodeB) // when there are more than 1 nodes between x (nodeB) and y (nodeH)
      nodeG->next = nodeB; // let nodeG.next point at nodeB
  }
}

template<typename T>
void SimpleList<T>::sort(int (*cmp)(T &, T &)) {
  // bubble sort because I'm lazy
  
  // Example: a -> b -> c -> d
  // we want to swap b with c when b.value > c.value
  // and repeat that until the list is sorted
  
  Node<T>* nodeA;
  Node<T>* nodeB;
  Node<T>* nodeC;
  Node<T>* nodeD;
  
  int c = listSize;
  
  while(c--){
    for(int i = 1; i <= c; i++){
      nodeA = getNode(i - 1);
      nodeB = nodeB ? nodeB->next : getNode(i);

      if(cmp(nodeA->data, nodeB->data) <= 0) {
        swap(i-1,i);
      }
    }
  }
}
/*
template<typename T>
void LinkedList<T>::sort(int (*cmp)(T &, T &)){
  if(listSize < 2) return;

  for(;;) { 

    ListNode<T> **joinPoint = &root;

    while(*joinPoint) {
      ListNode<T> *a = *joinPoint;
      ListNode<T> *a_end = findEndOfSortedString(a, cmp);
  
      if(!a_end->next ) {
        if(joinPoint == &root) {
          last = a_end;
          isCached = false;
          return;
        }
        else {
          break;
        }
      }

      ListNode<T> *b = a_end->next;
      ListNode<T> *b_end = findEndOfSortedString(b, cmp);

      ListNode<T> *tail = b_end->next;

      a_end->next = NULL;
      b_end->next = NULL;

      while(a && b) {
        if(cmp(a->data, b->data) <= 0) {
          *joinPoint = a;
          joinPoint = &a->next;
          a = a->next;  
        }
        else {
          *joinPoint = b;
          joinPoint = &b->next;
          b = b->next;  
        }
      }

      if(a) {
        *joinPoint = a;
        while(a->next) a = a->next;
        a->next = tail;
        joinPoint = &a->next;
      }
      else {
        *joinPoint = b;
        while(b->next) b = b->next;
        b->next = tail;
        joinPoint = &b->next;
      }
    }
  }
}
*/

/*
  - search
  - sort
  - bin search
*/
