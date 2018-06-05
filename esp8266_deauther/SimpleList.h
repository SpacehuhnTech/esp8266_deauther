#ifndef SimpleList_h
#define SimpleList_h

#include <stddef.h>
#include <functional>

template<class T>
struct Node {
    T        data;
    Node<T>* next;
};

template<typename T>
class SimpleList {
    public:
        SimpleList();
        ~SimpleList();

        virtual int size();
        virtual void add(int index, T obj);
        virtual void add(T obj);
        virtual void replace(int index, T obj);
        virtual void remove(int index);
        virtual T shift();
        virtual T pop();
        virtual T get(int index);
        virtual int search(std::function<bool(T& a)>cmp);
        virtual int searchNext(std::function<bool(T& a)>cmp);
        virtual int binSearch(std::function<int(T& a)>cmp, int lowerEnd, int upperEnd);
        virtual int binSearch(std::function<int(T& a)>cmp);
        virtual int count(std::function<bool(T& a)>cmp);
        virtual void clear();
        virtual void sort(std::function<bool(T& a, T& b)>cmp);
        virtual void swap(int x, int y);

    protected:
        int listSize;
        Node<T>* listBegin;
        Node<T>* listEnd;

        // Helps get() method by saving last position
        Node<T>* lastNodeGot;
        int lastIndexGot;
        bool isCached;

        Node<T>* getNode(int index);
};

template<typename T>
SimpleList<T>::SimpleList() {
    listBegin = NULL;
    listEnd   = NULL;
    listSize  = 0;

    lastNodeGot  = NULL;
    lastIndexGot = -1;
    isCached     = false;
}

// Clear Nodes and free Memory
template<typename T>
SimpleList<T>::~SimpleList() {
    clear();
    listBegin = NULL;
    listEnd   = NULL;
    listSize  = 0;

    lastNodeGot  = NULL;
    lastIndexGot = -1;
    isCached     = false;
}

template<typename T>
Node<T>* SimpleList<T>::getNode(int index) {
    if ((index < 0) || (index >= listSize)) return NULL;

    Node<T>* hNode = listBegin;
    int c          = 0;

    if (isCached && (index >= lastIndexGot)) {
        c     = lastIndexGot;
        hNode = lastNodeGot;
    }

    while (hNode != NULL && c < index) {
        hNode = hNode->next;
        c++;
    }

    if (hNode) {
        isCached     = true;
        lastIndexGot = c;
        lastNodeGot  = hNode;
    }

    return hNode;
}

template<typename T>
int SimpleList<T>::size() {
    return listSize;
}

template<typename T>
void SimpleList<T>::add(T obj) {
    Node<T>* node = new Node<T>();
    node->data = obj;
    node->next = NULL;

    if (!listBegin) listBegin = node;

    if (listEnd) {
        listEnd->next = node;
        listEnd       = node;
    } else {
        listEnd = node;
    }

    listSize++;
}

template<typename T>
void SimpleList<T>::add(int index, T obj) {
    if ((index < 0) || (index >= listSize)) {
        add(obj);
        return;
    }

    Node<T>* nodeNew = new Node<T>();
    nodeNew->data = obj;
    nodeNew->next = NULL;

    if (index == 0) listBegin = nodeNew;
    else {
        Node<T>* nodePrev = getNode(index - 1);
        nodeNew->next  = nodePrev->next;
        nodePrev->next = nodeNew;
    }

    listSize++;
}

template<typename T>
void SimpleList<T>::replace(int index, T obj) {
    if ((index >= 0) && (index < listSize)) {
        getNode(index)->data = obj;
    }
}

template<typename T>
void SimpleList<T>::remove(int index) {
    if ((index < 0) || (index >= listSize)) return;

    Node<T>* nodePrev     = getNode(index - 1);
    Node<T>* nodeToDelete = getNode(index);

    if (index == 0) {
        listBegin = nodeToDelete->next;
    } else {
        nodePrev->next = nodeToDelete->next;

        if (!nodePrev->next) listEnd = nodePrev;
    }

    delete nodeToDelete;

    isCached = false;

    listSize--;
}

template<typename T>
T SimpleList<T>::get(int index) {
    Node<T>* hNode = getNode(index);
    return hNode ? hNode->data : T();
}

template<typename T>
int SimpleList<T>::search(std::function<bool(T& a)>cmp) {
    int i = 0;

    Node<T>* hNode = getNode(i);
    bool     found = cmp(hNode->data);

    while (!found && i < listSize) {
        i++;
        hNode = getNode(i);
        found = cmp(hNode->data);
    }

    return found ? i : -1;
}

