#pragma once

// This file contains an ultra simplified version of the typical
// hashmap, but instead of using hashes, unix file descripters.
// Each file descriptor is unique, and once one become invalid
// you must remove it otherwise you will confuse one player with
// another. 


struct _MapElement{
    int descriptor;
    void* element;
    struct _MapElement* collisionBuddy;
};
typedef struct _MapElement MapElement;

typedef struct {
    int maxElements;
    MapElement elements[];
} DescriptorMap;

DescriptorMap* makeDescriptorMap(int maxElements);
void** lookupElement(DescriptorMap* map, int descriptor);
void* removeElement(DescriptorMap* map, int descriptor);
int setElement(DescriptorMap* map, int descriptor, void* data);
void freeMap(DescriptorMap* map);