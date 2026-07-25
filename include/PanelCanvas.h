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
    PanelCanvas(uint panelWidth, uint panelHeight, uint nx, uint ny);
    ~PanelCanvas();
    void addPanel(Panel* panel, uint x, uint y);

    uint getHeight() override { return height; };

    bool set(int x, int y, const RGB& color) override;
    int getPanelX(int x) { return x / panelWidth; }
    int getPanelY(int y) { return y / panelHeight; }
    const RGB& get(uint x, uint y) override;
    void copy(RGB* d, int n, int x, int y) override;
    void fillRow(uint row, const RGB& color) override;
    void fillColumn(uint col, const RGB& color) override;
    void fill(const RGB& color) override;
    void copyRow(int src, int dst) override;
    void copyColumn(int src, int dst) override;
    void rotateRight() override;
    void rotateLeft() override;
    void rotateUp() override;
    void rotateDown() override;
    void shiftLeft(int x, int y, uint w, uint h, int n);
    void shiftRight(int x, int y, uint w, uint h, int n);
    void shiftUp(int x, int y, uint w, uint h, int n);
    void shiftDown(int x, int y, uint w, uint h, int n);

   private:
    Panel ***panels;
    uint panelWidth;
    uint panelHeight;
    uint nx;
    uint ny;
    uint height;
};

#endif  // PANEL_CANVAS_H