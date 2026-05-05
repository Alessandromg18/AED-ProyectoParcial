#include "SparseMatrix.h"
#include <iostream>
#include <regex>
#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>

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
    std::string finalValue = value;

    // VALIDAMOS SI PUSIERON BIEN LA FORMULA

    if (!value.empty() && value[0] == '=') {

        try {

            // Intentamos evaluar primero
            double res = evaluateFormula(value);

            finalValue = std::to_string(res);
            finalValue.erase(finalValue.find_last_not_of('0') + 1, std::string::npos);
            if (finalValue.back() == '.') finalValue.pop_back();

        } catch (const std::exception& e) {
            throw std::runtime_error("Error en formula: " + std::string(e.what())); // Imprime en la visualizacion
            return;
        }
    }

    // Si nodo existe solo actualizamos

    Node* exist = findNode(row, col);
    if (exist) {
        exist->value = finalValue;
        return;
    }

    // Como no existe creamos  el nuevo nodo

    Node* n = new Node(row, col, finalValue);

    // ================= Obtenemos los headers de nuestro nodo =================

    Header* rh = getOrCreateHeader(detail->rowHeaders, row);
    Header* ch = getOrCreateHeader(detail->colHeaders, col);

    n->rowHeader = rh;
    n->colHeader = ch;

    // Gracias a la funcion GetorCreateHeader  obtenemos los headers de nuestro nodo o sino los crea :D.

    // Insertamos nuestro nodo en la fila - Conexiones con su Header Fila

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

    // Insertamos nuestro nodo en la columna - Conexiones con su Header Columna

    if (!ch->access) {
        ch->access = n;
        n->down = n;
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

    // CONEXIONES CON EL DETAIL

    // Para evitar conflicto con el next del Detal para eso usamos los punteros renombrados para asegurar que el next
    // del detail y cambie dependiendo de lo que insertamos

    detail->down_ptr = detail->rowHeaders;
    detail->right_ptr = detail->colHeaders;

    // ACTUALIZAMOS LAS DIMENSIONES DE NUESTRO NODO DETAIL

    updateDetailCounts();
}

// ================= Devuelve el valor que contiene el nodo ===========

std::string SparseMatrix::get(int row, int col) {
    Node* target = findNode(row, col);
    if (target) {
        return target->value;
    }
    return "";
}

// ================= Eliminar un nodo ===========

void SparseMatrix::remove(int row, int col) {
    // 1. Buscamos los headers de forma segura
    Header* rH = findRowHeader(row);
    Header* cH = findColHeader(col);

    // QUITADO: El if con get(row, col) == "" porque causa recursividad/crashes
    if (!rH || !cH) return;

    // 2. Buscar el nodo (targetR) y su anterior (prevR) en la FILA
    Node* prevR = nullptr;
    Node* targetR = rH->access;
    bool found = false;

    if (targetR) {
        Node* temp = targetR;
        do {
            if (temp->col == col) {
                targetR = temp;
                found = true;
                break;
            }
            prevR = temp;
            temp = temp->right;
        } while (temp != rH->access);
    }

    if (!found) return; // Si no existe el nodo, salimos silenciosamente

    // 3. Buscar el anterior (prevC) en la COLUMNA
    Node* prevC = nullptr;
    Node* targetC = cH->access;
    Node* tempC = targetC;
    // Buscamos quién apunta a nuestro nodo desde "arriba"
    while (tempC->down != targetR) {
        tempC = tempC->down;
    }
    prevC = (tempC->down == targetR) ? tempC : nullptr;
    // Si el que apunta es el mismo, es porque es el único
    if (prevC == targetR) prevC = nullptr;

    // --- RECONEXIÓN FILA ---
    if (prevR == nullptr) { // El nodo a borrar es el access del header
        if (targetR->right == targetR) { // Único nodo
            rH->access = nullptr;
            removeRowHeader(row);
        } else {
            Node* last = targetR;
            while(last->right != targetR) last = last->right;
            rH->access = targetR->right;
            last->right = rH->access;
        }
    } else {
        prevR->right = targetR->right;
    }

    // --- RECONEXIÓN COLUMNA ---
    if (targetR == cH->access) { // Usamos targetR porque es el mismo objeto que targetC
        if (targetR->down == targetR) { // Único nodo
            cH->access = nullptr;
            removeColHeader(col);
        } else {
            Node* last = targetR;
            while(last->down != targetR) last = last->down;
            cH->access = targetR->down;
            last->down = cH->access;
        }
    } else {
        // Para encontrar el prevC real si no es el access:
        Node* scan = cH->access;
        while(scan->down != targetR) scan = scan->down;
        scan->down = targetR->down;
    }

    delete targetR;

    // Sincronización final
    detail->down_ptr = detail->rowHeaders;
    detail->right_ptr = detail->colHeaders;
    updateDetailCounts();
}

