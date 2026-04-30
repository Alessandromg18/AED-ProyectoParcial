#include "SparseMatrix.h"
#include <iostream>

// ================= CONSTRUCTOR =================

SparseMatrix::SparseMatrix() {
    detail = new DetailNode();
}

// ================= DESTRUCTOR =================
SparseMatrix::~SparseMatrix() {}


// ================= Vemos si existe el Header o sino lo creamos =================

Header* SparseMatrix::getOrCreateHeader(Header*& head, int index) {

    // Si la lista está vacía, creamos el primero y apunta al detail

    if (!head || head == (Header*)detail) {
        Header* n = new Header(index);
        n->next = (Header*)detail;
        head = n;
        return n;
    }

    if (index < head->index) {
        Header* n = new Header(index);
        n->next = head;

        // Buscamos el último para que apunte al nuevo head si fuera necesario,

        Header* last = head;
        while (last->next != (Header*)detail) {
            last = last->next;
        }
        head = n;
        last->next = (Header*)detail;
        return n;
    }

    Header* current = head;
    while (current->next != (Header*)detail && current->next->index < index) {
        current = current->next;
    }

    if (current->index == index) return current;
    if (current->next != (Header*)detail && current->next->index == index) return current->next;

    Header* n = new Header(index);
    n->next = current->next;
    current->next = n;

    return n;
}


// ================= Saber si el nodo existe =================

Node* SparseMatrix::findNode(int row, int col) {

    Header* rh = detail->rowHeaders;
    if (!rh) return nullptr;

    // Recorremos las cabeceras

    while (rh != (Header*)detail) {
        if (rh->index == row) break;
        rh = rh->next;
    }

    if (rh == (Header*)detail || !rh->access)
        return nullptr;

    Node* start = rh->access;
    Node* c = start;

    do {
        if (c->col == col)
            return c;

        c = c->right;

    } while (c != start);

    return nullptr;
}


// ================= INSERT =================

void SparseMatrix::insert(int row, int col, const std::string& value) {

    // 1. Si el nodo ya existe, solo actualizamos el valor

    Node* exist = findNode(row, col);
    if (exist) {
        exist->value = value;
        return;
    }

    // 2. Creamos  el nuevo nodo

    Node* n = new Node(row, col, value);

    // ================= Obtenemos los headers de nuestro nodo =================

    Header* rh = getOrCreateHeader(detail->rowHeaders, row);
    Header* ch = getOrCreateHeader(detail->colHeaders, col);

    n->rowHeader = rh;
    n->colHeader = ch;

    // Gracias a la funcion GetorCreateHeader  obtenemos los headers de nuestro nodo o sino los crea :D.

    // ================= Insertamos nuestro nodo en la fila - Conexiones con su Header Fila =================

    if (!rh->access) {
        rh->access = n;
        n->right = n;
    }

    else if (col < rh->access->col) {
        Node* last = rh->access;
        while (last->right != rh->access)
            last = last->right;

        n->right = rh->access;
        last->right = n;
        rh->access = n;
    }
    
    else {
        Node* c = rh->access;
        while (c->right != rh->access && c->right->col < col)
            c = c->right;

        n->right = c->right;
        c->right = n;
    }

    // ================= Insertamos nuestro nodo en la columna - Conexiones con su Header Columna =================

    if (!ch->access) {
        ch->access = n;
        n->down = n; // Circular a sí mismo
    }
    else if (row < ch->access->row) {
        Node* last = ch->access;
        while (last->down != ch->access)
            last = last->down;

        n->down = ch->access;
        last->down = n;
        ch->access = n;
    }
    else {
        Node* c = ch->access;
        while (c->down != ch->access && c->down->row < row)
            c = c->down;

        n->down = c->down;
        c->down = n;
    }

    // ================= CONEXIONES CON EL DETAIL =================

    // Para evitar conflicto con el next del Detal para eso usamos los punteros renombrados para asegurar que el next
    // del detail y cambie dependiendo de lo que insertamos

    detail->down_ptr = detail->rowHeaders;
    detail->right_ptr = detail->colHeaders;

    // ================= ACTUALIZAMOS LAS DIMENSIONES DE NUESTRO NODO DETAIL =================

    updateDetailCounts();
}

// Buscamos nodo segun la fila y columna que nos den.

std::string SparseMatrix::get(int row, int col) {
    Node* target = findNode(row, col);
    if (target) {
        return target->value;
    }
    return "";
}