template<typename T>
int SimpleList<T>::searchNext(std::function<bool(T& a)>cmp) {
    int i = lastIndexGot;

    Node<T>* hNode = lastNodeGot;
    bool     found = cmp(hNode->data);

    while (!found && i < listSize) {
        i++;
        hNode = getNode(i);
        found = cmp(hNode->data);
    }

    return found ? i : -1;
}

template<typename T>
int SimpleList<T>::binSearch(std::function<int(T& a)>cmp, int lowerEnd, int upperEnd) {
    int res;
    int mid = (lowerEnd + upperEnd) / 2;

    Node<T>* hNode = listBegin;
    int hIndex     = 0;

    while (lowerEnd <= upperEnd) {
        hNode  = lastNodeGot;
        hIndex = lastIndexGot;
        res    = cmp(getNode(mid)->data);

        if (res == 0) {
            return mid;
        } else if (res < 0) {
            // when going left, set cached node back to previous cached node
            lastNodeGot  = hNode;
            lastIndexGot = hIndex;
            isCached     = true;

            upperEnd = mid - 1;
            mid      = (lowerEnd + upperEnd) / 2;
        } else if (res > 0) {
            lowerEnd = mid + 1;
            mid      = (lowerEnd + upperEnd) / 2;
        }
    }

    return -1;
}

template<typename T>
int SimpleList<T>::binSearch(std::function<int(T& a)>cmp) {
    return binSearch(cmp, 0, listSize - 1);
}

template<typename T>
int SimpleList<T>::count(std::function<bool(T& a)>cmp) {
    int c = 0;

    for (int i = 0; i < listSize; i++) {
        c += cmp(getNode(i)->data);
    }

    return c;
}

template<typename T>
T SimpleList<T>::pop() {
    if (listSize <= 0) return T();

    T data = listEnd->data;
    remove(listSize - 1);
    return data;
}

template<typename T>
T SimpleList<T>::shift() {
    if (listSize <= 0) return T();

    T data = listBegin->data;
    remove(0);
    return data;
}

template<typename T>
void SimpleList<T>::clear() {
    while (listSize > 0) remove(0);
}

template<typename T>
void SimpleList<T>::swap(int x, int y) {
    // only continue when the index numbers are unequal and at least 0
    if ((x != y) && (x >= 0) && (y >= 0)) {
        if (x > y) { // the first index should be smaller than the second. If not, swap them!
            int h = x;
            x = y;
            y = h;
        }

        // When data is small, copy it
        if (sizeof(T) < 24) {
            Node<T>* nodeA = getNode(x);
            Node<T>* nodeB = getNode(y);
            T h            = nodeA->data;
            nodeA->data = nodeB->data;
            nodeB->data = h;
        }

        // otherwise change the pointers
        else {
            // Example: a -> b -> c -> ... -> g -> h -> i
            //          we want to swap b with h
            Node<T>* nodeA = getNode(x - 1);                      // x.prev
            Node<T>* nodeB = getNode(x);                          // x
            Node<T>* nodeC = getNode(x + 1);                      // x.next
            Node<T>* nodeG = y - 1 == x ? nodeB : getNode(y - 1); // y.prev
            Node<T>* nodeH = getNode(y);                          // y
            Node<T>* nodeI = getNode(y + 1);                      // y.next

            // a -> h -> i      b -> c -> ... -> g -> h -> i
            if (nodeA) nodeA->next = nodeH;
            else listBegin = nodeH;

            // a -> h -> c -> ... -> g -> h -> i    b -> i
            if (nodeH != nodeC) // when nodes between b and h exist
                nodeH->next = nodeC;
            else nodeH->next = nodeB;

            // a -> h -> i      b -> i
            nodeB->next = nodeI;

            if (!nodeI) listEnd = nodeB;

            // a -> h -> c -> ... -> g -> b -> i
            if (nodeG != nodeB) // when more than 1 nodes between b and h exist
                nodeG->next = nodeB;
        }
    }
}

template<typename T>
void SimpleList<T>::sort(std::function<bool(T& a, T& b)>cmp) {
    // selection sort

    int indexH;       // index of node i
    int indexMin;     // index of next minimum node

    Node<T>* nodeMin; // next minimum node
    Node<T>* nodeH;   // helper node at index j

    for (int i = 0; i < listSize - 1; i++) {
        nodeMin  = getNode(i);
        indexH   = i;
        indexMin = i;

        for (int j = i + 1; j < listSize; j++) {
            nodeH = getNode(j);

            if (cmp(nodeMin->data, nodeH->data)) {
                nodeMin  = nodeH;
                indexMin = j;
            }
        }
        swap(indexH, indexMin);
    }
}

#endif // ifndef SimpleList_h