// ================= Buscamos la cabecera fila  ===========
// Sirve para ver si en la fila hay elementos

Header* SparseMatrix::findRowHeader(int row) {
    Header* curr = detail->rowHeaders;
    // Si la lista es null o apunta al detail (vacía), no hay nada que buscar
    if (!curr || curr == (Header*)detail) return nullptr;

    Header* start = curr;
    do {
        if (curr->index == row) return curr;
        curr = curr->next;
    } while (curr != start && curr != (Header*)detail);

    return nullptr;
}

// ================= Buscamos la cabecera columna  ===========

Header* SparseMatrix::findColHeader(int col) {
    Header* curr = detail->colHeaders;
    if (!curr || curr == (Header*)detail) return nullptr;

    Header* start = curr;
    do {
        if (curr->index == col) return curr;
        curr = curr->next;
    } while (curr != start && curr != (Header*)detail);

    return nullptr;
}

// ================== Para remover el header fila si el nodo que elimine era su unico elemento ======

void SparseMatrix::removeRowHeader(int row) {
    Header* prev = nullptr;
    Header* curr = detail->rowHeaders;

    // Caso lista vacía
    if (!curr || curr == (Header*)detail) return;

    // Buscar el nodo y su anterior
    while (curr != (Header*)detail && curr->index != row) {
        prev = curr;
        curr = curr->next;
    }

    if (curr && curr != (Header*)detail) {
        if (prev == nullptr) { // Es el primer nodo
            if (curr->next == (Header*)detail) {
                detail->rowHeaders = nullptr; // Era el único
            } else {
                // Si hay más, debemos actualizar el LAST para que apunte al nuevo head
                Header* last = curr;
                while (last->next != (Header*)detail) last = last->next;
                detail->rowHeaders = curr->next;
                // Opcional: si tu lógica es puramente circular sin detail intermedio:
                // last->next = detail->rowHeaders;
            }
        } else {
            prev->next = curr->next;
        }
        delete curr;
    }
}

// ================== Para remover el header columna si el nodo que elimine era su unico elemento ======
void SparseMatrix::removeColHeader(int col) {
    Header* prev = nullptr;
    Header* curr = detail->colHeaders;

    // Caso de seguridad: lista vacía
    if (!curr || curr == (Header*)detail) return;

    // Buscar el header y su anterior
    while (curr != (Header*)detail && curr->index != col) {
        prev = curr;
        curr = curr->next;
    }

    if (curr && curr != (Header*)detail) {
        if (prev == nullptr) { // Es el primer header de la lista
            if (curr->next == (Header*)detail) {
                detail->colHeaders = nullptr; // Era el único, lista vacía
            } else {
                // Actualizamos el LAST para que el círculo no se rompa
                Header* last = curr;
                while (last->next != (Header*)detail) last = last->next;
                detail->colHeaders = curr->next;
                last->next = (Header*)detail;
            }
        } else {
            // Saltamos el nodo actual
            prev->next = curr->next;
        }
        delete curr;
    }
}

// ========= Funcion que actualiza mi DETAIL para que cuente bien la cantidad de filas y columnas que se usan =======

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

// DESDE AQUI FALTA

// ========== Funcion que nos sirve para convertir "A1" a índices de matriz

bool parseRef(std::string ref, int& r, int& c) {

    std::string colPart = "";
    std::string rowPart = "";

    for (char ch : ref) {
        if (isalpha(ch)) colPart += toupper(ch);
        else if (isdigit(ch)) rowPart += ch;
    }

    // Si no hay filas o columnas devuelve false
    if (colPart.empty() || rowPart.empty()) return false;

    // Convertimos las columnas (A=0, B=1...)

    c = 0;
    for (char ch : colPart) c = c * 26 + (ch - 'A' + 1);
    c--;
    r = std::stoi(rowPart) - 1;

    // Si todo salio bien se devuelve true
    return true;
}

// ========= Funcion convierte el valor de nuestra celda en numero (Importante para operaciones) =======

double SparseMatrix::getCellValue(const std::string& ref) {
    int r, c;

    // Obtenemos sus indices de matriz

    if (!parseRef(ref, r, c)) {
        throw std::runtime_error("Referencia invalida: " + ref);
    }

    // Obtenemos su valor ya con los indices de matriz hallados

    std::string val = get(r, c);
    if (val.empty()) return 0.0; // Si el nodo no existe entonces se retorna 0 como en excel

    // Ahora con el valor intentamos convertir a un numero el string.
    try {
        return std::stod(val);
    } catch (...) {
        throw std::runtime_error("La celda " + ref + " no contiene un numero");
    }
}

