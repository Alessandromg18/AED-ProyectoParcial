#include <SFML/Graphics.hpp>
#include "matrix/SparseMatrix.h"
#include "matrix/MatrixRenderer.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <optional>

// --- FUNCIONES DE TRADUCCIÓN EXCEL ---

int excelColToIndex(std::string col) {
    int result = 0;
    for (char c : col) {
        result = result * 26 + (toupper(c) - 'A' + 1);
    }
    return result - 1;
}

bool parseExcelCoord(std::string coord, int& r_out, int& c_out) {
    std::string colPart = "";
    std::string rowPart = "";
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

// NUEVO: parsea formatos de RANGO: "A1:C4" (rango 2D), "A" (toda la columna), "3" (toda la fila) o "A1" (celda)
bool parseExcelRange(std::string rangeStr, int& r1, int& c1, int& r2, int& c2) {
    size_t colonPos = rangeStr.find(':');
    if (colonPos == std::string::npos) {
        bool hasLetter = false, hasDigit = false;
        for (char c : rangeStr) {
            if (isalpha(c)) hasLetter = true;
            if (isdigit(c)) hasDigit = true;
        }
        if (hasLetter && !hasDigit) { // Es una columna entera, ej "A"
            r1 = 0; r2 = 9999999;
            c1 = c2 = excelColToIndex(rangeStr);
            return true;
        }
        if (!hasLetter && hasDigit) { // Es una fila entera, ej "3"
            c1 = 0; c2 = 9999999;
            r1 = r2 = std::stoi(rangeStr) - 1;
            return true;
        }
        // Celda solitaria, ej "A1"
        bool res = parseExcelCoord(rangeStr, r1, c1);
        r2 = r1; c2 = c1;
        return res;
    } else {
        // Es un rango completo "A1:C4"
        std::string startCoord = rangeStr.substr(0, colonPos);
        std::string endCoord = rangeStr.substr(colonPos + 1);
        return parseExcelCoord(startCoord, r1, c1) && parseExcelCoord(endCoord, r2, c2);
    }
}

int main() {
    SparseMatrix matrix;

    matrix.insert(0, 0, "Inicio");
    matrix.insert(1, 2, "Dato1");
    matrix.insert(3, 1, "Dato2");

    sf::RenderWindow window(sf::VideoMode({900, 600}), "Sparse Excel Explorer");
    window.setFramerateLimit(60);

    MatrixRenderer renderer(matrix);

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cerr << "Error: No se pudo cargar la fuente." << std::endl;
    }

    std::string inputText = "";
    std::string lastQueryResult = "Esperando comando...";
    sf::Color statusColor = sf::Color(80, 80, 80);

    // --- CORRECCIÓN DE INTERFAZ (Para que no se cruce) ---
    sf::RectangleShape uiBackground({520.f, 160.f}); // Aumentado el alto y ancho
    uiBackground.setFillColor(sf::Color(255, 255, 255, 240));
    uiBackground.setPosition({10.f, 10.f});
    uiBackground.setOutlineThickness(1);
    uiBackground.setOutlineColor(sf::Color(180, 180, 180));

    sf::Text uiText(font);
    uiText.setCharacterSize(14); // Ligeramente más pequeña para mejor ajuste
    uiText.setFillColor(sf::Color::Black);
    uiText.setPosition({20.f, 20.f});

    sf::Text resultText(font);
    resultText.setCharacterSize(17);
    resultText.setPosition({20.f, 120.f}); // Bajado para dar aire a las instrucciones

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (wheel->delta > 0) renderer.getView().zoom(0.9f);
                else renderer.getView().zoom(1.1f);
            }

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
                    if (cmd == "get") {
                        std::string target;
                        if (ss >> target && parseExcelCoord(target, r, c)) {
                            std::string val = matrix.get(r, c);
                            if (val != "") {
                                lastQueryResult = ">> En " + target + " hay: " + val;
                                statusColor = sf::Color(0, 100, 250);
                                renderer.setHighlight(r, c);
                            } else {
                                lastQueryResult = ">> " + target + " esta vacia.";
                                statusColor = sf::Color(100, 100, 100);
                            }
                        }
                    }
                    // --- CORRECCIÓN DE LOGICA DELETE ---
                    else if (cmd == "del") {
                        std::string target;
                        if (ss >> target && parseExcelCoord(target, r, c)) {
                            // Validamos si la celda tiene contenido antes de borrar
                            if (!matrix.get(r, c).empty()) {
                                matrix.remove(r, c);
                                lastQueryResult = ">> OK: " + target + " eliminado.";
                                statusColor = sf::Color(200, 0, 0);
                            } else {
                                lastQueryResult = ">> Error: " + target + " no existe.";
                                statusColor = sf::Color::Magenta;
                            }
                            renderer.clearHighlight();
                        }
                    }
                    // --- NUEVAS OPERACIONES DE AGREGACION ---
                    else if (cmd == "sum" || cmd == "avg" || cmd == "max" || cmd == "min") {
                        std::string target;
                        if (ss >> target) {
                            int r1 = 0, c1 = 0, r2 = 0, c2 = 0;
                            if (parseExcelRange(target, r1, c1, r2, c2)) {
                                double res = 0;
                                if (cmd == "sum") res = matrix.aggSum(r1, c1, r2, c2);
                                else if (cmd == "avg") res = matrix.aggAverage(r1, c1, r2, c2);
                                else if (cmd == "max") res = matrix.aggMax(r1, c1, r2, c2);
                                else if (cmd == "min") res = matrix.aggMin(r1, c1, r2, c2);
                                
                                // Eliminar ceros decimales innecesarios
                                std::string resStr = std::to_string(res);
                                resStr.erase(resStr.find_last_not_of('0') + 1, std::string::npos);
                                if (resStr.back() == '.') resStr.pop_back();

                                lastQueryResult = ">> " + cmd + "(" + target + ") = " + resStr;
                                statusColor = sf::Color(255, 100, 0); // Naranja para resultados numericos
                                renderer.clearHighlight(); // Lo ideal aqui seria resaltar todo el rango, usamos clear por simplicidad
                            } else {
                                lastQueryResult = ">> Rango invalido. Use A1:C4, A, 3 o un celda";
                                statusColor = sf::Color::Red;
                            }
                        }
                    }
                    else if (parseExcelCoord(cmd, r, c)) {
                        std::string val;
                        if (ss >> val) {
                            matrix.insert(r, c, val);
                            lastQueryResult = ">> OK: " + cmd + " = " + val;
                            statusColor = sf::Color(0, 120, 0);
                            renderer.setHighlight(r, c);
                        }
                    }
                    else {
                        lastQueryResult = ">> Formato invalido (Ej: A1 valor)";
                        statusColor = sf::Color::Red;
                    }
                    inputText.clear();
                }
                else if (unicode >= 32 && unicode < 128) {
                    inputText += static_cast<char>(unicode);
                }
            }
        }

        float speed = 10.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) renderer.getView().move({0, -speed});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) renderer.getView().move({0, speed});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) renderer.getView().move({-speed, 0});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) renderer.getView().move({speed, 0});

        window.clear(sf::Color(245, 245, 245));
        renderer.draw(window);

        window.setView(window.getDefaultView());
        window.draw(uiBackground);

        uiText.setString("Controles Excel:\n- Insertar: [Celda] [Valor] (Ej: B2 Hola o B2 15)\n- Buscar:   get [Celda]       (Ej: get B2)\n- Borrar:   del [Celda]       (Ej: del B2)\n- Agregacion: sum/avg/max/min [Rango] (Ej: sum A1:B3, avg A, max 4)\n\nEscribiendo: " + inputText + "_");
        resultText.setString(lastQueryResult);
        resultText.setFillColor(statusColor);

        window.draw(uiText);
        window.draw(resultText);
        window.display();
    }
    return 0;
}