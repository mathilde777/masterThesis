#ifndef BOX_H
#define BOX_H

class Box{
public:
    int id;
    int boxId;
    int trayId;
    double last_x;
    double last_y;
    double last_z;

    double width;
    double height;
    double length;

    Box(int id, int boxId, int trayId, double x, double y, double z, double w, double h, double l)
        : id(id), boxId(boxId), trayId(trayId), last_x(x), last_y(y), last_z(z), width(w), height(h), length(l) {}

    // Setters
    void setId(int id) { this->id = id; }
    void setBoxId(int boxId) { this->boxId = boxId; }
    void setTrayId(int trayId) { this->trayId = trayId; }
    void setLastX(double x) { last_x = x; }
    void setLastY(double y) { last_y = y; }
    void setLastZ(double z) { last_z = z; }
    void setWidth(double width) { this->width = width; }
    void setHeight(double height) { this->height = height; }
    void setLength(double length) { this->length = length; }

    // Getters
    int getId() const { return id; }
    int getBoxId() const { return boxId; }
    int getTrayId() const { return trayId; }
    double getLastX() const { return last_x; }
    double getLastY() const { return last_y; }
    double getLastZ() const { return last_z; }
    double getWidth() const { return width; }
    double getHeight() const { return height; }
    double getLength() const { return length; }
};

#endif // BOX_H