// ========= Función para el soporte elemental para formulas =============

double SparseMatrix::evaluateFormula(const std::string& formula) {

    // La fórmula debe empezar con '='

    if (formula.empty() || formula[0] != '=')
        return 0.0;

    // Eliminamos el '=' para trabajar solo con la expresión

    std::string expr = formula.substr(1);
    if (expr.empty())
        throw std::runtime_error("Formula vacia despues del '='");


    // La funcion regex separa la expresión en funciones, celdas, numeros y operadores

    std::regex reg("([a-z]+)\\s*\\(?([^\\)\\+\\-\\*\\/\\s]+)\\)?|"
                   "([A-Z]+[0-9]+)"
                   "|([0-9]+\\.?[0-9]*)|"
                   "([\\+\\-\\*\\/])");

    auto words_begin = std::sregex_iterator(expr.begin(), expr.end(), reg);
    auto words_end = std::sregex_iterator();

    const int MAX_COORD = 1000000;

    auto colToIdx = [](std::string col) {
        int res = 0;
        for (char c : col)
            res = res * 26 + (toupper(c) - 'A' + 1);
        return res - 1;
    };

    // Extraemos los rangos

    auto extractRange = [&](std::string s, int& r1, int& c1, int& r2, int& c2) {

        size_t colon = s.find(':');

        // CASO 1: RANGO (tiene ":")
        if (colon != std::string::npos) {
            std::string start = s.substr(0, colon);
            std::string end = s.substr(colon + 1);

            // Subcaso: rango de filas
            if (std::all_of(start.begin(), start.end(), ::isdigit) &&
                std::all_of(end.begin(), end.end(), ::isdigit)) {
                r1 = std::stoi(start) - 1; r2 = std::stoi(end) - 1;
                c1 = 0; c2 = MAX_COORD;
                return true;
            }

            // Subcaso: rango de columnas
            if (std::all_of(start.begin(), start.end(), ::isalpha) &&
                std::all_of(end.begin(), end.end(), ::isalpha)) {
                c1 = colToIdx(start); c2 = colToIdx(end);
                r1 = 0; r2 = MAX_COORD;
                return true;
            }

            // Subcaso: rango estandar
            auto parse = [&](std::string p, int& r, int& c) {
                std::string cp = "", rp = "";
                for (char ch : p) {
                    if (isalpha(ch)) cp += ch;
                    else if (isdigit(ch)) rp += ch;
                }
                if (cp.empty() || rp.empty()) return false;
                c = colToIdx(cp); r = std::stoi(rp) - 1;
                return true;
            };
            return parse(start, r1, c1) && parse(end, r2, c2);
        }

        // CASO 2: SOLO FILA

        if (std::all_of(s.begin(), s.end(), ::isdigit)) {
            r1 = r2 = std::stoi(s) - 1; c1 = 0; c2 = MAX_COORD;
            return true;
        }

        // CASO 3: SOLO COLUMNA

        if (std::all_of(s.begin(), s.end(), ::isalpha)) {
            c1 = c2 = colToIdx(s); r1 = 0; r2 = MAX_COORD;
            return true;
        }

        std::string cp = "", rp = "";

        // CASO 4 : Celda Simple

        for (char ch : s) {
            if (isalpha(ch)) cp += ch;
            else if (isdigit(ch)) rp += ch;
        }

        if (!cp.empty() && !rp.empty()) {
            c1 = c2 = colToIdx(cp); r1 = r2 = std::stoi(rp) - 1;
            return true;
        }
        return false;
    };


    // LOGICA DE EVALUACIÓN CON JERARQUÍA

    double result = 0.0;
    double term = 0.0;

    // Ultimo operador del nivel 1 (+ o -)
    char lastOp = '+';

    // Ultimo operador del nivel 2
    char lastMulDiv = '*';

    // Si se esta iniciando un nuevo termino
    bool firstValueInTerm = true;


    // Recorremos todos la expresion

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        double currentTokenValue = 0.0;
        bool hasValue = false;

        // Si son funciones

        if (match[1].matched) {
            std::string funcName = match[1].str();
            std::string rangeStr = match[2].str();
            int r1, c1, r2, c2;
            if (extractRange(rangeStr, r1, c1, r2, c2)) {
                if (funcName == "sum") currentTokenValue = aggSum(r1, c1, r2, c2);
                else if (funcName == "avg") currentTokenValue = aggAverage(r1, c1, r2, c2);
                else if (funcName == "max") currentTokenValue = aggMax(r1, c1, r2, c2);
                else if (funcName == "min") currentTokenValue = aggMin(r1, c1, r2, c2);
                else throw std::runtime_error("Funcion desconocida: " + funcName);
                hasValue = true;
            }
        }

        // Si son celdas
        else if (match[3].matched) {
            currentTokenValue = getCellValue(match[3].str());
            hasValue = true;
        }

        // Si son numeros
        else if (match[4].matched) {
            currentTokenValue = std::stod(match[4].str());
            hasValue = true;
        }

        // Si son operadores - Aplicamos su jerarquia de estos

        else if (match[5].matched) {
            char op = match[5].str()[0];

            if (op == '+' || op == '-') {

                if (lastOp == '+') result += term;
                else result -= term;

                lastOp = op;
                lastMulDiv = '*';
                firstValueInTerm = true;

            } else {
                lastMulDiv = op;
            }
            continue;
        }

        if (hasValue) {
            if (firstValueInTerm) {
                term = currentTokenValue;
                firstValueInTerm = false;

            }

            else {
                if (lastMulDiv == '*') term *= currentTokenValue;
                else if (lastMulDiv == '/') {
                    if (currentTokenValue == 0) throw std::runtime_error("Div/0");
                    term /= currentTokenValue;
                }
            }
        }

    }

    if (lastOp == '+') result += term;
    else result -= term;

    return result;

}

