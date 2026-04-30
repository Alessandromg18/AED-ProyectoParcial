#include "MatrixRenderer.h"
#include <iostream>


void MatrixRenderer::drawNodeBox(sf::RenderWindow& window, float x, float y, float w, float h,
                                const std::string& top, const std::string& left, const std::string& right,
                                sf::Color color) {
    float topH = h * 0.5f;

    sf::RectangleShape box({w, h});
    box.setPosition({x, y});
    box.setFillColor(color);
    box.setOutlineThickness(1);
    box.setOutlineColor(sf::Color::Black);
    window.draw(box);

    sf::Vertex lines[] = {
        {{x, y + topH}, sf::Color::Black}, {{x + w, y + topH}, sf::Color::Black},
        {{x + w/2, y + topH}, sf::Color::Black}, {{x + w/2, y + h}, sf::Color::Black}
    };
    window.draw(lines, 4, sf::PrimitiveType::Lines);

    // Tamaño de fuente basado en el alto del nodo
    unsigned int fontSize = static_cast<unsigned int>(h * 0.25f);
    sf::Text t(font);
    t.setCharacterSize(fontSize > 8 ? fontSize : 8); // Mínimo 8px
    t.setFillColor(sf::Color::Black);

    auto centerText = [&](const std::string& str, float targetX, float targetY, float targetW, float targetH) {
        t.setString(str);
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin( {b.position.x + b.size.x / 2.f,b.position.y + b.size.y / 2.f});
        t.setPosition({targetX + targetW/2.f, targetY + targetH/2.f});
        window.draw(t);
    };

    centerText(top, x, y, w, topH);              // Valor arriba
    centerText(left, x, y + topH, w/2.f, topH);    // Fila izq
    centerText(right, x + w/2.f, y + topH, w/2.f, topH); // Col der
}

// ================= CONSTRUCTOR =================

MatrixRenderer::MatrixRenderer(SparseMatrix& m) : matrix(m) {
    if (!font.openFromFile("fuente/arial.ttf")) {
        std::cout << "Error cargando fuente\n";
    }
    // Inicializar la vista con un tamaño estándar, luego se ajustará
    view.setSize({1200.f, 800.f});
    view.setCenter({600.f, 400.f});
}

void MatrixRenderer::handleInput(const sf::Event& event) {
    // Teclado
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::W) view.move({0, -scrollSpeed});
        if (key->code == sf::Keyboard::Key::S) view.move({0, scrollSpeed});
        if (key->code == sf::Keyboard::Key::A) view.move({-scrollSpeed, 0});
        if (key->code == sf::Keyboard::Key::D) view.move({scrollSpeed, 0});
    }

    // Mouse wheel (zoom)
    if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (wheel->delta > 0)
            view.zoom(0.9f);
        else
            view.zoom(1.1f);
    }
}

