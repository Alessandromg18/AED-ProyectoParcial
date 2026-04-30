#include <SFML/Graphics.hpp>
#include "matrix/SparseMatrix.h"
#include "matrix/MatrixRenderer.h"
#include <iostream>
#include <sstream>
#include <algorithm>

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

        uiText.setString("Controles Excel:\n- Insertar: [Celda] [Valor] (Ej: B2 Hola)\n- Buscar:   get [Celda]       (Ej: get B2)\n- Borrar:   del [Celda]       (Ej: del B2)\n\nEscribiendo: " + inputText + "_");
        resultText.setString(lastQueryResult);
        resultText.setFillColor(statusColor);

        window.draw(uiText);
        window.draw(resultText);
        window.display();
    }
    return 0;
}