// =============== Funcion que obtiene todos los valores numericos dentro de un rango =============

std::vector<double> SparseMatrix::getNumericValuesInRange(int r1, int c1, int r2, int c2) {

    std::vector<double> values; // Aqui guardamos los valores numericos

    // Aseguraramos que el inicio sea menos que el fin por si nos pasan A5:A1

    int minRow = std::min(r1, r2);
    int maxRow = std::max(r1, r2);
    int minCol = std::min(c1, c2);
    int maxCol = std::max(c1, c2);

    // Recorremos fila por fila dentro de nuestro rango

    for (int r = minRow; r <= maxRow; ++r) {

        // Vemos si la fila tiene datos (Si tiene debe existir su nodo header)

        Header* rHead = findRowHeader(r);

        // Recorremos los nodos de la fila

        if (rHead && rHead->access) {
            Node* curr = rHead->access;

            Node* startNode = curr; // Este por si volvemos al inicio

            do {

                // Verificamos si esta en el rango de columnas

                if (curr->col >= minCol && curr->col <= maxCol) {
                    try {

                        // Convierto a numero

                        double val = std::stod(curr->value);
                        values.push_back(val);

                    } catch (...) {
                    }
                }
                curr = curr->right;
            } while (curr && curr != startNode);
        }
    }

    return values;
}


// ====== Funcion que calcula la suma total del rango que nos pasan ====================

double SparseMatrix::aggSum(int r1, int c1, int r2, int c2) {

    // Obtenemos los valores numericos de todos los elementos del rango que nos pasaron.

    std::vector<double> vals = getNumericValuesInRange(r1, c1, r2, c2);

    double sum = 0.0;

    for (double v : vals) sum += v;

    return sum;
}

// ====== Funcion que calcula el promedio total del rango que nos pasan ====================

double SparseMatrix::aggAverage(int r1, int c1, int r2, int c2) {

    std::vector<double> vals = getNumericValuesInRange(r1, c1, r2, c2);

    if (vals.empty()) {
        throw std::runtime_error("No hay valores numericos para promediar");
    }

    double sum = 0.0;

    for (double v : vals) sum += v;

    return sum / vals.size();
}

// ====== Funcion que calcula el maximo del rango que nos pasan ====================

double SparseMatrix::aggMax(int r1, int c1, int r2, int c2) {

    std::vector<double> vals = getNumericValuesInRange(r1, c1, r2, c2);

    if (vals.empty()) return 0.0;

    double max_val = std::numeric_limits<double>::lowest();

    for (double v : vals) max_val = std::max(max_val, v);

    return max_val;
}

// ====== Funcion que calcula el minimo del rango que nos pasan ====================

