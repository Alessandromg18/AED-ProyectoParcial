#ifndef NODE_H
#define NODE_H

#pragma once
#include <string>

struct Header;

struct Node {
    int row;
    int col;
    std::string value;

    Node* right;
    Node* down;

    Header* rowHeader;
    Header* colHeader;

    Node(int r, int c, const std::string& val)
        : row(r), col(c), value(val),
          right(nullptr), down(nullptr),
          rowHeader(nullptr), colHeader(nullptr) {}
};

#endif //NODE_H
