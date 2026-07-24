#include "PanelCanvas.h"

bool PanelCanvas::set(int x, int y, const RGB& color) { 
    if(x < 0 || x >= width || y < 0 || y >= height) {
        return false;
    }
    int panelX = getPanelX(x);
    int panelY = getPanelY(y);
    if (panelX < nx && panelY < ny) {
        return panels[panelX][panelY]->set(x % panelWidth, y % panelHeight, color);
    }
    return false;
 }

 const RGB& PanelCanvas::get(uint x, uint y) {
    int panelX = getPanelX(x);
    int panelY = getPanelY(y);
    if (panelX < nx && panelY < ny) {
        return panels[panelX][panelY]->get(x % panelWidth, y % panelHeight);
    }
    return background;
 }

 void PanelCanvas::fillRow(uint row, const RGB& color) {
     for (int x = 0; x < width; x++) {
         set(x, row, color);
     }
 }

void PanelCanvas::fillColumn(uint col, const RGB& color) {
    for(int y = 0; y < height; y++) {
        set(col, y, color);
    }
}

void PanelCanvas::fill(const RGB& color) {
    for(auto row : panels) {
        for(auto panel : row) {
            panel->fill(color);
        }
    }
}

void PanelCanvas::copyRow(int src, int dst) {
    for(int x = 0; x < width; x++) {
        set(x, dst, get(x, src));
    }
}