// ================= DRAW =================
void MatrixRenderer::draw(sf::RenderWindow& window) {

    window.setView(view);

    // 2. Obtener dimensiones de la matriz
    auto d = matrix.getDetail();
    // Obtener el detail y castearlo a Header para comparar
    Header* detailPtr = (Header*)matrix.getDetail();

    int usedRows = 0;
    int usedCols = 0;

    Header* tempR = matrix.getRowHeaders();
    while (tempR && tempR != detailPtr) {
        usedRows++;
        tempR = tempR->next;
    }

    Header* tempC = matrix.getColHeaders();
    while (tempC && tempC != detailPtr) {
        usedCols++;
        tempC = tempC->next;
    }

    float dx = 160.f;
    float dy = 120.f;
    float nodeW = 120.f;
    float nodeH = 55.f;

    float headerSizeX = nodeW;
    float headerSizeY = nodeH;

    // 4. POSICIONES INICIALES
    float startX = 300.f;
    float startY = 180.f;
    float detailX = 150.f;
    float detailY = 100.f;

    float detailW = headerSizeX;
    float detailH = headerSizeY;

    // Ajustar tamaño de fuente dinámico (mínimo 8, máximo 12)

    // ================= DETALLE =================

    drawNodeBox(window, detailX, detailY, nodeW, nodeH, "0",
                std::to_string(usedRows), std::to_string(usedCols), sf::Color::Yellow);

    // ================= CABECERAS FILA =================
    Header* rh = matrix.getRowHeaders();
    while (rh && rh != detailPtr) {  // 🔥 Evitamos dibujar el detail como fila
        float y = startY + rh->index * dy;
        drawNodeBox(window, 150.f, y, headerSizeX, headerSizeY,
                    "0", std::to_string(rh->index+1), "0", sf::Color::Green);
        rh = rh->next;
    }

    // ================= CABECERAS COLUMNA =================
    Header* ch = matrix.getColHeaders();
    while (ch && ch != detailPtr) {
        float x = startX + ch->index * dx;
        drawNodeBox(window, x, 100.f, headerSizeX, headerSizeY,
                    "0", "0", indexToExcelCol(ch->index), sf::Color::Magenta); // 👈 Cambiado aquí
        ch = ch->next;
    }

    // ================= CONEXIONES CABECERAS =================
// ================= CONEXIONES CABECERAS FILA =================
    Header* r = matrix.getRowHeaders();
    while (r && r != detailPtr) {
        float y1 = startY + r->index * dy;
        float xCenter = 150.f + headerSizeX / 2.f;
        float yStart = y1 + headerSizeY; // borde inferior

        if (r->next && r->next != detailPtr) {
            // Conexión normal al siguiente cabecero
            float yEnd = startY + r->next->index * dy;
            sf::Vertex line[] = {
                {{xCenter, yStart}, sf::Color::Green},
                {{xCenter, yEnd},   sf::Color::Green}
            };
            window.draw(line, 2, sf::PrimitiveType::Lines);
            drawArrowHead(window, {xCenter, yEnd}, 'D', sf::Color::Green);
        } else {
            // 🔥 CONEXIÓN DE RETORNO AL DETAIL (Línea por la izquierda)
            float offsetLeft = 30.f;
            sf::Vertex back[] = {
                {{xCenter, yStart}, sf::Color::Green},
                {{xCenter, yStart + 20.f}, sf::Color::Green}, // Baja un poco
                {{150.f - offsetLeft, yStart + 20.f}, sf::Color::Green}, // Va a la izq
                {{150.f - offsetLeft, detailY + detailH / 2.f}, sf::Color::Green}, // Sube
                {{detailX, detailY + detailH / 2.f}, sf::Color::Green} // Entra al detail por izq
            };
            window.draw(back, 5, sf::PrimitiveType::LineStrip);
            drawArrowHead(window, {detailX, detailY + detailH / 2.f}, 'R', sf::Color::Green);
        }
        r = r->next;
    }

    // ================= CONEXIONES CABECERAS COLUMNA =================
    Header* c = matrix.getColHeaders();
    while (c && c != detailPtr) {
        float x1 = startX + c->index * dx;
        float yCenter = 100.f + headerSizeY / 2.f;
        float xStart = x1 + headerSizeX; // borde derecho

        if (c->next && c->next != detailPtr) {
            // Conexión normal al siguiente cabecero
            float xEnd = startX + c->next->index * dx;
            sf::Vertex line[] = {
                {{xStart, yCenter}, sf::Color::Magenta},
                {{xEnd,   yCenter}, sf::Color::Magenta}
            };
            window.draw(line, 2, sf::PrimitiveType::Lines);
            drawArrowHead(window, {xEnd, yCenter}, 'R', sf::Color::Magenta); // Asumo derecha
        } else {
            // 🔥 CONEXIÓN DE RETORNO AL DETAIL (Línea por arriba)
            float offsetUp = 30.f;
            sf::Vertex back[] = {
                {{xStart, yCenter}, sf::Color::Magenta},
                {{xStart + 20.f, yCenter}, sf::Color::Magenta}, // Va a la derecha
                {{xStart + 20.f, 100.f - offsetUp}, sf::Color::Magenta}, // Sube
                {{detailX + detailW / 2.f, 100.f - offsetUp}, sf::Color::Magenta}, // Va a la izq
                {{detailX + detailW / 2.f, detailY}, sf::Color::Magenta} // Baja al detail
            };
            window.draw(back, 5, sf::PrimitiveType::LineStrip);
            drawArrowHead(window, {detailX + detailW / 2.f, detailY}, 'D', sf::Color::Magenta);
        }
        c = c->next;
    }

    // ================= CONEXIÓN DETALLE → CABECERAS =================

    // Solo dibujamos la flecha si realmente EXISTE al menos una fila
    Header* firstRow = matrix.getRowHeaders();
    if (firstRow && firstRow != detailPtr) {
        float yRow = startY + firstRow->index * dy;
        sf::Vertex line[] = {
            {{detailX + nodeW / 2.f, detailY + nodeH}, sf::Color::Black},
            {{detailX + nodeW / 2.f, yRow}, sf::Color::Black}
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);
        drawArrowHead(window, {detailX + nodeW / 2.f, yRow}, 'D', sf::Color::Black);
    }

    // Lo mismo para la primera columna
    Header* firstCol = matrix.getColHeaders();
    if (firstCol && firstCol != detailPtr) {
        float xCol = startX + firstCol->index * dx;
        sf::Vertex line[] = {
            {{detailX + nodeW, detailY + nodeH / 2.f}, sf::Color::Black},
            {{xCol, detailY + nodeH / 2.f}, sf::Color::Black}
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);
        drawArrowHead(window, {xCol, detailY + nodeH / 2.f}, 'R', sf::Color::Black);
    }

    // ================= FILAS =================
    rh = matrix.getRowHeaders();

    while (rh && rh != detailPtr) {

        if (!rh->access) {
            rh = rh->next;
            continue;
        }

        float y = startY + rh->index * dy;

        Node* start = rh->access;
        Node* current = start;

        float prevX = 150.f + headerSizeX;

        do {
            float headerCenter = startX + current->col * dx + headerSizeX / 2.f;
            float x = headerCenter - nodeW / 2.f;

            // 🔥 LOGICA DE COLOR: Si coincide con la consulta, pintamos de Amarillo
            sf::Color nodeColor = sf::Color(100, 200, 255); // Azul por defecto
            if (current->row == hRow && current->col == hCol) {
                nodeColor = sf::Color::Yellow;
            }

            drawNodeBox(window,
            x, y,
            nodeW, nodeH,
            current->value,
            std::to_string(current->row + 1), // Fila sigue siendo número
            indexToExcelCol(current->col),     // 👈 Columna ahora es letra
            nodeColor);

            sf::Vertex line[] = {
                {{prevX, y + nodeH / 2}, sf::Color::Red},
                {{x, y + nodeH / 2}, sf::Color::Red}
            };
            window.draw(line, 2, sf::PrimitiveType::Lines);

            // 🏹 Dibuja la flecha entrando por la izquierda del nodo
            drawArrowHead(window, {x, y + nodeH / 2}, 'R', sf::Color::Red);

            prevX = x + nodeW;
            current = current->right;

        } while (current != start);

        float offsetRegreso = 40.f; // Lo bajamos un poco más para que pase por el medio del pasillo
        float escalonX = 20.f;

        sf::Vertex back[] = {
            // --- SALIDA (DERECHA) ---
            // 1. Punto de salida (mitad derecha del último nodo)
            {{prevX, y + nodeH / 2.f}, sf::Color::Red},

            // 2. Extensión a la derecha
            {{prevX + escalonX, y + nodeH / 2.f}, sf::Color::Red},

            // 3. BAJA al nivel del carril de regreso (ahora es y + nodeH + offset)
            {{prevX + escalonX, y + nodeH + offsetRegreso}, sf::Color::Red},

            // --- TRAYECTO ---
            // 4. Viaja horizontalmente por debajo de la fila
            {{150.f - escalonX, y + nodeH + offsetRegreso}, sf::Color::Red},

            // --- ENTRADA (IZQUIERDA) ---
            // 5. SUBE hasta la altura media del nodo cabecera
            {{150.f - escalonX, y + nodeH / 2.f}, sf::Color::Red},

            // 6. Entra horizontalmente
            {{150.f, y + nodeH / 2.f}, sf::Color::Red}
        };

        window.draw(back, 6, sf::PrimitiveType::LineStrip);

        // 🏹 Flecha entrando por la izquierda
        drawArrowHead(window, {150.f, y + nodeH / 2.f}, 'R', sf::Color::Red);
        rh = rh->next;
    }

    // ================= COLUMNAS =================
    ch = matrix.getColHeaders();

    while (ch && ch != detailPtr) {

        if (!ch->access) {
            ch = ch->next;
            continue;
        }

        float x = startX + ch->index * dx;

        Node* start = ch->access;
        Node* current = start;

        // 🔥 centro real de la columna (alineado con header)
        float xCenter = startX + ch->index * dx + headerSizeX / 2.f;

        float prevY = 100.f + headerSizeY;

        do {
            float y = startY + current->row * dy;

            sf::Vertex line[] = {
                {{xCenter, prevY}, sf::Color::Blue},
                {{xCenter, y},     sf::Color::Blue}
            };
            window.draw(line, 2, sf::PrimitiveType::Lines);

            // 🏹 Dibuja la flecha entrando por arriba del nodo
            drawArrowHead(window, {xCenter, y}, 'D', sf::Color::Blue);

            prevY = y + nodeH;
            current = current->down;

        } while (current != start);

        // --- NUEVO RETORNO CON CUELLOS ---
        float offsetRegresoX = 24.f; // Distancia hacia la derecha para el carril de subida
        float escalonY = 20.f;       // El ancho del "cuello" (distancia vertical extra)

        sf::Vertex back[] = {
            // --- SALIDA (ABAJO) ---
            // 1. Punto de salida (centro inferior del último nodo)
            {{xCenter, prevY}, sf::Color::Blue},

            // 2. Extensión hacia abajo (el "cuello" de salida)
            {{xCenter, prevY + escalonY}, sf::Color::Blue},

            // 3. Se desplaza a la derecha al carril de retorno
            {{xCenter + (nodeW/2.f + offsetRegresoX), prevY + escalonY}, sf::Color::Blue},

            // --- TRAYECTO ---
            // 4. Sube por la derecha de la columna hasta pasar el header
            {{xCenter + (nodeW/2.f + offsetRegresoX), 100.f - escalonY}, sf::Color::Blue},

            // --- ENTRADA (ARRIBA) ---
            // 5. Se alinea con el centro horizontal del header (pero arriba de él)
            {{xCenter, 100.f - escalonY}, sf::Color::Blue},

            // 6. Entra verticalmente al borde superior del header (el "cuello" de entrada)
            {{xCenter, 100.f}, sf::Color::Blue}
        };

        // Dibujamos los 6 puntos
        window.draw(back, 6, sf::PrimitiveType::LineStrip);

        // 🏹 Flecha entrando por la parte superior del nodo cabecera (magenta)
        drawArrowHead(window, {xCenter, 100.f}, 'D', sf::Color::Blue);

        ch = ch->next;
    }
}


