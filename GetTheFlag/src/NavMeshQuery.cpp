//
//  NavMeshQuery.cpp
//  GetTheFlag
//
//  Created by Olivier on 24/12/15.
//
//

#include "NavMeshQuery.h"

struct Node {
    real32 cost;
    real32 estimate;
    uint32 pos;
};

struct Heap {
    Node nodes[1024];
    uint32 next = 0;
};

static inline uint32 parent(uint32 node) {
    return (node - 1) / 2;
}

static inline uint32 left(uint32 node) {
    return 2*node+1;
}

static inline uint32 right(uint32 node) {
    return 2*node+2;
}

static inline real32 cost(const Node& node) {
    return node.cost+node.estimate;
}

static void push(Heap* heap, Node node) {
    Node* nodes = heap->nodes;
    
    uint32 i = heap->next++;
    nodes[i] = node;
    
    while(i > 0) {
        uint32 p = parent(i);
        if(cost(nodes[i]) < cost(nodes[p])) {
            Node tmp = nodes[i];
            nodes[i] = nodes[p];
            nodes[p] = tmp;
            i = p;
        } else {
            return;
        }
    }
}

static Node top(Heap* heap) {
    return heap->nodes[0];
}

static bool isEmpty(Heap* heap) {
    return heap->next == 0;
}

static void pop(Heap* heap) {
    Node* nodes = heap->nodes;
    nodes[0] = nodes[--heap->next];
    
    uint32 i = 0;
    uint32 length = heap->next;
    
    while(i < heap->next) {
        uint32 m = length;
        
        uint32 l = left(i);
        if(l < length) {
            m = l;
            uint32 r = right(i);
            if(r < length && cost(nodes[r]) < cost(nodes[l])) {
                m = r;
            }
        }
        
        if(m < length && cost(nodes[m]) < cost(nodes[i])) {
            Node tmp = nodes[i];
            nodes[i] = nodes[m];
            nodes[m] = tmp;
            i = m;
        } else {
            return;
        }
    }
}

uint32 findContainingPolygon(NavMesh* navMesh, const Vec3& point) {
    return 0;
}


bool findPath(NavMesh* navMesh, const Vec3& start, const Vec3& end, Path* path) {
    
    uint32 startPoly = findContainingPolygon(navMesh, start);
    uint32 endPoly = findContainingPolygon(navMesh, end);
    
    Heap heap;
    real32 distStartEnd = 0;
    push(&heap, {.0f, distStartEnd, startPoly});
    
    int32 pred[1024];
    for(int32 i = 0; i < 1024; i++) {
        pred[i] = -2;
    }
    
    pred[startPoly] = -1;
    bool found = false;
    int32 pathLength;
    
    while(!isEmpty(&heap)) {
        Node curr = top(&heap);
        
        if(curr.pos == endPoly) {
            found = true;
            pathLength = curr.cost+1;
            break;
        }
        
        uint32 currPoly;
        pop(&heap);
        
        // for each neighboor
        uint32 neighboor;
        if(pred[neighboor] == -2) {
            pred[neighboor] = curr.pos;
            real32 distNEnd = 0;
            push(&heap,{curr.cost+1, distNEnd, neighboor});
        }
    }

    path->length = 0;
    if(found) {
        uint32* polyPath = path->polys;
        path->polyPathLength = pathLength;
        
        uint32 i = pathLength-1;
        uint32 pos = endPoly;
        do {
            polyPath[i--] = pos;
            pos = pred[pos];
        } while (pos != NULL_INDEX);
    }
}