void SparseMatrix::remove(int row, int col) {

    // Debemos obtener los headers de ese nodo.

    Header* rH = findRowHeader(row);
    Header* cH = findColHeader(col);

    if (!rH || !cH) return;

    // 1. Buscar el nodo a eliminar y su anterior en la fila (Importante para hacer las reconexiones)

    Node* prevR = nullptr;
    Node* targetR = rH->access;

    while (targetR && targetR->col != col) {
        prevR = targetR;
        targetR = targetR->right;
        if (targetR == rH->access) { targetR = nullptr; break; }
    }

    if (!targetR) return;

    // 2. Buscar el anterior en la columna

    Node* prevC = nullptr;
    Node* targetC = cH->access;

    while (targetC && targetC->row != row) {
        prevC = targetC;
        targetC = targetC->down;
        if (targetC == cH->access) { targetC = nullptr; break; }
    }

    // Hacemos las reconexiones para la fila

    // Si es primer nodo :

    if (prevR == nullptr) {

        // Si es el único nodo en la fila :

        if (targetR->right == targetR) {
            rH->access = nullptr;
            removeRowHeader(row); // Eliminamos el header de ese nodo ya que esa fila le hemos quitado su unico elemento

        } else {

            // Buscamos el último para que el círculo no se rompa. Y si es el ultimo el nodo entonces reconectamos.

            Node* last = targetR;
            while(last->right != targetR) last = last->right;
            rH->access = targetR->right;
            last->right = rH->access;

        }

    }

    else {
        prevR->right = targetR->right;
    }

    // Hacemos las reconexiones para la columna

    if (prevC == nullptr) {

        if (targetC->down == targetC) {
            cH->access = nullptr;
            removeColHeader(col);

        } else {

            Node* last = targetC;
            while(last->down != targetC) last = last->down;
            cH->access = targetC->down;
            last->down = cH->access;
        }

    }

    else {
        prevC->down = targetC->down;
    }

    delete targetR; // Una vez todo reconectado elimino mi nodo xd

    // Sincronizamos punteros del detail tras el borrado. (Importante para que no haya fallos en el detail)
    // Solo para asegurar que este bien mi detail

    detail->down_ptr = detail->rowHeaders;
    detail->right_ptr = detail->colHeaders;

    updateDetailCounts(); // Actualizamos nuestro detail (MUY IMPORTANTE)

}


// BUSCAR el header fila de mi nodo

Header* SparseMatrix::findRowHeader(int row) {
    Header* curr = detail->rowHeaders;
    while (curr) {
        if (curr->index == row) return curr;
        curr = curr->next;
        if (curr == detail->rowHeaders) break;
    }
    return nullptr;
}

// BUSCAR el header columna de mi nodo

Header* SparseMatrix::findColHeader(int col) {
    Header* curr = detail->colHeaders;
    while (curr) {
        if (curr->index == col) return curr;
        curr = curr->next;
        if (curr == detail->colHeaders) break;
    }
    return nullptr;
}

// Para remover el header fila si el nodo que elimine era su unico elemento

void SparseMatrix::removeRowHeader(int row) {
    Header* prev = nullptr;
    Header* curr = detail->rowHeaders;

    // Buscar el nodo y su anterior

    while (curr && curr != (Header*)detail && curr->index != row) {
        prev = curr;
        curr = curr->next;
    }

    // Misma logica que con los nodos, para asegurar que siga habiendo circularidad.

    if (curr && curr != (Header*)detail) {
        if (prev == nullptr) {

            // Si el que sigue es el detail, la lista quedara vacía (Importante para volver a reinsertar)

            if (curr->next == (Header*)detail) {
                detail->rowHeaders = nullptr;
            }

            else {
                detail->rowHeaders = curr->next;
            }
        }

        else {
            prev->next = curr->next;
        }

        delete curr;
    }
}

// Lo mismo que la funcion de arriba

void SparseMatrix::removeColHeader(int col) {

    Header* prev = nullptr;
    Header* curr = detail->colHeaders;

    while (curr && curr != (Header*)detail && curr->index != col) {
        prev = curr;
        curr = curr->next;
    }

    if (curr && curr != (Header*)detail) {

        if (prev == nullptr) {

            if (curr->next == (Header*)detail) {
                detail->colHeaders = nullptr;
            }

            else {
                detail->colHeaders = curr->next;
            }
        }

        else {
            prev->next = curr->next;
        }

        delete curr;
    }
}

// Funcion que actualiza mi DETAIL para que cuente bien (O(n)) :D

void SparseMatrix::updateDetailCounts() {
    int rowCount = 0;
    int colCount = 0;

    // Contamos la cantidad de headers fila (Cantidad de filas)

    Header* r = detail->rowHeaders;
    while (r && r != (Header*)detail) {
        rowCount++;
        r = r->next;
    }

    // Contamos la cantidad de headers columnas (Cantidad de columnas)

    Header* c = detail->colHeaders;
    while (c && c != (Header*)detail) {
        colCount++;
        c = c->next;
    }

    detail->rows = rowCount;
    detail->cols = colCount;
}