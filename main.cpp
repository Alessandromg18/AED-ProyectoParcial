#include <SFML/Graphics.hpp>
#include "matrix/SparseMatrix.h"
#include "matrix/MatrixRenderer.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

const int MAX_COORD = 1000000;
bool showHelp = false; // Variable si quieres ver la ayuda

int excelColToIndex(std::string col) {
    int result = 0;
    for (char c : col) { result = result * 26 + (toupper(c) - 'A' + 1); }
    return result - 1;
}

bool parseExcelCoord(std::string coord, int& r_out, int& c_out) {
    std::string colPart = ""; std::string rowPart = "";
    for (char c : coord) {
        if (isalpha(c)) colPart += c;
        else if (isdigit(c)) rowPart += c;
    }
    if (colPart.empty() || rowPart.empty()) return false;
    try {
        c_out = excelColToIndex(colPart);
        r_out = std::stoi(rowPart) - 1;
    } catch (...) { return false; }
    return (r_out >= 0 && c_out >= 0);
}

// ============== PARA PROCESAR RANGOS (A1:B5) =============

bool parseRange(std::string rangeStr, int& r1, int& c1, int& r2, int& c2) {
    size_t colonPos = rangeStr.find(':');

    // 1. Selección única (No hay rangos)

    if (colonPos == std::string::npos) {

        // Si es fila

        if (std::all_of(rangeStr.begin(), rangeStr.end(), ::isdigit)) {
            r1 = r2 = std::stoi(rangeStr) - 1;
            c1 = 0; c2 = MAX_COORD;
            return r1 >= 0;
        }

        // Si es columna

        if (std::all_of(rangeStr.begin(), rangeStr.end(), ::isalpha)) {
            c1 = c2 = excelColToIndex(rangeStr);
            r1 = 0; r2 = MAX_COORD;
            return c1 >= 0;
        }
        return false;
    }

    // 2. Rangos con ":"

    std::string start = rangeStr.substr(0, colonPos);
    std::string end = rangeStr.substr(colonPos + 1);

    // Rango de filas (Ej: "1:2")

    if (std::all_of(start.begin(), start.end(), ::isdigit) &&
        std::all_of(end.begin(), end.end(), ::isdigit)) {
        r1 = std::stoi(start) - 1;
        r2 = std::stoi(end) - 1;
        c1 = 0; c2 = MAX_COORD;
        return (r1 >= 0 && r2 >= 0);
        }

    // Rango de columnas (Ej: "C:D")

    if (std::all_of(start.begin(), start.end(), ::isalpha) &&
        std::all_of(end.begin(), end.end(), ::isalpha)) {
        c1 = excelColToIndex(start);
        c2 = excelColToIndex(end);
        r1 = 0; r2 = MAX_COORD;
        return (c1 >= 0 && c2 >= 0);
        }

    // Rango estándar (Ej: "A1:B5")

    return parseExcelCoord(start, r1, c1) && parseExcelCoord(end, r2, c2);
}