double SparseMatrix::aggMin(int r1, int c1, int r2, int c2) {

    std::vector<double> vals = getNumericValuesInRange(r1, c1, r2, c2);

    if (vals.empty()) return 0.0;

    double min_val = std::numeric_limits<double>::max();

    for (double v : vals) min_val = std::min(min_val, v);

    return min_val;
}
void SparseMatrix::removeRow(int row) {
    Header* rowH = findRowHeader(row);
    if (!rowH) {
        throw std::runtime_error("La fila " + std::to_string(row + 1) + " no tiene datos.");
    }

    if (rowH->access) {
        std::vector<int> colsToDelete;
        Node* start = rowH->access;
        Node* curr = start;
        do {
            colsToDelete.push_back(curr->col);
            curr = curr->right;
        } while (curr != start);

        for (int c : colsToDelete) {
            remove(row, c);
        }
    }
}

void SparseMatrix::removeCol(int col) {
    Header* columnH = findColHeader(col);
    if (!columnH) {
        // Podrías convertir el índice a letra aquí si quisieras,
        // pero por ahora lanzamos el error con el índice.
        throw std::runtime_error("La columna solicitada no tiene datos.");
    }

    if (columnH->access) {
        std::vector<int> rowsToDelete;
        Node* start = columnH->access;
        Node* curr = start;
        do {
            rowsToDelete.push_back(curr->row);
            curr = curr->down;
        } while (curr != start);

        for (int r : rowsToDelete) {
            remove(r, col);
        }
    }
}


void SparseMatrix::removeRange(int row1, int col1, int row2, int col2) {
    int minRow = std::min(row1, row2);
    int maxRow = std::max(row1, row2);
    int minCol = std::min(col1, col2);
    int maxCol = std::max(col1, col2);

    int totalDeleted = 0;

    // 1. Recolectamos índices de filas para evitar problemas con punteros volátiles
    std::vector<int> rowsToProcess;
    Header* tempRH = detail->rowHeaders;
    if (tempRH && tempRH != (Header*)detail) {
        Header* start = tempRH;
        do {
            if (tempRH->index >= minRow && tempRH->index <= maxRow) {
                rowsToProcess.push_back(tempRH->index);
            }
            tempRH = tempRH->next;
        } while (tempRH != start && tempRH != (Header*)detail);
    }

    if (rowsToProcess.empty()) {
        throw std::runtime_error("No hay datos en el rango de filas especificado.");
    }

    // 2. Procesamos cada fila de forma atómica
    for (int r : rowsToProcess) {
        Header* rH = findRowHeader(r);
        if (!rH || !rH->access) continue;

        // Recolectamos columnas de los nodos que caen en el rango
        std::vector<int> colsToDelete;
        Node* currN = rH->access;
        Node* startN = currN;
        do {
            if (currN->col >= minCol && currN->col <= maxCol) {
                colsToDelete.push_back(currN->col);
            }
            currN = currN->right;
        } while (currN != startN);

        // 3. Borramos cada nodo identificado
        for (int c : colsToDelete) {
            Header* cH = findColHeader(c);
            if (!rH || !cH) continue; // Seguridad extra

            // --- Búsqueda de nodos y anteriores ---
            Node *prevR = nullptr, *target = nullptr, *prevC = nullptr;

            // Encontrar en fila
            Node* scanR = rH->access;
            if (scanR) {
                do {
                    if (scanR->col == c) { target = scanR; break; }
                    prevR = scanR;
                    scanR = scanR->right;
                } while (scanR != rH->access);
            }
            if (!target) continue;

            // Encontrar anterior en columna
            Node* scanC = cH->access;
            do {
                if (scanC->down == target) { prevC = scanC; break; }
                scanC = scanC->down;
            } while (scanC != cH->access);

            // --- Reconexión Fila ---
            if (target->right == target) { // Último nodo de la fila
                rH->access = nullptr;
                removeRowHeader(r);
                rH = nullptr; // El header ya no existe
            } else {
                if (target == rH->access) rH->access = target->right;
                if (prevR) prevR->right = target->right;
                else { // Si prevR es null, target era el access, necesitamos el último para cerrar el círculo
                    Node* lastR = target;
                    while (lastR->right != target) lastR = lastR->right;
                    lastR->right = target->right;
                }
            }

            // --- Reconexión Columna ---
            if (target->down == target) { // Último nodo de la columna
                cH->access = nullptr;
                removeColHeader(c);
            } else {
                if (target == cH->access) cH->access = target->down;
                prevC->down = target->down;
            }

            delete target;
            totalDeleted++;

            // Si el rowHeader fue borrado, no podemos seguir en esta fila
            if (!rH) break;
        }
    }

    if (totalDeleted == 0) {
        throw std::runtime_error("No se encontraron celdas con datos en el rango.");
    }

    // Sincronización final única
    detail->down_ptr = detail->rowHeaders;
    detail->right_ptr = detail->colHeaders;
    updateDetailCounts();
}