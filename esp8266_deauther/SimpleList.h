#ifndef SimpleList_h
#define SimpleList_h

#include <stddef.h>

template<class T>
struct Node {
  T data;
  Node<T> *next;
};

template <typename T>
class SimpleList{
  public:
    SimpleList();
    ~SimpleList();

    int size();
    void add(int index, T obj);
    void add(T obj);
    bool set(int index, T obj);
    void remove(int index);
    T shift();
    T pop();
    T get(int index);
    void clear();
    void sort(int (*cmp)(T &nodeA, T &nodeB));
    void swap(int x, int y);
  private:
    int listSize;
    Node<T> *listBegin;
    Node<T> *listEnd;
  
    // Helps get() method by saving last position
    Node<T> *lastNodeGot;
    int lastIndexGot;
    bool isCached;
    
    Node<T>* getNode(int index);
};

#endif
