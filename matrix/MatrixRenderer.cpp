#include "MatrixRenderer.h"
#include <iostream>

// ================= Dibujar un nodo (rectangulo) =================

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

    sf::Vertex lines[] = { // Las lineas del rectangulo
        {{x, y + topH}, sf::Color::Black}, {{x + w, y + topH}, sf::Color::Black},
        {{x + w/2, y + topH}, sf::Color::Black}, {{x + w/2, y + h}, sf::Color::Black}
    };

    window.draw(lines, 4, sf::PrimitiveType::Lines);


    unsigned int fontSize = static_cast<unsigned int>(h * 0.25f);
    sf::Text t(font);
    t.setCharacterSize(fontSize > 8 ? fontSize : 8);
    t.setFillColor(sf::Color::Black);

    // Dibuja las lineas dentro de mi rectangulo (la que divide el valor, columna y fila)

    auto centerText = [&](const std::string& str, float targetX, float targetY, float targetW, float targetH) {
        t.setString(str);
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin( {b.position.x + b.size.x / 2.f,b.position.y + b.size.y / 2.f});
        t.setPosition({targetX + targetW/2.f, targetY + targetH/2.f});
        window.draw(t);
    };

    // Para centrar el texto que se visualiza dentro del nodo (rectangulo)

    centerText(top, x, y, w, topH);
    centerText(left, x, y + topH, w/2.f, topH);
    centerText(right, x + w/2.f, y + topH, w/2.f, topH);
}

// ================= CONSTRUCTOR =================

MatrixRenderer::MatrixRenderer(SparseMatrix& m) : matrix(m) {
    if (!font.openFromFile("fuente/arial.ttf")) { // La fuente es para los datos
        std::cout << "Error cargando fuente\n";
    }

    view.setSize({1200.f, 800.f});
    view.setCenter({600.f, 400.f});
}

// ================= Manejar la interfaz =================

void MatrixRenderer::handleInput(const sf::Event& event) {

    // Moverme con el teclado

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::W) view.move({0, -scrollSpeed});
        if (key->code == sf::Keyboard::Key::S) view.move({0, scrollSpeed});
        if (key->code == sf::Keyboard::Key::A) view.move({-scrollSpeed, 0});
        if (key->code == sf::Keyboard::Key::D) view.move({scrollSpeed, 0});
    }

    // Hacer zoom con la rueda del mouse

    if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (wheel->delta > 0)
            view.zoom(0.9f);
        else
            view.zoom(1.1f);
    }
}

// ================= Dibujar nuestra matriz =================