sf::View& MatrixRenderer::getView() {
    return view;
}
void MatrixRenderer::drawArrowHead(sf::RenderWindow& window,
                                   sf::Vector2f tip,
                                   char direction,
                                   sf::Color color) {

    sf::ConvexShape triangle(3);
    float size = 8.f;

    if (direction == 'R') {
        // 👉 punta EXACTA en el final de la línea
        triangle.setPoint(0, tip);
        triangle.setPoint(1, {tip.x - size, tip.y - size});
        triangle.setPoint(2, {tip.x - size, tip.y + size});
    }
    else if (direction == 'L') {
        triangle.setPoint(0, tip);
        triangle.setPoint(1, {tip.x + size, tip.y - size});
        triangle.setPoint(2, {tip.x + size, tip.y + size});
    }
    else if (direction == 'D') {
        triangle.setPoint(0, tip);
        triangle.setPoint(1, {tip.x - size, tip.y - size});
        triangle.setPoint(2, {tip.x + size, tip.y - size});
    }
    else if (direction == 'U') {
        triangle.setPoint(0, tip);
        triangle.setPoint(1, {tip.x - size, tip.y + size});
        triangle.setPoint(2, {tip.x + size, tip.y + size});
    }

    triangle.setFillColor(color);
    window.draw(triangle);
}

void MatrixRenderer::setHighlight(int r, int c) {
    hRow = r;
    hCol = c;

    // Calcular dónde debería estar ese nodo en el mundo
    float dx = 160.f;
    float dy = 120.f;
    float startX = 300.f;
    float startY = 180.f;
    float nodeW = 120.f;
    float nodeH = 55.f;

    float targetX = (startX + c * dx + nodeW / 2.f);
    float targetY = (startY + r * dy + nodeH / 2.f);

    // Mover el centro de la vista a esa posición
    view.setCenter({targetX, targetY});
}