int main() {
    SparseMatrix matrix; // Nuestra matriz dispersa y le insertamos valores
    matrix.insert(0, 0, "1");
    matrix.insert(0, 3, "2");

    matrix.insert(1, 1, "3");
    matrix.insert(1, 3, "4");

    matrix.insert(2, 0, "5");
    matrix.insert(2, 2, "6");

    matrix.insert(3, 1, "7");
    matrix.insert(3, 3, "8");

    matrix.insert(2, 3, "9");

    sf::RenderWindow window(sf::VideoMode({900, 600}), "Brenner Excel"); // Nuestra ventana
    sf::Image icon;

    if (icon.loadFromFile("C:/Users/facum/Videos/Proyecto_AED/logo/logo.png")) { // Icono de la ventana
        window.setIcon(icon);
    }
    window.setFramerateLimit(60);

    MatrixRenderer renderer(matrix); // Creamos MatrixRenderer para poder visualizar nuestra matriz

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cerr << "Error: No se pudo cargar la fuente." << std::endl;
    }

    // Creamos nuestro panel

    // --- CONFIGURACIÓN DE LA INTERFAZ DE USUARIO ---
    std::string inputText = "";
    std::string lastQueryResult = "Esperando comando...";
    sf::Color statusColor = sf::Color(80, 80, 80);
    sf::Color uiBorderColor = sf::Color(180, 180, 180);

    // Panel de fondo para la ayuda
    sf::RectangleShape uiBackground({880.f, 220.f});
    uiBackground.setFillColor(sf::Color(255, 255, 255, 245));
    uiBackground.setPosition({10.f, 330.f});
    uiBackground.setOutlineThickness(3);

    // Columna Izquierda: Gestión de celdas y Análisis
    sf::Text textLeft(font);
    textLeft.setCharacterSize(13);
    textLeft.setFillColor(sf::Color::Black);

    // Columna Derecha: Estructura y Fórmulas
    sf::Text textRight(font);
    textRight.setCharacterSize(13);
    textRight.setFillColor(sf::Color::Black);

    // Barra inferior: Controles del sistema
    sf::Text textBottom(font);
    textBottom.setCharacterSize(12);
    textBottom.setFillColor(sf::Color(100, 100, 100)); // Color gris para contraste

    // Texto de entrada y resultados (Línea de comandos)
    sf::Text resultText(font);
    resultText.setCharacterSize(18);
    // Subimos la posición Y de 565 a 535 para que no se corte con el borde de la ventana
    resultText.setPosition({30.f, 535.f});

    while (window.isOpen()) { // Si la ventana esta abierta
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close(); // Si cerramos la ventana

            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::F1) {
                    showHelp = !showHelp; // Alterna la visibilidad de ayuda
                }
            }

            if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (wheel->delta > 0) renderer.getView().zoom(0.9f);
                else renderer.getView().zoom(1.1f);
            }

            // El texto que escribimos

            if (const auto* textEvent = event->getIf<sf::Event::TextEntered>()) {
                char32_t unicode = textEvent->unicode;

                if (unicode == '\b') {
                    if (!inputText.empty()) inputText.pop_back();
                }
                else if (unicode == '\r' || unicode == '\n') {
                    std::stringstream ss(inputText);
                    std::string cmd;
                    ss >> cmd;
                    int r, c;

                    uiBorderColor = sf::Color(180, 180, 180);

                    if (cmd == "get") {
                        std::string target;
                        if (ss >> target && parseExcelCoord(target, r, c)) {
                            std::string val = matrix.get(r, c);
                            if (!val.empty()) {
                                lastQueryResult = ">> " + target + " = " + val;
                                statusColor = sf::Color(0, 100, 250);
                                renderer.setHighlight(r, c);
                            } else {
                                lastQueryResult = ">> " + target + " esta vacia.";
                                statusColor = sf::Color(100, 100, 100);
                            }
                        }
                    }

                    else if (cmd == "del") {
    std::string target;
    if (ss >> target) {
        try {
            int r1, c1, r2, c2;

            // 1. PRIORIDAD: ¿Es un rango? (Ej: A1:B2, A:C, 1:5)
            if (target.find(':') != std::string::npos) {
                if (parseRange(target, r1, c1, r2, c2)) {
                    // Si r2 o c2 son el MAX_COORD de parseRange,
                    // removeRange ahora lo manejará internamente con la nueva lógica.
                    matrix.removeRange(r1, c1, r2, c2);
                    lastQueryResult = ">> OK: Rango " + target + " eliminado.";
                } else {
                    throw std::runtime_error("Formato de rango invalido.");
                }
            }
            // 2. ¿Es solo un número? (Fila pura: del 1)
            else if (std::all_of(target.begin(), target.end(), ::isdigit)) {
                r1 = std::stoi(target) - 1;
                // Validamos si la fila existe antes de borrar para lanzar error si está vacía
                if (!matrix.findRowHeader(r1)) {
                    throw std::runtime_error("La fila " + target + " ya esta vacia.");
                }
                matrix.removeRow(r1);
                lastQueryResult = ">> OK: Fila " + target + " eliminada.";
            }
            // 3. ¿Es solo letras? (Columna pura: del A)
            else if (std::all_of(target.begin(), target.end(), ::isalpha)) {
                c1 = excelColToIndex(target);
                // Validamos si la columna existe
                if (!matrix.findColHeader(c1)) {
                    throw std::runtime_error("La columna " + target + " ya esta vacia.");
                }
                matrix.removeCol(c1);
                lastQueryResult = ">> OK: Columna " + target + " eliminada.";
            }
            // 4. ¿Es una celda única? (Ej: del A1)
            else if (parseExcelCoord(target, r1, c1)) {
                // Comprobamos si hay datos. Si get() devuelve "", es que no existe el nodo.
                if (matrix.get(r1, c1).empty()) {
                    throw std::runtime_error("La celda " + target + " ya esta vacia.");
                }
                matrix.remove(r1, c1);
                lastQueryResult = ">> OK: Celda " + target + " eliminada.";
            }
            else {
                throw std::runtime_error("Referencia no reconocida: " + target);
            }

            statusColor = sf::Color(0, 150, 0); // Verde éxito
            uiBorderColor = sf::Color(0, 150, 0);
        }
        catch (const std::exception& e) {
            lastQueryResult = ">> ERROR: " + std::string(e.what());
            statusColor = sf::Color::Red;
            uiBorderColor = sf::Color::Red;
        }
        renderer.clearHighlight();
    }
}
                    else if (cmd == "sum" || cmd == "avg" || cmd == "max" || cmd == "min") {
                        std::string range;
                        int r1, c1, r2, c2;
                        if (ss >> range && parseRange(range, r1, c1, r2, c2)) {
                            try {
                                double result = 0;
                                std::string opName = "";

                                if (cmd == "sum") {
                                    result = matrix.aggSum(r1, c1, r2, c2);
                                    opName = "Suma";
                                }
                                else if (cmd == "avg") {
                                    result = matrix.aggAverage(r1, c1, r2, c2);
                                    opName = "Promedio";
                                }
                                else if (cmd == "max") {
                                    result = matrix.aggMax(r1, c1, r2, c2);
                                    opName = "Maximo";
                                }
                                else if (cmd == "min") {
                                    result = matrix.aggMin(r1, c1, r2, c2);
                                    opName = "Minimo";
                                }

                                lastQueryResult = ">> " + opName + " (" + range + "): " + std::to_string(result);
                                statusColor = sf::Color(0, 150, 150);
                                uiBorderColor = sf::Color(0, 150, 150);
                            }
                            catch (const std::exception& e) {

                                // Si aggAverage lanza el error

                                lastQueryResult = ">> ERROR: " + std::string(e.what());
                                statusColor = sf::Color::Red;
                                uiBorderColor = sf::Color::Red;
                            }
                        }
                    }

                    else if (parseExcelCoord(cmd, r, c)) {
                        std::string val;
                        std::getline(ss >> std::ws, val);

                        if (val.empty()) {
                            lastQueryResult = ">> Error: Falta valor";
                            statusColor = sf::Color::Red;
                            uiBorderColor = sf::Color::Red;
                        }
                        else {
                            try {

                                // Intentamos insertar. Si falla la formula, lanza excepcion.

                                matrix.insert(r, c, val);
                                lastQueryResult = ">> OK: " + cmd + " actualizado.";
                                statusColor = sf::Color(0, 150, 0);
                                uiBorderColor = sf::Color(0, 200, 0);
                                renderer.setHighlight(r, c);
                            }
                            catch (const std::exception& e) {

                                // Aquí capturamos el mensaje de la excepción y lo mandamos a la interfaz de usuario

                                lastQueryResult = ">> ERROR: " + std::string(e.what());
                                statusColor = sf::Color::Red;
                                uiBorderColor = sf::Color::Red;
                                renderer.clearHighlight();
                            }
                        }
                    }
                    inputText.clear();
                }
                else if (unicode >= 32 && unicode < 128) {
                    inputText += static_cast<char>(unicode);
                }
            }
        }

        // Controles de cámara

        float speed = 10.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) renderer.getView().move({0, -speed});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) renderer.getView().move({0, speed});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) renderer.getView().move({-speed, 0});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) renderer.getView().move({speed, 0});

        window.clear(sf::Color(245, 245, 245));
        renderer.draw(window);

        window.setView(window.getDefaultView());

        // DIBUJO DE INTERFAZ
        uiBackground.setOutlineColor(uiBorderColor);

        if (showHelp) {
            window.draw(uiBackground);

            // Definimos posiciones base relativas al panel blanco
            float baseX = 40.f;
            float baseY = 345.f;

            // --- COLUMNA 1: EDICIÓN Y ANÁLISIS ---
            textLeft.setString(
                "1. CELDA Y CONSULTA\n"
                "------------------------------------------\n"
                "  INSERTAR:  [Celda] [Val]\n"
                "  GET:       get [Celda]\n"
                "  DELETE:    del [Celda]\n"
                "  (Ej: A1 100 | get A1)\n\n"
                "3. ANALISIS (SUM/AVG/MAX)\n"
                "------------------------------------------\n"
                "  FUNCIONES: sum | avg | max | min\n"
                "  RANGO COL: A:A\n"
                "  RANGO FILA: 1:1"
            );
            textLeft.setPosition({baseX, baseY});
            window.draw(textLeft);

            // --- COLUMNA 2: ESTRUCTURA Y FÓRMULAS ---
            textRight.setString(
                "2. ESTRUCTURA Y RANGOS\n"
                "------------------------------------------\n"
                "  BORRAR FILA: del [Num]\n"
                "  BORRAR COL:  del [Letra]\n"
                "  BORRAR RNG:  del [Ini]:[Fin]\n"
                "  (Ej: del 5 | del B | del A1:C5)\n\n"
                "4. FORMULAS Y EXPRESIONES\n"
                "------------------------------------------\n"
                "  FORMULAS:   =[Ecuacion]\n"
                "  OPERADORES: +, -, *, /\n"
                "  EJEMPLOS:   =A1*2 | =B1+10"
            );
            textRight.setPosition({baseX + 440.f, baseY});
            window.draw(textRight);

            // --- BARRA INFERIOR ---
            textBottom.setString("SISTEMA: SparseMatrix SFML | [F1] Ayuda | [Flechas] Camara | [Rueda] Zoom");
            textBottom.setPosition({baseX, baseY + 185.f});
            window.draw(textBottom);
        }

        // --- LÍNEA DE COMANDOS Y RESULTADOS ---
        std::string helpPrompt = showHelp ? " [F1: Ocultar]" : " [F1: Ayuda]";
        resultText.setString(lastQueryResult + "\n" + helpPrompt + " ESCRIBIENDO: " + inputText + "_");

        // Mantenemos la posición baja para que no se solape con el panel de ayuda
        resultText.setPosition({30.f, 555.f});
        resultText.setFillColor(statusColor);
        window.draw(resultText);

        window.display();
    }
    return 0;
}