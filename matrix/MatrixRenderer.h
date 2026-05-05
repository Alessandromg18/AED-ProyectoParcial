//
// Created by facum on 27/04/2026.
//

#ifndef MATRIXRENDERER_H
#define MATRIXRENDERER_H


#pragma once
#include <SFML/Graphics.hpp>
#include "SparseMatrix.h"

// Para graficar nuestra matriz dispersa

class MatrixRenderer {
private:
    SparseMatrix& matrix;
    sf::Font font;
    sf::View view;
    float scrollSpeed = 20.f;
    int hRow = -1;
    int hCol = -1;

public:
    MatrixRenderer(SparseMatrix& m);

    void draw(sf::RenderWindow& window);

    void drawNodeBox(sf::RenderWindow& window,
                 float x, float y,
                 float w, float h,
                 const std::string& top,
                 const std::string& left,
                 const std::string& right,
                 sf::Color color);

    void handleInput(const sf::Event& event);
    sf::View& getView();

    void drawArrowHead(sf::RenderWindow& window, sf::Vector2f target, char direction, sf::Color color);

    void setHighlight(int r, int c);

    void clearHighlight() { hRow = -1; hCol = -1; }

    std::string indexToExcelCol(int index) { // Para que tenga los valores alfabeticos como un excel
        std::string colName = "";
        while (index >= 0) {
            colName = (char)('A' + (index % 26)) + colName;
            index = (index / 26) - 1;
        }

        return colName;
    }
};


#endif //MATRIXRENDERER_H
