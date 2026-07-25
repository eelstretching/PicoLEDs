#include "PanelCanvas.h"

PanelCanvas::PanelCanvas(uint panelWidth, uint panelHeight, uint nx, uint ny)
    : Canvas(panelWidth * nx),
      panelWidth(panelWidth),
      panelHeight(panelHeight),
      nx(nx),
      ny(ny) {
    height = ny * panelHeight;
    width = nx * panelWidth;

    panels = new Panel**[nx];
    for (uint i = 0; i < nx; i++) {
        panels[i] = new Panel*[ny];
        for (uint j = 0; j < ny; j++) {
            panels[i][j] = nullptr;
        }
    }
}

PanelCanvas::~PanelCanvas() {
    for (uint i = 0; i < nx; i++) {
        delete[] panels[i];
    }
    delete[] panels;
}

void PanelCanvas::addPanel(Panel* panel, uint x, uint y) {
    if (x < nx && y < ny) {
        panels[x][y] = panel;
    }
    renderer.add(panel);
}

bool PanelCanvas::set(int x, int y, const RGB& color) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return false;
    }
    int panelX = getPanelX(x);
    int panelY = getPanelY(y);
    if (panelX < nx && panelY < ny) {
        return panels[panelX][panelY]->set(x % panelWidth, y % panelHeight,
                                           color);
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

void PanelCanvas::copy(RGB* d, int n, int x, int y) {
    // This will work the same as the copy in the superclass, which writes data
    // into the rows, starting at the given position and wrapping around, which
    // doesn't make a whole lot of sense here, but we're going to keep the
    // behavior the same.
    for(int i = 0; i < n; i++) {
        set(x, y, d[i]);
        x++;
        if (x >= width) {
            x = 0;
            y++;
        }
    }
}

void PanelCanvas::fillRow(uint row, const RGB& color) {
    for (int x = 0; x < width; x++) {
        set(x, row, color);
    }
}

void PanelCanvas::fillColumn(uint col, const RGB& color) {
    for (int y = 0; y < height; y++) {
        set(col, y, color);
    }
}

void PanelCanvas::fill(const RGB& color) {
    for(int i = 0; i < nx; i++) {
        for(int j = 0; j < ny; j++) {
            panels[i][j]->fill(color);
        }
    }
}

void PanelCanvas::copyRow(int src, int dst) {
    for (int x = 0; x < width; x++) {
        set(x, dst, get(x, src));
    }
}

void PanelCanvas::copyColumn(int src, int dst) {
    for (int y = 0; y < height; y++) {
        set(dst, y, get(src, y));
    }
}

void PanelCanvas::rotateRight() {
    RGB temp[height];
    for (int y = 0; y < height; y++) {
        temp[y] = get(width - 1, y);
    }
    for (int x = width - 1; x >= 0; x--) {
        copyColumn(x - 1, x);
    }

    for (int y = 0; y < height; y++) {
        set(0, y, temp[y]);
    }
}

void PanelCanvas::rotateLeft() {
    RGB temp[height];
    for (int y = 0; y < height; y++) {
        temp[y] = get(0, y);
    }
    for (int x = 0; x < width - 1; x++) {
        copyColumn(x + 1, x);
    }

    for (int y = 0; y < height; y++) {
        set(width - 1, y, temp[y]);
    }
}

void PanelCanvas::rotateUp() {
    RGB temp[width];
    for (int x = 0; x < width; x++) {
        temp[x] = get(x, height - 1);
    }
    for (int y = height - 1; y > 0; y--) {
        copyRow(y - 1, y);
    }
    for (int x = 0; x < width; x++) {
        set(x, 0, temp[x]);
    }
}

void PanelCanvas::rotateDown() {
    RGB temp[width];
    for (int x = 0; x < width; x++) {
        temp[x] = get(x, 0);
    }
    for (int y = 0; y < height - 1; y++) {
        copyRow(y + 1, y);
    }
    for (int x = 0; x < width; x++) {
        set(x, height - 1, temp[x]);
    }
}

void PanelCanvas::shiftLeft(int x, int y, uint w, uint h, int n) {
    uint yf = MIN(y + h, height);
    uint xf = MIN(x + w, width);
    for(int i = x; i < xf; i++) {
        for(int j = y; j < yf; j++) {
            set(i - n, j, get(i, j));
            set(i, j, background);
        }
    }
}

void PanelCanvas::shiftRight(int x, int y, uint w, uint h, int n) {
    uint yf = MIN(y + h, height);
    uint xf = MIN(x + w, width);
    for(int i = xf - 1; i >= x; i--) {
        for(int j = y; j < yf; j++) {
            set(i + n, j, get(i, j));
            set(i, j, background);
        }
    }
}

void PanelCanvas::shiftUp(int x, int y, uint w, uint h, int n) {
    uint yf = MIN(y + h, height);
    uint xf = MIN(x + w, width);
    for(int i = x; i < xf; i++) {
        for(int j = y; j < yf; j++) {
            set(i, j - n, get(i, j));
            set(i, j, background);
        }
    }
}

void PanelCanvas::shiftDown(int x, int y, uint w, uint h, int n) {
    uint yf = MIN(y + h, height);
    uint xf = MIN(x + w, width);
    for(int i = x; i < xf; i++) {
        for(int j = yf - 1; j >= y; j--) {
            set(i, j + n, get(i, j));
            set(i, j, background);
        }
    }
}
