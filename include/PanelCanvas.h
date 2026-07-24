#if !defined(PANEL_CANVAS_H)
#define PANEL_CANVAS_H

#include <vector>
#include "Canvas.h"
#include "Panel.h"

class PanelCanvas : public Canvas {
   public:
    // @brief Constructs a PanelCanvas where each panel is of size panelWidth x
    // panelHeight and there are nx x ny panels.
    // @param panelWidth The width of each panel.
    // @param panelHeight The height of each panel.
    // @param nx The number of panels in the x direction.
    // @param ny The number of panels in the y direction.
    PanelCanvas(uint panelWidth, uint panelHeight, uint nx, uint ny)
        : panelWidth(panelWidth), panelHeight(panelHeight), nx(nx), ny(ny) {
            height = ny * panelHeight;
            width = nx * panelWidth;
        }

    void addPanel(Panel* panel, uint x, uint y) {
        if (x < nx && y < ny) {
            panels[x][y] = panel;
        }
    }

    uint getHeight() override { return height; };

    bool set(int x, int y, const RGB& color) override;
    int getPanelX(int x) { return x / panelWidth; }
    int getPanelY(int y) { return y / panelHeight; }
    const RGB& get(uint x, uint y) override;
    void copy(RGB* d, int n, int x, int y) override = 0;
    void fillRow(uint row, const RGB& color) override;
    void fillColumn(uint col, const RGB& color) override;
    void fill(const RGB& color) override;
    void copyRow(int src, int dst) override;
    void copyColumn(int src, int dst) override = 0;
    void rotateRight() override = 0;
    void rotateLeft() override = 0;
    void rotateUp() override = 0;
    void rotateDown() override = 0;
    void shiftLeft(int x, int y, uint w, uint h, int n) override = 0;
    void shiftRight(int x, int y, uint w, uint h, int n) override = 0;
    void shiftUp(int x, int y, uint w, uint h, int n) override = 0;
    void shiftDown(int x, int y, uint w, uint h, int n) override = 0;

   private:
    std::vector<std::vector<Panel*>> panels;
    uint panelWidth;
    uint panelHeight;
    uint nx;
    uint ny;
    uint height;
};

#endif  // PANEL_CANVAS_H