void MatrixRenderer::draw(sf::RenderWindow& window) {

    window.setView(view);

    // Colores que usamos para los punteros y nodos

    sf::Color myGreen(140, 240, 60);
    sf::Color myGreen_Fuerte(113, 194, 48);
    sf::Color myGris(216, 227, 216);
    sf::Color myGris_Fuerte(150, 158, 150);

    Header* detailPtr = (Header*)matrix.getDetail();

    int usedRows = 0;
    int usedCols = 0;

    Header* tempR = matrix.getRowHeaders();

    while (tempR && tempR != detailPtr && tempR != nullptr) {
        usedRows++;
        tempR = tempR->next;
    }

    Header* tempC = matrix.getColHeaders();

    while (tempC && tempC != detailPtr && tempC != nullptr) {
        usedCols++;
        tempC = tempC->next;
    }

    // POSICIONES INICIALES

    float dx = 160.f;
    float dy = 120.f;
    float nodeW = 120.f;
    float nodeH = 55.f;

    float headerSizeX = nodeW;
    float headerSizeY = nodeH;

    float startX = 300.f;
    float startY = 180.f;
    float detailX = 150.f;
    float detailY = 100.f;

    float detailW = headerSizeX;
    float detailH = headerSizeY;

    // ================= NODO DETAIL =================

    drawNodeBox(window, detailX, detailY, nodeW, nodeH, "Nodo Detail",
                std::to_string(usedRows), std::to_string(usedCols), myGreen);

    // ================= NODO CABECERA FILA =================

    Header* rh = matrix.getRowHeaders();

    // Evitamos dibujar el detail como Nodo cabecera fila

    while (rh && rh != detailPtr) {
        float y = startY + rh->index * dy;

        if (rh->access == nullptr) {
            rh = rh->next;
            continue;
        }

        drawNodeBox(window, 150.f, y, headerSizeX, headerSizeY,
                    "0", std::to_string(rh->index+1), "0", myGreen);
        rh = rh->next;
    }

    // ================= NODO CABECERA COLUMNA =================

    Header* ch = matrix.getColHeaders();

    while (ch && ch != detailPtr) {
        float x = startX + ch->index * dx;

        if (ch->access == nullptr) {
            ch = ch->next;
            continue;
        }

        drawNodeBox(window, x, 100.f, headerSizeX, headerSizeY,
                    "0", "0", indexToExcelCol(ch->index), myGreen);
        ch = ch->next;
    }

    // ================= CONEXIONES CABECERAS FILA (Punteros entre cabeceras) =================

    Header* r = matrix.getRowHeaders();

    while (r && r != detailPtr) {
        float y1 = startY + r->index * dy;
        float xCenter = 150.f + headerSizeX / 2.f;
        float yStart = y1 + headerSizeY;

        if (r->next && r->next != detailPtr) {

            // Conexión normal al siguiente cabecero

            float yEnd = startY + r->next->index * dy;
            sf::Vertex line[] = {
                {{xCenter, yStart}, myGreen_Fuerte},
                {{xCenter, yEnd},   myGreen_Fuerte}
            };

            window.draw(line, 2, sf::PrimitiveType::Lines);

            drawArrowHead(window, {xCenter, yEnd}, 'D', myGreen_Fuerte);

        } else {
            // CONEXIÓN DE RETORNO AL DETAIL

            float offsetLeft = 30.f;
            sf::Vertex back[] = {
                {{xCenter, yStart}, myGreen_Fuerte},
                {{xCenter, yStart + 20.f}, myGreen_Fuerte},
                {{150.f - offsetLeft, yStart + 20.f}, myGreen_Fuerte},
                {{150.f - offsetLeft, detailY + detailH / 2.f}, myGreen_Fuerte},
                {{detailX, detailY + detailH / 2.f}, myGreen_Fuerte}
            };

            window.draw(back, 5, sf::PrimitiveType::LineStrip);
            drawArrowHead(window, {detailX, detailY + detailH / 2.f}, 'R', myGreen_Fuerte);
        }
        r = r->next;
    }

    // ================= CONEXIONES CABECERAS COLUMNA =================

    Header* c = matrix.getColHeaders();

    while (c && c != detailPtr) {
        float x1 = startX + c->index * dx;
        float yCenter = 100.f + headerSizeY / 2.f;
        float xStart = x1 + headerSizeX;

        if (c->next && c->next != detailPtr) {

            // Conexión normal al siguiente cabecero

            float xEnd = startX + c->next->index * dx;
            sf::Vertex line[] = { // Dibuja una linea que representa el puntero
                {{xStart, yCenter}, myGreen_Fuerte},
                {{xEnd,   yCenter}, myGreen_Fuerte}
            };

            window.draw(line, 2, sf::PrimitiveType::Lines);
            drawArrowHead(window, {xEnd, yCenter}, 'R', myGreen_Fuerte);

        } else {

            // CONEXIÓN DE RETORNO AL DETAIL

            float offsetUp = 30.f;
            sf::Vertex back[] = {
                {{xStart, yCenter}, myGreen_Fuerte},
                {{xStart + 20.f, yCenter}, myGreen_Fuerte},
                {{xStart + 20.f, 100.f - offsetUp}, myGreen_Fuerte},
                {{detailX + detailW / 2.f, 100.f - offsetUp}, myGreen_Fuerte},
            {{detailX + detailW / 2.f, detailY}, myGreen_Fuerte}
            };

            window.draw(back, 5, sf::PrimitiveType::LineStrip);
            drawArrowHead(window, {detailX + detailW / 2.f, detailY}, 'D', myGreen_Fuerte);
        }
        c = c->next;
    }

    // ================= CONEXIÓN DETAIL A CABECERAS =================

    // Solo dibujamos la flecha si realmente existe al menos una fila

    Header* firstRow = matrix.getRowHeaders();

    if (firstRow && firstRow != detailPtr && firstRow != nullptr) {

        float yRow = startY + firstRow->index * dy;

        sf::Vertex line[] = { // Dibuja la flecha
            {{detailX + nodeW / 2.f, detailY + nodeH}, myGreen_Fuerte},
            {{detailX + nodeW / 2.f, yRow}, myGreen_Fuerte}
        };

        window.draw(line, 2, sf::PrimitiveType::Lines);
        drawArrowHead(window, {detailX + nodeW / 2.f, yRow}, 'D', myGreen_Fuerte);
    }

    // Lo mismo para la primera columna

    Header* firstCol = matrix.getColHeaders();

    if (firstCol && firstCol != detailPtr && firstCol != nullptr) {
        float xCol = startX + firstCol->index * dx;
        sf::Vertex line[] = {
            {{detailX + nodeW, detailY + nodeH / 2.f}, myGreen_Fuerte},
            {{xCol, detailY + nodeH / 2.f}, myGreen_Fuerte}
        };

        window.draw(line, 2, sf::PrimitiveType::Lines);
        drawArrowHead(window, {xCol, detailY + nodeH / 2.f}, 'R', myGreen_Fuerte);
    }

    // ================= CREAMOS LOS NODOS =================

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

            // Para la parte del get si coincide con la consulta nuestro nodo se pintara de amarillo

            sf::Color nodeColor = myGris;
            if (current->row == hRow && current->col == hCol) {
                nodeColor = sf::Color::Yellow;
            }

            // Crea los nodos con los valores pasados y con su columna y fila respectiva
            drawNodeBox(window,
            x, y,
            nodeW, nodeH,
            current->value,
            std::to_string(current->row + 1),
            indexToExcelCol(current->col),
            nodeColor);

            // Graficas las lineas que representan los punteros de nodos de la misma fila

            sf::Vertex line[] = {
                {{prevX, y + nodeH / 2}, myGris_Fuerte},
                {{x, y + nodeH / 2}, myGris_Fuerte}
            };

            window.draw(line, 2, sf::PrimitiveType::Lines);

            drawArrowHead(window, {x, y + nodeH / 2}, 'R', myGris_Fuerte);

            prevX = x + nodeW;
            current = current->right;

        } while (current != start);

        float offsetRegreso = 40.f;
        float escalonX = 20.f;

        sf::Vertex back[] = {  // Puntero de retorno a la cabecera fila

            {{prevX, y + nodeH / 2.f}, sf::Color::Black},

            {{prevX + escalonX, y + nodeH / 2.f}, sf::Color::Black},

            {{prevX + escalonX, y + nodeH + offsetRegreso}, sf::Color::Black},

            {{150.f - escalonX, y + nodeH + offsetRegreso}, sf::Color::Black},

            {{150.f - escalonX, y + nodeH / 2.f}, sf::Color::Black},

            {{150.f, y + nodeH / 2.f}, sf::Color::Black}
        };

        window.draw(back, 6, sf::PrimitiveType::LineStrip);

        drawArrowHead(window, {150.f, y + nodeH / 2.f}, 'R', sf::Color::Black);
        rh = rh->next;
    }

    // ================= Hacemos lo mismo para las columnas =================

    ch = matrix.getColHeaders();

    while (ch && ch != detailPtr) {

        if (!ch->access) {
            ch = ch->next;
            continue;
        }

        Node* start = ch->access;
        Node* current = start;

        float xCenter = startX + ch->index * dx + headerSizeX / 2.f;

        float prevY = 100.f + headerSizeY;

        do {
            float y = startY + current->row * dy;

            sf::Vertex line[] = { // Linea que representa el puntero de nodos de misma columna

                {{xCenter, prevY}, myGris_Fuerte},

                {{xCenter, y},     myGris_Fuerte}
            };

            window.draw(line, 2, sf::PrimitiveType::Lines);

            drawArrowHead(window, {xCenter, y}, 'D', myGris_Fuerte);

            prevY = y + nodeH;

            current = current->down;

        } while (current != start);

        float offsetRegresoX = 24.f;
        float escalonY = 20.f;

        sf::Vertex back[] = { // Linea que representa puntero que va a la cabecera columna

            {{xCenter, prevY}, sf::Color::Black},

            {{xCenter, prevY + escalonY}, sf::Color::Black},

            {{xCenter + (nodeW/2.f + offsetRegresoX), prevY + escalonY}, sf::Color::Black},

            {{xCenter + (nodeW/2.f + offsetRegresoX), 100.f - escalonY}, sf::Color::Black},

            {{xCenter, 100.f - escalonY}, sf::Color::Black},

            {{xCenter, 100.f}, sf::Color::Black}
        };

        window.draw(back, 6, sf::PrimitiveType::LineStrip);

        drawArrowHead(window, {xCenter, 100.f}, 'D', sf::Color::Black);

        ch = ch->next;
    }
}

