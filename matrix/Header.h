#ifndef HEADER_H
#define HEADER_H

#pragma once

struct Node;

struct Header {
    int index;

    Header* next; // Para haceder al header siguiente

    Node* access; // Como acceder al child

    Header(int idx)
        : index(idx), next(nullptr), access(nullptr) {}
};
#endif //HEADER_H
