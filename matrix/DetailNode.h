//
// Created by facum on 27/04/2026.
//

#ifndef DETAILNODE_H
#define DETAILNODE_H

#pragma once

#include "Header.h"

struct DetailNode : public Header {
    int rows;
    int cols;

    Header* rowHeaders;
    Header* colHeaders;

    // Estos son los punteros a los primeros elementos (Esto es muy importante)

    Header* right_ptr;
    Header* down_ptr;

    DetailNode(int r = 0, int c = 0)
        : Header(-1),

    // Índice -1 para el nodo raíz, ya que los headers inician desde 0.
          rows(r), cols(c),
          rowHeaders(nullptr), colHeaders(nullptr),
          right_ptr(nullptr), down_ptr(nullptr) {
        this->next = nullptr;
    }
};
#endif //DETAILNODE_H
