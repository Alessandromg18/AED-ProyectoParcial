#ifndef SPARSEMATRIX_H
#define SPARSEMATRIX_H

#pragma once

// Aqui es donde se crea la Matriz dispersa xd

#include "Header.h"
#include "DetailNode.h"
#include "Node.h"
#include <string>
#include <vector>

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

    // EmPuiquin24: Operaciones sobre fila y columna
    // -- Eliminar todos los elementos de una fila
    void removeRow(int row);
    // -- Eliminar todos los elementos de una columna
    void removeCol(int col);
    // -- Eliminar un rango de celdas (Ejemplo: Eliminar todas las celdas dentro del bounding box definido por (r1, c1) y (r2, c2))
    void removeRange(int row1, int col1, int row2, int col2);

    // -- OPERACIONES DE AGREGACION --
    std::vector<double> getNumericValuesInRange(int r1, int c1, int r2, int c2);
    double aggSum(int r1, int c1, int r2, int c2);
    double aggAverage(int r1, int c1, int r2, int c2);
    double aggMax(int r1, int c1, int r2, int c2);
    double aggMin(int r1, int c1, int r2, int c2);
};

#endif //SPARSEMATRIX_H

