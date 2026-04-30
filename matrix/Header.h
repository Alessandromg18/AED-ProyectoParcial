#ifndef HEADER_H
#define HEADER_H

#pragma once

struct Node;

struct Header {
    int index;

    Header* next; // Para haceder al header siguiente a estos

    Node* access; // Para poder acceder al primer nodo que tienen (Como su child)

    Header(int idx)
        : index(idx), next(nullptr), access(nullptr) {}
};
#endif //HEADER_H