// ================= Para visualizar todo lo creado =================

sf::View& MatrixRenderer::getView() {
    return view;
}

// ================= Graficar triangulo =================

// En este caso cuando graficamos nuestros punteros lo hacemos con linea, esta funcion crea un triangulo
// que se une a la linea y lo hace ver como una flecha.

void MatrixRenderer::drawArrowHead(sf::RenderWindow& window,
                                   sf::Vector2f tip,
                                   char direction,
                                   sf::Color color) {

    sf::ConvexShape triangle(3); // Aqui esta justamente el triangulo
    float size = 8.f;

    // Son como deberia apuntar el triangulo si hacia abajo, a la derecha, a la izquierda o hacia arriba.

    if (direction == 'R') {
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

// ================= Ubicacion del nodo =================
// En nuestra visualización como lo ven dejamos espacios por si insertamos nodos de fila o columna menores
// Esta funcion justamente calcula donde deberia estar cada uno de nuestros nodos y su espacio que debaria dejar
// si es necesario.

void MatrixRenderer::setHighlight(int r, int c) {
    hRow = r;
    hCol = c;

    float dx = 160.f;
    float dy = 120.f;
    float startX = 300.f;
    float startY = 180.f;
    float nodeW = 120.f;
    float nodeH = 55.f;

    float targetX = (startX + c * dx + nodeW / 2.f);
    float targetY = (startY + r * dy + nodeH / 2.f);

    view.setCenter({targetX, targetY});
}



