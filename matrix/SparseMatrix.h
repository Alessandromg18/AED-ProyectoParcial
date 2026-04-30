#ifndef SPARSEMATRIX_H
#define SPARSEMATRIX_H

#pragma once

// Aqui es donde se crea la Matriz dispersa xd

#include "Header.h"
#include "DetailNode.h"
#include "Node.h"
#include <string>

class SparseMatrix {
private:
    DetailNode* detail;

    Header* getOrCreateHeader(Header*& head, int index);

public:
    SparseMatrix();
    ~SparseMatrix();

    void insert(int row, int col, const std::string& value);
    std::string get(int row, int col);
    void remove(int row, int col);

    Node* findNode(int row, int col);

    Header* getRowHeaders() { return detail->rowHeaders; }
    Header* getColHeaders() { return detail->colHeaders; }
    DetailNode* getDetail() { return detail; }
    Header* findRowHeader(int row);
    Header* findColHeader(int col);
    void removeRowHeader(int row);
    void removeColHeader(int col);
    void updateDetailCounts();
    // FALTA MAS FUNCIONES PERO AQUI ESTA LAS PRINCIPALES PARA QUE LA LOGICA DE LOS HEADERS, DETAIL Y NODOS SE MANTENGA (PRINCIPALMENTE LAS CONEXIONES)


};

#endif //SPARSEMATRIX_H
